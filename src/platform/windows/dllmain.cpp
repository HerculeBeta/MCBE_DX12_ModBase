#include "minecraft/hooks/client_instance_hook.h"
#include "modules/module_manager.h"
#include "render/dx12/dx12_hook.h"

#include <Windows.h>

DWORD WINAPI main_thread(LPVOID param) {
    const auto module = static_cast<HMODULE>(param);

    ModuleManager::instance().initialize();
    minecraft::start_client_instance_hook();

    if (!dx12_hook::start(module)) {
        minecraft::stop_client_instance_hook();
        ModuleManager::instance().shutdown();
        FreeLibraryAndExitThread(module, 0);
    }

    while (!(GetAsyncKeyState(VK_END) & 1)) {
        Sleep(50);
    }

    dx12_hook::stop();
    minecraft::stop_client_instance_hook();
    ModuleManager::instance().shutdown();
    FreeLibraryAndExitThread(module, 0);
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);

        HANDLE thread = CreateThread(nullptr, 0, main_thread, module, 0, nullptr);
        if (thread) {
            CloseHandle(thread);
        }
    }

    return TRUE;
}
