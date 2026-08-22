#include "ui/menu.h"
#include "ui/widgets/widgets.h"
#include "ui/ui_settings.h"
#include "input/input_manager.h"
#include "input/key_names.h"
#include "modules/module_manager.h"
#include "config/config_manager.h"

#include <Windows.h>
#include <imgui.h>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
    static char g_searchFilter[64] = "";
    static std::unordered_set<Module*> g_expandedModules;
    static std::unordered_map<std::string, bool> g_collapsedCategories;

    bool matches_search(const Module& module, const char* filter) {
        if (!filter || filter[0] == '\0') return true;
        std::string search(filter);
        std::transform(search.begin(), search.end(), search.begin(), ::tolower);

        std::string modName = module.name();
        std::transform(modName.begin(), modName.end(), modName.begin(), ::tolower);

        std::string modCategory = module.category();
        std::transform(modCategory.begin(), modCategory.end(), modCategory.begin(), ::tolower);

        return modName.find(search) != std::string::npos || modCategory.find(search) != std::string::npos;
    }

    void render_header_bar() {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        const float headerWidth = 440.0f;

        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 16.0f, viewport->WorkPos.y + 16.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(headerWidth, 48.0f), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;

        if (ImGui::Begin("##HeaderBar", nullptr, flags)) {
            const ui::Palette& p = ui::get_palette();

            if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                ImVec2 pos = ImGui::GetWindowPos();
                ImGui::SetWindowPos(ImVec2(pos.x + delta.x, pos.y + delta.y));
            }

            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImColor(p.accent), "BEDROCK");
            ImGui::SameLine();
            ImGui::TextColored(ImColor(p.text_primary), "Client Base");

            ImGui::SameLine(180.0f);
            ImGui::SetNextItemWidth(248.0f);
            ui::SearchBar("##HeaderSearch", g_searchFilter, sizeof(g_searchFilter), "Search features...");
        }
        ImGui::End();
    }

    void render_module_card(Module& module) {
        ImGui::PushID(&module);
        const ui::Palette& p = ui::get_palette();

        const bool enabled = module.enabled();
        const bool isExpanded = g_expandedModules.find(&module) != g_expandedModules.end();

        const float width = ImGui::GetContentRegionAvail().x;
        const float cardHeight = 36.0f;

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##CardBgBtn", ImVec2(width, cardHeight));

        const bool hovered = ImGui::IsItemHovered();
        const bool leftClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
        const bool rightClicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        const ImU32 bgColor = enabled ? p.card_enabled : (hovered ? p.card_hover : p.card_bg);
        const ImU32 borderColor = enabled ? p.border_focused : p.border;

        drawList->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + cardHeight), bgColor, 6.0f);
        drawList->AddRect(pos, ImVec2(pos.x + width, pos.y + cardHeight), borderColor, 6.0f);

        drawList->AddCircleFilled(ImVec2(pos.x + 12.0f, pos.y + cardHeight * 0.5f), 3.5f,
            enabled ? p.accent : p.text_muted);

        const ImVec2 nameSize = ImGui::CalcTextSize(module.name().c_str());
        drawList->AddText(
            ImVec2(pos.x + 22.0f, pos.y + (cardHeight - nameSize.y) * 0.5f),
            p.text_primary, module.name().c_str());

        const float switchW = 32.0f;
        const float switchH = 16.0f;
        const ImVec2 switchPos(pos.x + width - switchW - 8.0f, pos.y + (cardHeight - switchH) * 0.5f);

        const ImU32 switchBg = enabled ? p.accent : (hovered ? p.card_hover : p.card_bg);
        const ImU32 knobColor = enabled ? p.text_primary : p.text_secondary;
        drawList->AddRectFilled(switchPos, ImVec2(switchPos.x + switchW, switchPos.y + switchH), switchBg, 8.0f);
        drawList->AddRect(switchPos, ImVec2(switchPos.x + switchW, switchPos.y + switchH), p.border, 8.0f);

        const float knobX = enabled ? (switchPos.x + switchW - 8.0f) : (switchPos.x + 8.0f);
        drawList->AddCircleFilled(ImVec2(knobX, switchPos.y + switchH * 0.5f), 5.0f, knobColor);

        if (leftClicked) {
            module.toggle();
        }

        if (rightClicked) {
            if (isExpanded) g_expandedModules.erase(&module);
            else g_expandedModules.insert(&module);
        }

        if (hovered && !module.tooltip().empty()) {
            ImGui::SetTooltip("%s", module.tooltip().c_str());
        }

        if (isExpanded) {
            ImGui::Spacing();
            ImGui::Indent(4.0f);

            const bool isBinding = InputManager::instance().binding_module() == &module;
            if (ui::KeybindBadge("Keybind", module.keybind(), isBinding)) {
                if (isBinding) InputManager::instance().cancel_binding();
                else InputManager::instance().begin_binding(&module);
            }

            bool holdState = module.hold_to_enable();
            if (ui::ToggleSwitch("Hold to Enable", &holdState)) {
                module.set_hold_to_enable(holdState);
            }

            if (module.has_settings()) {
                ImGui::Separator();
                module.on_settings_render();
            }

            ImGui::Unindent(4.0f);
            ImGui::Spacing();
        }

        ImGui::PopID();
    }

    void render_config_card() {
        ui::SectionHeader("Profile Manager", "Configs");

        static char configInputBuffer[64] = "default";
        ui::InputText("Config Name", configInputBuffer, sizeof(configInputBuffer));

        ImGui::Spacing();
        const float buttonWidth = (ImGui::GetContentRegionAvail().x - 6.0f) * 0.5f;

        if (ui::PrimaryButton("Save", ImVec2(buttonWidth, 26.0f))) {
            if (strlen(configInputBuffer) > 0) {
                ConfigManager::instance().save_config(configInputBuffer);
            }
        }

        ImGui::SameLine(0.0f, 6.0f);

        if (ui::SecondaryButton("Load", ImVec2(buttonWidth, 26.0f))) {
            if (strlen(configInputBuffer) > 0) {
                ConfigManager::instance().load_config(configInputBuffer);
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ui::SectionHeader("Saved Profiles");

        auto availableConfigs = ConfigManager::instance().get_available_configs();
        if (availableConfigs.empty()) {
            ImGui::TextDisabled("No configs created.");
        }
        else {
            for (const auto& cfg : availableConfigs) {
                ImGui::PushID(cfg.c_str());
                const bool isSelected = (cfg == configInputBuffer);

                if (ImGui::Selectable(cfg.c_str(), isSelected)) {
                    strncpy_s(configInputBuffer, cfg.c_str(), sizeof(configInputBuffer) - 1);
                }

                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Load Profile")) {
                        ConfigManager::instance().load_config(cfg);
                        strncpy_s(configInputBuffer, cfg.c_str(), sizeof(configInputBuffer) - 1);
                    }
                    if (ImGui::MenuItem("Overwrite Profile")) {
                        ConfigManager::instance().save_config(cfg);
                    }
                    if (ImGui::MenuItem("Delete Profile")) {
                        ConfigManager::instance().delete_config(cfg);
                    }
                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }
        }
    }

    void render_category_window(ModuleManager& moduleManager, const std::string& category, size_t index) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const float windowWidth = 220.0f;
        const float spacing = 12.0f;

        const float startX = viewport->WorkPos.x + 16.0f;
        const float startY = viewport->WorkPos.y + 76.0f;
        const ImVec2 initialPos(startX + static_cast<float>(index) * (windowWidth + spacing), startY);

        ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(windowWidth, 36.0f), ImVec2(windowWidth, 800.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar;

        const std::string title = "##Category_" + category;
        if (ImGui::Begin(title.c_str(), nullptr, flags)) {
            const ui::Palette& p = ui::get_palette();

            bool& collapsed = g_collapsedCategories[category];
            const bool isSearching = (g_searchFilter[0] != '\0');
            const bool showContents = !collapsed || isSearching;

            const float headerHeight = 36.0f;
            ImVec2 headerPos = ImGui::GetCursorScreenPos();

            ImGui::InvisibleButton("##CategoryHeaderArea", ImVec2(windowWidth, headerHeight));
            const bool headerHovered = ImGui::IsItemHovered();
            const bool headerActive = ImGui::IsItemActive();
            const bool rightClicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);

            if (headerActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                ImVec2 currentPos = ImGui::GetWindowPos();
                ImGui::SetWindowPos(ImVec2(currentPos.x + delta.x, currentPos.y + delta.y));
            }

            if (rightClicked) {
                collapsed = !collapsed;
            }

            ImDrawList* drawList = ImGui::GetWindowDrawList();

            const ImU32 headerBg = headerHovered ? p.card_hover : p.card_bg;
            drawList->AddRectFilled(
                headerPos,
                ImVec2(headerPos.x + windowWidth, headerPos.y + headerHeight),
                headerBg,
                6.0f,
                showContents ? ImDrawFlags_RoundCornersTop : ImDrawFlags_RoundCornersAll
            );

            drawList->AddRectFilled(
                headerPos,
                ImVec2(headerPos.x + windowWidth, headerPos.y + 2.5f),
                p.accent,
                6.0f,
                ImDrawFlags_RoundCornersTop
            );

            const ImVec2 titleSize = ImGui::CalcTextSize(category.c_str());
            drawList->AddText(
                ImVec2(headerPos.x + 12.0f, headerPos.y + (headerHeight - titleSize.y) * 0.5f),
                p.text_primary, category.c_str()
            );

            const char* indicator = showContents ? "-" : "+";
            const ImVec2 indSize = ImGui::CalcTextSize(indicator);
            drawList->AddText(
                ImVec2(headerPos.x + windowWidth - 14.0f - indSize.x, headerPos.y + (headerHeight - indSize.y) * 0.5f),
                p.text_secondary, indicator
            );

            if (showContents) {
                drawList->AddLine(
                    ImVec2(headerPos.x, headerPos.y + headerHeight),
                    ImVec2(headerPos.x + windowWidth, headerPos.y + headerHeight),
                    p.border, 1.0f
                );

                ImGui::SetCursorScreenPos(ImVec2(headerPos.x + 6.0f, headerPos.y + headerHeight + 6.0f));
                ImGui::PushItemWidth(windowWidth - 12.0f);

                if (category == "Configs") {
                    render_config_card();
                }
                else {
                    auto modules = moduleManager.modules_in_category(category);
                    size_t renderedCount = 0;

                    for (Module* mod : modules) {
                        if (!mod || !matches_search(*mod, g_searchFilter)) continue;
                        render_module_card(*mod);
                        ImGui::Spacing();
                        renderedCount++;
                    }

                    if (renderedCount == 0) {
                        ImGui::TextDisabled("No features match.");
                        ImGui::Spacing();
                    }
                }

                ImGui::PopItemWidth();
                ImGui::Dummy(ImVec2(0.0f, 2.0f));
            }
        }
        ImGui::End();
        ImGui::PopStyleVar(3);
    }
}

namespace menu {
    void render() {
        ModuleManager& moduleManager = ModuleManager::instance();

        render_header_bar();

        const auto& categories = moduleManager.categories();
        for (size_t i = 0; i < categories.size(); ++i) {
            render_category_window(moduleManager, categories[i], i);
        }
    }
}