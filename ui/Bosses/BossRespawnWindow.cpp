#include "BossRespawnWindow.h"
#include "../../includes/Bosses/BossSpawns.h"
#include "../../libs/imgui/imgui.h"
#include <chrono>

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

                // Calculate the time difference
                auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                auto diff = std::difftime(respawnTime, now);

                int hours = static_cast<int>(diff) / 3600;
                int minutes = (static_cast<int>(diff) % 3600) / 60;
                int seconds = static_cast<int>(diff) % 60;

                ImGui::Text("Next Spawn in: %02d:%02d:%02d", hours, minutes, seconds);
            }
        }
    }
}