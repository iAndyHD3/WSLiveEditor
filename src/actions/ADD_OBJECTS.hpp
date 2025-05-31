#pragma once

#include <matjson.hpp>
#include <string>
#include "Action.hpp"
#include "Geode/binding/LevelEditorLayer.hpp"
#include <Geode/loader/Log.hpp>



struct ADD_OBJECTS
{
    ActionCommon common;
    std::string objects;

    DEFINE_PARSE_FUNCTION(ADD_OBJECTS)

    static std::optional<ADD_OBJECTS> load(ADD_OBJECTS& ret, const mjValue& parsed)
    {
        std::string objects = parsed.get<std::string>("objects").unwrapOrDefault();
        if(!objects.starts_with("1,")) return {};
        ret.objects = objects;
        return ret;
    }

    Response execute(LevelEditorLayer* editor)
    {
        geode::log::debug("START ADDING OBJECTS {}", objects.length() > 20 ? objects.substr(0, 20) : objects);
        editor->createObjectsFromString(objects, true, true);
        geode::log::debug("ADDING OBJECTS SUCCESSFUL");
        return common.response_success();
    }
};

