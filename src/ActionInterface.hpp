#pragma once
#include <matjson.hpp>
#include <string_view>
#include <optional>
#include "ActionResponse.hpp"
#include "Geode/binding/LevelEditorLayer.hpp"

struct ActionInterface
{
    virtual bool isValid(const matjson::Value&) = 0;
    virtual std::string_view type() = 0;
    inline virtual bool isType(std::string_view actionname)
    {
        return actionname == type(); 
    }
    virtual ActionResponse run(LevelEditorLayer* editor, const matjson::Value&) = 0;

    inline bool isJsonActionType(const matjson::Value& j)
    {
        return isType(getOpt<std::string>(j, "action").value_or(""));
    }

    virtual ~ActionInterface() = default;

protected:

    template<typename T>
    [[nodiscard]] static std::optional<T> getOpt(const matjson::Value& j, std::string_view key)
    {
        return j[key].as<T>().ok();
    }


    template<typename T>
    [[nodiscard]] static bool checkType(const matjson::Value& j, std::string_view key)
    {
        return j[key].as<T>().isOk();
    }
    template<typename T>
    [[nodiscard]] static std::pair<bool, T> checkTypeGetVal(const matjson::Value& j, std::string_view key)
    {
        auto res = j[key].as<T>();
        return std::pair<bool, T>({res.isOk(), res.unwrapOrDefault()});
    }
};