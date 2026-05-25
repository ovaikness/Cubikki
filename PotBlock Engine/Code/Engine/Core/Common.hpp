#pragma once
#include <cstdint>
#include <string>

constexpr char const*  PBE_ENGINE_NAME = "PotBlock Engine";

constexpr uint32_t PBE_VERSION_MAJOR = 0;
constexpr uint32_t PBE_VERSION_MINOR = 1;
constexpr uint32_t PBE_VERSION_PATCH = 0;

#define PBE_VERSION_STRING #PBE_VERSION_MAJOR "." #PBE_VERSION_MINOR "." #PBE_VERSION_PATCH

#define PBE_ENABLE_VALIDATION_LAYERS