#include "Geode/binding/LevelEditorLayer.hpp"
#include "Geode/loader/Log.hpp"
#include "proxy/proxy_macros.h"
#include <matjson.hpp>
#include <proxy/proxy.h>
#include <ixwebsocket/IXWebSocket.h>

enum class ActionType
{
    ADD_OBJECTS,
    REMOVE_OBJECTS
};



inline std::string_view ActionTypeToString(ActionType a)
{
    switch(a)
    {
        case ActionType::ADD_OBJECTS: return "ADD_OBJECTS";
        case ActionType::REMOVE_OBJECTS: return "REMOVE_OBJECTS";
    }
}


struct Response
{
    bool success;
    bool close;
    std::string response; //success
    std::string message; //error
    std::optional<matjson::Value> echo;

    std::string get()
    {
        matjson::Value ret = matjson::Value::object();
        ret.set("status", success ? "successful" : "error");
        ret.set("response", response);
        ret.set("message", message);
        if(echo)
        {
            ret.set("echo", *echo);
        }
        return ret.dump(4);
    }

    static Response make_success(bool close, std::optional<matjson::Value> echo, const std::string& response)
    {
        return Response{
            .success = true,
            .close = close,
            .response = response,
            .message = {},
            .echo = echo
        };
    }

    static Response make_error(bool close, std::optional<matjson::Value> echo, const std::string& error)
    {
        return Response{
            .success = false,
            .close = close,
            .response = {},
            .message = error,
            .echo = echo
        };
    }

    static Response json_error(const std::string& error)
    {
        return Response{
            .success = false,
            .close = false,
            .response = {},
            .message = error,
            .echo = {}
        };
    }
    void send(ix::WebSocket& client)
    {
        client.sendText(get());
        if(close)
        {
            client.close();
        }
    }
};

PRO_DEF_MEM_DISPATCH(FnExecute, execute);
PRO_DEF_MEM_DISPATCH(FnLog, log);

struct Actionable : pro::facade_builder
    ::add_convention<FnExecute, Response(LevelEditorLayer*)>
    ::add_convention<FnLog, void()>
    ::build{};






struct ActionCommon
{
    ActionType type;
    bool close = false;
    std::optional<matjson::Value> echo;

    template<ActionType action_type>
    static std::optional<ActionCommon> parse(const matjson::Value& parsed)
    {
        if(!parsed.isObject()) return {};

        if(parsed.get<std::string>("action").unwrapOrDefault() != ActionTypeToString(action_type))
        {
            return {};
        }

        ActionCommon ret;

        ret.close = parsed.get<bool>("close").unwrapOr(false);
        if(parsed.contains("echo"))
        {
            ret.echo.emplace(parsed.get("close").unwrap());
        }
        ret.type = action_type;

        return ret;
    }

    void log()
    {
        geode::log::info("TYPE: {}", ActionTypeToString(type));
        geode::log::info("CLOSE: {}", close);
        geode::log::info("HAS ECHO: {}", echo.has_value());
    }

    Response response_success(const std::string& response = {})
    {
        return Response::make_success(close, echo, response);
    }

    Response response_error(const std::string& message)
    {
        return Response::make_error(close, echo, message);
    }
};
