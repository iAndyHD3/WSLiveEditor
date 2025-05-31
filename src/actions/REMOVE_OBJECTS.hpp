#pragma once
#include <matjson.hpp>
#include <string>
#include "Action.hpp"
#include "Geode/binding/EditorUI.hpp"
#include "Geode/binding/LevelEditorLayer.hpp"
#include "Geode/utils/cocos.hpp"
#include <Geode/loader/Log.hpp>
#include <Geode/binding/GameObject.hpp>


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


struct REMOVE_OBJECTS
{
    ActionCommon common;
    int group;

    DEFINE_PARSE_FUNCTION(REMOVE_OBJECTS)

    static std::optional<REMOVE_OBJECTS> load(REMOVE_OBJECTS& ret, const mjValue& parsed)
    {
        geode::Result<int> group = parsed.get<int>("group");
        if(!group) return {};
        ret.group = *group;
        return ret;
    }

    Response execute(LevelEditorLayer* editor)
    {
        geode::log::debug("START DELETE GROUP {}", group);
        geode::cocos::CCArrayExt<GameObject*> toDelete;
        for(GameObject* obj : geode::cocos::CCArrayExt<GameObject*>(editor->m_objects))
        {
            if (hasGroup(obj, group))
            {
                toDelete.push_back(obj);
            }
        }

        if(toDelete.size() == 0) return common.response_success();


        auto selected = editor->m_editorUI->getSelectedObjects();
        editor->m_editorUI->deselectAll();
        editor->m_editorUI->selectObjects(toDelete.inner(), false);
        editor->m_editorUI->onDeleteSelected(nullptr);
        editor->m_editorUI->selectObjects(selected, false);

        geode::log::debug("DELETE GROUP {} SUCCESSFUL", group);
        return common.response_success();
    }
};

