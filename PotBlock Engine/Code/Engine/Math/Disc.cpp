#include "Engine/Math/Disc.hpp"

namespace PBE
{
	Disc::Disc(Vec2 center, float radius)
		: m_Center(center)
		, m_Radius(radius)
	{
	}

	bool Disc::IsPointInside(Vec2 const& point) const
	{
		float distance = (point - m_Center).GetLengthSquared();
		return distance <= (m_Radius * m_Radius);
	}

	Vec2 Disc::GetNearestPoint(float x, float y) const
	{
		Vec2 point(x, y);
		return GetNearestPoint(point);
	}

	bool Disc::Intersects(Disc const& other) const
	{
		float distance = (other.m_Center - m_Center).GetLengthSquared();
		float radiusSum = m_Radius + other.m_Radius;
		return distance <= (radiusSum * radiusSum);
	}

	Vec2 Disc::GetNearestPoint(Vec2 const& point) const
	{
		Vec2 displacement = point - m_Center;
		if (displacement.GetLengthSquared() < m_Radius * m_Radius)
		{
			return point;
		}
		else
		{
			Vec2 direction = displacement.GetNormalized();
			return m_Center + (direction * m_Radius);
		}
	}

	bool Disc::IsPointInside(float x, float y) const
	{
		Vec2 point = Vec2(x, y);
		return IsPointInside(point);
	}
}