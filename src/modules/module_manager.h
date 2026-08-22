#pragma once

#include "modules/module.h"

#include <memory>
#include <string>
#include <vector>
#include <shared_mutex>
#include <algorithm>
#include <array>

// include module headers here
#include "modules/movement/Jetpack.h"
#include "modules/visual/Arraylist.h"
#include "modules/combat/Killaura.h"


class ModuleManager {
public:
    static ModuleManager& instance();

    void initialize();
    void shutdown();

    void on_imgui_render();

    std::vector<Module*> modules() const;
    std::vector<std::string> categories() const;
    std::vector<Module*> modules_in_category(const std::string& category) const;

private:
    ModuleManager() = default;

    void add_module(std::unique_ptr<Module> module);
    void rebuild_categories();

    mutable std::shared_mutex m_mutex;
    bool m_initialized = false;
    std::vector<std::unique_ptr<Module>> m_modules;
    std::vector<std::string> m_categories;
};