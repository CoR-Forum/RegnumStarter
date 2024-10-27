// includes/BossRespawnWindow.cpp
#include "BossRespawnWindow.h"
#include "../../includes/Bosses/BossSpawns.h"
#include "../../libs/imgui/imgui.h"

void ShowBossRespawnWindow(bool& show_boss_respawn_window) {
    if (show_boss_respawn_window) {
        // Display boss respawn times
        for (const auto& pair : bossRespawns) {
            const BossRespawn& boss = pair.second;
            ImGui::Text("Boss: %s", boss.name.c_str());
            for (const auto& respawnTime : boss.nextRespawns) {
                std::tm* tm = std::localtime(&respawnTime);
                char buffer[64];
                std::strftime(buffer, sizeof(buffer), "%A, %Y-%m-%d %H:%M:%S", tm);
                ImGui::Text("%s", buffer);
            }
        }
    }
}