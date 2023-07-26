#include <Windows.h>

void mod_main(HMODULE);

namespace {
	DWORD WINAPI mat_dash_thread_function__(void* module) {
		mod_main(reinterpret_cast<HMODULE>(module));
		return 0;
	}
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
	if (reason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(module);
		if (auto handle = CreateThread(0, 0, mat_dash_thread_function__, module, 0, 0)) {
			CloseHandle(handle);
		} else {
			return FALSE;
		}
	}
	return TRUE;
}