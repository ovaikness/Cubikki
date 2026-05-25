#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Shape3D.hpp"

#include <array>

namespace PBE
{
	class AABB3 : public Shape3D
	{
	public:
		Vec3 m_Mins;
		Vec3 m_Maxs;

		Vec3 GetNearestPoint(Vec3 const& point) const override;

		bool IsPointInside(Vec3 const& point) const;
		bool DoesSphereIntersect(Sphere const& sphere) const override;
		bool IsOnOrInFrontOfPlane(Plane const& plane) const;


		RaycastResult3D Raycast(Vec3 const& origin, Vec3 const& fwdNormal, float length) const override;

	public:
		~AABB3() = default;
		AABB3() = default;
		AABB3(Vec3 const& min, Vec3 const& max);
		AABB3(AABB3 const& other) = default;

		void Translate(Vec3 const& translation);
		void Enclose(Vec3 const& point);

		bool Intersects(AABB3 const& other) const;
		bool Intersects(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) const;

		std::array<Vec3, 8> GetCorners() const;

		Vec3 GetCenter() const;
		Vec3 GetDimensions() const;
		float GetWidth() const;
		float GetHeight() const;
		float GetDepth() const;

		AABB3 GetTranslated(Vec3 const& translation) const;
	};
}