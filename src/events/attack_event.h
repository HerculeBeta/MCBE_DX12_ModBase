#pragma once

#include "events/event.h"

class GameMode;
class LocalPlayer;
class Actor;

struct AttackEvent : public Event {
    GameMode* gameMode = nullptr;
    LocalPlayer* localPlayer = nullptr;
    Actor* target = nullptr;

    AttackEvent(GameMode* gm, LocalPlayer* player, Actor* targetActor)
        : gameMode(gm), localPlayer(player), target(targetActor) {
    }
};