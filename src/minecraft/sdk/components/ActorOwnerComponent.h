#pragma once

#include "minecraft/sdk/entity/i_entity_component.h"

class Actor;

struct ActorOwnerComponent : public IEntityComponent {
    Actor* mActor = nullptr;
};