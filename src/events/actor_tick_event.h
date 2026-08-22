#pragma once

#include "events/event.h"

class LocalPlayer;

struct ActorTickEvent : public Event {
    void* clientInstance = nullptr;
    LocalPlayer* localPlayer = nullptr;

    ActorTickEvent(void* instance, LocalPlayer* player)
        : clientInstance(instance), localPlayer(player) {
    }
};