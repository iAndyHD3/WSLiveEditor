#include "GetLevelString.hpp"
#include "ActionResponse.hpp"

#include <Geode/loader/Log.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>

bool GetLevelString::isValid(const matjson::Value& j)
{
    return LevelEditorLayer::get() != nullptr;
}

ActionResponse GetLevelString::run(LevelEditorLayer* editor, const matjson::Value& j)
{
    return ActionResponse::make_success(matjson::Value(std::string(editor->getLevelString())));
}