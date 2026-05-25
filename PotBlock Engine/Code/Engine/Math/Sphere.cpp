#include "Sphere.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/MathUtils.hpp"

namespace PBE
{
	Sphere::~Sphere()
	{
	}

	Sphere::Sphere(Vec3 const& center, float radius)
		: m_radius(radius)
		, m_Center(center)
	{
	}

	Sphere::Sphere(Sphere const& sphere)
		: m_radius(sphere.m_radius)
		, m_Center(sphere.m_Center)
	{
	}

	bool Sphere::IsOnOrInFrontOfPlane(Plane const& plane) const
	{
		float distance = plane.GetSignedDistanceToPoint(m_Center);
		return distance > -m_radius;
	}

	bool Sphere::IsPointInside(Vec3 const& point) const
	{
		Vec3 displacement = point - m_Center;
		return displacement.GetLengthSquared() <= (m_radius * m_radius);
	}

	PBE::Vec3 Sphere::GetNearestPoint(Vec3 const& point) const
	{
		Vec3 displacement = point - m_Center;
		float distance = displacement.GetLength();

		if (distance <= m_radius)
		{
			return point;
		}
		else
		{
			return m_Center + displacement.GetNormalized() * m_radius;
		}
	}

	bool Sphere::DoesSphereIntersect(Sphere const& sphere) const
	{
		Vec3 displacement = sphere.m_Center - m_Center;
		float distanceSquared = displacement.GetLengthSquared();

		return distanceSquared <= (m_radius + sphere.m_radius) * (m_radius + sphere.m_radius);
	}

	PBE::RaycastResult3D Sphere::Raycast(Vec3 const& origin, Vec3 const& fwdNormal, float length) const
	{
		RaycastResult3D result 
		{
			.m_Hit = false,
			.m_HitDistance = 0.f,
			.m_HitPoint = Vec3::ZERO,
			.m_HitNormal = Vec3::ZERO
		};

		Vec3 displacement = m_Center - origin;

		float projection = displacement.Dot(fwdNormal);

		if (projection < 0.f)
		{
			return result;
		}

		Vec3 rejection = displacement - projection * fwdNormal;

		float rejectionLength = rejection.GetLength();

		if (rejectionLength > m_radius)
		{
			return result;
		}

		float hitDistance = projection - PBE::Sqrt(m_radius * m_radius - rejectionLength * rejectionLength);

		if (hitDistance > length)
		{
			return result;
		}

		result.m_HitPoint = origin + fwdNormal * hitDistance;
		result.m_HitNormal = (result.m_HitPoint - m_Center).GetNormalized();
		result.m_HitDistance = hitDistance;
		result.m_Hit = true;

		return result;
	}

	Sphere::Sphere()
		: m_radius(1.f)
		, m_Center(0.f, 0.f, 0.f)
	{
	}
}