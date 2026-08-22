#pragma once

#include "minecraft/sdk/entity/i_entity_component.h"
#include <cstdint>
#include <type_traits>

enum class GameType : std::int32_t {
    Undefined = -1,
    Survival = 0,
    Creative = 1,
    Adventure = 2,
    Default = 5,
    Spectator = 6,
    WorldDefault = 0
};

struct ActorGameTypeComponent : public IEntityComponent {
    GameType mGameType = GameType::Undefined;
};

static_assert(sizeof(ActorGameTypeComponent) == 0x4, "ActorGameTypeComponent size mismatch!");
static_assert(std::is_standard_layout_v<ActorGameTypeComponent>, "ActorGameTypeComponent must be standard layout!");