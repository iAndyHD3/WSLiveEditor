#pragma once

#include "Geode/binding/LevelEditorLayer.hpp"
#include "Geode/loader/Log.hpp"
#include "proxy/proxy_macros.h"
#include <matjson.hpp>
#include <proxy/proxy.h>
#include <ixwebsocket/IXWebSocket.h>

enum class ActionType
{
    ADD_OBJECTS,
    REMOVE_OBJECTS,
    GET_LEVEL_STRING
};

using mjValue = matjson::Value;
namespace mj = matjson;


#define DEFINE_PARSE_FUNCTION(ACTION_NAME)                                      \
    static std::optional<ACTION_NAME> parse(const matjson::Value& parsed) {    \
        ACTION_NAME ret;                                                       \
        auto common = ActionCommon::parse<ActionType::ACTION_NAME>(parsed);    \
        if (!common) return {};                                                \
        ret.common = *common;                                                  \
        return ACTION_NAME::load(ret, parsed);                                 \
    }



inline std::string_view ActionTypeToString(ActionType a)
{
    switch(a)
    {
        case ActionType::ADD_OBJECTS: return "ADD_OBJECTS";
        case ActionType::REMOVE_OBJECTS: return "REMOVE_OBJECTS";
        case ActionType::GET_LEVEL_STRING: return "GET_LEVEL_STRING";
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
        geode::log::error("error running action, {}", error);
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

struct Actionable : pro::facade_builder
    ::add_convention<FnExecute, Response(LevelEditorLayer*)>
    ::build{};


using ActionableProxy = pro::proxy<Actionable>;





struct ActionCommon
{
    ActionType type;
    bool close = false;
    std::optional<matjson::Value> echo;

    template<ActionType action_type>
    static std::optional<ActionCommon> parse(const mjValue& parsed)
    {
        auto parsedAction = parsed.get<std::string>("action").unwrapOrDefault();
        if(parsedAction != ActionTypeToString(action_type))
        {
            geode::log::debug("{} != {}", parsedAction, ActionTypeToString(action_type));
            return {};
        }

        ActionCommon ret;

        ret.close = parsed.get<bool>("close").unwrapOr(false);
        if(parsed.contains("echo"))
        {
            ret.echo.emplace(parsed.get("echo").unwrap());
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
