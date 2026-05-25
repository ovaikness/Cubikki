#include "Engine/Time/TimeUtils.hpp"

#include <chrono>

uint64_t PBE::GetTimeMilliseconds()
{
	return std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::steady_clock::now().time_since_epoch()).count();
}

uint64_t PBE::GetTimeMicroseconds()
{
	return std::chrono::duration_cast<std::chrono::microseconds> (std::chrono::steady_clock::now().time_since_epoch()).count();
}

float PBE::MicrosecondsToSeconds(uint64_t microseconds)
{
	return static_cast<float>(microseconds) / 1000000.f;
}

float PBE::MillisecondsToSeconds(uint64_t milliseconds)
{
	return static_cast<float>(milliseconds) / 1000.f;
}
