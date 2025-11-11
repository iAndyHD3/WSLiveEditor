#include "GetSelectedObjects.hpp"
#include "Geode/binding/LevelEditorLayer.hpp"

#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/utils/cocos.hpp>

bool GetSelectedObjects::isValid(const matjson::Value& j)
{
    return true;
}

geode::cocos::CCArrayExt<GameObject*> GetSelectedObjects::getObjects(LevelEditorLayer* editor)
{
    return editor->m_editorUI->getSelectedObjects();
}

