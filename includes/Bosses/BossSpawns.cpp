// includes/BossSpawns.cpp
#include "BossSpawns.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

const std::unordered_map<std::string, time_t> firstRespawns = {
    {"Thorkul", 1721681749},
    {"Evendim", 1721348488},
    {"Daen", 1721042368},
    {"ServerRestart", 1721210400 + 37 * 60}
};

const int SERVER_RESPAWN_TIME = 168 * 3600; // 1 week in seconds
const int BOSS_RESPAWN_TIME = 109 * 3600; // 109 hours in seconds

std::unordered_map<std::string, BossRespawn> bossRespawns;

time_t getCurrentTimestamp() {
    return std::time(nullptr);
}

std::string unixstamp2human(time_t unixstamp) {
    std::tm* dt = std::localtime(&unixstamp);
    std::ostringstream oss;
    oss << std::put_time(dt, "%A, %B %d, %Y %H:%M");
    return oss.str();
}

void calculateNextRespawns(const std::string& boss) {
    time_t triedRespawn = firstRespawns.at(boss);
    time_t now = getCurrentTimestamp();
    time_t respawnTime = 0;

    bossRespawns[boss].nextRespawns.clear();

    while (true) {
        if (boss == "ServerRestart")
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

void getNextRespawns(const std::string& boss) {
    time_t triedRespawn = firstRespawns.at(boss);
    time_t now = getCurrentTimestamp();
    time_t respawnTime = 0;

    bossRespawns[boss].nextRespawns.clear();

    while (true) {
        if (boss == "ServerRestart")
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