#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Sphere.hpp"
#include "Engine/Math/Plane.hpp"
#include <array>

namespace PBE
{
	bool AABB3::DoesSphereIntersect(Sphere const& sphere) const
	{
		Vec3 nearestPoint = sphere.GetNearestPoint(GetCenter());
		return IsPointInside(nearestPoint);
	}

	AABB3::AABB3(Vec3 const& min, Vec3 const& max)
		: m_Mins(min)
		, m_Maxs(max)
	{
	}

	void AABB3::Translate(Vec3 const& translation)
	{
		m_Mins += translation;
		m_Maxs += translation;
	}

	void AABB3::Enclose(Vec3 const& point)
	{
		if (point.x < m_Mins.x)
		{
			m_Mins.x = point.x;
		}
		if (point.y < m_Mins.y)
		{
			m_Mins.y = point.y;
		}
		if (point.z < m_Mins.z)
		{
			m_Mins.z = point.z;
		}
		if (point.x > m_Maxs.x)
		{
			m_Maxs.x = point.x;
		}
		if (point.y > m_Maxs.y)
		{
			m_Maxs.y = point.y;
		}
		if (point.z > m_Maxs.z)
		{
			m_Maxs.z = point.z;
		}
	}

	bool AABB3::IsPointInside(Vec3 const& point) const
	{
		if (
			point.x >= m_Mins.x && point.x <= m_Maxs.x &&
			point.y >= m_Mins.y && point.y <= m_Maxs.y &&
			point.z >= m_Mins.z && point.z <= m_Maxs.z
			)
		{
			return true;
		}
		return false;
	}

	bool AABB3::Intersects(AABB3 const& other) const
	{
		return Intersects(other.m_Mins.x, other.m_Mins.y, other.m_Mins.z, other.m_Maxs.x, other.m_Maxs.y, other.m_Maxs.z);
	}

	bool AABB3::Intersects(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) const
	{
		bool xIntersects = m_Mins.x <= maxX && m_Maxs.x >= minX;
		bool yIntersects = m_Mins.y <= maxY && m_Maxs.y >= minY;
		bool zIntersects = m_Mins.z <= maxZ && m_Maxs.z >= minZ;

		return xIntersects && yIntersects && zIntersects;
	}

	bool AABB3::IsOnOrInFrontOfPlane(Plane const& plane) const
	{
		Vec3 mostPositivePoint(
			(plane.m_Normal.x >= 0.f) ? m_Maxs.x : m_Mins.x,
			(plane.m_Normal.y >= 0.f) ? m_Maxs.y : m_Mins.y,
			(plane.m_Normal.z >= 0.f) ? m_Maxs.z : m_Mins.z
		);

		float distance = Vec3::DotProduct(plane.m_Normal, mostPositivePoint) - plane.m_Distance;
		if (distance >= 0.f)
		{
			return true;
		}

		return false;
	}

	RaycastResult3D AABB3::Raycast(Vec3 const& origin, Vec3 const& fwdNormal, float length) const
	{
		RaycastResult3D result;

		if (IsPointInside(origin))
		{
			result.m_Hit = true;
			result.m_HitDistance = 0.f;
			result.m_HitNormal = -fwdNormal;
			result.m_HitPoint = origin;

			return result;
		}

		Vec3 displacement = fwdNormal * length;
		Vec3 end = origin + displacement;

		float rangeX = end.x - origin.x;
		float rangeY = end.y - origin.y;
		float rangeZ = end.z - origin.z;

		float rangeXInverse = 1.f / rangeX;
		float rangeYInverse = 1.f / rangeY;
		float rangeZInverse = 1.f / rangeZ;

		float t1 = (m_Mins.x - origin.x) / rangeXInverse;
		float t2 = (m_Maxs.x - origin.x) / rangeXInverse;
		float tmin = std::max(t1, t2);

		t1 = (m_Mins.y - origin.y) / rangeYInverse;
		t2 = (m_Maxs.y - origin.y) / rangeYInverse;

		tmin = std::max(tmin, std::max(t1, t2));

		t1 = (m_Mins.z - origin.z) / rangeZInverse;
		t2 = (m_Maxs.z - origin.z) / rangeZInverse;

		tmin = std::max(tmin, std::max(t1, t2));

		if (tmin > 1.f || tmin < 0.f)
		{
			result.m_Hit = false;
			return result;
		}

		result.m_Hit = true;
		result.m_HitDistance = tmin * length;
		result.m_HitPoint = origin + fwdNormal * result.m_HitDistance;
		result.m_HitNormal = fwdNormal;

		return result;
	}

	Vec3 AABB3::GetNearestPoint(Vec3 const& point) const
	{
		Vec3 nearestPoint = point;
		if (nearestPoint.x < m_Mins.x)
		{
			nearestPoint.x = m_Mins.x;
		}
		else if (nearestPoint.x > m_Maxs.x)
		{
			nearestPoint.x = m_Maxs.x;
		}

		if (nearestPoint.y < m_Mins.y)
		{
			nearestPoint.y = m_Mins.y;
		}
		else if (nearestPoint.y > m_Maxs.y)
		{
			nearestPoint.y = m_Maxs.y;
		}

		if (nearestPoint.z < m_Mins.z)
		{
			nearestPoint.z = m_Mins.z;
		}
		else if (nearestPoint.z > m_Maxs.z)
		{
			nearestPoint.z = m_Maxs.z;
		}

		return nearestPoint;
	}

	std::array<Vec3, 8> AABB3::GetCorners() const
	{
		Vec3 topLeftFront = Vec3(m_Mins.x, m_Maxs.y, m_Maxs.z);
		Vec3 topRightFront = Vec3(m_Maxs.x, m_Maxs.y, m_Maxs.z);
		Vec3 bottomLeftFront = Vec3(m_Mins.x, m_Mins.y, m_Maxs.z);
		Vec3 bottomRightFront = Vec3(m_Maxs.x, m_Mins.y, m_Maxs.z);

		Vec3 topLeftBack = Vec3(m_Mins.x, m_Maxs.y, m_Mins.z);
		Vec3 topRightBack = Vec3(m_Maxs.x, m_Maxs.y, m_Mins.z);
		Vec3 bottomLeftBack = Vec3(m_Mins.x, m_Mins.y, m_Mins.z);
		Vec3 bottomRightBack = Vec3(m_Maxs.x, m_Mins.y, m_Mins.z);

		std::array<Vec3, 8> corners = { 
			topLeftFront, 
			topRightFront, 
			bottomLeftFront, 
			bottomRightFront, 
			topLeftBack, 
			topRightBack, 
			bottomLeftBack, 
			bottomRightBack 
		};

		return corners;
	}

	Vec3 AABB3::GetCenter() const
	{
		Vec3 center(
			(m_Mins.x + m_Maxs.x) * 0.5f,
			(m_Mins.y + m_Maxs.y) * 0.5f,
			(m_Mins.z + m_Maxs.z) * 0.5f
		);

		return center;
	}

	PBE::Vec3 AABB3::GetDimensions() const
	{
		Vec3 dimensions(
			m_Maxs.x - m_Mins.x,
			m_Maxs.y - m_Mins.y,
			m_Maxs.z - m_Mins.z
		);

		return dimensions;
	}

	float AABB3::GetWidth() const
	{
		return m_Maxs.x - m_Mins.x;
	}

	float AABB3::GetHeight() const
	{
		return m_Maxs.y - m_Mins.y;
	}

	float AABB3::GetDepth() const
	{
		return m_Maxs.z - m_Mins.z;
	}

	AABB3 AABB3::GetTranslated(Vec3 const& translation) const
	{
		AABB3 other = *this;
		other.Translate(translation);
		return other;
	}

}