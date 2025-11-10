#include "FlexibleServer.hpp"
#include <Geode/loader/Log.hpp>
#include <string_view>

void FlexibleServer::initializeWebSocket(int port)
{
    if(m_mode != Mode::NotRunning) return;

    ix::initNetSystem();

    m_serverInfo = WebSocketInfo{};
    

    std::get<WebSocketInfo>(m_serverInfo).server = std::make_unique<ix::WebSocketServer>(port);

    auto* ws = std::get<WebSocketInfo>(m_serverInfo).server.get();

    ws->setOnClientMessageCallback([this](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket & webSocket, const ix::WebSocketMessagePtr & msg)
    {
        using enum ix::WebSocketMessageType;
        auto connected_client = std::get<WebSocketInfo>(m_serverInfo).client;
        switch(msg->type)
        {
        case Message:
            if(connected_client != &webSocket) {
                connected_client = &webSocket;
            }
            run_callbacks(msg->str);
            break;
        case Open:
            geode::log::debug("connection opened");
            break;
        case Close:
            geode::log::debug("connection closed");
        default: break;
        }
    });

    ws->disablePerMessageDeflate();
    ws->disablePong();
    ws->start();
    ws->listen();
    m_mode = Mode::WebSocket;
    m_isWaiting = true;
    geode::log::info("INITIALIZED SERVER WEBSOCKET!");
    
}

void FlexibleServer::run_callbacks(std::string_view message) {
    for(const auto& f : m_listeners) {
        f(message);
    }
}
void FlexibleServer::close_current() {
    if(m_mode == Mode::WebSocket) {
        std::get<WebSocketInfo>(m_serverInfo).client->close();
    }
}

void FlexibleServer::initializeZeroMQ(std::string_view ipcname)
{

}


void FlexibleServer::send(const std::string& msg)
{
    if(!shouldRespond()) return;

    if(m_mode == Mode::WebSocket)
    {
        std::get<WebSocketInfo>(m_serverInfo).client->send(msg);
    }
}


void FlexibleServer::stop()
{

}