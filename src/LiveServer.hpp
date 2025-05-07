#include <atomic>
#include <ixwebsocket/IXWebSocket.h>
#include <matjson.hpp>
#include <mutex>
#include <vector>
#include <ixwebsocket/IXWebSocketServer.h>
#include <memory>
#include "ActionInterface.hpp"
#include "Geode/binding/LevelEditorLayer.hpp"

class LiveServer
{
private:
    struct Action
    {
        matjson::Value response;
        ActionInterface* runner;
        ix::WebSocket* client;
        void checkAndCloseConnection() const
        {
            //if(auto closekey = object.find("close"); closekey != object.end() && closekey->second.is_bool() && closekey->second.as_bool())
            if(response["close"].asBool().unwrapOr(false))
            {
                client->close();
            }
        }
    };
    std::unique_ptr<ix::WebSocketServer> ws;
    std::mutex actionMutex;
    std::vector<Action> actions;
    std::vector<std::unique_ptr<ActionInterface>> actionRunners;
    bool running = false;
    std::atomic_bool queuedActions = false;

    void onMessage(std::string_view message, ix::WebSocket* client);
    void onConnectionOpen();
    void onServerCallback(std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg);

    inline void addActionNoLock(const matjson::Value& obj, ActionInterface* runner, ix::WebSocket* client)
    {
        //doesn't run under macOS idk stupid appleclang
        //actions.emplace_back(obj, runner, client);
        auto& action = actions.emplace_back();
        action.response = obj;
        action.runner = runner;
        action.client = client;
    }

public:
    ~LiveServer();

    bool init();
    void stop();
    void runQueuedActions(LevelEditorLayer*);

    struct FindActionResult
    {
        enum Status
        {
            Success,
            NotFound,
            InvalidJson
        } status = Status::NotFound;
        ActionInterface* action = nullptr;
        inline operator bool() { return action && status == Status::Success; }
    };

    FindActionResult getActionForJson(const matjson::Value& actionJson);

    void handleAction(const matjson::Value& action, ix::WebSocket* client);
    
    template <typename... T>
    void AddActionRunners()
    {
        (actionRunners.emplace_back(std::make_unique<T>()), ...);
    }

    template<typename T>
    void addSingleRunner()
    {
        actionRunners.emplace_back(std::make_unique<T>());
    }
};