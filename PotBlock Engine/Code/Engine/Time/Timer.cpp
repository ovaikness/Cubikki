#include "Engine/Time/Timer.hpp"
#include "Engine/Time/Clock.hpp"
namespace PBE
{

	Timer::Timer(Clock* clock, float m_periodSeconds)
		: m_periodMicroseconds(static_cast<uint64_t>(m_periodSeconds * 1000000.f))
	{
		if (clock == nullptr)
		{
			m_clock = Clock::GetSystemClock();
		}
		else
		{
			m_clock = clock;
		}
		m_startTimeMicroseconds = m_clock->GetTimeMicroseconds();
	}

	Timer::Timer()
		: Timer(Clock::GetSystemClock(), 0.f)
	{
	}

	Timer::~Timer()
	{

	}

	void Timer::Start()
	{
		m_startTimeMicroseconds = m_clock->GetTimeMicroseconds();
	}

	void Timer::Stop()
	{
		m_startTimeMicroseconds = ~0ui64;
	}

	bool Timer::IsPaused() const
	{
		return m_startTimeMicroseconds == ~0ui64;
	}

	float Timer::GetElapsedFraction() const
	{
		uint64_t currentTime = m_clock->GetTimeMicroseconds();
		return static_cast<float>(currentTime - m_startTimeMicroseconds) / 1000000.f;
	}

	float Timer::GetPeriodSeconds() const
	{
		return MicrosecondsToSeconds(m_periodMicroseconds);
	}

	bool Timer::DecrementPeriodIfElapsed()
	{
		if (m_startTimeMicroseconds == ~0ui64)
		{
			return false;
		}

		uint64_t currentTime = m_clock->GetTimeMicroseconds();
		if (currentTime - m_startTimeMicroseconds >= m_periodMicroseconds)
		{
			m_startTimeMicroseconds = currentTime;
			return true;
		}

		return false;
	}
}