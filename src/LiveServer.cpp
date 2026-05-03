#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <Geode/Geode.hpp>
#include <Geode/binding/EditorPauseLayer.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/loader/Log.hpp>
#include <arc/time/Sleep.hpp>
#include <matjson.hpp>
#include <string_view>

#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

#include <fmt/format.h>
#include <proxy/proxy.h>
#include "ActionUtils.hpp"

#include <glaze/glaze.hpp>
#include <glaze/thread/shared_async_vector.hpp>

#include <arc/prelude.hpp>

#include <mutex>
#include <thread>

using websocketpp::connection_hdl;
using websocketpp::lib::bind;
using websocketpp::lib::lock_guard;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

using WSServer = websocketpp::server<websocketpp::config::asio>;

WSServer* g_wsServer = nullptr;
std::thread g_wsThread;


using namespace geode::prelude;

struct Response {
    std::string_view status;
    std::string error;
    std::optional<glz::generic> response;
    static Response make_success() { return {.status = "successful"}; }

    static Response make_success(glz::generic&& payload) noexcept {
        log::info("constructing success with size: {}", payload.size());
        Response res;
        res.status = "successful";
        res.response.emplace(std::move(payload));
        return res;
    }

    static Response make_error(std::string&& error_msg) { return {.status = "error", .error = std::move(error_msg)}; }
};

PRO_DEF_MEM_DISPATCH(MemRun, run);

struct Runnable : pro::facade_builder ::add_convention<MemRun, Response(LevelEditorLayer*)>::build {};


struct Action {
    connection_hdl hdl;
    pro::proxy<Runnable> runner;
    bool shouldClose = false;
};


struct Add {
    static constexpr auto ACTION_NAME = "ADD_OBJECTS";
    static constexpr auto EDITOR_ACTION = true;
    std::string action;
    std::string objects;
    bool close;
    Response run(LevelEditorLayer* editor) {
        EditorUI::get()->m_alertShown = true;
        editor->createObjectsFromString(objects, false, true);
        return Response::make_success();
    }
};

GLZ_ACTION_META(Add)

static gd::vector<short> getGroupIDs(GameObject* obj) {
    gd::vector<short> res;

    if (obj->m_groups && obj->m_groups->at(0))
        for (auto i = 0; i < obj->m_groupCount; i++)
            res.push_back(obj->m_groups->at(i));
    return res;
}

static bool hasGroup(GameObject* obj, int group) {
    for (const auto& g : getGroupIDs(obj)) {
        if (g == group)
            return true;
    }
    return false;
}


struct Remove {
    static constexpr auto ACTION_NAME = "REMOVE_OBJECTS";
    static constexpr auto EDITOR_ACTION = true;
    std::string action;
    int group;
    bool close;
    Response run(LevelEditorLayer* editor) {
        geode::cocos::CCArrayExt<GameObject*> toDelete;
        for (GameObject* obj : geode::cocos::CCArrayExt<GameObject*>(editor->m_objects)) {
            if (hasGroup(obj, group)) {
                toDelete.push_back(obj);
            }
        }

        if (toDelete.size() == 0)
            return Response::make_success();


        auto selected = editor->m_editorUI->getSelectedObjects();
        editor->m_editorUI->deselectAll();

        editor->m_editorUI->selectObjects(toDelete.inner(), false);

        editor->m_editorUI->onDeleteSelected(nullptr);

        editor->m_editorUI->selectObjects(selected, false);
        return Response::make_success();
    }
};

GLZ_ACTION_META(Remove)

struct GetLevelString {
    static constexpr auto ACTION_NAME = "GET_LEVEL_STRING";
    static constexpr auto EDITOR_ACTION = true;
    std::string action;
    bool close;
    Response run(LevelEditorLayer* editor) {
        glz::generic s = std::string(editor->getLevelString());
        return Response::make_success(std::move(s));
    }
};

GLZ_ACTION_META(GetLevelString)

struct ReplaceLevelString {
    static constexpr auto ACTION_NAME = "REPLACE_LEVEL_STRING";
    static constexpr auto EDITOR_ACTION = true;
    std::string action;
    std::string levelString;
    bool close = false;
    bool save = false;

    static GJGameLevel* globallevel;
    static std::string globalNewlevelString;
    void enterEditorAfterExiting(float) {
        if (!LevelEditorLayer::get()) {
            globallevel->m_levelString = std::move(globalNewlevelString);
            globalNewlevelString.clear();

            cocos2d::CCDirector::get()->replaceScene(LevelEditorLayer::scene(globallevel, false));
            CCScheduler::get()->unscheduleSelector(
                    schedule_selector(ReplaceLevelString::enterEditorAfterExiting), globallevel);
        }
    }

    Response run(LevelEditorLayer* editor) {
        auto pause = EditorPauseLayer::create(editor);
        auto level = editor->m_level;
        if (save) {
            pause->saveLevel();
        }
        globalNewlevelString = std::move(levelString);
        globallevel = level;
        CCDirector::get()->getScheduler()->scheduleSelector(
                schedule_selector(ReplaceLevelString::enterEditorAfterExiting), level, 0.05, false);

        pause->onExitEditor(nullptr);
        log::info("exited");


        log::info("returning");
        return Response::make_success();
    }
};

GJGameLevel* ReplaceLevelString::globallevel = nullptr;
std::string ReplaceLevelString::globalNewlevelString = {};


GLZ_ACTION_META(ReplaceLevelString)


std::vector<Action> g_actions;
std::mutex g_actionsMutex;

std::atomic<bool> g_inEditor;


void on_open(websocketpp::connection_hdl hdl) { log::info("open"); }

void on_message(websocketpp::connection_hdl hdl, WSServer::message_ptr msg) {

    std::string msgStr = msg->get_payload();

    CHECK_ACTION(Add, hdl)
    CHECK_ACTION(Remove, hdl)
    CHECK_ACTION(GetLevelString, hdl)
    CHECK_ACTION(ReplaceLevelString, hdl)

    log::info("exiting message handler!");
}

void on_close(websocketpp::connection_hdl hdl) { geode::log::debug("onclose"); }


$on_mod(Loaded) {
    g_wsServer = new WSServer();

    g_wsServer->set_access_channels(websocketpp::log::alevel::none);
    g_wsServer->set_error_channels(websocketpp::log::elevel::none);

    g_wsServer->init_asio();

    g_wsServer->set_open_handler(bind(&on_open, _1));
    g_wsServer->set_message_handler(bind(&on_message, _1, _2));
    g_wsServer->set_close_handler(bind(&on_close, _1));

    g_wsServer->set_listen_backlog(1024);

    int port = geode::Mod::get()->getSettingValue<int>("ws-port");
    g_wsServer->listen(port);

    g_wsServer->start_accept();

    g_wsThread = std::thread([&]() { g_wsServer->run(); });
}

struct LSHooks : geode::Modify<LSHooks, LevelEditorLayer> {
    struct Fields {
        ~Fields() { g_inEditor = false; }
    };

    void performQueuedActions(float dt) {
        std::lock_guard lock(g_actionsMutex);
        if (g_actions.empty())
            return;

        log::info("entering loop!");
        for (auto& [hdl, runner, shouldClose] : g_actions) {
            if (auto resp = glz::write_json(runner->run(this))) {
                log::info("Sending Response to client");
                if (hdl.expired()) {
                    log::error("EXPIRED!");
                    break;
                }
                g_wsServer->send(hdl, *resp, websocketpp::frame::opcode::text);
            } else {
                log::error("Sending error to client");
                g_wsServer->send(
                        hdl, std::string("{\"status\":\"error\",\"error\":\"Could not produce response object\"}"),
                        websocketpp::frame::opcode::text);
            }
            if (shouldClose) {
                log::info("Closing client");
                g_wsServer->close(hdl, 1000, "");
            }
        }
        log::info("exiting loop! clearing!");
        g_actions.clear();
    }

    bool init(GJGameLevel* level, bool idk) {
        if (!LevelEditorLayer::init(level, idk))
            return false;
        g_inEditor = true;
        this->schedule(schedule_selector(LSHooks::performQueuedActions), 0.1f);
        return true;
    }
};
