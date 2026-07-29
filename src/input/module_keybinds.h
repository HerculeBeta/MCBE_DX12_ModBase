#pragma once

class Module;

namespace input {
Module* binding_module();
bool is_binding();
void begin_binding(Module* module);
void cancel_binding();
bool handle_bind_key(int key);
bool handle_module_key(int key, bool down);
}
