// includes/BossSpawns.h
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include "../Logger/Logger.h"

struct BossRespawn {
    std::string name;
    std::vector<time_t> nextRespawns;
    time_t previousRespawn;
};

extern const std::unordered_map<std::string, time_t> firstRespawns;
extern const int SERVER_RESPAWN_TIME;
extern const int BOSS_RESPAWN_TIME;
extern std::unordered_map<std::string, BossRespawn> bossRespawns;

time_t getCurrentTimestamp();
void calculateNextRespawns(const std::string& boss);
std::string unixstamp2human(time_t unixstamp);