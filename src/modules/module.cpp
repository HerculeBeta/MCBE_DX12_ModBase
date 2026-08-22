#include "modules/module.h"
#include <utility>
#include <utils/logger.h>

Module::Module(std::string category, std::string name, std::string tooltip)
    : m_category(std::move(category)),
    m_name(std::move(name)),
    m_tooltip(std::move(tooltip)) {
}

const std::string& Module::category() const { return m_category; }
const std::string& Module::name() const { return m_name; }
const std::string& Module::tooltip() const { return m_tooltip; }

bool Module::enabled() const { return m_enabled.load(std::memory_order_acquire); }
int Module::keybind() const { return m_keybind.load(std::memory_order_relaxed); }
bool Module::hold_to_enable() const { return m_holdToEnable.load(std::memory_order_relaxed); }

void Module::set_enabled(bool enabled) {
    bool expected = !enabled;
    if (!m_enabled.compare_exchange_strong(expected, enabled, std::memory_order_acq_rel)) {
        return;
    }

    if (enabled) {
        LOG_INFO("Module §e{}§r -> §aENABLED§r", m_name);
        on_enable();
    }
    else {
        LOG_INFO("Module §e{}§r -> §cDISABLED§r", m_name);
        on_disable();
    }
}

void Module::set_keybind(int keybind) {
    m_keybind.store(keybind, std::memory_order_relaxed);
}

void Module::set_hold_to_enable(bool holdToEnable) {
    m_holdToEnable.store(holdToEnable, std::memory_order_relaxed);
}

void Module::toggle() {
    set_enabled(!enabled());
}

void Module::on_settings_render() {
    for (const auto& setting : m_settings) {
        if (setting && setting->is_visible()) {
            setting->render_ui();
        }
    }
}

bool Module::has_settings() const {
    return !m_settings.empty();
}

void Module::on_enable() {}
void Module::on_disable() {}
void Module::on_imgui_render() {}