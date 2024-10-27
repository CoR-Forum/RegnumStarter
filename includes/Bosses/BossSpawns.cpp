// includes/BossSpawns.cpp
#include "BossSpawns.h"
#include <iostream>

time_t getCurrentTimestamp() {
    return std::time(nullptr);
}

void calculateNextRespawns(const std::string& boss) {
    time_t triedRespawn = firstRespawns.at(boss);
    time_t now = getCurrentTimestamp();
    time_t respawnTime = 0;

    bossRespawns[boss].nextRespawns.clear();

    while (true) {
        if (boss == "server")
            respawnTime = SERVER_RESPAWN_TIME;
        else
            respawnTime = BOSS_RESPAWN_TIME;

        triedRespawn += respawnTime;
        if (triedRespawn >= now)
            bossRespawns[boss].nextRespawns.push_back(triedRespawn);

        if (bossRespawns[boss].nextRespawns.size() == 3)
            break;
    }

    bossRespawns[boss].previousRespawn = bossRespawns[boss].nextRespawns[0] - respawnTime;
    std::cout << boss << " previous respawn (to put in js file) is " << bossRespawns[boss].previousRespawn << std::endl;
}

void initializeBossRespawns() {
    for (const auto& pair : firstRespawns) {
        bossRespawns[pair.first] = BossRespawn{pair.first, {}, pair.second};
        calculateNextRespawns(pair.first);
    }
}