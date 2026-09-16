#include "minecraft/sdk/client/client_instance.h"

#include <imgui.h>
#include <cmath>

bool ClientInstance::world_to_screen(const math::Vec3& worldPos, math::Vec2& screenPos) const {
    if (!this) return false;

    __try {
        LevelRenderer* lr = getLevelRenderer();
        if (!lr) return false;

        LevelRendererPlayer* rp = lr->getLevelRendererPlayer();
        if (!rp) return false;

        const math::Vec3 origin = rp->getOrigin();
        const math::Vec3 rel = worldPos - origin;

        const GLMatrix mat = getGLMatrix();
        const math::Vec3 transformed = mat.transform(rel);

        if (transformed.z > 0.0f) {
            return false;
        }

        const float divisor = -transformed.z;
        if (divisor < 0.001f) {
            return false;
        }

        math::Vec2 screenSize{};
        if (GuiData* gui = getGuiData()) {
            screenSize = gui->getWindowSize();
        }
        if (screenSize.x <= 0.0f || screenSize.y <= 0.0f) {
            const ImVec2 display = ImGui::GetIO().DisplaySize;
            screenSize = { display.x, display.y };
        }

        if (screenSize.x <= 0.0f || screenSize.y <= 0.0f) {
            return false;
        }

        const float halfW = screenSize.x * 0.5f;
        const float halfH = screenSize.y * 0.5f;
        const float fovX = rp->getFovX();
        const float fovY = rp->getFovY();

        screenPos.x = halfW + (halfW * transformed.x / divisor * fovX);
        screenPos.y = halfH - (halfH * transformed.y / divisor * fovY);

        return std::isfinite(screenPos.x) && std::isfinite(screenPos.y);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
