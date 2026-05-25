#pragma once

#include "Engine/Time/TimeUtils.hpp"
#include <vector>
#include <memory>

namespace PBE
{
	class Clock
	{
	public:
		Clock(Clock&& other) noexcept;
		Clock& operator=(Clock&& other) noexcept;
		Clock(Clock const&) = delete;
		Clock& operator=(Clock const&) = delete;

		~Clock();

		static void InitializeSystemClock();
		static void Shutdown();
		static Clock* GetSystemClock();
		static void TickSystemClock();

		Clock* CreateNewChild();
		uint64_t GetTimeMicroseconds() const;
		uint64_t GetTimeMilliseconds() const;
		float GetDeltaSeconds() const;

	private:
		static Clock* s_systemClock;
		Clock(Clock* parent = nullptr);

		void Tick(uint64_t timeDeltaMilliseconds);
		void UpdateChildren(uint64_t timeDeltaMilliseconds);
		void AddChild(Clock* child);
		void RemoveChild(Clock* child);

	private:
		bool m_paused = false;
		float m_timeScale = 1.0f;

		Clock* m_parent = nullptr;
		std::vector<Clock*> m_children;
		uint64_t m_timeMicroseconds = 0;
		uint64_t m_timeDeltaMicroseconds = 0;
	};
}
