#pragma once

#include "modules/module.h"

class ESP final : public Module {
public:
    ESP();

    void on_imgui_render() override;

private:
    config::BoolSetting* m_targetPlayers = nullptr;
    config::BoolSetting* m_targetMobs = nullptr;
    config::ColorSetting* m_color = nullptr;
    config::FloatSetting* m_thickness = nullptr;
    config::FloatSetting* m_fillOpacity = nullptr;
};
