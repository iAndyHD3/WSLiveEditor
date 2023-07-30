add_rules("mode.release")

--equivalent to
--if is_mode("release") then
--	set_symbols("hidden")
--	set_optimize("fastest")
--	set_strip("all")
--end

add_rules("mode.releasedbg")

--equivalent to
--if is_mode("releasedbg") then
--	set_symbols("debug")
--	set_optimize("fastest")
--	set_strip("all")
--end

set_allowedarchs("windows|x86") --allow only windows x86
set_defaultarchs("windows|x86") --set default windows x86
set_allowedmodes("release") --allow only Release and RelWithDebInfo
set_languages("c++20") --set c++20


--create options with loop and array
options_table = {"cppath", "gdpath", "gdexec"}

for i = 1, #options_table do
	option(options_table[i])
		set_default("")
		set_showmenu(true)
	option_end()
end

add_requires("fmt")

includes("**/xmake.lua") -- recursively add files through pattern matching

target("WSLiveEditor") --dll name and target name
	set_default(true)
	set_kind("shared")
	add_files("src/*.cpp")
	add_packages("fmt")
	add_deps("cocos-headers", "mat-dash", "gd.h", "ixwebsocket")
	add_syslinks("ws2_32")
	
	on_load(function(target)
		target:add("defines", "TARGET_NAME=\"" .. target:basename() .. "\"")
	end)
	
	
	--add minhook manually here since it doesnt seem to work when added with target
	add_includedirs("libs/minhook/include")
	add_files("libs/minhook/src/**.c")
	
	--this will run when xmake run gdmod is called
	on_run(function (target)
		local function isempty(s) return s == nil or s == '' or s == "" end
		local function notempty(s) return isempty(s) == false end
		
		--print("cppath: $(cppath) | gdpath: $(gdpath) | gdexec: $(gdexec)")
		
		--copy dll
		if notempty("$(cppath)") then
			if os.exists("$(cppath)") then
				cprint(string.format("${green}%s${clear} -> ${green}%s${clear}", target:targetfile(), "$(cppath)"))
				os.cp(target:targetfile(), "$(cppath)")
			else
				cprint("${red}error:${clear} the copy path (cppath) does not exist, set the paths in cfg.txt and run `xmake f --import=cfg.txt`")
				os.exit()
			end
		else
			cprint("${red}error:${clear} the copy path (cppath) is not set, set the paths in cfg.txt and run `xmake f --import=cfg.txt`")
			os.exit()
		end
		
		--run game
		if notempty("$(gdexec") then
			if notempty("$(gdpath)") then
				if os.isfile("$(gdpath)\\$(gdexec)") then
					if os.exists("$(gdpath)") then
						os.cd("$(gdpath)")
						cprint("${green}$(gdpath)\\$(gdexec)${clear} -> CTRL+C or close the game to exit")
						os.run("$(gdexec)")
					else
						cprint("${red}error:${clear} the geometry dash path (gdpath) does not exist, set the paths in cfg.txt and run `xmake f --import=cfg.txt`")
						os.exit()
					end
				else
					cprint("${red}error:${clear} the geometry dash executable path (gdexec) does not exist, set the paths in cfg.txt and run `xmake f --import=cfg.txt`")
					os.exit()
				end
			else
				cprint("${red}error:${clear} the geometry dash path (gdpath) is not set, set the paths in cfg.txt and run `xmake f --import=cfg.txt`")
				os.exit()
			end
		else
			cprint("${red}error:${clear} the geometry dash executable name (gdexec) is not set, set the paths in cfg.txt and run `xmake f --import=cfg.txt`")
			os.exit()
		end
	end)
	
target_end() --target_end() is optional


--xmake website (recommended) https://xmake.io
-- xmake f --import=cfg.txt