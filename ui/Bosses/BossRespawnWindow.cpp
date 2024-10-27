#include "BossRespawnWindow.h"
#include "../../includes/Bosses/BossSpawns.h"
#include "../../libs/imgui/imgui.h"
#include "../../libs/ImageLoader/ImageLoader.h"
#include "../../includes/API/ApiHandler.h"
#include <chrono>

void ShowBossRespawnWindow(bool& show_boss_respawn_window) {
    static LPDIRECT3DTEXTURE9 texture_evendim = nullptr;
    static LPDIRECT3DTEXTURE9 texture_daen = nullptr;
    static LPDIRECT3DTEXTURE9 texture_thorkul = nullptr;
    static LPDIRECT3DTEXTURE9 texture_server = nullptr;

    if (!texture_evendim) {
        texture_evendim = LoadTextureFromResource(g_pd3dDevice, IDR_PNG_EVENDIM_ICON); // Assuming you have a resource ID for Evendim
    }
    if (!texture_daen) {
        texture_daen = LoadTextureFromResource(g_pd3dDevice, IDR_PNG_DAEN_ICON); // Assuming you have a resource ID for Daen
    }
    if (!texture_thorkul) {
        texture_thorkul = LoadTextureFromResource(g_pd3dDevice, IDR_PNG_THORKUL_ICON); // Assuming you have a resource ID for Daen
    }
    if (!texture_server) {
        texture_server = LoadTextureFromResource(g_pd3dDevice, IDR_PNG_SERVER_ICON); // Assuming you have a resource ID for Daen
    }

    if (show_boss_respawn_window) {
        // Display boss respawn times
        for (const auto& pair : bossRespawns) {
            const BossRespawn& boss = pair.second;
            ImGui::SeparatorText(boss.name.c_str());

            ImGui::BeginChild(boss.name.c_str(), ImVec2(0, 120), false, ImGuiWindowFlags_NoScrollbar);
            ImGui::Columns(2, nullptr, false);

            // Display the text lines on the left side
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

            ImGui::NextColumn();

            // Add some spacing before displaying the image
            ImGui::Dummy(ImVec2(0.0f, 20.0f)); // Adjust the vertical spacing as needed

            // Display the boss image on the right side
            if (boss.name == "Evendim" && texture_evendim) {
                ImGui::Image((void*)texture_evendim, ImVec2(70, 70)); // Adjust the size as needed
            } else if (boss.name == "Daen" && texture_daen) {
                ImGui::Image((void*)texture_daen, ImVec2(70, 70)); // Adjust the size as needed
            } else if (boss.name == "Thorkul" && texture_thorkul) {
                ImGui::Image((void*)texture_thorkul, ImVec2(70, 70)); // Adjust the size as needed
            } else if (boss.name == "ServerRestart" && texture_server) {
                ImGui::Image((void*)texture_server, ImVec2(70, 70)); // Adjust the size as needed
            }

            ImGui::Columns(1);
            ImGui::EndChild();
        }
    }
}