

#include "hv/WebSocketChannel.h"

#include <Geode/Geode.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/loader/Log.hpp>
#include <matjson.hpp>
#include <string_view>

#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/GameManager.hpp>

#include <fmt/format.h>
#include <proxy/proxy.h>
#include "ActionUtils.hpp"

#include <glaze/glaze.hpp>
#include <glaze/thread/shared_async_vector.hpp>
#include <hv/WebSocketServer.h>

hv::WebSocketServer server;
hv::WebSocketService ws;

struct Response {
    std::string_view status; //this is only ever set to compile-time strings
    std::string error;
    std::optional<glz::generic> response;
    static Response make_success() {
        return {.status = "successful"};
    }


    static Response make_success(glz::generic&& payload) noexcept {
        Response res;
        res.status = "successful";
        res.response.emplace(std::move(payload));  // move into the optional
        return res;
    }

    static Response make_error(std::string&& error_msg) {
        return {.status = "error", .error = std::move(error_msg)};
    }
};

PRO_DEF_MEM_DISPATCH(MemRun, run);

struct Runnable : pro::facade_builder
    ::add_convention<MemRun, Response(LevelEditorLayer*)>
    ::build {};



struct Action
{
    WebSocketChannelPtr client;
    pro::proxy<Runnable> runner;
    bool shouldClose = false;
};


struct Add
{
    static constexpr auto ACTION_NAME = "ADD_OBJECTS";
    static constexpr auto EDITOR_ACTION = true;
    std::string action;
    std::string objects;
    bool close;
    Response run(LevelEditorLayer* editor)
    {
        EditorUI::get()->m_alertShown = true;
        editor->createObjectsFromString(objects, false, false);
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
static bool hasGroup(GameObject* obj, int group)
{
    for (const auto& g : getGroupIDs(obj))
    {
        if (g == group)
            return true;
    }
    return false;
}



//remove
struct Remove
{
    static constexpr auto ACTION_NAME = "REMOVE_OBJECTS";
    static constexpr auto EDITOR_ACTION = true;
    std::string action;
    int group;
    bool close;
    Response run(LevelEditorLayer* editor)
    {
        geode::cocos::CCArrayExt<GameObject*> toDelete;
        for(GameObject* obj : geode::cocos::CCArrayExt<GameObject*>(editor->m_objects))
        {
            if (hasGroup(obj, group))
            {
                toDelete.push_back(obj);
            }
        }

        if(toDelete.size() == 0) return Response::make_success();


        auto selected = editor->m_editorUI->getSelectedObjects();
        editor->m_editorUI->deselectAll();

        editor->m_editorUI->selectObjects(toDelete.inner(), false);

        editor->m_editorUI->onDeleteSelected(nullptr);

        editor->m_editorUI->selectObjects(selected, false);
        return Response::make_success();
    }
};



GLZ_ACTION_META(Remove)

//GetLevelString
struct GetLevelString
{
    static constexpr auto ACTION_NAME = "GET_LEVEL_STRING";
    static constexpr auto EDITOR_ACTION = true;
    std::string action;
    bool close;
    Response run(LevelEditorLayer* editor)
    {
        glz::generic s = std::string(editor->getLevelString());
        return Response::make_success(std::move(s));
    }
};



std::vector<Action> g_actions;
std::mutex g_actionsMutex;

std::atomic<bool> g_inEditor;




$on_mod(Loaded)
{
    ws.onopen = [](const WebSocketChannelPtr& channel, const HttpRequestPtr& req)
    {
        geode::log::debug("open");
    };
    ws.onmessage = [](const WebSocketChannelPtr& channel, const std::string& msg)
    {
        geode::log::debug("recieved: {}", msg);


        CHECK_ACTION(Add)
        CHECK_ACTION(Remove)
        CHECK_ACTION(GetLevelString)
    };

    ws.onclose = [](const WebSocketChannelPtr& channel) {
        geode::log::debug("onclose");
    };

    server = hv::WebSocketServer(&ws);
    server.setPort(1313);
    server.setThreadNum(1);
    server.start();

}


struct LSHooks : geode::Modify<LSHooks, LevelEditorLayer>
{
    struct Fields {
        ~Fields() {
            g_inEditor = false;
        }
    };

    void performQueuedActions(float dt)
    {
        std::lock_guard lock(g_actionsMutex);
        if(g_actions.empty()) return;

        for(auto&[client, runner, shouldClose] : g_actions)
        {
            //geode::log::info("hi");
            if(client->isClosed()) return;

            if(auto resp = glz::write_json(runner->run(this)))
            {
                client->send(*resp);
            }
            else
            {
                client->send("{\"status\":\"error\",\"error\":\"Could not produce response object\"}");
            }
            if(shouldClose)
            {
                client->close();
            }
        }
        g_actions.clear();
    }

    bool init(GJGameLevel* level, bool idk)
    {
        if(!LevelEditorLayer::init(level, idk)) return false;
        g_inEditor = true;
        this->schedule(schedule_selector(LSHooks::performQueuedActions), 0.1f);
        return true;
    }


};
