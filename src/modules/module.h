#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include "config/setting.h"

class Module {
public:
    Module(std::string category, std::string name, std::string tooltip);
    virtual ~Module() = default;

    const std::string& category() const;
    const std::string& name() const;
    const std::string& tooltip() const;

    bool enabled() const;
    int keybind() const;
    bool hold_to_enable() const;
    void set_enabled(bool enabled);
    void set_keybind(int keybind);
    void set_hold_to_enable(bool holdToEnable);
    void toggle();

    virtual void on_enable();
    virtual void on_disable();
    virtual void on_imgui_render();
    virtual void on_settings_render();
    virtual bool has_settings() const;

    template <typename T, typename... Args>
    T* add_setting(Args&&... args) {
        auto setting = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = setting.get();
        m_settings.push_back(std::move(setting));
        return ptr;
    }

    const std::vector<std::unique_ptr<config::Setting>>& settings() const { return m_settings; }

private:
    std::string m_category;
    std::string m_name;
    std::string m_tooltip;
    std::atomic<bool> m_enabled{ false };
    std::atomic<int> m_keybind{ 0 };
    std::atomic<bool> m_holdToEnable{ false };

    std::vector<std::unique_ptr<config::Setting>> m_settings;
};