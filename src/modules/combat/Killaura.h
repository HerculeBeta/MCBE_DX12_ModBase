#pragma once

#include "modules/module.h"
#include "events/event_bus.h"
#include "events/actor_tick_event.h"

class Killaura final : public Module {
public:
    Killaura();

    void on_enable() override;
    void on_disable() override;

private:
    EventBus::ListenerId m_tickListener = 0;
    int m_tickCounter = 0;

    config::FloatSetting* m_range = nullptr;
    config::IntSetting* m_delay = nullptr;
    config::BoolSetting* m_targetMobs = nullptr;
    config::BoolSetting* m_targetPlayers = nullptr;
};