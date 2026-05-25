#include "Engine/Time/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Time/TimeUtils.hpp"
#include <iostream>
namespace PBE {

	Clock* Clock::s_systemClock = nullptr;

	Clock::Clock(Clock* parent /*= nullptr*/)
		: m_paused(false), m_timeScale(1.0f)
	{
		if (s_systemClock == nullptr)
		{
			s_systemClock = this;
			m_parent = nullptr;
			m_timeMicroseconds = PBE::GetTimeMicroseconds();
			m_timeDeltaMicroseconds = 0;
		}
		else
		{
			m_parent = parent ? parent : s_systemClock;
			m_timeMicroseconds = m_parent->m_timeMicroseconds;
			m_timeDeltaMicroseconds = m_parent->m_timeDeltaMicroseconds;
		}
	}

	Clock::Clock(Clock&& other) noexcept
		: m_paused(other.m_paused), m_timeScale(other.m_timeScale), m_parent(other.m_parent),
		m_children(std::move(other.m_children)), m_timeMicroseconds(other.m_timeMicroseconds),
		m_timeDeltaMicroseconds(other.m_timeDeltaMicroseconds)
	{
		if (m_parent)
		{
			m_parent->RemoveChild(&other); // Remove the old clock from the parent's children (if it has a parent
			m_parent->AddChild(this); // Add this clock to the parent's children
		}

		other.m_parent = nullptr;
	}

	Clock& Clock::operator=(Clock&& other) noexcept
	{
		if (this != &other)
		{
			m_paused = other.m_paused;
			m_timeScale = other.m_timeScale;
			m_parent = other.m_parent;
			m_children = std::move(other.m_children);
			m_timeMicroseconds = other.m_timeMicroseconds;
			m_timeDeltaMicroseconds = other.m_timeDeltaMicroseconds;
			other.m_parent = nullptr;
		}

		if (m_parent)
		{
			m_parent->RemoveChild(&other); // Remove the old clock from the parent's children (if it has a parent
			m_parent->AddChild(this); // Add this clock to the parent's children
		}

		return *this;
	}

	Clock::~Clock()
	{
		if (m_parent)
		{
			m_parent->RemoveChild(this);
			for (auto& child : m_children)
			{
				if (child)
				{
					child->m_parent = m_parent;
					m_parent->AddChild(child);
				}
			}
		}
		else
		{
			// This is the system clock; delete all trailing clocks.
			for (auto& child : m_children)
			{
				delete child;
			}
		}
	}

	void Clock::InitializeSystemClock()
	{
		if (s_systemClock == nullptr)
		{
			s_systemClock = new Clock(nullptr);
		}
	}

	void Clock::Shutdown()
	{
		delete s_systemClock;
		s_systemClock = nullptr;
	}

	Clock* Clock::GetSystemClock()
	{
		return s_systemClock;
	}

	void Clock::TickSystemClock()
	{
		uint64_t delta = PBE::GetTimeMicroseconds() - s_systemClock->m_timeMicroseconds;
		s_systemClock->Tick(delta);
	}

	void Clock::Tick(uint64_t timeDeltaMicroseconds)
	{
		if (!m_paused)
		{
			m_timeDeltaMicroseconds = static_cast<uint64_t>(static_cast<float>(timeDeltaMicroseconds) * m_timeScale);
			m_timeMicroseconds += m_timeDeltaMicroseconds;
			UpdateChildren(m_timeDeltaMicroseconds);
		}
		else
		{
			m_timeDeltaMicroseconds = 0;
		}
	}

	void Clock::UpdateChildren(uint64_t timeDeltaMicroseconds)
	{
		for (auto& child : m_children)
		{
			if (child)
			{
				child->Tick(timeDeltaMicroseconds);
			}
		}
	}

	void Clock::AddChild(Clock* child)
	{
		if (!child)
		{
			throw std::invalid_argument("Child is nullptr");
		}

		for (auto& existingChild : m_children)
		{
			if (existingChild == child)
			{
				throw std::logic_error("Child already exists in this clock");
			}
		}

		child->m_parent = this;
		for (auto& existingChild : m_children)
		{
			if (!existingChild)
			{
				existingChild = child;
				return;
			}
		}

		m_children.push_back(child);
	}

	void Clock::RemoveChild(Clock* child)
	{
		for (auto& existingChild : m_children)
		{
			if (existingChild == child)
			{
				existingChild = nullptr;
				return;
			}
		}
	}

	Clock* Clock::CreateNewChild()
	{
		Clock* newClock = new Clock(this);
		AddChild(newClock);
		return newClock;
	}

	uint64_t Clock::GetTimeMicroseconds() const
	{
		return m_timeMicroseconds;
	}

	uint64_t Clock::GetTimeMilliseconds() const
	{
		return m_timeMicroseconds / 1000;
	}

	float Clock::GetDeltaSeconds() const
	{
		return MicrosecondsToSeconds(m_timeDeltaMicroseconds);
	}

} // namespace PBE
