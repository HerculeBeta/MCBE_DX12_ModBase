#include "modules/movement/Fly.h"
#include "input/input_manager.h"

Fly::Fly() : Module("Movement", "Fly", "Allows creative-style flight.") {
    m_speed = add_setting<config::FloatSetting>("Speed", "Horizontal speed", 1.5f, 0.1f, 10.0f);
    m_verticalSpeed = add_setting<config::FloatSetting>("Vertical Speed", "Ascent/descent speed", 0.8f, 0.1f, 5.0f);
    m_glide = add_setting<config::FloatSetting>("Glide", "Downward pull", 0.0f, 0.0f, 0.5f);
}

void Fly::on_enable() {
    m_tickListener = EventBus::instance().subscribe<ActorTickEvent>([this](ActorTickEvent& event) {
        if (!event.localPlayer) return;

        Vec3 forward, right;
        math::get_horizontal_vectors(event.localPlayer->getRotation().yaw, forward, right);

        Vec3 move{};
        if (InputManager::instance().is_key_down('W')) move += forward;
        if (InputManager::instance().is_key_down('S')) move -= forward;
        if (InputManager::instance().is_key_down('D')) move += right;
        if (InputManager::instance().is_key_down('A')) move -= right;

        Vec3 velocity = (move.length_sq() > 0.001f) ? move.normalized() * m_speed->value() : Vec3{};

        if (InputManager::instance().is_key_down(VK_SPACE)) {
            velocity.y = m_verticalSpeed->value();
        } else if (InputManager::instance().is_key_down(VK_SHIFT)) {
            velocity.y = -m_verticalSpeed->value();
        } else {
            velocity.y = -m_glide->value();
        }

        event.localPlayer->setVelocity(velocity);
    });
}

void Fly::on_disable() {
    EventBus::instance().unsubscribe(m_tickListener);
}
