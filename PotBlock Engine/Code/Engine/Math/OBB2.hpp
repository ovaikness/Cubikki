#pragma once

#include "Engine/Math/Vec2.hpp"

namespace PBE
{
	class AABB2;
	class OBB2
	{
	public:
		Vec2 m_IBasis;
		Vec2 m_Center;
		Vec2 m_Extents;
	public:
		~OBB2() = default;
		OBB2() = default;
		OBB2(Vec2 const& center, float width, float height, float rotationDegrees);
		OBB2(Vec2 const& center, float width, float height, Vec2 const& iBasis);
		OBB2(Vec2 const& center, Vec2 const& extents, float rotationDegrees);
		OBB2(Vec2 const& center, Vec2 const& extents, Vec2 const& iBasis);
		OBB2(OBB2 const& other) = default;

		void SetCenter(Vec2 const& center);
		void SetCenter(float x, float y);

		void SetExtents(Vec2 const& extents);
		void SetExtents(float x, float y);

		void SetRotationDegrees(float rotationDegrees);

		float const GetRotationDegrees() const;
		float const GetRotationRadians() const;

		Vec2 const GetIBasis() const;
		Vec2 const GetJBasis() const;

		bool IsPointInside(Vec2 const& point) const;
		bool IsPointInside(float x, float y) const;
		Vec2 GetNearestPoint(float x, float y) const;
		Vec2 GetNearestPoint(Vec2 const& point) const;

		bool Intersects(OBB2 const& other) const;
		bool Intersects(AABB2 const& other) const;
		bool Intersects(float minX, float minY, float maxX, float maxY) const;

		Vec2 GetCenter() const;
		Vec2 GetDimensions() const;

		float GetWidth() const;
		float GetHeight() const;
	};
}