#pragma once
#include <ActionInterface.hpp>

struct RemoveObjects : public ActionInterface
{
    bool isValid(const matjson::Value&) override;
    inline std::string_view type() override { return "REMOVE_OBJECTS"; } 

    //'REMOVE' for compat with < 2.0 versions
    inline bool isType(std::string_view actionname) override {
        return actionname == type() || actionname == "REMOVE";
    }
    ActionResponse run(LevelEditorLayer* editor, const matjson::Value&) override;
};
