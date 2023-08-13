#include "json.hpp"

#define WIN32_LEAN_AND_MEAN  
#include <winsock2.h>

#include <matdash.hpp>

#include <matdash/minhook.hpp>
#include <matdash/boilerplate.hpp>
#include <fmt/format.h>
#include <gd.h>
#include <support/zip_support/ZipUtils.h>

#include <ixwebsocket/IXWebSocketServer.h>
#include "wsle.h"

#include <string>
#include <fstream>
#include <streambuf>
#include "base64.h"

using namespace gd;
using namespace cocos2d;

#include <functional>
#include <vector>

using json = nlohmann::json;

std::vector<std::function<void(gd::LevelEditorLayer*)>> workFuncs;
std::mutex workMutex;
ix::WebSocketServer* serverPtr = nullptr;



std::string decompressStr(std::string compressedLvlStr)
{
	if (compressedLvlStr.empty()) return "";

	std::replace(compressedLvlStr.begin(), compressedLvlStr.end(), '_', '/');
	std::replace(compressedLvlStr.begin(), compressedLvlStr.end(), '-', '+');

	std::string decoded = base64_decode(compressedLvlStr);

	unsigned char* data = (unsigned char*)decoded.data();
	unsigned char* a = nullptr;
	ssize_t deflatedLen = ZipUtils::ccInflateMemory(data, decoded.length(), &a);

	std::string levelString((char *)a);

	//robtop changed ccInflateMemory to use new instead of malloc
	delete a;

	return levelString;
}

void runServer()
{
	ix::initNetSystem();
	// Run a server on localhost at a given port.
	// Bound host name, max connections and listen backlog can also be passed in as parameters.
	int port = 1313;
	std::string host("127.0.0.1"); // If you need this server to be accessible on a different machine, use "0.0.0.0"
	ix::WebSocketServer server(port, host);
	serverPtr = &server;

	server.setOnClientMessageCallback([&server](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg)
	{
		// The ConnectionState object contains information about the connection,
		// at this point only the client ip address and the port.

		if (msg->type == ix::WebSocketMessageType::Open)
		{
			webSocket.send("hello there");
			puts("new connection opened");
		}
		else if (msg->type == ix::WebSocketMessageType::Message)
		{
			if(msg->str == "ping") return (void)webSocket.send("pong");
			if(msg->str == "pong") return (void)webSocket.send("ping");

			if(!gd::LevelEditorLayer::get())
			{
				return sendResultData(wsle::ActionResult::USER_NOT_IN_EDITOR, webSocket);
			}
			
			try
			{
				bool gzip = msg->str.starts_with("H4sIAAAAAAAA");
				nlohmann::json result = nlohmann::json::parse(gzip ? decompressStr(msg->str) : msg->str);
				
				wsle::redirect_to_handler(result, webSocket);
			}
			catch(std::exception& e)
			{
				std::cout << "exception: " << e.what() << '\n';
				return sendResultData(wsle::ActionResult::UNKNOWN_EXCEPTION, webSocket);
			}
			catch(...)
			{
				std::cout << "something very bad happened\n";
				return sendResultData(wsle::ActionResult::UNKNOWN_ERROR, webSocket);
			}
			

		}
		else if(msg->type == ix::WebSocketMessageType::Close)
		{
		}
	});

	auto res = server.listen();
	if (!res.first)
	{
		std::cout << res.second << '\n';
		return;
	}
	
	std::cout << "Server started correctly\n";
	// Per message deflate connection is enabled by default. It can be disabled
	// which might be helpful when running on low power devices such as a Rasbery Pi
	server.disablePerMessageDeflate();

	// Run the server in the background. Server can be stoped by calling server.stop()
	server.start();

	// Block until server.stop() is called.
	server.wait();

	ix::uninitNetSystem();
	serverPtr = nullptr;
	fmt::println("Server stopped correctly");
}

struct MenuLayerMod : public MenuLayer
{
	bool init_()
	{
		if (!matdash::orig<&MenuLayerMod::init_>(this)) return false;
		
		std::thread([]{ runServer(); }).detach();

		return true;
	}
};

void (__thiscall* LevelEditorLayer_updateO)(void*, float);
void __fastcall LevelEditorLayer_updateH(LevelEditorLayer* self, void* edx, float dt)
{
	workMutex.lock();
	if(!workFuncs.empty())
	{
		for(const auto& f : workFuncs)
		{
			puts("calling action function");
			f(self);
		}
		workFuncs.clear();
	}
	workMutex.unlock();
	
	LevelEditorLayer_updateO(self, dt);
	
}

void wsle::queueAction(const std::function<void(gd::LevelEditorLayer*)>& func)
{
	workMutex.lock();
	workFuncs.emplace_back(func);
	workMutex.unlock();
}

void wsle::splitCallback(const std::string& str, char delim, std::function<void(const std::string& s)> callback) {
    size_t pos = 0;
    size_t len = str.length();

    while (pos < len) {
        size_t end = str.find_first_of(delim, pos);
        if (end == std::string::npos) {
            callback(str.substr(pos));
            break;
        }
        callback(str.substr(pos, end - pos));
        pos = end + 1;
    }
}

std::vector<std::string> wsle::splitByDelim(const std::string& str, char delim)
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
	
void startServer()
{
	if(!serverPtr) std::thread([]{runServer();}).detach();
}

void stopServer()
{
	if(serverPtr) serverPtr->stop();
}

void LevelEditorLayer_init(void* self, void* level)
{
	startServer();
	matdash::orig<&LevelEditorLayer_init>(self, level);
}

void LevelEditorLayer_dtor(void* self)
{
	stopServer();
	matdash::orig<&LevelEditorLayer_dtor>(self);
}


void mod_main(HMODULE) {

	// if(AllocConsole()) {
	// 	freopen("CONOUT$", "wt", stdout);
	// 	freopen("CONIN$", "rt", stdin);
	// 	freopen("CONOUT$", "w", stderr);
	// 	std::ios::sync_with_stdio(1);
	// }

	matdash::add_hook<&LevelEditorLayer_dtor>(base + 0x15e8d0);
	matdash::add_hook<&LevelEditorLayer_init>(base + 0x15EE00);
	
	MH_CreateHook(
		reinterpret_cast<void*>(gd::base + 0x1632b0),
		reinterpret_cast<void*>(&LevelEditorLayer_updateH),
		reinterpret_cast<void**>(&LevelEditorLayer_updateO)
	);
	MH_EnableHook(reinterpret_cast<void*>(gd::base + 0x1632b0));

}