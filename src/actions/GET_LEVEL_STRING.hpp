#pragma once
#include <matjson.hpp>
#include <string>
#include "Action.hpp"
#include "Geode/binding/EditorUI.hpp"
#include "Geode/binding/LevelEditorLayer.hpp"
#include "Geode/utils/cocos.hpp"
#include <Geode/loader/Log.hpp>
#include <Geode/binding/GameObject.hpp>





struct GET_LEVEL_STRING
{
    ActionCommon common;

    DEFINE_PARSE_FUNCTION(GET_LEVEL_STRING)

    static std::optional<GET_LEVEL_STRING> load(GET_LEVEL_STRING& ret, const mjValue& parsed)
    {
        return ret;
    }

    Response execute(LevelEditorLayer* editor)
    {
        return common.response_success(editor->getLevelString());
    }
};

