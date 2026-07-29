#include "modules/module.h"

#include <utility>

Module::Module(std::string category, std::string name, std::string tooltip)
    : m_category(std::move(category)),
      m_name(std::move(name)),
      m_tooltip(std::move(tooltip)) {
}

const std::string& Module::category() const {
    return m_category;
}

const std::string& Module::name() const {
    return m_name;
}

const std::string& Module::tooltip() const {
    return m_tooltip;
}

bool Module::enabled() const {
    return m_enabled;
}

int Module::keybind() const {
    return m_keybind;
}

bool Module::hold_to_enable() const {
    return m_holdToEnable;
}

void Module::set_enabled(bool enabled) {
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;
    if (m_enabled) {
        on_enable();
    } else {
        on_disable();
    }
}

void Module::set_keybind(int keybind) {
    m_keybind = keybind;
}

void Module::set_hold_to_enable(bool holdToEnable) {
    m_holdToEnable = holdToEnable;
}

void Module::toggle() {
    set_enabled(!m_enabled);
}

void Module::on_enable() {
}

void Module::on_disable() {
}

void Module::on_imgui_render() {
}

void Module::on_settings_render() {
}

void Module::on_normal_tick(const NormalTickContext&) {

}

bool Module::has_settings() const {
    return false;
}
