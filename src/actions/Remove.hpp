#pragma once
#include <ActionResponse.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <matjson.hpp>

struct LevelEditorLayer;

struct RemoveObjects
{
    static constexpr auto ACTION_TYPE = "REMOVE_OBJECTS";
    static bool isValid(const matjson::Value&);
    static ActionResponse run(LevelEditorLayer* editor, const matjson::Value&);
};
