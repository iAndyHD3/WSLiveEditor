#include "Remove.hpp"
#include "ActionResponse.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/EditorUI.hpp>

bool RemoveObjects::isValid(const matjson::Value& j)
{
    if(auto group = checkTypeGetVal<int>(j, "group"); group.first)
    {
        return group.second >= 0 && group.second <= 99'999;
    }
    return false;
}

static gd::vector<short> getGroupIDs(GameObject* obj) {
    gd::vector<short> res;

    if (obj->m_groups && obj->m_groups->at(0))
        for (auto i = 0; i < obj->m_groupCount; i++)
            res.push_back(obj->m_groups->at(i));
    return res;
}

static bool hasGroup(GameObject* obj, int group)
{
    for (const auto& g : getGroupIDs(obj))
    {
        if (g == group)
            return true;
    }
    return false;
}



ActionResponse RemoveObjects::run(LevelEditorLayer* editor, const matjson::Value& j)
{
    int groupToDelete = j["group"].asInt().unwrapOrDefault();

    geode::cocos::CCArrayExt<GameObject*> toDelete;
    for(GameObject* obj : geode::cocos::CCArrayExt<GameObject*>(editor->m_objects))
    {
        if (hasGroup(obj, groupToDelete))
        {
            toDelete.push_back(obj);
        }
    }

    if(toDelete.size() == 0) return ActionResponse::make_success();


    auto selected = editor->m_editorUI->getSelectedObjects();
    editor->m_editorUI->deselectAll();

    editor->m_editorUI->selectObjects(toDelete.inner(), false);

    editor->m_editorUI->onDeleteSelected(nullptr);

    editor->m_editorUI->selectObjects(selected, false);
    return ActionResponse::make_success();
}