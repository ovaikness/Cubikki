#pragma once

namespace PBE
{
	struct FloatRange
	{
	public:
		float m_Min;
		float m_Max;
	public:
		FloatRange(float min = 0.f, float max = 0.f)
			: m_Min(min)
			, m_Max(max)
		{
		}

		bool IsInRange(float value) const
		{
			return value >= m_Min && value <= m_Max;
		}

		bool Overlaps(FloatRange const& other) const
		{
			return IsInRange(other.m_Min) || IsInRange(other.m_Max);
		}

		float GetMagnitude() const
		{
			return m_Max - m_Min;
		}

		float GetClamped(float value) const
		{
			if (value < m_Min)
			{
				return m_Min;
			}
			if (value > m_Max)
			{
				return m_Max;
			}
			return value;
		}
	};
}