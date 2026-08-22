#pragma once

class LocalPlayer;

namespace minecraft::hooks {
    void install_attack_hook(LocalPlayer* localPlayer);
    void remove_attack_hook();
}