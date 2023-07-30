

#include <optional>
#include <string>
#include <ixwebsocket/IXWebSocketServer.h>
#include <utility>

#define _snprintf snprintf
#include "json.h"
#undef _snprintf

namespace gd
{
	class LevelEditorLayer;
}

namespace wsle
{
	void handle(const json::jobject& j, ix::WebSocket* socket);
	std::pair<bool, std::string> handleAction(const json::jobject& action);
	
	void sendResult(const std::pair<bool, std::string>&, ix::WebSocket* socket);
	
	struct add_objects_string
	{
		std::string value {};

		std::pair<bool, std::string> handle(gd::LevelEditorLayer*);
	};
	
	struct remove_objects
	{
		std::string value {};
		std::string filter {};
		
		std::pair<bool, std::string> handle(gd::LevelEditorLayer*);
	};
	
	void queueAction(const std::function<void()>& func);
	std::vector<std::string> splitByDelim(const std::string& str, char delim);
}