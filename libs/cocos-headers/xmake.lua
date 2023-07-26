target("cocos-headers") --create target cocos headers
	set_kind("headeronly") --set header only because it doesnt build any files
	add_includedirs(
		"cocos2dx",
		"cocos2dx/include",
		"cocos2dx/kazmath/include",
		"cocos2dx/platform/win32",
		"cocos2dx/platform/third_party/win32",
		"cocos2dx/platform/third_party/win32/OGLES",
		"extensions",
		{public = true} --by setting public it applies to any target that adds this target as a dependency
	)
	add_linkdirs("cocos2dx", "extensions", {public = true})
	add_links("libcocos2d", "libExtensions", {public = true})
target_end() --optional