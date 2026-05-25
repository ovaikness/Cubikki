#pragma once
#include <cinttypes>

namespace PBE
{
	uint64_t GetTimeMilliseconds();
	uint64_t GetTimeMicroseconds();
	float MillisecondsToSeconds(uint64_t milliseconds);
	float MicrosecondsToSeconds(uint64_t microseconds);
}