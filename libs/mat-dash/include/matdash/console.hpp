#pragma once
#include <fstream>
#include <iostream>
#include <consoleapi.h>
#include <stdio.h>

namespace matdash {
	struct Console {
		std::ofstream out, in;
		Console() {
			AllocConsole();
			out = decltype(out)("CONOUT$", std::ios::out);
			in = decltype(in)("CONIN$", std::ios::in);
			std::cout.rdbuf(out.rdbuf());
			std::cin.rdbuf(in.rdbuf());
			
			FILE* dummy;
			freopen_s(&dummy, "CONOUT$", "w", stdout);
		}
		~Console() {
			out.close();
			in.close();
		}
	};

	inline void create_console() {
		static Console console;
	}
}
