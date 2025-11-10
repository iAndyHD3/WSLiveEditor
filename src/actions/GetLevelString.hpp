#pragma once

struct LevelEditorLayer;
#include <ActionResponse.hpp>
#include <matjson.hpp>

struct GetLevelString
{
    static constexpr auto ACTION_TYPE = "GET_LEVEL_STRING";  
    static bool isValid(const matjson::Value&);
    static ActionResponse run(LevelEditorLayer* editor, const matjson::Value&);
};
