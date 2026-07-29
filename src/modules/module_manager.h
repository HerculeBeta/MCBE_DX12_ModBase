#pragma once

#include "modules/module.h"

#include <memory>
#include <string>
#include <vector>

class ModuleManager {
public:
    static ModuleManager& instance();

    void initialize();
    void shutdown();

    void on_imgui_render();
    void on_normal_tick(const NormalTickContext& context);

    const std::vector<std::unique_ptr<Module>>& modules() const;
    const std::vector<std::string>& categories() const;
    std::vector<Module*> modules_in_category(const std::string& category) const;

private:
    ModuleManager() = default;

    void add_module(std::unique_ptr<Module> module);
    void rebuild_categories();

    bool m_initialized = false;
    std::vector<std::unique_ptr<Module>> m_modules;
    std::vector<std::string> m_categories;
};
