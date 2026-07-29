#include "ui/menu.h"

#include "input/key_names.h"
#include "input/module_keybinds.h"
#include "modules/module_manager.h"
#include "ui/ui_settings.h"

#include <Windows.h>
#include <imgui.h>
#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

namespace {
bool g_showDemo = false;
std::unordered_set<Module*> g_openSettingsModules;

void render_text_label(const char* label, const char* value) {
    ImGui::TextDisabled("%s", label);
    ImGui::SameLine();
    ImGui::TextUnformatted(value);
}

void render_module_controls(Module& module, float scale) {
    const bool binding = input::binding_module() == &module;
    const std::string keyText = binding
        ? std::string("[...]")
        : std::string("[") + input::key_name(module.keybind()) + "]";
    const float rowHeight = 22.0f * scale;

    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("Bind");
    ImGui::SameLine(0.0f, 6.0f * scale);
    if (ImGui::Button(keyText.c_str(), ImVec2(-1.0f, rowHeight))) {
        if (binding) {
            input::cancel_binding();
        } else {
            input::begin_binding(&module);
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Press a key to bind. Backspace/Delete clears. Escape cancels.");
    }

    bool holdToEnable = module.hold_to_enable();
    if (ImGui::Checkbox("Hold to enable", &holdToEnable)) {
        module.set_hold_to_enable(holdToEnable);
    }
}

void render_module_button(Module& module, float scale) {
    ImGui::PushID(&module);

    const bool enabled = module.enabled();
    const bool settingsOpen = g_openSettingsModules.find(&module) != g_openSettingsModules.end();
    const float moduleHeight = 30.0f * scale;
    const float width = ImGui::GetContentRegionAvail().x;

    ImGui::InvisibleButton("ModuleButton", ImVec2(width, moduleHeight));
    const bool hovered = ImGui::IsItemHovered();
    const bool leftClicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    const bool rightClicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right);
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImU32 bgColor = enabled
        ? IM_COL32(30, 96, 65, 255)
        : (hovered ? IM_COL32(38, 40, 48, 255) : IM_COL32(27, 29, 36, 255));
    const ImU32 borderColor = enabled ? IM_COL32(80, 210, 140, 165) : IM_COL32(70, 74, 86, 130);
    const ImU32 textColor = enabled ? IM_COL32(226, 255, 236, 255) : IM_COL32(220, 224, 232, 235);
    const ImU32 mutedColor = IM_COL32(150, 157, 170, 230);

    drawList->AddRectFilled(min, max, bgColor, 4.0f * scale);
    drawList->AddRect(min, max, borderColor, 4.0f * scale);
    drawList->AddCircleFilled(ImVec2(min.x + 10.0f * scale, min.y + moduleHeight * 0.5f), 3.0f * scale,
        enabled ? IM_COL32(105, 245, 157, 255) : IM_COL32(100, 104, 116, 255));

    const ImVec2 nameSize = ImGui::CalcTextSize(module.name().c_str());
    drawList->AddText(
        ImVec2(min.x + 20.0f * scale, min.y + (moduleHeight - nameSize.y) * 0.5f),
        textColor,
        module.name().c_str());

    const ImVec2 arrowCenter(max.x - 12.0f * scale, min.y + moduleHeight * 0.5f);
    if (settingsOpen) {
        drawList->AddTriangleFilled(
            ImVec2(arrowCenter.x - 4.5f * scale, arrowCenter.y - 2.0f * scale),
            ImVec2(arrowCenter.x + 4.5f * scale, arrowCenter.y - 2.0f * scale),
            ImVec2(arrowCenter.x, arrowCenter.y + 4.0f * scale),
            mutedColor);
    } else {
        drawList->AddTriangleFilled(
            ImVec2(arrowCenter.x - 2.0f * scale, arrowCenter.y - 4.5f * scale),
            ImVec2(arrowCenter.x - 2.0f * scale, arrowCenter.y + 4.5f * scale),
            ImVec2(arrowCenter.x + 4.0f * scale, arrowCenter.y),
            mutedColor);
    }

    const char* stateText = enabled ? "ON" : "OFF";
    const ImVec2 stateSize = ImGui::CalcTextSize(stateText);
    drawList->AddText(
        ImVec2(max.x - stateSize.x - 28.0f * scale, min.y + (moduleHeight - stateSize.y) * 0.5f),
        enabled ? IM_COL32(105, 245, 157, 255) : mutedColor,
        stateText);

    if (leftClicked) {
        module.toggle();
    }

    if (rightClicked) {
        if (settingsOpen) {
            g_openSettingsModules.erase(&module);
        } else {
            g_openSettingsModules.insert(&module);
        }
    }

    if (hovered && !module.tooltip().empty()) {
        ImGui::SetTooltip("%s", module.tooltip().c_str());
    }

    if (settingsOpen) {
        const float settingsHeight = module.has_settings() ? 106.0f * scale : 58.0f * scale;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f * scale, 5.0f * scale));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f * scale, 3.0f * scale));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5.0f * scale, 4.0f * scale));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.09f, 0.11f, 0.96f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.23f, 0.29f, 0.80f));
        ImGui::BeginChild("Settings", ImVec2(0.0f, settingsHeight), true);

        render_module_controls(module, scale);

        if (module.has_settings()) {
            ImGui::Separator();
            ImGui::PushItemWidth(-1.0f);
            module.on_settings_render();
            ImGui::PopItemWidth();
        } else {
            ImGui::Spacing();
            ImGui::TextDisabled("No custom settings.");
        }

        ImGui::EndChild();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(3);
    }

    ImGui::PopID();
}

void render_modules_for_category(ModuleManager& moduleManager, const std::string& category, float scale) {
    std::vector<Module*> modules = moduleManager.modules_in_category(category);

    if (modules.empty()) {
        ImGui::TextDisabled("No modules yet.");
        return;
    }

    for (Module* module : modules) {
        if (!module) {
            continue;
        }

        render_module_button(*module, scale);
        ImGui::Spacing();
    }
}

void render_category_window(ModuleManager& moduleManager, const std::string& category, size_t index, float scale) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const size_t categoryCount = std::max<size_t>(moduleManager.categories().size(), 1);
    const float spacing = 8.0f * scale;
    const float desiredWidth = 220.0f * scale;
    const float minWidth = 170.0f * scale;
    const float availableWidth = viewport->WorkSize.x - 48.0f * scale - static_cast<float>(categoryCount - 1) * spacing;
    const float windowWidth = std::max(minWidth, std::min(desiredWidth, availableWidth / static_cast<float>(categoryCount)));
    const ImVec2 windowSize(windowWidth, 320.0f * scale);
    const float totalWidth = static_cast<float>(categoryCount) * windowSize.x + static_cast<float>(categoryCount - 1) * spacing;
    const float centeredX = (viewport->WorkSize.x - totalWidth) * 0.5f;
    const float startX = viewport->WorkPos.x + std::max(24.0f * scale, centeredX);
    const ImVec2 initialPos(startX + static_cast<float>(index) * (windowSize.x + spacing), viewport->WorkPos.y + 65.0f * scale);

    ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f * scale);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f * scale);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f, 0.08f, 0.10f, 0.96f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.24f, 0.27f, 0.33f, 0.70f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.10f, 0.11f, 0.14f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.12f, 0.15f, 0.18f, 1.00f));

    const std::string title = category + "###CategoryWindow_" + category;
    if (ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
        render_modules_for_category(moduleManager, category, scale);
    }
    ImGui::End();
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
}

void render_debug_panel(ModuleManager& moduleManager) {
    ImGui::TextUnformatted("Runtime settings");
    ImGui::Separator();

    float uiScale = ui_settings::ui_scale();
    ImGui::SetNextItemWidth(260.0f);
    if (ImGui::SliderFloat("UI Scale", &uiScale, 0.65f, 1.75f, "%.2fx")) {
        ui_settings::set_ui_scale(uiScale);
    }

    ImGui::Checkbox("Show ImGui demo", &g_showDemo);
    render_text_label("Registered modules:", std::to_string(moduleManager.modules().size()).c_str());
    render_text_label("Menu key:", "INSERT");
    render_text_label("Unload key:", "END");
}
}

namespace menu {
void render() {
    ModuleManager& moduleManager = ModuleManager::instance();
    const float scale = ui_settings::ui_scale();
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    const std::vector<std::string>& categories = moduleManager.categories();
    for (size_t i = 0; i < categories.size(); ++i) {
        render_category_window(moduleManager, categories[i], i, scale);
    }

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 24.0f * scale, viewport->WorkPos.y + 24.0f * scale), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280.0f * scale, 210.0f * scale), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("ModBase Info", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Separator();
        render_debug_panel(moduleManager);
    }
    ImGui::End();

    if (g_showDemo) {
        ImGui::ShowDemoWindow(&g_showDemo);
    }
}
}
