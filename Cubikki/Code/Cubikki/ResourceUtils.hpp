#pragma once

#include <unordered_map>

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Scene/SkeletalModel.hpp"

extern std::unordered_map<std::string, PBE::Sound*> g_Sounds;
extern std::unordered_map<std::string, PBE::SkeletalModel*> g_Models;