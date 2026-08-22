#pragma once

#include <Windows.h>
#include <array>

class Module;

class InputManager {
public:
    static InputManager& instance();

    bool handle_win32_message(UINT msg, WPARAM wparam, LPARAM lparam);

    bool is_key_down(int vkCode) const;

    void begin_binding(Module* module);
    void cancel_binding();
    bool is_binding() const;
    Module* binding_module() const;

private:
    InputManager() = default;

    int extract_mouse_key(UINT msg, WPARAM wparam, bool& isDown);

    std::array<bool, 256> m_keyStates{};
    Module* m_bindingModule = nullptr;
};