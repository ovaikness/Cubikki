#pragma once
#include "Engine/Math/Vec2.hpp"

namespace PBE
{
	class AABB2
	{
	public:
		static AABB2 const ZERO_TO_ONE;
		Vec2 m_Mins;
		Vec2 m_Maxs;
	public:
		~AABB2() = default;
		AABB2() = default;
		AABB2(float minX, float minY, float maxX, float maxY);
		AABB2(Vec2 const& min, Vec2 const& max);
		AABB2(AABB2 const& other) = default;

		void Translate(Vec2 const& translation);

		bool IsPointInside(Vec2 const& point) const;
		bool IsPointInside(float x, float y) const;

		bool Intersects(AABB2 const& other) const;
		bool Intersects(float minX, float minY, float maxX, float maxY) const;

		Vec2 GetNearestPoint(float x, float y) const;
		Vec2 GetNearestPoint(Vec2 const& point) const;

		Vec2 GetCenter() const;
		Vec2 GetDimensions() const;

		float GetWidth() const;
		float GetHeight() const;

		Vec2 GetPositionAtUV(Vec2 const& uv) const;
	};
}