#include "input/key_names.h"

#include <Windows.h>
#include <cctype>

namespace input {
const char* key_name(int key) {
    if (key == 0) {
        return "None";
    }

    struct KeyName {
        int key;
        const char* name;
    };

    static constexpr KeyName kNames[] = {
        {VK_LBUTTON, "M1"}, {VK_RBUTTON, "M2"}, {VK_MBUTTON, "M3"},
        {VK_XBUTTON1, "M4"}, {VK_XBUTTON2, "M5"},
        {VK_BACK, "Back"}, {VK_TAB, "Tab"}, {VK_RETURN, "Enter"},
        {VK_SHIFT, "Shift"}, {VK_LSHIFT, "LShift"}, {VK_RSHIFT, "RShift"},
        {VK_CONTROL, "Ctrl"}, {VK_LCONTROL, "LCtrl"}, {VK_RCONTROL, "RCtrl"},
        {VK_MENU, "Alt"}, {VK_LMENU, "LAlt"}, {VK_RMENU, "RAlt"},
        {VK_PAUSE, "Pause"}, {VK_CAPITAL, "Caps"}, {VK_ESCAPE, "Esc"},
        {VK_SPACE, "Space"}, {VK_PRIOR, "PgUp"}, {VK_NEXT, "PgDn"},
        {VK_END, "End"}, {VK_HOME, "Home"}, {VK_LEFT, "Left"},
        {VK_UP, "Up"}, {VK_RIGHT, "Right"}, {VK_DOWN, "Down"},
        {VK_INSERT, "Ins"}, {VK_DELETE, "Del"},
        {VK_NUMPAD0, "N0"}, {VK_NUMPAD1, "N1"}, {VK_NUMPAD2, "N2"},
        {VK_NUMPAD3, "N3"}, {VK_NUMPAD4, "N4"}, {VK_NUMPAD5, "N5"},
        {VK_NUMPAD6, "N6"}, {VK_NUMPAD7, "N7"}, {VK_NUMPAD8, "N8"},
        {VK_NUMPAD9, "N9"},
        {VK_F1, "F1"}, {VK_F2, "F2"}, {VK_F3, "F3"}, {VK_F4, "F4"},
        {VK_F5, "F5"}, {VK_F6, "F6"}, {VK_F7, "F7"}, {VK_F8, "F8"},
        {VK_F9, "F9"}, {VK_F10, "F10"}, {VK_F11, "F11"}, {VK_F12, "F12"},
        {VK_OEM_1, ";"}, {VK_OEM_PLUS, "="}, {VK_OEM_COMMA, ","},
        {VK_OEM_MINUS, "-"}, {VK_OEM_PERIOD, "."}, {VK_OEM_2, "/"},
        {VK_OEM_3, "`"}, {VK_OEM_4, "["}, {VK_OEM_5, "\\"},
        {VK_OEM_6, "]"}, {VK_OEM_7, "'"}
    };

    for (const KeyName& item : kNames) {
        if (item.key == key) {
            return item.name;
        }
    }

    static char text[2] = {};
    if (key > 0 && key < 256 && std::isalnum(key)) {
        text[0] = static_cast<char>(key);
        text[1] = '\0';
        return text;
    }

    return "?";
}
}
