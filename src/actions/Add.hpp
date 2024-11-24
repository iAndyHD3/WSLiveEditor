#pragma once
#include <ActionInterface.hpp>

struct AddObjectsAction : public ActionInterface
{
    bool isValid(const matjson::Value&) override;
    inline std::string_view type() override { return "ADD_OBJECTS"; }

    //'ADD' for compat with < 2.0 versions
    inline bool isType(std::string_view actionname) override {
        return actionname == type() || actionname == "ADD";
    }
    ActionResponse run(LevelEditorLayer* editor, const matjson::Value&) override;
};
