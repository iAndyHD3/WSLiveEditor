#include <ixwebsocket/IXWebSocketServer.h>
#include <zmq.hpp>
#include <variant>
#include <std23/function.h>

struct FlexibleServer
{
    enum class Mode { NotRunning, WebSocket, ZeroMQ };
    using ServerListener = void(std::string_view);
    using ServerListenerFn = std23::function<ServerListener>;


    Mode m_mode = Mode::NotRunning;
    bool m_isWaiting = true;

    struct WebSocketInfo {
        std::unique_ptr<ix::WebSocketServer> server = nullptr;
        ix::WebSocket* client = nullptr;
    };
    struct ZeroMQInfo {
        zmq::context_t context;
        zmq::socket_t socket;
    };

    std::variant<WebSocketInfo, ZeroMQInfo> m_serverInfo;

    std::vector<ServerListenerFn> m_listeners;

    void addListener(const ServerListenerFn& fn) {
        m_listeners.push_back(std::move(fn));
    }

    void initializeWebSocket(int port);
    void initializeZeroMQ(std::string_view ipcname);

    void send(const std::string&);

    bool isWaiting() { return m_isWaiting; }
    bool shouldRespond() { return !m_isWaiting; }

    void run_callbacks(std::string_view message);
    void close_current();

    void stop();

};