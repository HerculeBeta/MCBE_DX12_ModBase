#include "input/module_keybinds.h"

#include "modules/module.h"
#include "modules/module_manager.h"

#include <Windows.h>

namespace {
Module* g_bindingModule = nullptr;

bool is_reserved_bind_key(int key) {
    return key == VK_INSERT
        || key == VK_END
        || key == VK_LBUTTON
        || key == VK_RBUTTON;
}
}

namespace input {
Module* binding_module() {
    return g_bindingModule;
}

bool is_binding() {
    return g_bindingModule != nullptr;
}

void begin_binding(Module* module) {
    g_bindingModule = module;
}

void cancel_binding() {
    g_bindingModule = nullptr;
}

bool handle_bind_key(int key) {
    if (!g_bindingModule) {
        return false;
    }

    if (key == VK_ESCAPE) {
        g_bindingModule = nullptr;
        return true;
    }

    if (key == VK_DELETE || key == VK_BACK) {
        g_bindingModule->set_keybind(0);
        g_bindingModule = nullptr;
        return true;
    }

    if (!is_reserved_bind_key(key)) {
        g_bindingModule->set_keybind(key);
        g_bindingModule = nullptr;
        return true;
    }

    return true;
}

bool handle_module_key(int key, bool down) {
    if (key == 0) {
        return false;
    }

    bool handled = false;

    for (const auto& module : ModuleManager::instance().modules()) {
        if (!module || module->keybind() != key) {
            continue;
        }

        handled = true;
        if (module->hold_to_enable()) {
            module->set_enabled(down);
        } else if (down) {
            module->toggle();
        }
    }

    return handled;
}
}
