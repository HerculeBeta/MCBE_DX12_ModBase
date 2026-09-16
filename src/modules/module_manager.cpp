#include "modules/module_manager.h"
#include "input/input_manager.h"
#include "events/key_event.h"
#include "events/event_bus.h"
#include "utils/logger.h"

ModuleManager& ModuleManager::instance() {
    static ModuleManager manager;
    return manager;
}

void ModuleManager::initialize() {
    std::unique_lock lock(m_mutex);
    if (m_initialized) {
        return;
    }

    LOG_INFO("Initializing ModuleManager...");

    m_modules.push_back(std::make_unique<Jetpack>());
    m_modules.push_back(std::make_unique<Fly>());
    m_modules.push_back(std::make_unique<ArrayList>());
    m_modules.push_back(std::make_unique<Killaura>());

    rebuild_categories();

    EventBus::instance().subscribe<KeyEvent>([this](KeyEvent& event) {
        if (InputManager::instance().is_binding()) return;

        std::shared_lock lock(m_mutex);
        for (const auto& module : m_modules) {
            if (!module || module->keybind() != event.key) continue;

            if (module->hold_to_enable()) {
                if (event.action == KeyAction::Press)   module->set_enabled(true);
                if (event.action == KeyAction::Release) module->set_enabled(false);
            }
            else {
                if (event.action == KeyAction::Press) {
                    module->toggle();
                }
            }
        }
        });

    LOG_INFO("Registered {} modules across {} categories.", m_modules.size(), m_categories.size());
    m_initialized = true;
}

void ModuleManager::shutdown() {
    std::unique_lock lock(m_mutex);
    LOG_INFO("Shutting down ModuleManager...");
    m_categories.clear();
    m_modules.clear();
    m_initialized = false;
}

void ModuleManager::on_imgui_render() {
    std::shared_lock lock(m_mutex);
    for (const auto& module : m_modules) {
        if (module && module->enabled()) {
            module->on_imgui_render();
        }
    }
}

std::vector<Module*> ModuleManager::modules() const {
    std::shared_lock lock(m_mutex);
    std::vector<Module*> result;
    result.reserve(m_modules.size());
    for (const auto& mod : m_modules) {
        if (mod) result.push_back(mod.get());
    }
    return result;
}

std::vector<std::string> ModuleManager::categories() const {
    std::shared_lock lock(m_mutex);
    return m_categories;
}

std::vector<Module*> ModuleManager::modules_in_category(const std::string& category) const {
    std::shared_lock lock(m_mutex);
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

    constexpr std::array<const char*, 5> defaultCategories{
        "Combat",
        "Movement",
        "Visual",
        "Player",
        "Configs"
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