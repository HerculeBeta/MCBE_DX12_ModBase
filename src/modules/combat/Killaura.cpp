#include "modules/combat/Killaura.h"
#include "minecraft/sdk/actor/local_player.h"

Killaura::Killaura()
    : Module("Combat", "Killaura", "Automatically attacks nearby entities.") {
    m_range = add_setting<config::FloatSetting>("Range", "Maximum attack reach in blocks.", 3.8f, 1.0f, 8.0f);
    m_delay = add_setting<config::IntSetting>("Attack Delay", "Game ticks between attacks.", 2, 0, 20);
    m_targetPlayers = add_setting<config::BoolSetting>("Target Players", "Attack player entities.", true);
    m_targetMobs = add_setting<config::BoolSetting>("Target Mobs", "Attack non-player mobs.", false);
}

void Killaura::on_enable() {
    m_tickCounter = 0;
    m_tickListener = EventBus::instance().subscribe<ActorTickEvent>([this](ActorTickEvent& event) {
        if (!event.localPlayer) return;

        if (++m_tickCounter < m_delay->value()) return;

        GameMode* gameMode = event.localPlayer->getGameMode();
        if (!gameMode) return;

        const Vec3 localPos = event.localPlayer->getPosition();
        Actor* bestTarget = nullptr;
        float bestDistance = m_range->value();

        for (Actor* target : Actor::get_all(event.localPlayer)) {
            if (target == event.localPlayer) continue;

            const bool isPlayer = target->isPlayer();
            if (isPlayer && !m_targetPlayers->value()) continue;
            if (!isPlayer && !m_targetMobs->value()) continue;

            const float dist = localPos.distance(target->getPosition());
            if (dist < bestDistance) {
                bestDistance = dist;
                bestTarget = target;
            }
        }

        if (bestTarget) {
            gameMode->attack(bestTarget);
            m_tickCounter = 0;
        }
    });
}

void Killaura::on_disable() {
    EventBus::instance().unsubscribe(m_tickListener);
}