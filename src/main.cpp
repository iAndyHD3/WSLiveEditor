

#include <ixwebsocket/IXWebSocketServer.h>
#include <atomic>
#include <mutex>
#include <functional>

#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <matjson.hpp>
#include <fmt/color.h>

using ActionFunction = std::function<void(LevelEditorLayer*)>;

namespace global
{
    std::mutex actionMutex; //lock/unlock this mutex when working with the vector
    std::vector<ActionFunction> actionFuncs;


    ix::WebSocketServer* ws = nullptr;
    std::atomic<bool> serverRunning = false;
    std::atomic<bool> actionsQueued = false;
}

enum class WSLiveAction
{
    ADD,
    REMOVE
};

enum class WSLiveStatus
{
    Success,
    NotInEditor,
    InvalidJson,
};

struct WSLiveColorChannel
{
    std::optional<std::string> str;

    int id = 0;

    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;

    bool p1 = false;
    bool p2 = false;
    bool blending = false;

    std::optional<float> opacity;

    std::string getStringAsGDLevel() const
    {
        auto str = fmt::format("1_{}_2_{}_3_{}", r, g, b);

        if(blending) str += "_5_1";
        if(opacity) str += fmt::format("_7_{}_8_1", *opacity);

        if(p1) str += "_4_1";
        else if(p2) str += "_4_2";


        return str;
    }

    //throws
    static WSLiveColorChannel getFromObject(const matjson::Object& obj)
    {
        auto id = obj.find("id");
        if(id == obj.end()) throw std::exception("id key not found");
        int id_val = id->second.as_int();
        if(id_val < 0 || id_val > 999) throw std::exception("id key must be an int");

        auto colstring = obj.find("string");
        if(colstring != obj.end() && colstring->second.is_string())
        {
            return WSLiveColorChannel{.str = colstring->second.as_string(), .id = id_val};
        }

        auto rgb = obj.find("rgb");
        if(rgb == obj.end()) throw std::exception("rgb nor string key found");
        if(!rgb->second.is_array()) throw std::exception("rgb key must be an array");



        const auto& rgbarr = rgb->second.as_array();
        return WSLiveColorChannel {
            .str = {},
            .id = id_val,
            .r = static_cast<unsigned char>(rgbarr[0].as_int()),
            .g = static_cast<unsigned char>(rgbarr[1].as_int()),
            .b = static_cast<unsigned char>(rgbarr[2].as_int()),
            .p1 = obj.contains("p1") ? obj.find("p1")->second.as_bool() : false,
            .p2 = obj.contains("p2") ? obj.find("p2")->second.as_bool() : false,
            .blending = obj.contains("blending") ? obj.find("blending")->second.as_bool() : false,
            .opacity = obj.contains("opacity") ? static_cast<float>(obj.find("opacity")->second.as_double()) : std::optional<float>{},
        };
    }
    static std::vector<WSLiveColorChannel> getFromArray(const matjson::Array& arr)
    {
        std::vector<WSLiveColorChannel> ret;
        ret.reserve(arr.size());

        for(const auto& v : arr)
        {
            if(!v.is_object()) continue;
            
            ret.push_back(WSLiveColorChannel::getFromObject(v.as_object()));
        }
        return ret;
    }
};

const char* WSLiveStatus_toString(WSLiveStatus s)
{
    switch (s)
    {
    case WSLiveStatus::Success: return "Success";
    case WSLiveStatus::NotInEditor: return "NotInEditor";
    case WSLiveStatus::InvalidJson: return "InvalidJson";
    default: return "";
    }
}

void runServer();

struct MyPause : geode::Modify<MyPause, EditorPauseLayer>
{
    void onExitEditor(cocos2d::CCObject* sender)
    {
        if (global::ws)
        {
            geode::log::info("stoping WSLiveEditor server");
            global::ws->stop();
        }
        EditorPauseLayer::onExitEditor(sender);
    }
};

struct MyEditor : geode::Modify<MyEditor, LevelEditorLayer>
{
    void performQueuedActions(float dt)
    {
        if (global::actionsQueued.load())
        {
            global::actionMutex.lock();
            for (const auto& f : global::actionFuncs)
            {
                f(this);
            }
            global::actionFuncs.clear();
            global::actionMutex.unlock();
            global::actionsQueued = false;
        }
    }
    bool init(GJGameLevel* level, bool idk)
    {
        std::thread([] { runServer(); }).detach();
        this->schedule(schedule_selector(MyEditor::performQueuedActions), 0.0f);
        return LevelEditorLayer::init(level, idk);
    }


};

void queueFunction(const ActionFunction& fun)
{
    global::actionMutex.lock();
    global::actionFuncs.push_back(fun);
    global::actionMutex.unlock();
    global::actionsQueued = true;
}
void sendStatus(ix::WebSocket& client, WSLiveStatus status, const std::string& message, const matjson::Value& jsonmsg)
{
    if (!jsonmsg.try_get<bool>("response").value_or(true))
    {
        return;
    }

    try
    {
        matjson::Value ret = matjson::Object();

        if (status == WSLiveStatus::Success)
        {
            ret.set("status", "successful");
        }
        else
        {
            ret.set("status", "error");
            ret.set("error", WSLiveStatus_toString(status));
        }

        if (!message.empty()) ret.set("message", message);
        if (jsonmsg.contains("id")) ret.set("id", jsonmsg["id"]);

        std::string status = ret.dump(matjson::NO_INDENTATION);
        geode::log::info("Sending status: {}", status);

        if (client.sendText(status).success && jsonmsg.try_get<bool>("close").value_or(false))
        {
            client.close();
        }
    }
    catch (std::exception& e)
    {
        geode::log::error("exception: {}", e.what());
    }
}

inline std::string getIdOrEmpty(const matjson::Value& v)
{
    return v.try_get<std::string>("id").value_or(std::string{});
}

gd::vector<short> getGroupIDs(GameObject* obj) {
    gd::vector<short> res;

    if (obj->m_groups && obj->m_groups->at(0))
        for (auto i = 0; i < obj->m_groupCount; i++)
            res.push_back(obj->m_groups->at(i));

    return res;
}

bool hasGroup(GameObject* obj, int group)
{
    for (const auto& g : getGroupIDs(obj))
    {
        if (g == group)
            return true;
    }
    return false;
}

void handleMessage(const matjson::Value& msg, ix::WebSocket& client)
{

    if (!msg.contains("action") || !msg["action"].is_string())
    {
        return sendStatus(client, WSLiveStatus::InvalidJson, "'action' field is missing or is the wrong type", msg);
    }

    auto toUpper_branchless = [](char* d, int count)
    {
            for (int i = 0; i < count; i++)
                d[i] -= 32 * (d[i] >= 'a' && d[i] <= 'z');
    };

    std::string actionstr = msg["action"].as_string();
    toUpper_branchless(actionstr.data(), actionstr.size());

    if (actionstr == "ADD")
    {
        if (!msg.contains("objects") || !msg["objects"].is_string())
        {
            return sendStatus(client, WSLiveStatus::InvalidJson, "'objects' field is missing or is the wrong type", msg);
        }
        queueFunction([msg, &client](LevelEditorLayer* editor)
        {
            editor->createObjectsFromString(msg["objects"].as_string(), true, true); 
            sendStatus(client, WSLiveStatus::Success, std::string{}, msg);
        });
    }
    else if (actionstr == "REMOVE")
    {
        if (!msg.contains("group") || !msg["group"].is_number())
        {
            return sendStatus(client, WSLiveStatus::InvalidJson, "'group' field is missing or is the wrong type", msg);
        }
        if (msg["group"].as_int() <= 0) return;

        queueFunction([msg, &client](LevelEditorLayer* editor)
        {
            int groupToDelete = msg["group"].as_int();
            auto toDelete = cocos2d::CCArray::create();

            for(GameObject* obj : geode::cocos::CCArrayExt<GameObject*>(editor->m_objects))
            {
                if (hasGroup(obj, groupToDelete))
                {
                    toDelete->addObject(obj);
                }
            }

            for (GameObject* obj : geode::cocos::CCArrayExt<GameObject*>(toDelete))
            {
                editor->m_editorUI->deleteObject(obj, false);
            }
            sendStatus(client, WSLiveStatus::Success, {}, msg);

        });
    }
    else if(actionstr == "COLOR_CHANNEL")
    {
        if(!msg.contains("channels") || !msg["channels"].is_array())
            return sendStatus(client, WSLiveStatus::InvalidJson, "''channels' key is missing or is the wrong type", msg);

        auto channelsarr = msg["channels"].as_array();
        std::vector<WSLiveColorChannel> channelsvec; //god i hate this but whatever
        try
        {
            channelsvec = WSLiveColorChannel::getFromArray(channelsarr);
        }
        catch(std::exception& e)
        {
            return sendStatus(client, WSLiveStatus::InvalidJson, e.what(), msg);
        }

        if(channelsarr.size() != channelsvec.size())
        {
            return sendStatus(client, WSLiveStatus::InvalidJson, "Could not create some color channels. check geode console for more info", msg);
        }

        queueFunction([msg, channelsvec, &client](LevelEditorLayer* editor){
            for(const auto& ch : channelsvec)
            {
                if(auto ca = editor->m_levelSettings->m_effectManager->getColorAction(ch.id))
                {
                    if(ch.str) ca->setupFromString(*ch.str);
                    else ca->setupFromString(ch.getStringAsGDLevel());
                }
            }
            sendStatus(client, WSLiveStatus::Success, std::string{}, msg);
        });
    }
    else
    {
        return sendStatus(client, WSLiveStatus::InvalidJson, "Invalid action", msg);
    }
    
}
//will block (call it on a different thread)
void runServer()
{
    if (global::ws)
    {
        geode::log::error("WSLiveEditor instance is already running");
        return;
    }
    ix::initNetSystem();

    ix::WebSocketServer ws(1313, "127.0.0.1");
    global::ws = &ws;

    ws.setOnClientMessageCallback([](std::shared_ptr<ix::ConnectionState> state, ix::WebSocket& sender, const ix::WebSocketMessagePtr& msg) {
        switch (msg->type)
        {
        default: break;
        case ix::WebSocketMessageType::Open:
            geode::log::info("Client {}:{} {} with ID {}", state->getRemoteIp(), state->getRemotePort(), fmt::styled("connected", fmt::fg(fmt::color::lime)), state->getId());
            break;
        case ix::WebSocketMessageType::Close:
            geode::log::info("Client {}:{} {} with ID {}", state->getRemoteIp(), state->getRemotePort(), fmt::styled("disconnected", fmt::fg(fmt::color::red)), state->getId());
            break;
        case ix::WebSocketMessageType::Message:
        {
            //try to catch errors as early as possible
            geode::log::debug("recieved: {}", msg->str);
            //skull
            std::string error;
            matjson::Value parsed;
            try
            {
                parsed = matjson::parse(msg->str);
            }
            catch (std::exception& e)
            {
                return sendStatus(sender, WSLiveStatus::InvalidJson, fmt::format("error parsing json [{}]", e.what()), "");
            }

            bool inEditor = GameManager::get()->getEditorLayer() != nullptr;
            if (!inEditor)
            {
                return sendStatus(sender, WSLiveStatus::NotInEditor, "User is not in the editor", getIdOrEmpty(parsed));
            }

            handleMessage(parsed, sender);
            return;
        }
        }
    });

    ws.disablePerMessageDeflate();
    auto res = ws.listen();
    if (!res.first)
    {
        geode::log::error("error server");
        return;
    }
    ws.start();
    geode::log::info("Listening server at {}:{}", ws.getHost(), ws.getPort());
    ws.wait();


    global::ws = nullptr;
}

