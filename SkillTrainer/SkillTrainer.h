#pragma once

#ifndef SKILL_TRAINER_H
#define SKILL_TRAINER_H

#include "../libs/imgui/imgui.h"
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "../includes/API/ApiHandler.h"


// This array of skills is used in ShowSkillTrainer to list available options
static const char* skills[] = {"Hunter", "Marksman", "Conjurer", "Warlock", "Barbarian", "Knight"};

// Function declaration to show the Skill Trainer window in ImGui
void ShowSkilltrainer(bool& show_Skilltrainer_window, ImVec4 textColor);


#endif // SKILL_TRAINER_H