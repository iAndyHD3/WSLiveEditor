#include "LiveServer.hpp"
#include "ActionInterface.hpp"
#include "ActionResponse.hpp"
#include "actions/Add.hpp"
#include "actions/Remove.hpp"
#include "actions/GetLevelString.hpp"
#include "actions/GetSelectedObjects.hpp"
#include "actions/RemoveSelected.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketMessageType.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <Geode/loader/Log.hpp>
#include <matjson.hpp>
#include <mutex>

#include <Geode/modify/LevelEditorLayer.hpp>

using LS = LiveServer;

bool LS::init()
{
    if(running)
    {
        return false;
    }
    ix::initNetSystem();

    ws = std::make_unique<ix::WebSocketServer>(1313);


    ws->setOnClientMessageCallback([this](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket & webSocket, const ix::WebSocketMessagePtr & msg)
    {
        using enum ix::WebSocketMessageType;
        switch(msg->type)
        {
        case Message:
            onMessage(msg->str, &webSocket);
            break;
        case Open:
            onConnectionOpen();
            break;
        default: break;
        }
    });

    ws->disablePerMessageDeflate();
    ws->disablePong();
    ws->start();
    ws->listen();
    running = true;
    
    return true;
}

void LS::onMessage(std::string_view message, ix::WebSocket* client)
{
    auto json = matjson::parse(message);
    if(!json)
    {
        auto err = json.unwrapErr();
        ActionResponse::make_error(fmt::format("{}:{} :", err.line, err.column, err.message)).send(client);
        return;
    }

    switch((*json).type())
    {
        case matjson::Type::Object:
            handleAction(json.unwrap(), client);
            break;
        //TODO mutliple actions
        //case matjson::type::Array:
        default: break;
    }
}

LS::FindActionResult LS::getActionForJson(const matjson::Value& actionJson)
{
    using enum LS::FindActionResult::Status;

    LS::FindActionResult ret {.status = NotFound, .action = nullptr};

    for(const auto& action : actionRunners)
        if(action->isJsonActionType(actionJson))
            ret.action = action.get();

    if(!ret.action) return ret;

    ret.status = ret.action->isValid(actionJson) ? Success : InvalidJson;
    return ret;
}


void LS::handleAction(const matjson::Value& actionJson, ix::WebSocket* client)
{
    //for(const auto& actionType : actionRunners)
    //{
    //    if(!actionType->isType(action)) continue;
    //    
    //    if(actionType->isType(action) && actionType->isValid(action))
    //    {
    //        std::lock_guard lock(actionMutex);
    //        addActionNoLock(action, actionType.get(), client);
    //        queuedActions = true;
    //        return;
    //    }
    //}

    auto actionResult = getActionForJson(actionJson);
    if(actionResult)
    {
        std::lock_guard lock(actionMutex);
        addActionNoLock(actionJson, actionResult.action, client);
        queuedActions = true;
    }
    else if(actionResult.status == FindActionResult::NotFound)
    {
        ActionResponse::make_error("action type not found").send(client);
    }
    else if(actionResult.status == FindActionResult::InvalidJson)
    {
        ActionResponse::make_error("Invalid json for specified action").send(client);
    }

}

void LS::onConnectionOpen()
{
    geode::log::info("connection opened");
}

void LS::stop()
{
    if(!running) return;

    for(const auto& client : ws->getClients())
    {
        client->close(1, "User left the editor");
    }
    
    ws->stop();
    ix::uninitNetSystem();
    running = false;
}

LS::~LiveServer()
{
    stop();
}

void LS::runQueuedActions(LevelEditorLayer* editor)
{
    if(queuedActions)
    {
        std::lock_guard lock(actionMutex);
        for(const auto& action : actions)
        {
            geode::log::info("Running action of type {}", action.runner->type());
            if(action.runner->run(editor, action.object).send(action.client))
            {
                action.checkAndCloseConnection();
            }
        }
        actions.clear();
    }
}


struct LSHooks : geode::Modify<LSHooks, LevelEditorLayer>
{
    struct Fields
    {
        LS server;
    };

    void performQueuedActions(float dt)
    {
        m_fields->server.runQueuedActions(this);
    }

    bool init(GJGameLevel* level, bool idk)
    {
        if(!LevelEditorLayer::init(level, idk)) return false;

        m_fields->server.AddActionRunners<
                AddObjectsAction,
                RemoveObjects,
                //RemoveSelectedObjects,
                GetLevelString,
                GetSelectedObjects>();
        m_fields->server.init();
        this->schedule(schedule_selector(LSHooks::performQueuedActions), 0.0f);
        geode::log::info("STARTED WSLIVEEDITOR SERVER");

        return true;
    }
};