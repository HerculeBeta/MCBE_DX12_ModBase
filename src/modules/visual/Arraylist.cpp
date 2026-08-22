#include "modules/visual/Arraylist.h"
#include "modules/module_manager.h"
#include "ui/widgets/widgets.h"

#include <imgui.h>
#include <algorithm>

ArrayList::ArrayList() : Module("Visual", "ArrayList", "Displays enabled modules on screen.") {
    m_position = add_setting<config::EnumSetting>(
        "Position",
        "Screen corner placement",
        0,
        std::vector<std::string>{"Top-Right", "Top-Left", "Bottom-Right", "Bottom-Left"}
    );

    m_showSideBar = add_setting<config::BoolSetting>(
        "Bar Indicator",
        "Draws an accent color indicator bar on the side of entries.",
        true
    );

    m_bgOpacity = add_setting<config::FloatSetting>(
        "Background Opacity",
        "Opacity of entry background boxes.",
        0.65f, 0.0f, 1.0f
    );

    m_hideSelf = add_setting<config::BoolSetting>(
        "Hide Self",
        "Hides the ArrayList module itself from the list.",
        true
    );

    set_enabled(true);
}

void ArrayList::on_imgui_render() {
    std::vector<Element> activeElements;
    const auto modules = ModuleManager::instance().modules();

    for (Module* mod : modules) {
        if (!mod || !mod->enabled()) continue;
        if (m_hideSelf->value() && mod == this) continue;

        Element elem;
        elem.text = mod->name();
        elem.width = ImGui::CalcTextSize(elem.text.c_str()).x;
        activeElements.push_back(std::move(elem));
    }

    if (activeElements.empty()) return;

    const int posMode = m_position->value();
    const bool isRightAligned = (posMode == TopRight || posMode == BottomRight);
    const bool isBottomAligned = (posMode == BottomRight || posMode == BottomLeft);

    std::sort(activeElements.begin(), activeElements.end(), [isRightAligned](const Element& a, const Element& b) {
        return isRightAligned ? (a.width > b.width) : (a.width > b.width);
        });

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);

    if (!ImGui::Begin("##ArrayListOverlay", nullptr, windowFlags)) {
        ImGui::End();
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ui::Palette& p = ui::get_palette();

    const float paddingX = 8.0f;
    const float paddingY = 4.0f;
    const float textHeight = ImGui::GetTextLineHeight();
    const float itemHeight = textHeight + (paddingY * 2.0f);
    const float barThickness = 3.0f;
    const float margin = 12.0f;

    float currentY = isBottomAligned
        ? (viewport->WorkSize.y - margin - itemHeight)
        : (viewport->WorkPos.y + margin);

    const ImU32 bgBaseColor = p.bg_dark;
    const float alpha = m_bgOpacity->value();
    const ImU32 bgFinalColor = (bgBaseColor & 0x00FFFFFF) | (static_cast<ImU32>(alpha * 255.0f) << 24);

    for (const auto& elem : activeElements) {
        const float boxWidth = elem.width + (paddingX * 2.0f) + (m_showSideBar->value() ? barThickness : 0.0f);

        float currentX = isRightAligned
            ? (viewport->WorkSize.x - margin - boxWidth)
            : (viewport->WorkPos.x + margin);

        ImVec2 rectMin(currentX, currentY);
        ImVec2 rectMax(currentX + boxWidth, currentY + itemHeight);

        drawList->AddRectFilled(rectMin, rectMax, bgFinalColor, 3.0f);

        float textX = currentX + paddingX;
        if (!isRightAligned && m_showSideBar->value()) {
            textX += barThickness;
        }

        ImVec2 textPos(textX, currentY + paddingY);

        drawList->AddText(textPos, p.text_primary, elem.text.c_str());

        if (m_showSideBar->value()) {
            ImVec2 barMin, barMax;
            if (isRightAligned) {
                barMin = ImVec2(rectMax.x - barThickness, rectMin.y);
                barMax = rectMax;
            }
            else {
                barMin = rectMin;
                barMax = ImVec2(rectMin.x + barThickness, rectMax.y);
            }
            drawList->AddRectFilled(barMin, barMax, p.accent, 2.0f);
        }

        if (isBottomAligned) {
            currentY -= (itemHeight + 2.0f);
        }
        else {
            currentY += (itemHeight + 2.0f);
        }
    }

    ImGui::End();
}