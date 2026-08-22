#pragma once

#include "modules/module.h"
#include <events/event_bus.h>
#include <events/actor_tick_event.h>
#include "minecraft/sdk/actor/local_player.h"

class Jetpack final : public Module {
public:
    Jetpack();

    void on_enable() override;
    void on_disable() override;

private:
    EventBus::ListenerId m_tickListener = 0;

    config::FloatSetting* m_speed = nullptr;
};