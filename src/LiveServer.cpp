#include <FlexibleServer.hpp>
#include "ActionResponse.hpp"
#include "Geode/loader/Loader.hpp"
#include "actions/Add.hpp"
#include "actions/Remove.hpp"
#include "actions/GetLevelString.hpp"
#include "actions/GetSelectedObjects.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/loader/Log.hpp>
#include <matjson.hpp>
#include <string_view>

#include <Geode/modify/LevelEditorLayer.hpp>
#include <fmt/format.h>

#include <ZeroMQServer.hpp>
#include <std23/function_ref.h>
#include <thread>

std::string onMessage(std::string_view msg);
std::unique_ptr<ZmqIpcServer> zmqServer;


std::string handleAction(const matjson::Value& action)
{
    auto action_type = action.get<std::string>("action");
    if(!action_type) return ActionResponse::make_error("no action found").get();

#define DO_ACTION_CHECK(FN_ACTION_TYPE) \
    if(FN_ACTION_TYPE::ACTION_TYPE == *action_type && FN_ACTION_TYPE::isValid(action))\
        return FN_ACTION_TYPE::run(LevelEditorLayer::get(), action).get()
    
    DO_ACTION_CHECK(AddObjectsAction);
    DO_ACTION_CHECK(RemoveObjects);
    DO_ACTION_CHECK(GetLevelString);



    return ActionResponse::make_error("No matching action found").get();
}

//runs on polling thread still
std::string onMessage(std::string_view msg)
{
    auto json = matjson::parse(msg);
    if(!json)
    {
        auto err = json.unwrapErr();
        return ActionResponse::make_error(fmt::format("{}:{} :", err.line, err.column, err.message)).get();
    }
    std::string response;
    std::atomic<bool> response_ready = false;

    
    geode::Loader::get()->queueInMainThread([&](){
        response = handleAction(json.unwrap());
        response_ready.store(true);
    });

    while(!response_ready.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return response;
}

$on_mod(Loaded) {
    try {
    zmqServer = std::make_unique<ZmqIpcServer>("iandyhd3-wsliveeditor", onMessage);
    zmqServer->start();
    } catch(std::exception e) {
        geode::log::error("{}", e.what());
        return;
    }
        geode::log::error("all good");
}