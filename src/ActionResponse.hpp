#pragma once

#include <ixwebsocket/IXWebSocket.h>
#include <matjson.hpp>
#include <optional>
#include <string>

struct ActionResponse
{
    enum Status
    {
        None, Success, Error
    };
    
    Status status = Status::None;
    std::optional<std::string> error_message;
    std::optional<matjson::Value> response;

    inline bool send(ix::WebSocket* client, bool closeConnection = false)
    {
        matjson::Value jsonresp;
        if(status == Status::Success)
        {
            jsonresp["status"] = "successful";
            if(response) jsonresp["response"] = *response;
        }
        else if(status == Status::Error)
        {
            jsonresp["status"] = "error";
            jsonresp["message"] = error_message.value_or("Unknown error running action");
        }
        return client->sendText(matjson::Value(jsonresp).dump(matjson::NO_INDENTATION)).success;
    }

    inline static ActionResponse make_success(const std::optional<matjson::Value>& resp_opt = {})
    {
        return
        {
            .status = Success,
            .error_message = {},
            .response = resp_opt,
        };
    }
    inline static ActionResponse make_error(const std::optional<std::string>& error_msg_opt = {})
    {
        return
        {
            .status = Error,
            .error_message = error_msg_opt,
            .response = {},
        };
    }
};