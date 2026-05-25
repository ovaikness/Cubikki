#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/RaycastResult3D.hpp"
#include "Engine/Math/HitResult3D.hpp"

namespace PBE
{
	class Sphere;
	class AABB3;
	class OBB3;
	class Capsule3;

	class Shape3D
	{
	public:
		virtual ~Shape3D() = default;
		virtual Vec3 GetNearestPoint(Vec3 const& point) const = 0;

		virtual bool IsPointInside(Vec3 const& point) const = 0;
		virtual bool IsOnOrInFrontOfPlane(Plane const& plane) const = 0;

		bool DoesShapeIntersect(Shape3D const& shape) const;
		virtual bool DoesSphereIntersect(Sphere const& sphere) const = 0;
		//virtual bool DoesAABBIntersect(AABB3 const& aabb) const = 0;
		//virtual bool DoesOBBIntersect(OBB3 const& obb) const = 0;
		//virtual bool DoesCapsuleIntersect(Capsule3 const& capsule) const = 0;

		//virtual HitResult3D GetHitResult(Sphere const& sphere) const = 0;
		//virtual HitResult3D GetHitResult(AABB3 const& aabb) const = 0;
		//virtual HitResult3D GetHitResult(OBB3 const& obb) const = 0;
		//virtual HitResult3D GetHitResult(Capsule3 const& capsule) const = 0;

		virtual RaycastResult3D Raycast(Vec3 const& origin, Vec3 const& fwdNormal, float length) const = 0;
	};
}