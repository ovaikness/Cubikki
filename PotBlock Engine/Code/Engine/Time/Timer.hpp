#pragma once
#include <cinttypes>

namespace PBE
{
	class Clock;
	class Timer
	{
	public:
		uint64_t m_periodMicroseconds = 0;
	public:
		Timer();
		Timer(Clock* clock,float m_periodSeconds);
		~Timer();

		void Start();
		void Stop();

		bool IsPaused() const;
		float GetElapsedFraction() const;
		float GetPeriodSeconds() const;
		bool DecrementPeriodIfElapsed();
	protected:
		Clock* m_clock = nullptr;
		uint64_t m_startTimeMicroseconds = (~0ui64);
	};
}