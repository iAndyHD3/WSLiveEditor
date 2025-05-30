#include <matjson.hpp>
#include <string>
#include "Action.hpp"
#include "Geode/binding/LevelEditorLayer.hpp"
#include <Geode/loader/Log.hpp>
struct ADD_OBJECTS
{
    ActionCommon common;
    std::string objects;

    static std::optional<ADD_OBJECTS> parse(const matjson::Value& parsed)
    {
        ADD_OBJECTS ret;
        auto common = ActionCommon::parse<ActionType::ADD_OBJECTS>(parsed);
        if(!common) return {};
        ret.common = *common;

        std::string objects = parsed.get<std::string>("objects").unwrapOrDefault();
        if(!objects.starts_with("1,")) return {};
        ret.objects = objects;
        return ret;
    }

    void log()
    {
        common.log();
        geode::log::info("OBJECTS: {}", objects.length() > 20 ? objects.substr(0, 20) : objects);
    }
    Response execute(LevelEditorLayer* editor)
    {
        editor->createObjectsFromString(objects, true, true);
        return common.response_success();
    }
};
