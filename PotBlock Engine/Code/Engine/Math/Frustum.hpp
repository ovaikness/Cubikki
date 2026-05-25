#pragma once
#include "Engine/Math/Plane.hpp"
namespace PBE
{
	class AABB3;
	class Camera;
	class Sphere;

	class Frustum
	{
	public:
		~Frustum();
		Frustum() = default;
		Frustum(
			Plane const& m_Left, Plane const& m_Right, 
			Plane const& m_Top,  Plane const& m_Bottom, 
			Plane const& m_Near, Plane const& m_Far);
		Frustum(Frustum const& other);

		Plane m_Left;
		Plane m_Right;
		Plane m_Top;
		Plane m_Bottom;
		Plane m_Near;
		Plane m_Far;

		bool IsAABB3Inside(AABB3 const& aabb) const;
		bool IsSphereInside(Sphere const& sphere) const;

		bool IsPointInside(Vec3 const& point) const;
	};
}