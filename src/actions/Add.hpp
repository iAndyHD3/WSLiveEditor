#pragma once
#include "ActionResponse.hpp"

struct LevelEditorLayer;

struct AddObjectsAction
{
    static constexpr auto ACTION_TYPE = "ADD_OBJECTS";
    static bool isValid(const matjson::Value&);

    static ActionResponse run(LevelEditorLayer* editor, const matjson::Value&);
};
