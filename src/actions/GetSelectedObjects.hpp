#pragma once
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/Utils.hpp>


struct GetSelectedObjects
{
    static constexpr auto ACTION_TYPE = "GET_SELECTED_OBJECTS";
    static bool isValid(const matjson::Value&);
    static geode::cocos::CCArrayExt<GameObject*> getObjects(LevelEditorLayer* editor);
};