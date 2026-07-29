#pragma once

namespace minecraft {
bool start_client_instance_hook();
void stop_client_instance_hook();

void* client_instance();
void* local_player();
}
