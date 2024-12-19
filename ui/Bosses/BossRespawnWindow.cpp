#include "BossRespawnWindow.h"
#include "../../libs/imgui/imgui.h"
#include "../../libs/ImageLoader/ImageLoader.h"
#include "../../includes/API/ApiHandler.h"
#include <chrono>
#include <unordered_map>
#include <string>

// Texture cache
std::unordered_map<std::string, LPDIRECT3DTEXTURE9> textureCache;

LPDIRECT3DTEXTURE9 LoadTextureWithCache(const std::string& textureName, int resourceId) {
    auto it = textureCache.find(textureName);
    if (it != textureCache.end()) {
        return it->second;
    }

    LPDIRECT3DTEXTURE9 texture = LoadTextureFromResource(g_pd3dDevice, resourceId);
    textureCache[textureName] = texture;
    return texture;
}

void ShowBossRespawnWindow(bool& show_calendar_window) {
    static LPDIRECT3DTEXTURE9 texture_evendim = nullptr;
    static LPDIRECT3DTEXTURE9 texture_daen = nullptr;
    static LPDIRECT3DTEXTURE9 texture_thorkul = nullptr;
    static LPDIRECT3DTEXTURE9 texture_server = nullptr;

    static bool bossRespawnTimesLoaded = false;

    if (!texture_evendim) {
        texture_evendim = LoadTextureWithCache("evendim", IDR_PNG_EVENDIM_ICON);
    }
    if (!texture_daen) {
        texture_daen = LoadTextureWithCache("daen", IDR_PNG_DAEN_ICON);
    }
    if (!texture_thorkul) {
        texture_thorkul = LoadTextureWithCache("thorkul", IDR_PNG_THORKUL_ICON);
    }
    if (!texture_server) {
        texture_server = LoadTextureWithCache("server", IDR_PNG_SERVER_ICON);
    }

    if (show_calendar_window) {
        int bossCount = 0;
        ImGui::Columns(2, nullptr, false);

        for (auto& pair : bossRespawns) {
            BossRespawn& boss = pair.second;
            ImGui::SeparatorText(boss.name.c_str());

            ImGui::BeginChild(boss.name.c_str(), ImVec2(0, 80), false, ImGuiWindowFlags_NoScrollbar);

            // Calculate the time difference for the next respawn
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            auto diff = std::difftime(boss.nextRespawns[0], now);

            int days = static_cast<int>(diff) / 86400;
            int hours = (static_cast<int>(diff) % 86400) / 3600;
            int minutes = (static_cast<int>(diff) % 3600) / 60;
            int seconds = static_cast<int>(diff) % 60;

            ImGui::Text("Next Spawn in: %d d, %02d h, %02d m, %02d s", days, hours, minutes, seconds);

            // Display the text lines
            for (const auto& respawnTime : boss.nextRespawns) {
                std::tm* tm = std::localtime(&respawnTime);
                char buffer[64];
                std::strftime(buffer, sizeof(buffer), "%A, %Y-%m-%d %H:%M:%S", tm);
                ImGui::Text("%s", buffer);
            }

            // Display the boss image
            if (boss.name == "Evendim" && texture_evendim) {
                ImGui::Image((void*)texture_evendim, ImVec2(70, 70));
            } else if (boss.name == "Daen" && texture_daen) {
                ImGui::Image((void*)texture_daen, ImVec2(70, 70));
            } else if (boss.name == "Thorkul" && texture_thorkul) {
                ImGui::Image((void*)texture_thorkul, ImVec2(70, 70));
            } else if (boss.name == "ServerRestart" && texture_server) {
                ImGui::Image((void*)texture_server, ImVec2(70, 70));
            }

            ImGui::EndChild();

            bossCount++;
            if (bossCount % 2 == 0) {
                ImGui::NextColumn();
            }
        }

        ImGui::Columns(1);
    }
}