#include <matdash.hpp>

// defines add_hook to use minhook
#include <matdash/minhook.hpp>

// lets you use mod_main
#include <matdash/boilerplate.hpp>

#include <fmt/format.h>
#include <gd.h>

using namespace gd;
using namespace cocos2d;

#define USE_WIN32_CONSOLE

class MenuLayerMod : public MenuLayer {
public:
	// here the name cant be `init` as that would make it a virtual
	// which doesnt work with the current code
	bool init_() {
		if (!matdash::orig<&MenuLayerMod::init_>(this)) return false;
		
		std::string str = fmt::format("Hello from {}", "xmake");
		auto label = CCLabelBMFont::create(str.c_str(), "bigFont.fnt");
		label->setPosition(ccp(200, 200));
		addChild(label);

		return true;
	}
};

void MenuLayer_onNewgrounds(MenuLayer* self, CCObject* sender) {
	//create and enter empty level
	auto level = GJGameLevel::create();
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, PlayLayer::scene(level)));
}

bool GJDropDownLayer_init(GJDropDownLayer* self, const char* title, float height) {
	return matdash::orig<&GJDropDownLayer_init>(self, "my own title", height);
}

//static cocos function
matdash::cc::c_decl<CCLabelBMFont*> CCLabelBMFont_create(const char* text, const char* fontfile) {
	fmt::print("creating label with text: {}\n", text);
    return matdash::orig<&CCLabelBMFont_create>(text, fontfile);
}

//speed hack
void PlayLayer_update_(PlayLayer* self, float dt) {
	// another way of specifying the calling convention
	matdash::orig<&PlayLayer_update_, matdash::Thiscall>(self, dt * 0.5f);
}

void mod_main(HMODULE) {
	
	#ifdef USE_WIN32_CONSOLE
		if(AllocConsole()) {
			freopen("CONOUT$", "wt", stdout);
			freopen("CONIN$", "rt", stdin);
			freopen("CONOUT$", "w", stderr);
			std::ios::sync_with_stdio(1);
		}
	#endif

	matdash::add_hook<&MenuLayerMod::init_>(base + 0x1907b0);
	matdash::add_hook<&MenuLayer_onNewgrounds>(base + 0x191e90);
	matdash::add_hook<&GJDropDownLayer_init>(base + 0x113530);
	
	// another way of specifying the calling convention
	matdash::add_hook<&PlayLayer_update_, matdash::Thiscall>(base + 0x2029c0);
	
	//hook cocos function
	static auto cocos_addr = [](const char* symbol) -> auto {
		static const auto module = GetModuleHandleA("libcocos2d.dll");
		return GetProcAddress(module, symbol);
	};
	
	matdash::add_hook<&CCLabelBMFont_create>(cocos_addr("?create@CCLabelBMFont@cocos2d@@SAPAV12@PBD0@Z"));
}