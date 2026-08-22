#pragma once

#include "events/event.h"

enum class KeyAction {
    Press,
    Release,
    Repeat
};

struct KeyEvent : public Event {
    int key = 0;
    KeyAction action = KeyAction::Press;
    bool isMouse = false;

    KeyEvent(int k, KeyAction act, bool mouse = false)
        : key(k), action(act), isMouse(mouse) {
    }
};