#include "modules/visual/ESP.h"
#include "minecraft/sdk/client/client_instance.h"
#include "minecraft/sdk/actor/local_player.h"

#include <imgui.h>

ESP::ESP() : Module("Visual", "ESP", "Renders 3D bounding boxes around entities.") {
    m_targetPlayers = add_setting<config::BoolSetting>("Players", "Render ESP on players.", true);
    m_targetMobs = add_setting<config::BoolSetting>("Mobs", "Render ESP on mobs.", false);
    m_color = add_setting<config::ColorSetting>("Color", "Bounding box color.", 1.0f, 0.2f, 0.2f, 1.0f);
    m_thickness = add_setting<config::FloatSetting>("Thickness", "Line thickness.", 1.5f, 0.5f, 5.0f);
    m_fillOpacity = add_setting<config::FloatSetting>("Fill Opacity", "Interior fill opacity.", 0.2f, 0.0f, 1.0f);
}

void ESP::on_imgui_render() {
    auto* ci = ClientInstance::get();
    if (!ci) return;

    LocalPlayer* localPlayer = ci->getLocalPlayer();
    if (!localPlayer) return;

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    if (!drawList) return;

    const ImVec4 col = m_color->as_imvec4();
    const ImU32 boxColor = ImGui::ColorConvertFloat4ToU32(col);
    const float thickness = m_thickness->value();
    const float fillOpacity = m_fillOpacity->value();
    const ImU32 fillColor = ImGui::ColorConvertFloat4ToU32(ImVec4(col.x, col.y, col.z, col.w * fillOpacity));

    static constexpr int faces[6][4] = {
        { 0, 1, 2, 3 }, { 4, 5, 6, 7 },
        { 0, 1, 5, 4 }, { 3, 2, 6, 7 },
        { 0, 3, 7, 4 }, { 1, 2, 6, 5 }
    };

    static constexpr int edges[12][2] = {
        { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
        { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },
        { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
    };

    for (Actor* actor : Actor::get_all(localPlayer)) {
        if (!actor || actor == localPlayer) continue;

        const bool isPlayer = actor->isPlayer();
        if (isPlayer && !m_targetPlayers->value()) continue;
        if (!isPlayer && !m_targetMobs->value()) continue;

        const math::Vec3 renderPos = actor->getRenderPosition();
        const float yOffset = isPlayer ? -1.62f : 0.0f;
        const math::Vec3 center = { renderPos.x, renderPos.y + yOffset, renderPos.z };

        float halfW = 0.3f;
        float h = 1.8f;
        if (auto* aabb = actor->getAABB()) {
            halfW = aabb->mWidth * 0.5f;
            h = aabb->mHeight;
        }

        const math::Vec3 min = { center.x - halfW, center.y, center.z - halfW };
        const math::Vec3 max = { center.x + halfW, center.y + h, center.z + halfW };

        const math::Vec3 corners[8] = {
            { min.x, min.y, min.z }, { max.x, min.y, min.z },
            { max.x, min.y, max.z }, { min.x, min.y, max.z },
            { min.x, max.y, min.z }, { max.x, max.y, min.z },
            { max.x, max.y, max.z }, { min.x, max.y, max.z }
        };

        math::Vec2 sc[8];
        bool allValid = true;
        for (int i = 0; i < 8; ++i) {
            if (!ci->world_to_screen(corners[i], sc[i])) {
                allValid = false;
                break;
            }
        }

        if (!allValid) continue;

        if (fillOpacity > 0.001f) {
            for (const auto& f : faces) {
                drawList->AddQuadFilled(
                    ImVec2(sc[f[0]].x, sc[f[0]].y),
                    ImVec2(sc[f[1]].x, sc[f[1]].y),
                    ImVec2(sc[f[2]].x, sc[f[2]].y),
                    ImVec2(sc[f[3]].x, sc[f[3]].y),
                    fillColor
                );
            }
        }

        for (const auto& edge : edges) {
            drawList->AddLine(
                ImVec2(sc[edge[0]].x, sc[edge[0]].y),
                ImVec2(sc[edge[1]].x, sc[edge[1]].y),
                boxColor,
                thickness
            );
        }
    }
}
