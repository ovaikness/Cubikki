#include "Engine/Math/AABB2.hpp"

namespace PBE
{
	AABB2 const AABB2::ZERO_TO_ONE(Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f));

	AABB2::AABB2(float minX, float minY, float maxX, float maxY)
		: m_Mins(minX, minY)
		, m_Maxs(maxX, maxY)
	{
	}

	AABB2::AABB2(Vec2 const& min, Vec2 const& max)
		: m_Mins(min)
		, m_Maxs(max)
	{

	}

	void AABB2::Translate(Vec2 const& translation)
	{
		m_Mins += translation;
		m_Maxs += translation;
	}

	bool AABB2::IsPointInside(Vec2 const& point) const
	{
		return IsPointInside(point.x, point.y);
	}

	bool AABB2::IsPointInside(float x, float y) const
	{
		return x >= m_Mins.x && x <= m_Maxs.x && y >= m_Mins.y && y <= m_Maxs.y;
	}

	bool AABB2::Intersects(AABB2 const& other) const
	{
		return Intersects(other.m_Mins.x, other.m_Mins.y, other.m_Maxs.x, other.m_Maxs.y);
	}

	bool AABB2::Intersects(float minX, float minY, float maxX, float maxY) const
	{
		return m_Mins.x <= maxX && m_Maxs.x >= minX && m_Mins.y <= maxY && m_Maxs.y >= minY;
	}

	Vec2 AABB2::GetNearestPoint(Vec2 const& point) const
	{
		return GetNearestPoint(point.x, point.y);
	}

	Vec2 AABB2::GetNearestPoint(float x, float y) const
	{
		float nearestX = x;
		float nearestY = y;

		if (x < m_Mins.x)
		{
			nearestX = m_Mins.x;
		}
		else if (x > m_Maxs.x)
		{
			nearestX = m_Maxs.x;
		}

		if (y < m_Mins.y)
		{
			nearestY = m_Mins.y;
		}
		else if (y > m_Maxs.y)
		{
			nearestY = m_Maxs.y;
		}

		return Vec2(nearestX, nearestY);
	}

	Vec2 AABB2::GetCenter() const
	{
		return Vec2((m_Mins.x + m_Maxs.x) * 0.5f, (m_Mins.y + m_Maxs.y) * 0.5f);
	}

	Vec2 AABB2::GetDimensions() const
	{
		return Vec2(m_Maxs.x - m_Mins.x, m_Maxs.y - m_Mins.y);
	}
	float AABB2::GetWidth() const
	{
		return m_Maxs.x - m_Mins.x;
	}
	float AABB2::GetHeight() const
	{
		return m_Maxs.y - m_Mins.y;
	}
	Vec2 AABB2::GetPositionAtUV(Vec2 const& uv) const
	{
		float x = m_Mins.x + uv.x * (m_Maxs.x - m_Mins.x);
		float y = m_Mins.y + uv.y * (m_Maxs.y - m_Mins.y);
		return Vec2(x, y);
	}
}