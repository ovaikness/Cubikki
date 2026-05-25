#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/CollisionResult3D.hpp"

#include <vector>

namespace PBE
{
	class AABB3;
	class OBB3
	{
	public:
		Vec3 m_IBasis{ 1.f,0.f,0.f };
		Vec3 m_JBasis{ 0.f,1.f,0.f };
		Vec3 m_KBasis{ 0.f,0.f,1.f };

		Vec3 m_Center{ 0.f, 0.f, 0.f };
		Vec3 m_Extents{ 1.f,1.f,1.f };
	public:
		~OBB3() = default;
		OBB3() = default;
		OBB3(Vec3 const& center, float width, float height, float depth, EulerAngles const& orientation);
		OBB3(Vec3 const& center, float width, float height, float depth, Mat4 const& transform);
		OBB3(Vec3 const& center, Vec3 const& extends, Mat4 const& transform);
		OBB3(Vec3 const& center, float width, float height, float depth, Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis);
		OBB3(OBB3 const& other) = default;
		OBB3(AABB3 const& aabb);
		void SetCenter(Vec3 const& center);
		void SetCenter(float x, float y, float z);
		void SetExtents(Vec3 const& extents);
		void SetExtents(float x, float y, float z);
		void SetRotationDegrees(EulerAngles const& angles);
		void Translate(Vec3 const& translation);

		std::vector<Vec3> GetCorners() const;

		Vec3 const GetIBasis() const;
		Vec3 const GetJBasis() const;
		Vec3 const GetKBasis() const;
		bool IsPointInside(Vec3 const& point) const;
		bool IsPointInside(float x, float y, float z) const;
		Vec3 GetNearestPoint(float x, float y, float z) const;
		Vec3 GetNearestPoint(Vec3 const& point) const;
		bool Intersects(OBB3 const& other) const;
		bool Intersects(AABB3 const& other) const;

		RaycastResult3D Raycast(Vec3 const& origin, Vec3 const& fwdNormal, float maxDist) const;
		CollisionResult3D GetCollision(OBB3 const& other, float stepHeight = 0.f) const;

		float GetProjectedHalfExtent(Vec3 const& axis) const;
		Vec3 GetCenter() const;
		Vec3 GetDimensions() const;
		float GetWidth() const;
		float GetHeight() const;
		float GetDepth() const;
	private:
		bool IntersectsAxis(Vec3 const& displacement, Vec3 const& axis, OBB3 const& other) const;
	};
}