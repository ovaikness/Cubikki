#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Math/FloatRange.hpp"

PBE::OBB3::OBB3(Vec3 const& center, float width, float height, float depth, EulerAngles const& orientation)
	: m_Center(center)
	, m_Extents(Vec3(width, height, depth))
{
	orientation.GetAsVectors(&m_IBasis, &m_JBasis, &m_KBasis);
}

PBE::OBB3::OBB3(Vec3 const& center, float width, float height, float depth, Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis)
	: m_Center(center)
	, m_Extents(Vec3(width, height, depth))
	, m_IBasis(iBasis)
	, m_JBasis(jBasis)
	, m_KBasis(kBasis)
{

}

PBE::OBB3::OBB3(Vec3 const& center, float width, float height, float depth, Mat4 const& transform)
	: m_Center(center)
	, m_Extents(Vec3(width, height, depth))
{
	m_IBasis = transform.GetIBasis3D();
	m_JBasis = transform.GetJBasis3D();
	m_KBasis = transform.GetKBasis3D();
}

PBE::OBB3::OBB3(Vec3 const& center, Vec3 const& extents, Mat4 const& transform)
	: m_Center(center)
	, m_Extents(extents)
{
	m_IBasis = transform.GetIBasis3D();
	m_JBasis = transform.GetJBasis3D();
	m_KBasis = transform.GetKBasis3D();
}

PBE::OBB3::OBB3(AABB3 const& aabb)
{
	m_Center = aabb.GetCenter();
	m_Extents = aabb.GetDimensions();
	m_IBasis = Vec3(1.f, 0.f, 0.f);
	m_JBasis = Vec3(0.f, 1.f, 0.f);
	m_KBasis = Vec3(0.f, 0.f, 1.f);
}

void PBE::OBB3::SetCenter(Vec3 const& center)
{
	m_Center = center;
}

void PBE::OBB3::SetExtents(Vec3 const& extents)
{
	m_Extents = extents;
}

std::vector<PBE::Vec3> PBE::OBB3::GetCorners() const
{
	std::vector<PBE::Vec3> results;
	PBE::Vec3 iExtents = m_IBasis * m_Extents.x * 0.5f;
	PBE::Vec3 jExtents = m_JBasis * m_Extents.y * 0.5f;
	PBE::Vec3 kExtents = m_KBasis * m_Extents.z * 0.5f;

	results.push_back(m_Center + iExtents + jExtents + kExtents);
	results.push_back(m_Center + iExtents + jExtents - kExtents);
	results.push_back(m_Center + iExtents - jExtents + kExtents);
	results.push_back(m_Center + iExtents - jExtents - kExtents);
	results.push_back(m_Center - iExtents + jExtents + kExtents);
	results.push_back(m_Center - iExtents + jExtents - kExtents);
	results.push_back(m_Center - iExtents - jExtents + kExtents);
	results.push_back(m_Center - iExtents - jExtents - kExtents);

	return results;
}

PBE::Vec3 const PBE::OBB3::GetIBasis() const
{
	return m_IBasis;
}

PBE::Vec3 const PBE::OBB3::GetJBasis() const
{
	return m_JBasis;
}

PBE::Vec3 const PBE::OBB3::GetKBasis() const
{
	return m_KBasis;
}

bool PBE::OBB3::IsPointInside(Vec3 const& point) const
{
	Vec3 displacement = point - m_Center;
	float iDist = Vec3::DotProduct(displacement, m_IBasis);
	if (fabs(iDist) > m_Extents.x * 0.5f)
	{
		return false;
	}
	float jDist = Vec3::DotProduct(displacement, m_JBasis);
	if (fabs(jDist) > m_Extents.y * 0.5f)
	{
		return false;
	}
	float kDist = Vec3::DotProduct(displacement, m_KBasis);
	if (fabs(kDist) > m_Extents.z * 0.5f)
	{
		return false;
	}
	return true;
}

bool PBE::OBB3::IsPointInside(float x, float y, float z) const
{
	return IsPointInside(Vec3(x, y, z));
}

PBE::Vec3 PBE::OBB3::GetNearestPoint(float x, float y, float z) const
{
	return GetNearestPoint(Vec3(x, y, z));
}

PBE::Vec3 PBE::OBB3::GetNearestPoint(Vec3 const& point) const
{
	Vec3 displacement = point - m_Center;
	float iDist = Vec3::DotProduct(displacement, m_IBasis);
	float jDist = Vec3::DotProduct(displacement, m_JBasis);
	float kDist = Vec3::DotProduct(displacement, m_KBasis);

	float iClamped = Clamp(iDist, -m_Extents.x * 0.5f, m_Extents.x * 0.5f);
	float jClamped = Clamp(jDist, -m_Extents.y * 0.5f, m_Extents.y * 0.5f);
	float kClamped = Clamp(kDist, -m_Extents.z * 0.5f, m_Extents.z * 0.5f);

	return m_Center + iClamped * m_IBasis + jClamped * m_JBasis + kClamped * m_KBasis;
}

bool PBE::OBB3::Intersects(AABB3 const& other) const
{
	Vec3 center = other.GetCenter();
	OBB3 otherOBB(center, other.GetWidth(), other.GetHeight(), other.GetDepth(), Vec3(1.f,0.f,0.f), Vec3(0.f,1.f,0.f), Vec3(0.f,0.f,1.f));
	return Intersects(otherOBB);
}

PBE::RaycastResult3D PBE::OBB3::Raycast(Vec3 const& origin, Vec3 const& fwdNormal, float maxDist) const
{
	RaycastResult3D result{};
	result.m_Hit = false;
	result.m_HitDistance = 0.f;

	// Assume fwdNormal is normalized. If not, normalize + scale maxDist accordingly.

	// Early out if starting inside
	if (IsPointInside(origin))
	{
		result.m_Hit = true;
		result.m_HitNormal = -fwdNormal;                 // exiting immediately
		result.m_HitDistance = 0.f;
		result.m_BlockPos = PBE::IntVec3(origin);
		return result;
	}

	// --- 1) Ray to OBB local space ---
	// Local origin is origin relative to center, projected onto each OBB axis.
	Vec3 rel = origin - m_Center;
	Vec3 oLocal = Vec3(
		Vec3::DotProduct(rel, m_IBasis),
		Vec3::DotProduct(rel, m_JBasis),
		Vec3::DotProduct(rel, m_KBasis)
	);

	// Local direction is components along each axis.
	Vec3 dLocal = Vec3(
		Vec3::DotProduct(fwdNormal, m_IBasis),
		Vec3::DotProduct(fwdNormal, m_JBasis),
		Vec3::DotProduct(fwdNormal, m_KBasis)
	);

	// Half extents of the OBB in its local space
	const Vec3 e = m_Extents * 0.5f; // change to your field name

	float tMin = 0.0f;
	float tMax = maxDist;

	// Track which axis produced tMin for normal derivation
	int hitAxis = -1;
	float EPS = 1e-6f;

	auto intersectSlab = [&](float o, float d, float eHalf, int axis) -> bool
		{
			if (std::fabs(d) < EPS)
			{
				// Ray parallel to slab; must be inside the slab range
				if (o < -eHalf || o > eHalf) return false;
				return true; // no constraint added
			}
			float t1 = (-eHalf - o) / d;
			float t2 = (eHalf - o) / d;
			if (t1 > t2) std::swap(t1, t2);

			// tighten interval
			if (t1 > tMin) {
				tMin = t1; hitAxis = axis;
			}
			if (t2 < tMax) {
				tMax = t2;
			}

			return tMin <= tMax;
		};

	if (!intersectSlab(oLocal.x, dLocal.x, e.x, 0)) return result;
	if (!intersectSlab(oLocal.y, dLocal.y, e.y, 1)) return result;
	if (!intersectSlab(oLocal.z, dLocal.z, e.z, 2)) return result;

	// No intersection in [0, maxDist]?
	if (tMax < 0.0f) return result;         // box is entirely behind
	float tHit = (tMin >= 0.0f) ? tMin : tMax;
	if (tHit > maxDist) return result;

	// Hit!
	result.m_Hit = true;
	result.m_HitDistance = tHit;

	// World-space hit point
	Vec3 hitPos = origin + fwdNormal * tHit;
	result.m_HitPoint = hitPos;            // if you track it

	// --- 5) Compute normal ---
	// Get local hit point to see which face we touched, then map normal to world.
	Vec3 pLocal = oLocal + dLocal * tHit;

	Vec3 nLocal(0, 0, 0);
	// If we entered on axis hitAxis, normal is +/- that axis depending on which side.
	switch (hitAxis)
	{
		case 0: nLocal.x = (pLocal.x > 0.f) ? 1.f : -1.f; break;
		case 1: nLocal.y = (pLocal.y > 0.f) ? 1.f : -1.f; break;
		case 2: nLocal.z = (pLocal.z > 0.f) ? 1.f : -1.f; break;
		default: break; // Parallel start inside shouldn't happen due to early-out
	}

	// Transform normal back to world space (rotation only)
	Vec3 nWorld =
		nLocal.x * m_IBasis +
		nLocal.y * m_JBasis +
		nLocal.z * m_KBasis;
	nWorld.Normalize();
	result.m_HitNormal = nWorld;

	// Whatever you want for block/voxel index:
	result.m_BlockPos = PBE::IntVec3(hitPos);

	return result;
}

PBE::CollisionResult3D PBE::OBB3::GetCollision( OBB3 const& other , float stepHeight ) const
{
	CollisionResult3D result;

	Vec3 axes[15];
	axes[0] = m_IBasis;
	axes[1] = m_JBasis;
	axes[2] = m_KBasis;
	axes[3] = other.m_IBasis;
	axes[4] = other.m_JBasis;
	axes[5] = other.m_KBasis;

	// Cross products to generate potential separating axes
	int index = 6;
	for (int i = 0; i < 3; ++i) 
	{
		for (int j = 0; j < 3; ++j) 
		{
			Vec3 cross = Vec3::CrossProduct(axes[i], axes[j + 3]);
			if (!cross.IsNearlyZero())
			{
				// Normalize to prevent instability, since opposing axes on same plane are not guaranteed to be a right angle.
				axes[index++] = cross.GetNormalized();
			}
		}
	}

	float trackedOverlap = 999999.f;
	float overlapDirection = 1.f;
	Vec3 bestAxis = Vec3::ZERO;

	for (int i = 0; i < index; ++i) 
	{
		Vec3 axis = axes[i].GetNormalized(); // Ensure normalization

		float myCenter = Vec3::DotProduct(m_Center, axis);
		float otherCenter = Vec3::DotProduct(other.m_Center, axis);
		float centerDist = otherCenter - myCenter;

		// Project both OBBs onto this axis
		float myExtent = GetProjectedHalfExtent(axis);
		float otherExtent = other.GetProjectedHalfExtent(axis);

		// Compute overlap correctly
		float overlap = (myExtent + otherExtent) - std::abs(centerDist);
		if (overlap < 0.f) 
		{
			return result;
		}

		if (overlap < trackedOverlap) 
		{
			bestAxis = axis;
			trackedOverlap = overlap;
			overlapDirection = centerDist < 0.f ? -1.f : 1.f;
		}
	}

	if (stepHeight > 0.f)
	{
		Vec3 axis = Vec3(0.f, 0.f, 1.f);

		float myCenter = Vec3::DotProduct(m_Center, axis);
		float otherCenter = Vec3::DotProduct(other.m_Center, axis);
		float centerDist = otherCenter - myCenter;

		float myExtent = GetProjectedHalfExtent(axis);
		float otherExtent = other.GetProjectedHalfExtent(axis);

		float overlap = (myExtent + otherExtent) - std::abs(centerDist);

		if (centerDist < 0.f && overlap <= stepHeight)
		{
			result.m_Hit = true;
			result.m_HitSurfaceNormal = axis;
			result.m_HitPushDisplacement = axis * overlap;
			result.m_HitPosition = other.m_Center + result.m_HitPushDisplacement;
			return result;
		}
	}

	// Return the displacement along the best penetration axis
	result.m_Hit = true;
	result.m_HitSurfaceNormal = bestAxis;
	result.m_HitPushDisplacement = bestAxis * -trackedOverlap * overlapDirection;
	result.m_HitPosition = other.m_Center + result.m_HitPushDisplacement;

	return result;
}

// Helper function to compute the half-extent projection of an OBB onto an axis
float PBE::OBB3::GetProjectedHalfExtent(const Vec3& axis) const
{
	return std::abs(Vec3::DotProduct(m_IBasis * (m_Extents.x * 0.5f), axis)) +
		std::abs(Vec3::DotProduct(m_JBasis * (m_Extents.y * 0.5f), axis)) +
		std::abs(Vec3::DotProduct(m_KBasis * (m_Extents.z * 0.5f), axis));
}

bool PBE::OBB3::Intersects(OBB3 const& other) const
{
	Vec3 displacement = other.m_Center - m_Center;

	if (!IntersectsAxis(displacement, m_IBasis, other))
	{
		return false;
	}
	if (!IntersectsAxis(displacement, m_JBasis, other))
	{
		return false;
	}
	if (!IntersectsAxis(displacement, m_KBasis, other))
	{
		return false;
	}

	if (!other.IntersectsAxis(displacement, other.m_IBasis, *this))
	{
		return false;
	}
	if (!other.IntersectsAxis(displacement, other.m_JBasis, *this))
	{
		return false;
	}
	if (!other.IntersectsAxis(displacement, other.m_KBasis, *this))
	{
		return false;
	}

	Vec3 cross = Vec3::CrossProduct(m_IBasis, other.m_IBasis);
	if (!IntersectsAxis(displacement, cross, other))
	{
		return false;
	}

	cross = Vec3::CrossProduct(m_IBasis, other.m_JBasis);
	if (!IntersectsAxis(displacement, cross, other))
	{
		return false;
	}

	cross = Vec3::CrossProduct(m_IBasis, other.m_KBasis);
	if (!IntersectsAxis(displacement, cross, other))
	{
		return false;
	}

	cross = Vec3::CrossProduct(m_JBasis, other.m_IBasis);
	if (!IntersectsAxis(displacement, cross, other))
	{
		return false;
	}

	cross = Vec3::CrossProduct(m_JBasis, other.m_JBasis);
	if (!IntersectsAxis(displacement, cross, other))
	{
		return false;
	}

	cross = Vec3::CrossProduct(m_JBasis, other.m_KBasis);
	if (!IntersectsAxis(displacement, cross, other))
	{
		return false;
	}

	cross = Vec3::CrossProduct(m_KBasis, other.m_IBasis);
	if (!IntersectsAxis(displacement, cross, other))
	{
		return false;
	}

	cross = Vec3::CrossProduct(m_KBasis, other.m_JBasis);
	if (!IntersectsAxis(displacement, cross, other))
	{
		return false;
	}

	cross = Vec3::CrossProduct(m_KBasis, other.m_KBasis);
	if (!IntersectsAxis(displacement, cross, other))
	{
		return false;
	}

	return true;
}

PBE::Vec3 PBE::OBB3::GetCenter() const
{
	return m_Center;
}

PBE::Vec3 PBE::OBB3::GetDimensions() const
{
	return m_Extents;
}

float PBE::OBB3::GetWidth() const
{
	return m_Extents.x;
}

float PBE::OBB3::GetHeight() const
{
	return m_Extents.y;
}

float PBE::OBB3::GetDepth() const
{
	return m_Extents.z;
}

void PBE::OBB3::SetExtents(float x, float y, float z)
{
	m_Extents = Vec3(x, y, z);
}

void PBE::OBB3::SetRotationDegrees(EulerAngles const& angles)
{
	angles.GetAsVectors(&m_IBasis, &m_JBasis, &m_KBasis);
}

void PBE::OBB3::SetCenter(float x, float y, float z)
{
	m_Center = Vec3(x, y, z);
}

void PBE::OBB3::Translate(Vec3 const& translation)
{
	m_Center += translation;
}

bool PBE::OBB3::IntersectsAxis(Vec3 const& displacement, Vec3 const& axis, OBB3 const& other) const
{
	float myRadius =
		0.5f * m_Extents.x * fabs(Vec3::DotProduct(axis, m_IBasis)) +
		0.5f * m_Extents.y * fabs(Vec3::DotProduct(axis, m_JBasis)) +
		0.5f * m_Extents.z * fabs(Vec3::DotProduct(axis, m_KBasis));

	float otherRadius =
		0.5f * other.m_Extents.x * fabs(Vec3::DotProduct(axis, other.m_IBasis)) +
		0.5f * other.m_Extents.y * fabs(Vec3::DotProduct(axis, other.m_JBasis)) +
		0.5f * other.m_Extents.z * fabs(Vec3::DotProduct(axis, other.m_KBasis));

	float centerDist = fabs(Vec3::DotProduct(displacement, axis));

	return centerDist < myRadius + otherRadius;
}
