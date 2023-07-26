target("gd.h")
	set_kind("headeronly")
	add_deps("cocos-headers")
	add_includedirs("include", ".", {public = true}) -- "." = current dir
target_end()