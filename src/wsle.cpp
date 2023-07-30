
#include "json.h"
#include "wsle.h"
#include <set>
#include <utility>
#include <gd.h>
#include <algorithm>
#include <fmt/format.h>

namespace wsle 
{
	void handle(const json::jobject& j, ix::WebSocket* socket)
	{
		try
		{
			//first is the "order" key, second is the index the array is in.
			std::set<std::pair<int, int>> actionList;
			
			int size = j.size();
			for(int i = 0; i < size; i++)
			{
				int order = j.array(i).get("order");
				actionList.emplace(order, i);
			}
			
			for(const auto& action : actionList)
			{
				printf("calling action | order: %d, index: %d\n", action.first, action.second);
				sendResult(handleAction(j.array(action.second)), socket);
			}
		}
		catch(std::exception& e)
		{
			return sendResult(std::make_pair(false, e.what()), socket);
		}
	}
	
	std::pair<bool, std::string> handleAction(const json::jobject& action)
	{
		try
		{
			auto editor = gd::LevelEditorLayer::get();
			
			if(!editor) 				return { false, "User is not in the editor" };
			if(!action.has_key("type")) return { false, "type key is missing" };
			
			std::string type = action["type"];
			
			//helper
			
			if(type == "add_objects_string")
			{
				if(!action.has_key("value")) return { false, "value key is missing" };
				
				add_objects_string aos;
				aos.value = action["value"].as_string();
				return aos.handle(editor);
			}
			if(type == "remove_objects")
			{
				if(!action.has_key("value")) 	return { false, "value key is missing" };
				if(!action.has_key("filter")) 	return { false, "filter key is missing" };
				
				remove_objects ro;
				ro.value = action["value"].as_string();
				ro.filter = action["filter"].as_string();
				return ro.handle(editor);
			}
		}
		catch(std::exception& e)
		{
			return std::make_pair(false, e.what());
		}
	}


	void sendResult(const std::pair<bool, std::string>& result, ix::WebSocket* socket)
	{
		if(!socket) return;
		
		if(result.first)
		{
			socket->send("{\"ok\":true}");
		}
		else if(!result.second.empty())
		{
			std::string msg = fmt::format("{{\"ok\":false,\"error\":\"{}\"}}", result.second);
			socket->send(msg);
		}
		else
		{
			socket->send("{\"ok\":false}");
		}
	}
	
	std::vector<std::string> splitByDelim(const std::string& str, char delim)
	{
		std::vector<std::string> tokens;
		size_t pos = 0;
		size_t len = str.length();
		tokens.reserve(len / 2); // allocate memory for expected number of tokens
	
		while (pos < len)
		{
			size_t end = str.find_first_of(delim, pos);
			if (end == std::string::npos)
			{
				tokens.emplace_back(str.substr(pos));
				break;
			}
			tokens.emplace_back(str.substr(pos, end - pos));
			pos = end + 1;
		}
	
		return tokens;
	}

	std::pair<bool, std::string> add_objects_string::handle(gd::LevelEditorLayer* editor)
	{
		if(this->value.empty()) return {false, "empty value key"};
		
		auto objects = splitByDelim(this->value, ';');
		for(const auto& str : objects)
		{
			if(str.empty()) continue;
			editor->addObjectFromString(str);
		}
		
		return {true, {}};
	}
	
	std::pair<bool, std::string> remove_objects::handle(gd::LevelEditorLayer* editor)
	{
		cocos2d::CCArray* all = editor->getAllObjects();
		int count = all->count();
		short removeGroup = static_cast<short>(std::stoi(this->value));
		cocos2d::CCArray* toDelete = cocos2d::CCArray::create();
		
		for(int i = 0; i < count; i++)
		{
			auto obj = reinterpret_cast<gd::GameObject*>(all->objectAtIndex(i));
			std::vector<short> groups = obj->getGroupIDs();
			if(std::find(groups.begin(), groups.end(), removeGroup) != groups.end())
			{
				toDelete->addObject(obj);
			}
		}
		
		if(toDelete->count() > 0)
		{
			auto ui = editor->m_pEditorUI;
			ui->deselectAll();
			ui->selectObjects(toDelete, false);
			ui->onDeleteSelected(nullptr);
			return {true, {}};
		}
		
		return {false, "no objects to delete"};
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