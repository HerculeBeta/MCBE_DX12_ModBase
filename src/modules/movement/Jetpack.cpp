#include "modules/movement/jetpack.h"

Jetpack::Jetpack() : Module("Movement", "Jetpack", "Flies in direction you look.") {
    m_speed = add_setting<config::FloatSetting>("Speed", "Speed", 1.5f, 0.1f, 10.0f);
}

void Jetpack::on_enable() {
    m_tickListener = EventBus::instance().subscribe<ActorTickEvent>([this](ActorTickEvent& event) {
        if (!event.localPlayer) return;

        Vec3 direction = math::get_direction(event.localPlayer->getRotation());
        event.localPlayer->setVelocity(direction * m_speed->value());
        });
}

void Jetpack::on_disable() {
    EventBus::instance().unsubscribe(m_tickListener);
}