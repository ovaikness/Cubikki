#pragma once
#include "Engine/Math/Shape3D.hpp"

namespace PBE
{
	class Plane;
	class Sphere : public Shape3D
	{
	public:
		Vec3  m_Center;
		float m_radius;
	public:
		~Sphere();
		Sphere();
		Sphere(Vec3 const& center, float radius);
		Sphere(Sphere const& sphere);

		bool IsOnOrInFrontOfPlane(Plane const& plane) const;
		bool IsPointInside(Vec3 const& point) const;
		Vec3 GetNearestPoint(Vec3 const& point) const override;
		bool DoesSphereIntersect(Sphere const& sphere) const override;


		RaycastResult3D Raycast(Vec3 const& origin, Vec3 const& direction, float length) const override;

	};
}