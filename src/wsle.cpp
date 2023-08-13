#include "json.hpp"

#include "wsle.h"
#include <utility>
#include <gd.h>
#include <iostream>
#include <fmt/format.h>
#include <chrono>
#include "MinHook.h"
#include "magic_enum.hpp"
#include <charconv>

using json = nlohmann::json;

namespace wsle 
{
	//all valid json messages go here
	void redirect_to_handler(const nlohmann::json& j, ix::WebSocket& socket)
	{
		bool in_editor = gd::LevelEditorLayer::get() != nullptr;
		if(!in_editor) return sendResultData(ActionResult::USER_NOT_IN_EDITOR, socket);
		
		try
		{
			if(!j.contains("value") || !j.contains("type"))
			{
				return sendResultData(ActionResult::INVALID_JSON, socket);
			}
		}
		catch(std::exception& e)
		{
			return sendResultData(ActionResult::INVALID_JSON, socket);
		}
		
		auto toUpper_branchless = [](char* d, int count)
		{
			for (int i = 0; i < count; i++)
				d[i] -= 32 * (d[i] >= 'a' && d[i] <= 'z');
		};
		
		std::string strtype = j["type"].get<std::string>();
		toUpper_branchless(strtype.data(), strtype.size());
		std::cout << strtype << '\n';
		
		std::optional<ActionType> type = magic_enum::enum_cast<ActionType>(strtype);
		if(type.has_value())
		{
			switch(*type)
			{
				case ActionType::ADD_OBJECTS_STRING:   return ADD_OBJECTS_STRING::handle(j, socket);
				case ActionType::REMOVE_OBJECTS_GROUP: return REMOVE_OBJECTS_GROUP::handle(j, socket);
				case ActionType::GET_LEVEL_STRING:     return GET_LEVEL_STRING::handle(j, socket);
			}
		}
		fmt::println("Unknown type: {}", strtype);
		sendResultData(ActionResult::INVALID_TYPE, socket);
	}

	void ADD_OBJECTS_STRING::handle(const nlohmann::json& j, ix::WebSocket& socket)
	{
		queueAction([j, &socket](gd::LevelEditorLayer* self)
		{
			splitCallback(j["value"].get<std::string>(), ';', [self](const std::string &s) { self->addObjectFromString(s); });
			sendResultData(ActionResult::OK, socket);
		});
	}
	
	auto containsAny(const auto& v1, const auto& v2)
	{
		for (const auto& i1 : v1)
			for (const auto& i2 : v2)
				if (i1 == i2) return true;
		return false;
	}

	void REMOVE_OBJECTS_GROUP::handle(const nlohmann::json& j, ix::WebSocket& socket)
	{
		queueAction([j,&socket](gd::LevelEditorLayer* self)
		{
			auto removeGroups = REMOVE_OBJECTS_GROUP::getGroupsFromJson(j["value"]);
			if(removeGroups.empty()) return sendResultData(ActionResult::INVALID_JSON, socket);
			
			cocos2d::CCArray* all = self->getAllObjects();
			int objectCount = all->count();
			if(objectCount <= 0) return sendResultData(ActionResult::OK, socket);
			
			cocos2d::CCArray* toDelete = cocos2d::CCArray::create();
			
			for(int i = 0; i < objectCount; i++)
			{
				auto obj = reinterpret_cast<gd::GameObject*>(all->objectAtIndex(i));
				std::vector<short> groups = obj->getGroupIDs();
				if(containsAny(groups, removeGroups))
				{
					toDelete->addObject(obj);
				}
			}
			int deleteCount = toDelete->count();
			for(int i = 0; i < deleteCount; i++)
			{
				auto obj = reinterpret_cast<gd::GameObject*>(toDelete->objectAtIndex(i));
				self->m_pEditorUI->deleteObject(obj, false);
			}
			sendResultData(ActionResult::OK, socket);
		});
	}
	

	std::set<short> REMOVE_OBJECTS_GROUP::getGroupsFromJson(const json& j)
	{
		//return single group
		if (j.is_number_unsigned())
		{
			return { j.template get<short>() };
		}

		if (j.is_array())
		{
			std::set<short> groups;
			for (const auto& element : j)
			{
				if(element.is_number_unsigned())
					groups.insert(element.get<short>());
			}
			return groups;
		}
		return {};
	}

	void GET_LEVEL_STRING::handle(const json& j, ix::WebSocket& socket)
	{
		queueAction([j, &socket](gd::LevelEditorLayer* self){

			cocos2d::CCArray* all = self->getAllObjects();
			int objectCount = all->count();
			if(objectCount <= 0) return sendResultData(ActionResult::EMPTY_LEVEL, socket);

			std::string ret{};

			constexpr int AVERAGE_OBJECT_STRING_LENGTH = 10; 
			ret.reserve(objectCount * AVERAGE_OBJECT_STRING_LENGTH);

			if(const auto& ls = j["levelsettings"]; ls.is_boolean() && ls.get<bool>())
			{
				ret.append(self->m_pLevelSettings->getSaveString());
				ret.push_back(';');
			}

			for(int i = 0; i < objectCount; i++)
			{
				auto obj = reinterpret_cast<gd::GameObject*>(all->objectAtIndex(i));
				ret.append(obj->getSaveString());
				ret.push_back(';');
			}
			ret.pop_back(); //remove last ;

			json retjson = {
				{"ok", true},
				{"value", ret}
			};
			socket.send(retjson.dump());
		});

	}
	
	void sendResultData(ActionResult result, ix::WebSocket& socket, std::source_location location)
	{
		using enum wsle::ActionResult;
		
		switch(result)
		{
			case OK: socket.send("{\"ok\":true}"); break;
			default: 
			{
				nlohmann::json j;
				j["ok"] = false;
				j["error"] = static_cast<std::string>(magic_enum::enum_name(result));
				socket.send(j.dump());
			}
		}
		fmt::println("Sending result: {} | {}({})", magic_enum::enum_name(result), location.file_name(), location.line());
	}
}


/*

[
  {
    "type": "add_object_string",
    "value": "1,1,2,2,3,3",
    "order": 1
  },
  {
    "type": "remove_objects",
    "filter": "groupid",
    "value": 5,
    "order": 2
  }
]

*/