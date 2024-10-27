// includes/BossSpawns.h
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>

struct BossRespawn {
    std::string name;
    std::vector<time_t> nextRespawns;
    time_t previousRespawn;
};

const std::unordered_map<std::string, time_t> firstRespawns = {
    {"thorkul", 1721681749},
    {"evendim", 1721348488},
    {"daen", 1721042368},
    {"server", 1721210400 + 37 * 60}
};

const int SERVER_RESPAWN_TIME = 168 * 3600; // 1 week in seconds
const int BOSS_RESPAWN_TIME = 109 * 3600; // 109 hours in seconds

std::unordered_map<std::string, BossRespawn> bossRespawns;