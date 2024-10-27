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
            for (size_t i = 0; i < boss.nextRespawns.size(); ++i) {
                ImGui::Text("Next Respawn %zu: %s", i + 1, std::ctime(&boss.nextRespawns[i]));
            }
        }
    }
}