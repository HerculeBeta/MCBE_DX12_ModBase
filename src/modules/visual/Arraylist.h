#pragma once

#include "modules/module.h"
#include <string>
#include <vector>

class ArrayList final : public Module {
public:
    ArrayList();

    void on_imgui_render() override;

private:
    struct Element {
        std::string text;
        float width = 0.0f;
    };

    enum Position {
        TopRight = 0,
        TopLeft,
        BottomRight,
        BottomLeft
    };

    config::EnumSetting* m_position = nullptr;
    config::BoolSetting* m_showSideBar = nullptr;
    config::FloatSetting* m_bgOpacity = nullptr;
    config::BoolSetting* m_hideSelf = nullptr;
};