#include "ui/ui_settings.h"

#include <algorithm>
#include <imgui.h>

namespace {
float g_uiScale = 1.0f;
float g_appliedUiScale = 0.0f;
ImGuiStyle g_baseStyle{};
bool g_hasBaseStyle = false;
}

namespace ui_settings {
float ui_scale() {
    return g_uiScale;
}

void set_ui_scale(float scale) {
    g_uiScale = std::clamp(scale, 0.65f, 1.75f);
}

void apply_ui_scale() {
    if (!ImGui::GetCurrentContext()) {
        return;
    }

    if (!g_hasBaseStyle) {
        g_baseStyle = ImGui::GetStyle();
        g_hasBaseStyle = true;
    }

    if (g_appliedUiScale == g_uiScale) {
        return;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style = g_baseStyle;
    style.ScaleAllSizes(g_uiScale);
    ImGui::GetIO().FontGlobalScale = g_uiScale;
    g_appliedUiScale = g_uiScale;
}

void reset_ui_scale_cache() {
    g_appliedUiScale = 0.0f;
    g_baseStyle = ImGuiStyle{};
    g_hasBaseStyle = false;
}
}
