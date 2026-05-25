#include "Engine/Math/OBB2.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"

namespace PBE
{
	OBB2::OBB2(Vec2 const& center, float width, float height, Vec2 const& iBasis)
		: OBB2(center, Vec2(width, height), iBasis)
	{
	}

	OBB2::OBB2(Vec2 const& center, float width, float height, float rotationDegrees)
		: OBB2(center, Vec2(width, height), rotationDegrees)
	{
	}

	OBB2::OBB2(Vec2 const& center, Vec2 const& extents, float rotationDegrees)
		: m_Center(center)
		, m_Extents(extents)
		, m_IBasis(Vec2::MakeFromPolarDegrees(rotationDegrees))
	{
	}

	OBB2::OBB2(Vec2 const& center, Vec2 const& extents, Vec2 const& iBasis)
		: m_Center(center)
		, m_Extents(extents)
		, m_IBasis(iBasis)
	{
	}

	void OBB2::SetCenter(Vec2 const& center)
	{
		m_Center += center;
	}

	void OBB2::SetExtents(Vec2 const& extents)
	{
		m_Extents = extents;
	}

	void OBB2::SetRotationDegrees(float rotationDegrees)
	{
		m_IBasis = Vec2::MakeFromPolarDegrees(rotationDegrees);
	}

	float const OBB2::GetRotationDegrees() const
	{
		return m_IBasis.GetAngleDegrees();
	}

	float const OBB2::GetRotationRadians() const
	{
		return m_IBasis.GetAngle();
	}

	Vec2 const OBB2::GetIBasis() const
	{
		return m_IBasis;
	}

	Vec2 const OBB2::GetJBasis() const
	{
		return m_IBasis.GetRotated90Degrees();
	}

	bool OBB2::IsPointInside(Vec2 const& point) const
	{
		Vec2 translatedPoint = point - m_Center;
		float lengthInIBasis = PBE::Abs(translatedPoint.GetProjectedLength(m_IBasis));
		float lengthInJBasis = PBE::Abs(translatedPoint.GetProjectedLength(GetJBasis()));

		return lengthInIBasis <= m_Extents.x * 0.5f && lengthInJBasis <= m_Extents.y * 0.5f;
	}

	Vec2 OBB2::GetNearestPoint(float x, float y) const
	{
		Vec2 translatedPoint = Vec2(x, y) - m_Center;
		float transformedPointX = translatedPoint.GetProjectedLength(m_IBasis);
		float transformedPointY = translatedPoint.GetProjectedLength(GetJBasis());
		return Vec2(
			PBE::Clamp(transformedPointX, -m_Extents.x * 0.5f, m_Extents.x * 0.5f),
			PBE::Clamp(transformedPointY, -m_Extents.y * 0.5f, m_Extents.y * 0.5f)
		);
	}

	bool OBB2::Intersects(OBB2 const& other) const
	{
		Vec2 axes[4] = { m_IBasis, GetJBasis(), other.m_IBasis, other.GetJBasis() };

		// Separating Axis Theorem
		for (int i = 0; i < 4; ++i)
		{
			Vec2 axis = axes[i];
			float lengthInIBasis = m_Extents.x * 0.5f * axis.GetProjectedLength(m_IBasis);
			float lengthInJBasis = m_Extents.y * 0.5f * axis.GetProjectedLength(GetJBasis());
			float otherLengthInIBasis = other.m_Extents.x * 0.5f * axis.GetProjectedLength(other.m_IBasis);
			float otherLengthInJBasis = other.m_Extents.y * 0.5f * axis.GetProjectedLength(other.GetJBasis());

			float totalLength = lengthInIBasis + lengthInJBasis;
			float otherTotalLength = otherLengthInIBasis + otherLengthInJBasis;
			float distance = abs((m_Center - other.m_Center).GetProjectedLength(axis));

			if (distance > totalLength + otherTotalLength)
			{
				return false;
			}
		}

		return true;
	}

	bool OBB2::Intersects(AABB2 const& other) const
	{
		return Intersects(other.m_Mins.x, other.m_Mins.y, other.m_Maxs.x, other.m_Maxs.y);
	}

	Vec2 OBB2::GetCenter() const
	{
		return m_Center;
	}

	Vec2 OBB2::GetDimensions() const
	{
		return m_Extents;
	}

	bool OBB2::Intersects(float minX, float minY, float maxX, float maxY) const
	{
		OBB2 otherAABB(Vec2(minX, minY), Vec2(maxX - minX, maxY - minY), 0.f);
		return Intersects(otherAABB);
	}

	Vec2 OBB2::GetNearestPoint(Vec2 const& point) const
	{
		Vec2 translatedPoint = point - m_Center;
		float transformedPointX = translatedPoint.GetProjectedLength(m_IBasis);
		float transformedPointY = translatedPoint.GetProjectedLength(GetJBasis());
		return Vec2(
			PBE::Clamp(transformedPointX, -m_Extents.x * 0.5f, m_Extents.x * 0.5f),
			PBE::Clamp(transformedPointY, -m_Extents.y * 0.5f, m_Extents.y * 0.5f)
		) + m_Center;
	}

	bool OBB2::IsPointInside(float x, float y) const
	{
		Vec2 point(x, y);
		return IsPointInside(point);
	}

	void OBB2::SetExtents(float x, float y)
	{
		m_Extents = Vec2(x, y);
	}

	void OBB2::SetCenter(float x, float y)
	{
		m_Center = Vec2(x, y);
	}

	float OBB2::GetWidth() const
	{
		return m_Extents.x;
	}

	float OBB2::GetHeight() const
	{
		return m_Extents.y;
	}
}
