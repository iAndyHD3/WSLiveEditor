

#include <ixwebsocket/IXWebSocketServer.h>
#include <atomic>
#include <mutex>

#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>


struct MyEditor;

enum class WSLiveAction
{
    ADD,
    REMOVE
};

namespace global
{
    std::mutex webSocketMutex;
    ix::WebSocketServer* ws = nullptr;
    std::atomic<bool> serverRunning = false;
}

//runs server in on a different thread
void runServer()
{
    ix::initNetSystem();

    ix::WebSocketServer ws(1313, "127.0.0.1");
    global::ws = &ws;

    ws.setOnClientMessageCallback([](std::shared_ptr<ix::ConnectionState>, ix::WebSocket&, const ix::WebSocketMessagePtr&) {
        geode::log::info("got something");

    });

    ws.disablePerMessageDeflate();
    ws.enablePong();
    auto res = ws.listen();
    if (!res.first)
    {
        geode::log::error("error server");
        return;
    }
    geode::log::debug("ok server");
    ws.start();
    global::serverRunning = true;
    ws.wait();
}


struct MyEditor : geode::Modify<MyEditor, LevelEditorLayer>
{

    void handleClientMessage(ix::WebSocket& sender, std::string_view msg)
    {
        geode::log::info("got: {}", msg);
    }
    bool init(GJGameLevel* level, bool idk)
    {
        std::thread([] { runServer(); }).detach();

        if (!LevelEditorLayer::init(level, idk)) return false;


        //global::ws->setOnClientMessageCallback([this](
        //    std::shared_ptr<ix::ConnectionState> connectionState,
        //    ix::WebSocket& webSocket,
        //    const ix::WebSocketMessagePtr& msg)
        //    {
        //        geode::Loader::get()->queueInMainThread([&] {handleClientMessage(webSocket, msg->str); });
        //    }
        //);

        return true;
    }
};