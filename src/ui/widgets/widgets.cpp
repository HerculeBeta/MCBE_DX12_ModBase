#include "ui/widgets/widgets.h"
#include "input/key_names.h"

#include <imgui_internal.h>
#include <algorithm>
#include <cstdio>

namespace ui {

    static Palette g_palette{};

    const Palette& get_palette() {
        return g_palette;
    }

    void apply_theme() {
        const Palette& p = get_palette();
        ImGuiStyle& style = ImGui::GetStyle();

        style.WindowPadding = ImVec2(12.0f, 12.0f);
        style.FramePadding = ImVec2(10.0f, 6.0f);
        style.ItemSpacing = ImVec2(8.0f, 8.0f);
        style.ItemInnerSpacing = ImVec2(6.0f, 6.0f);

        style.WindowRounding = 8.0f;
        style.ChildRounding = 6.0f;
        style.FrameRounding = 5.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 4.0f;

        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(p.text_primary);
        colors[ImGuiCol_TextDisabled] = ImGui::ColorConvertU32ToFloat4(p.text_secondary);
        colors[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(p.bg_dark);
        colors[ImGuiCol_ChildBg] = ImGui::ColorConvertU32ToFloat4(p.panel_bg);
        colors[ImGuiCol_PopupBg] = ImGui::ColorConvertU32ToFloat4(p.panel_bg);
        colors[ImGuiCol_Border] = ImGui::ColorConvertU32ToFloat4(p.border);
        colors[ImGuiCol_FrameBg] = ImGui::ColorConvertU32ToFloat4(p.card_bg);
        colors[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(p.card_hover);
        colors[ImGuiCol_FrameBgActive] = ImGui::ColorConvertU32ToFloat4(p.card_hover);
        colors[ImGuiCol_TitleBg] = ImGui::ColorConvertU32ToFloat4(p.panel_bg);
        colors[ImGuiCol_TitleBgActive] = ImGui::ColorConvertU32ToFloat4(p.panel_bg);
        colors[ImGuiCol_ScrollbarBg] = ImGui::ColorConvertU32ToFloat4(p.bg_dark);
        colors[ImGuiCol_ScrollbarGrab] = ImGui::ColorConvertU32ToFloat4(p.card_hover);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImGui::ColorConvertU32ToFloat4(p.border);
        colors[ImGuiCol_ScrollbarGrabActive] = ImGui::ColorConvertU32ToFloat4(p.accent);
        colors[ImGuiCol_CheckMark] = ImGui::ColorConvertU32ToFloat4(p.accent);
        colors[ImGuiCol_SliderGrab] = ImGui::ColorConvertU32ToFloat4(p.accent);
        colors[ImGuiCol_SliderGrabActive] = ImGui::ColorConvertU32ToFloat4(p.accent_active);
        colors[ImGuiCol_Button] = ImGui::ColorConvertU32ToFloat4(p.card_bg);
        colors[ImGuiCol_ButtonHovered] = ImGui::ColorConvertU32ToFloat4(p.card_hover);
        colors[ImGuiCol_ButtonActive] = ImGui::ColorConvertU32ToFloat4(p.accent_muted);
        colors[ImGuiCol_Header] = ImGui::ColorConvertU32ToFloat4(p.accent_muted);
        colors[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(p.card_hover);
        colors[ImGuiCol_HeaderActive] = ImGui::ColorConvertU32ToFloat4(p.accent);
        colors[ImGuiCol_Separator] = ImGui::ColorConvertU32ToFloat4(p.border);
    }

    bool ToggleSwitch(const char* label, bool* v) {
        ImGui::PushID(label);
        const Palette& p = get_palette();

        const float switchW = 34.0f;
        const float switchH = 18.0f;
        const float availW = ImGui::GetContentRegionAvail().x;

        ImVec2 cursor = ImGui::GetCursorScreenPos();

        if (label && !(label[0] == '#' && label[1] == '#')) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(label);
        }

        ImVec2 switchPos(cursor.x + availW - switchW, cursor.y + (ImGui::GetTextLineHeight() - switchH) * 0.5f);

        ImGui::SetCursorScreenPos(switchPos);
        ImGui::InvisibleButton("##sw_btn", ImVec2(switchW, switchH));

        const bool clicked = ImGui::IsItemClicked();
        const bool hovered = ImGui::IsItemHovered();

        if (clicked) {
            *v = !(*v);
        }

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImU32 bgColor = *v ? p.accent : (hovered ? p.card_hover : p.card_bg);
        const ImU32 knobColor = *v ? p.text_primary : p.text_secondary;

        drawList->AddRectFilled(switchPos, ImVec2(switchPos.x + switchW, switchPos.y + switchH), bgColor, 9.0f);
        drawList->AddRect(switchPos, ImVec2(switchPos.x + switchW, switchPos.y + switchH), p.border, 9.0f);

        const float knobX = *v ? (switchPos.x + switchW - 9.0f) : (switchPos.x + 9.0f);
        drawList->AddCircleFilled(ImVec2(knobX, switchPos.y + switchH * 0.5f), 5.5f, knobColor);

        ImGui::PopID();
        return clicked;
    }

    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format) {
        ImGui::PushID(label);
        const Palette& p = get_palette();

        if (label && !(label[0] == '#' && label[1] == '#')) {
            ImGui::TextUnformatted(label);
        }

        char val_buf[64];
        std::snprintf(val_buf, sizeof(val_buf), format, *v);
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(val_buf).x);
        ImGui::TextColored(ImColor(p.accent_hover), "%s", val_buf);

        const float w = ImGui::GetContentRegionAvail().x;
        const float h = 18.0f;
        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImGui::InvisibleButton("##slider_btn", ImVec2(w, h));
        const bool hovered = ImGui::IsItemHovered();
        const bool active = ImGui::IsItemActive();
        bool changed = false;

        if (active) {
            float mouse_x = ImGui::GetIO().MousePos.x - pos.x;
            float percent = ImClamp(mouse_x / w, 0.0f, 1.0f);
            float new_val = v_min + percent * (v_max - v_min);
            if (*v != new_val) {
                *v = new_val;
                changed = true;
            }
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        float percent = ImClamp((*v - v_min) / (v_max - v_min), 0.0f, 1.0f);

        draw_list->AddRectFilled(ImVec2(pos.x, pos.y + 5.0f), ImVec2(pos.x + w, pos.y + 13.0f), p.card_hover, 4.0f);
        if (percent > 0.0f) {
            draw_list->AddRectFilled(ImVec2(pos.x, pos.y + 5.0f), ImVec2(pos.x + (w * percent), pos.y + 13.0f), active ? p.accent_active : p.accent, 4.0f);
        }

        float knob_x = pos.x + (w * percent);
        draw_list->AddCircleFilled(ImVec2(knob_x, pos.y + 9.0f), hovered || active ? 7.0f : 5.5f, p.text_primary);

        ImGui::PopID();
        return changed;
    }

    bool SliderInt(const char* label, int* v, int v_min, int v_max) {
        float f_val = static_cast<float>(*v);
        bool changed = SliderFloat(label, &f_val, static_cast<float>(v_min), static_cast<float>(v_max), "%.0f");
        if (changed) {
            *v = static_cast<int>(f_val);
        }
        return changed;
    }

    bool Combo(const char* label, int* current_item, const std::vector<std::string>& items) {
        if (items.empty()) return false;
        ImGui::PushID(label);
        const Palette& p = get_palette();

        if (label && !(label[0] == '#' && label[1] == '#')) {
            ImGui::TextUnformatted(label);
            ImGui::Spacing();
        }

        std::string current_str = (*current_item >= 0 && *current_item < static_cast<int>(items.size()))
            ? items[*current_item] : "Select...";

        bool changed = false;

        ImGui::PushStyleColor(ImGuiCol_FrameBg, p.card_bg);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, p.card_hover);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, p.panel_bg);
        ImGui::PushStyleColor(ImGuiCol_Header, p.accent_muted);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::BeginCombo("##combo_box", current_str.c_str())) {
            for (int i = 0; i < static_cast<int>(items.size()); ++i) {
                const bool is_selected = (*current_item == i);
                if (ImGui::Selectable(items[i].c_str(), is_selected)) {
                    *current_item = i;
                    changed = true;
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
        ImGui::PopID();
        return changed;
    }

    bool ColorPicker(const char* label, float col[4]) {
        ImGui::PushID(label);

        if (label && !(label[0] == '#' && label[1] == '#')) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(label);
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 36.0f);
        }

        ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        bool changed = ImGui::ColorEdit4("##color_edit", col, flags);
        ImGui::PopStyleVar();

        ImGui::PopID();
        return changed;
    }

    bool InputText(const char* label, char* buf, size_t buf_size) {
        ImGui::PushID(label);
        const Palette& p = get_palette();

        if (label && !(label[0] == '#' && label[1] == '#')) {
            ImGui::TextUnformatted(label);
            ImGui::Spacing();
        }

        ImGui::PushStyleColor(ImGuiCol_FrameBg, p.card_bg);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, p.card_hover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, p.panel_bg);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

        ImGui::SetNextItemWidth(-1.0f);
        bool changed = ImGui::InputText("##text_input", buf, buf_size);

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        ImGui::PopID();
        return changed;
    }

    bool KeybindBadge(const char* label, int keybind, bool is_binding) {
        ImGui::PushID(label);
        const Palette& p = get_palette();

        std::string key_str = is_binding ? "[ ... ]" : (std::string("[ ") + input::get_key_name(keybind) + " ]");
        ImVec2 text_size = ImGui::CalcTextSize(key_str.c_str());
        ImVec2 btn_size(text_size.x + 14.0f, text_size.y + 6.0f);

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##keybind_btn", btn_size);

        const bool hovered = ImGui::IsItemHovered();
        const bool clicked = ImGui::IsItemClicked();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImU32 bg = is_binding ? p.accent_muted : (hovered ? p.card_hover : p.card_bg);
        ImU32 border = is_binding ? p.accent : (hovered ? p.border_focused : p.border);
        ImU32 text_col = is_binding ? p.accent_hover : (hovered ? p.text_primary : p.text_secondary);

        draw_list->AddRectFilled(pos, ImVec2(pos.x + btn_size.x, pos.y + btn_size.y), bg, 4.0f);
        draw_list->AddRect(pos, ImVec2(pos.x + btn_size.x, pos.y + btn_size.y), border, 4.0f);
        draw_list->AddText(ImVec2(pos.x + 7.0f, pos.y + 3.0f), text_col, key_str.c_str());

        ImGui::PopID();
        return clicked;
    }

    bool SearchBar(const char* id, char* buf, size_t buf_size, const char* placeholder) {
        ImGui::PushID(id);
        const Palette& p = get_palette();

        ImGui::PushStyleColor(ImGuiCol_FrameBg, p.card_bg);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, p.card_hover);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, p.panel_bg);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f));

        bool changed = ImGui::InputTextWithHint("##search_input", placeholder, buf, buf_size);

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
        ImGui::PopID();
        return changed;
    }

    void SectionHeader(const char* title, const char* badge) {
        const Palette& p = get_palette();
        ImGui::TextColored(ImColor(p.text_primary), "%s", title);
        if (badge && badge[0] != '\0') {
            ImGui::SameLine();
            ImGui::TextColored(ImColor(p.accent_hover), "(%s)", badge);
        }
        ImGui::Spacing();
    }

    bool PrimaryButton(const char* label, const ImVec2& size) {
        const Palette& p = get_palette();
        ImGui::PushStyleColor(ImGuiCol_Button, p.accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, p.accent_hover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, p.accent_active);
        ImGui::PushStyleColor(ImGuiCol_Text, p.bg_dark);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

        bool clicked = ImGui::Button(label, size);

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
        return clicked;
    }

    bool SecondaryButton(const char* label, const ImVec2& size) {
        const Palette& p = get_palette();
        ImGui::PushStyleColor(ImGuiCol_Button, p.card_bg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, p.card_hover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, p.panel_bg);
        ImGui::PushStyleColor(ImGuiCol_Text, p.text_primary);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

        bool clicked = ImGui::Button(label, size);

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
        return clicked;
    }

    bool DangerButton(const char* label, const ImVec2& size) {
        const Palette& p = get_palette();
        ImGui::PushStyleColor(ImGuiCol_Button, p.danger);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, p.danger_hover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, p.danger);
        ImGui::PushStyleColor(ImGuiCol_Text, p.text_primary);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

        bool clicked = ImGui::Button(label, size);

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
        return clicked;
    }
}