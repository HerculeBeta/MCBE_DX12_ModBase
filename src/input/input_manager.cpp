#include "input/input_manager.h"
#include "events/event_bus.h"
#include "events/key_event.h"
#include "modules/module.h"

InputManager& InputManager::instance() {
    static InputManager manager;
    return manager;
}

bool InputManager::is_key_down(int vkCode) const {
    if (vkCode <= 0 || vkCode >= 256) return false;
    return m_keyStates[vkCode];
}

void InputManager::begin_binding(Module* module) {
    m_bindingModule = module;
}

void InputManager::cancel_binding() {
    m_bindingModule = nullptr;
}

bool InputManager::is_binding() const {
    return m_bindingModule != nullptr;
}

Module* InputManager::binding_module() const {
    return m_bindingModule;
}

int InputManager::extract_mouse_key(UINT msg, WPARAM wparam, bool& isDown) {
    switch (msg) {
    case WM_LBUTTONDOWN: isDown = true;  return VK_LBUTTON;
    case WM_LBUTTONUP:   isDown = false; return VK_LBUTTON;
    case WM_RBUTTONDOWN: isDown = true;  return VK_RBUTTON;
    case WM_RBUTTONUP:   isDown = false; return VK_RBUTTON;
    case WM_MBUTTONDOWN: isDown = true;  return VK_MBUTTON;
    case WM_MBUTTONUP:   isDown = false; return VK_MBUTTON;
    case WM_XBUTTONDOWN: isDown = true;  return (HIWORD(wparam) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;
    case WM_XBUTTONUP:   isDown = false; return (HIWORD(wparam) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;
    default: return 0;
    }
}

bool InputManager::handle_win32_message(UINT msg, WPARAM wparam, LPARAM lparam) {
    bool isKeyDown = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
    bool isKeyUp = (msg == WM_KEYUP || msg == WM_SYSKEYUP);
    bool isMouse = false;
    int vkCode = 0;

    if (isKeyDown || isKeyUp) {
        vkCode = static_cast<int>(wparam);
    }
    else {
        vkCode = extract_mouse_key(msg, wparam, isKeyDown);
        if (vkCode != 0) {
            isMouse = true;
            isKeyUp = !isKeyDown;
        }
    }

    if (vkCode <= 0 || vkCode >= 256) {
        return false;
    }

    const bool repeated = isKeyDown && ((lparam & (1 << 30)) != 0);
    const KeyAction action = isKeyUp ? KeyAction::Release : (repeated ? KeyAction::Repeat : KeyAction::Press);

    if (action == KeyAction::Press)   m_keyStates[vkCode] = true;
    if (action == KeyAction::Release) m_keyStates[vkCode] = false;

    if (m_bindingModule && action == KeyAction::Press) {
        if (vkCode == VK_ESCAPE) {
            cancel_binding();
        }
        else if (vkCode == VK_DELETE || vkCode == VK_BACK) {
            m_bindingModule->set_keybind(0);
            cancel_binding();
        }
        else {
            m_bindingModule->set_keybind(vkCode);
            cancel_binding();
        }
        return true; // Consume message during binding
    }

    KeyEvent event(vkCode, action, isMouse);
    EventBus::instance().publish(event);

    return event.is_cancelled();
}