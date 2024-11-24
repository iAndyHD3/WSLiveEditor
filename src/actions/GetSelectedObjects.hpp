#pragma once
#include "IGetObjectsAction.hpp"

struct GetSelectedObjects : public IGetObjectsAction
{
    bool isValid(const matjson::Value&) override;
    inline std::string_view type() override { return "GET_SELECTED_OBJECTS"; }
    virtual geode::cocos::CCArrayExt<GameObject*> getObjects(LevelEditorLayer* editor) override;
};