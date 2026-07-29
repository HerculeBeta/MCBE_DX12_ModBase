#include "modules/module_manager.h"



#include <algorithm>
#include <array>
#include <memory>

ModuleManager& ModuleManager::instance() {
    static ModuleManager manager;
    return manager;
}

void ModuleManager::initialize() {
    if (m_initialized) {
        return;
    }

    rebuild_categories();
    m_initialized = true;
}

void ModuleManager::shutdown() {
    m_categories.clear();
    m_modules.clear();
    m_initialized = false;
}

void ModuleManager::on_imgui_render() {
    for (const auto& module : m_modules) {
        if (module && module->enabled()) {
            module->on_imgui_render();
        }
    }
}

void ModuleManager::on_normal_tick(const NormalTickContext& context) {
    for (const auto& module : m_modules) {
        if (module && module->enabled()) {
            module->on_normal_tick(context);
        }
    }
}

const std::vector<std::unique_ptr<Module>>& ModuleManager::modules() const {
    return m_modules;
}

const std::vector<std::string>& ModuleManager::categories() const {
    return m_categories;
}

std::vector<Module*> ModuleManager::modules_in_category(const std::string& category) const {
    std::vector<Module*> result;

    for (const auto& module : m_modules) {
        if (module && module->category() == category) {
            result.push_back(module.get());
        }
    }

    return result;
}

void ModuleManager::add_module(std::unique_ptr<Module> module) {
    m_modules.push_back(std::move(module));
}

void ModuleManager::rebuild_categories() {
    m_categories.clear();

    constexpr std::array<const char*, 4> defaultCategories{
        "Combat",
        "Movement",
        "Visual",
        "Player"
    };

    for (const char* category : defaultCategories) {
        m_categories.emplace_back(category);
    }

    for (const auto& module : m_modules) {
        if (!module) {
            continue;
        }

        const std::string& category = module->category();
        if (std::find(m_categories.begin(), m_categories.end(), category) == m_categories.end()) {
            m_categories.push_back(category);
        }
    }
}
