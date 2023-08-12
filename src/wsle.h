
#include "json.hpp"

#include <optional>
#include <string>
#include <ixwebsocket/IXWebSocketServer.h>
#include <utility>
#include <vector>
#include <set>
#include <source_location>

using json = nlohmann::json;

namespace gd
{
	class LevelEditorLayer;
}

namespace wsle
{
	enum class ActionType : int
	{
		ADD_OBJECTS_STRING = 0,
		REMOVE_OBJECTS_GROUP
	};

	enum class ActionResult : int 
	{
		OK = 0,
		INVALID_JSON,
		INVALID_TYPE,
		UNKNOWN_JSON_ERROR,
		EMPTY_LEVEL,
		USER_NOT_IN_EDITOR,
		UNKNOWN_EXCEPTION,
		UNKNOWN_ERROR,
	};
	
	void redirect_to_handler(const nlohmann::json&, ix::WebSocket& socket);
	
	namespace ADD_OBJECTS_STRING
	{ 
		void handle(const json&, ix::WebSocket&);
	}
	namespace REMOVE_OBJECTS_GROUP
	{ 
		void handle(const json&, ix::WebSocket&);
		std::set<short> getGroupsFromJson(const json&);
	}
	
	
	//utils
	void queueAction(const std::function<void(gd::LevelEditorLayer*)>&);
	std::vector<std::string> splitByDelim(const std::string&, char);
	void splitCallback(const std::string& str, char delim, std::function<void(const std::string& s)>);
	
	
	void sendResultData(ActionResult result, ix::WebSocket&, std::source_location = std::source_location::current());
}