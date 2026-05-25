#include "Engine/Math/Frustum.hpp"

#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Sphere.hpp"
#include "Engine/Math/Plane.hpp"

namespace PBE
{
	Frustum::~Frustum()
	{

	}

	Frustum::Frustum(
		Plane const& m_Left, Plane const& m_Right, 
		Plane const& m_Top, Plane const& m_Bottom, 
		Plane const& m_Near, Plane const& m_Far)
		: m_Left(m_Left), m_Right(m_Right)
		, m_Top(m_Top), m_Bottom(m_Bottom)
		, m_Near(m_Near), m_Far(m_Far)
	{

	}

	Frustum::Frustum(Frustum const& other)
		: m_Left(other.m_Left), m_Right(other.m_Right)
		, m_Top(other.m_Top), m_Bottom(other.m_Bottom)
		, m_Near(other.m_Near), m_Far(other.m_Far)
	{

	}

	bool Frustum::IsAABB3Inside(AABB3 const& aabb) const
	{
		bool isInside = aabb.IsOnOrInFrontOfPlane(m_Left) && aabb.IsOnOrInFrontOfPlane(m_Right)
			&& aabb.IsOnOrInFrontOfPlane(m_Top) && aabb.IsOnOrInFrontOfPlane(m_Bottom)
			&& aabb.IsOnOrInFrontOfPlane(m_Near) && aabb.IsOnOrInFrontOfPlane(m_Far);
		return isInside;
	}

	bool Frustum::IsSphereInside(Sphere const& sphere) const
	{
		bool isInside = sphere.IsOnOrInFrontOfPlane(m_Left) && sphere.IsOnOrInFrontOfPlane(m_Right)
			&& sphere.IsOnOrInFrontOfPlane(m_Top) && sphere.IsOnOrInFrontOfPlane(m_Bottom)
			&& sphere.IsOnOrInFrontOfPlane(m_Near) && sphere.IsOnOrInFrontOfPlane(m_Far);
		return isInside;
	}

	bool Frustum::IsPointInside(Vec3 const& point) const
	{
		bool isInside = m_Left.GetSignedDistanceToPoint(point) >= 0.f
			&& m_Right.GetSignedDistanceToPoint(point) >= 0.f
			&& m_Top.GetSignedDistanceToPoint(point) >= 0.f
			&& m_Bottom.GetSignedDistanceToPoint(point) >= 0.f
			&& m_Near.GetSignedDistanceToPoint(point) >= 0.f
			&& m_Far.GetSignedDistanceToPoint(point) >= 0.f;

		return isInside;
	}

}