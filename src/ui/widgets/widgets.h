#pragma once

#include <imgui.h>
#include <string>
#include <vector>

namespace ui {

    struct Palette {
        ImU32 bg_dark = IM_COL32(13, 15, 20, 245);
        ImU32 panel_bg = IM_COL32(19, 22, 30, 250);
        ImU32 card_bg = IM_COL32(26, 30, 42, 255);
        ImU32 card_hover = IM_COL32(34, 40, 56, 255);
        ImU32 card_enabled = IM_COL32(18, 48, 38, 255);
        ImU32 border = IM_COL32(45, 52, 70, 180);
        ImU32 border_focused = IM_COL32(16, 185, 129, 220);

        ImU32 text_primary = IM_COL32(240, 244, 248, 255);
        ImU32 text_secondary = IM_COL32(150, 160, 178, 255);
        ImU32 text_muted = IM_COL32(100, 110, 128, 255);

        ImU32 accent = IM_COL32(16, 185, 129, 255);   // Emerald Accent
        ImU32 accent_hover = IM_COL32(52, 211, 153, 255);
        ImU32 accent_active = IM_COL32(5, 150, 105, 255);
        ImU32 accent_muted = IM_COL32(16, 185, 129, 60);

        ImU32 danger = IM_COL32(220, 53, 69, 255);
        ImU32 danger_hover = IM_COL32(248, 113, 113, 255);
    };

    const Palette& get_palette();
    void apply_theme();

    // UI Widget Controls
    bool ToggleSwitch(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.2f");
    bool SliderInt(const char* label, int* v, int v_min, int v_max);
    bool Combo(const char* label, int* current_item, const std::vector<std::string>& items);
    bool ColorPicker(const char* label, float col[4]);
    bool InputText(const char* label, char* buf, size_t buf_size);
    bool KeybindBadge(const char* label, int keybind, bool is_binding);
    bool SearchBar(const char* id, char* buf, size_t buf_size, const char* placeholder = "Search modules...");

    void SectionHeader(const char* title, const char* badge = nullptr);
    bool PrimaryButton(const char* label, const ImVec2& size = ImVec2(0, 0));
    bool SecondaryButton(const char* label, const ImVec2& size = ImVec2(0, 0));
    bool DangerButton(const char* label, const ImVec2& size = ImVec2(0, 0));
}