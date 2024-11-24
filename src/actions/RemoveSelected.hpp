/*

#pragma once
#include <ActionInterface.hpp>

struct RemoveSelectedObjects : public ActionInterface
{
    bool isValid(const matjson::Object&) override;
    inline std::string_view type() override { return "REMOVE_SELECTED_OBJECTS"; }
    
    ActionResponse run(LevelEditorLayer* editor, const matjson::Object&) override;
};
*/