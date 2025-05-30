
#include <functional>
#include <ixwebsocket/IXConnectionState.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <ixwebsocket/IXWebSocketMessage.h>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketMessageType.h>


#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <matjson.hpp>
#include <mutex>
#include "Geode/loader/ModEvent.hpp"
#include "actions/ADD_OBJECTS.hpp"
#include "proxy/proxy.h"


std::string json = R"(
{
  "action": "ADD_OBJECTS",
  "objects": "1,1,2,60,3,90,57,1.9999",
  "close": true,
  "echo": "my string"
}
)";

struct ActionClient
{
    pro::proxy<Actionable> action;
    std::reference_wrapper<ix::WebSocket> client;
};

ix::WebSocketServer* ws;
std::vector<ActionClient> actionsVec;
std::atomic_bool hasQueuedActions;
std::mutex actionsVecMutex;



void handleMessage(std::shared_ptr<ix::ConnectionState> _, ix::WebSocket & webSocket, const ix::WebSocketMessagePtr& msg);


$on_mod(Loaded)
{
    ix::initNetSystem();
    int port = geode::Mod::get()->getSettingValue<int>("port");
    ws = new ix::WebSocketServer(port);
    ws->disablePerMessageDeflate();
    ws->disablePong();
    ws->setOnClientMessageCallback(handleMessage);
    ws->listenAndStart();
    geode::log::info("start ws: {}", true);
}


std::optional<pro::proxy<Actionable>> getAction(const matjson::Value& json)
{
    std::optional<ADD_OBJECTS> opt = ADD_OBJECTS::parse(json);
    if(!opt) return {};
    return pro::make_proxy<Actionable>(*opt);
}

void handleMessage(std::shared_ptr<ix::ConnectionState> _, ix::WebSocket& client, const ix::WebSocketMessagePtr& msg)
{
    if(msg->type != ix::WebSocketMessageType::Message) return;
    
    geode::log::info("GOT: {}", msg->str);
    geode::Result<matjson::Value, matjson::ParseError> json = matjson::parse(msg->str);
    if (!json)
    {
        Response::json_error(json.unwrapErr()).send(client);
        return;
    }

    
    if(std::optional<pro::proxy<Actionable>> proxy = getAction(*json))
    {
        std::lock_guard l(actionsVecMutex);
        actionsVec.push_back(ActionClient{std::move(*proxy), client});
        hasQueuedActions = true;
    }
    else
    {
        Response::json_error("wrong action object. check type and keys").send(client);
    }
}

void handleAction(pro::proxy<Actionable> action)
{
    action->log();
}

class $modify(LiveEditor, LevelEditorLayer)
{
    void handleLiveEditor(float dt)
    {
        if(hasQueuedActions)
        {
            std::lock_guard l(actionsVecMutex);
            for(ActionClient& pair : actionsVec)
            {
                pair.action->execute(this).send(pair.client);
            }
            actionsVec.clear();
            hasQueuedActions = false;
        }
    }

    bool init(GJGameLevel* p0, bool p1) {
        if (!LevelEditorLayer::init(p0, p1)) return false;
        schedule(schedule_selector(LiveEditor::handleLiveEditor), 0.5f);

        return true;
    }
};