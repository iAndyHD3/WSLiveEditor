#include <FlexibleServer.hpp>
#include "ActionResponse.hpp"
#include "Geode/loader/Loader.hpp"
#include "actions/Add.hpp"
#include "actions/Remove.hpp"
#include "actions/GetLevelString.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/loader/Log.hpp>
#include <matjson.hpp>
#include <memory>
#include <string_view>

#include <Geode/modify/LevelEditorLayer.hpp>
#include <fmt/format.h>



std::unique_ptr<FlexibleServer> server = std::make_unique<FlexibleServer>();
matjson::Value actionToRun;



void handleAction()
{

    geode::Loader::get()->queueInMainThread([]() -> void {
        auto action_type = actionToRun.get<std::string>("action");
        if(!action_type) return;
        if(AddObjectsAction::ACTION_TYPE == *action_type && AddObjectsAction::isValid(actionToRun))
        {
            
        }
    });

}


void onMessage(std::string_view msg)
{
    auto json = matjson::parse(msg);
    if(!json)
    {
        auto err = json.unwrapErr();
        server->send(ActionResponse::make_error(fmt::format("{}:{} :", err.line, err.column, err.message)).get());
        actionToRun = json.unwrap();
    }
    
    handleAction();
}


$on_mod(Loaded)
{
    server->initializeWebSocket(1313);
    server->addListener(onMessage);
}
