#include "Add.hpp"
#include "ActionResponse.hpp"
#include <string>

#include <Geode/loader/Log.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/binding/GameObject.hpp>

// class $modify(LevelEditorLayer) {
//     struct Fields {
//         std::vector<GameObject*>
//     }
// };

bool AddObjectsAction::isValid(const matjson::Value& j)
{
    if(!LevelEditorLayer::get()) return false;
    
    if(auto objects = checkTypeGetVal<std::string>(j, "objects"); objects.first)
    {
        //atleast an object id specifier (usually even the first 2 chars)
        return objects.second.find("1,") != std::string::npos;
    }
    return false;
}


ActionResponse AddObjectsAction::run(LevelEditorLayer* editor, const matjson::Value& j)
{
    auto str = j["objects"].asString().unwrap();
    editor->createObjectsFromString(str, true, true);
    return ActionResponse::make_success(); 
}