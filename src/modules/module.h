#pragma once

#include <string>
#include <string_view>
#include "events/normal_tick_context.h"

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
    virtual void on_normal_tick(const NormalTickContext& context);

private:
    std::string m_category;
    std::string m_name;
    std::string m_tooltip;
    bool m_enabled = false;
    int m_keybind = 0;
    bool m_holdToEnable = false;
};
