#include "IGetObjectsAction.hpp"
#include "ActionResponse.hpp"
#include "Geode/binding/LevelEditorLayer.hpp"
#include "Geode/utils/cocos.hpp"
#include <cstddef>
#include <matjson.hpp>
#include <matjson/stl_serialize.hpp>
#include <fmt/format.h>

using geode::cocos::CCArrayExt;

//TODO: filters


//static bool isNumberRangeValid(const matjson::Array& numberFilter)
//{
//    if(numberFilter.size() != 2) return false;
//
//    auto min = numberFilter[0];
//    auto max = numberFilter[1];
//
//    if(!min.is_number() || !max.is_number()) return false;
//
//    return max.as_int() > min.as_int();
//}
//TODO
//bool IGetObjectsAction::Filters::isFilterJsonValid(const matjson::Value& actionJson)
//{
//    return true;
//}
//
//TODO
//bool IGetObjectsAction::Filters::shouldKeepObjectString(std::string_view)
//{
//    return true;
//}
//
//TODO
//std::optional<IGetObjectsAction::Filters> IGetObjectsAction::Filters::getFromActionJson(const matjson::Value& j)
//{
//    return {};
//}


bool IGetObjectsAction::isValid(const matjson::Value& j)
{
    return true;
}

std::vector<std::string> IGetObjectsAction::getFilteredObjectStrings(LevelEditorLayer* editor, const matjson::Value& j)
{
    std::vector<std::string> ret;
    CCArrayExt<GameObject*> objects = getObjects(editor);
    ret.reserve(objects.size());
    for(const auto& obj : objects)
    {
        ret.emplace_back(obj->getSaveString(editor));
    }
    return ret;
}


ActionResponse IGetObjectsAction::run(LevelEditorLayer* editor, const matjson::Value& j)
{
    //if we have a separator...
    std::vector<std::string> objstrings = getFilteredObjectStrings(editor, j);
    if(objstrings.empty())
    {
        return ActionResponse::make_success();
    }

    auto separatorOpt = getOpt<std::string>(j, "separator");
    if(!separatorOpt)
    {
        return ActionResponse::make_success(matjson::Value(objstrings));
    }

    std::string& separator = *separatorOpt;
    std::string ret;
    if(separator.empty()) [[likely]]
    {
        for(const auto& str : objstrings)
        {
            ret += str;
        }
        return ActionResponse::make_success(matjson::Value(ret));
    }

    //atleast this size
    ret.reserve(separator.size() * objstrings.size());

    for(const auto& str : objstrings)
    {
        ret += fmt::format("{}{}", str, separator);
    }

    for(size_t i = separator.size(); i > 0; i--)
    {
        ret.pop_back();
    }

    return ActionResponse::make_success(matjson::Value(ret));
}
