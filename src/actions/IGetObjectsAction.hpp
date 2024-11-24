#pragma once
#include "Geode/binding/LevelEditorLayer.hpp"
#include <ActionInterface.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/binding/GameObject.hpp>
#include <matjson.hpp>
#include <optional>


//base interface for a get object action
//avoids code duplication, eg for get selected/all objects/other kinds of objects
struct IGetObjectsAction : public ActionInterface
{
    struct Filters
    {
        std::optional<std::vector<short>> allowed_groupids;
        std::optional<std::vector<int>> allowed_objectids;
        bool isFilterJsonValid(const matjson::Value& actionJson);
        static std::optional<Filters> getFromActionJson(const matjson::Value& j);
        bool shouldKeepObjectString(std::string_view);
    };

    virtual geode::cocos::CCArrayExt<GameObject*> getObjects(LevelEditorLayer* editor) = 0;

    std::vector<std::string> getFilteredObjectStrings(LevelEditorLayer* editor, const matjson::Value& j);

    virtual bool isValid(const matjson::Value&) override;
    ActionResponse run(LevelEditorLayer* editor, const matjson::Value&) override;
};