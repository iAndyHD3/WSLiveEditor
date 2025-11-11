#pragma once
#include <zmq.hpp>
#include <thread>
#include <atomic>
#include <functional>
#include <string>
#include <optional>
#include <Geode/loader/Log.hpp>
/*
 *  High-performance, header-only C++17 wrapper around a ZeroMQ REP socket
 *  that listens on a local IPC endpoint and processes requests on its own
 *  event loop.
 *
 *  Usage:
 *      ZmqIpcServer srv("my_service",
 *          [](std::string_view req) -> std::optional<std::string> {
 *              return "echo: " + std::string(req);
 *          });
 *      srv.start();   // blocks current thread
 */
class ZmqIpcServer {
public:
    using Handler = std::function<std::string(std::string_view)>;

    ZmqIpcServer(std::string ipc_name, Handler handler)
        : ipc_name_(std::move(ipc_name)),
          handler_(std::move(handler)),
          ctx_(1),                     // 1 I/O thread
          socket_(ctx_, zmq::socket_type::rep),
          stop_(false) {
        socket_.set(zmq::sockopt::linger, 0); // drop pending msgs on close
    }

    ~ZmqIpcServer() { stop(); }

    // Start the event loop (blocks until stop() is called)
    void start() {
        std::string endpoint = "tcp://127.0.0.1:57411";
        socket_.bind(endpoint);

        std::thread(
        [this]()
        {
            while (!stop_.load(std::memory_order_relaxed))
            {
                zmq::message_t req;
                // non-blocking receive with 100 ms timeout
                
                auto res = socket_.recv(req);
                std::string reply; 
                if(!req.more()) {
                    std::string_view req_str{static_cast<char*>(req.data()), req.size()};
                    reply = handler_(req_str);
                }
                else {
                    int i = 1;
                    std::string msg;
                    while(req.more()) {
                        msg += std::string_view{static_cast<char*>(req.data()), req.size()};
                        geode::log::error("{}", i);
                        i++;
                    }
                    geode::log::error("FINISHED!");
                    reply = handler_(msg);
                }

                if(reply.empty())
                {
                    socket_.send(zmq::message_t{}, zmq::send_flags::none);
                }
                else
                {
                    socket_.send(zmq::buffer(reply));
                }
            }
        }
        ).detach();
    }

    // Request graceful shutdown (thread-safe)
    void stop() { stop_.store(true, std::memory_order_relaxed); }

    void load_response() {
        
    }

private:
    std::string ipc_name_;
    Handler handler_;
    zmq::context_t ctx_;
    zmq::socket_t socket_;
    std::atomic<bool> stop_;
    std::string response_string;
};