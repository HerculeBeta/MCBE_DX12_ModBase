#pragma once

#include <Windows.h>

namespace dx12_hook {
bool start(HMODULE module);
void stop();
bool is_menu_open();
void set_menu_open(bool open);
}
