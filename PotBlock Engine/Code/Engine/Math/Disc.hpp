#pragma once

#include "Engine/Math/Vec2.hpp"

namespace PBE
{
	class Disc
	{
	public:
		float m_Radius;
		Vec2  m_Center;

		Disc(Vec2 center, float radius);

		bool IsPointInside(Vec2 const& point) const;
		bool IsPointInside(float x, float y) const;

		Vec2 GetNearestPoint(float x, float y) const;
		Vec2 GetNearestPoint(Vec2 const& point) const;

		bool Intersects(Disc const& other) const;
	};
}