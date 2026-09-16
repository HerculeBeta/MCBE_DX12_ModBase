#pragma once

#include "events/event.h"

class ClientInstance;
class LocalPlayer;

struct ActorTickEvent : public Event {
    ClientInstance* clientInstance = nullptr;
    LocalPlayer* localPlayer = nullptr;

    ActorTickEvent(ClientInstance* instance, LocalPlayer* player)
        : clientInstance(instance), localPlayer(player) {
    }
};