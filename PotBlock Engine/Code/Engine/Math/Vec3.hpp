#pragma once

#include "Engine/Math/Vec2.hpp"
#include <string>

namespace PBE
{
	class IntVec3;
	class Rotor3D;
	class Vec3
	{
	public:
		float x;
		float y;
		float z;

		static Vec3 const ZERO;
		static Vec3 const ONE;
		static Vec3 const YNEG;
		static Vec3 const YPOS;
		static Vec3 const LEFT;
		static Vec3 const RIGHT;
		static Vec3 const FORWARD;
		static Vec3 const BACKWARD;
		static Vec3 const UP;
		static Vec3 const DOWN;
	public:
		Vec3();
		Vec3(Vec2 const& other, float z = 0.0f);
		Vec3(float x, Vec2 const& other);
		Vec3(float x, float y, float z);
		Vec3(float all);
		Vec3(IntVec3 const& other);
		Vec3(Vec3 const& other) noexcept;

		Vec3 const operator+(Vec3 const& other) const;
		Vec3 const operator-(Vec3 const& other) const;
		Vec3 const operator*(float scalar) const;
		Vec3 const operator/(float scalar) const;
		Vec3 const operator-() const;

		friend Vec3 const operator*(float scalar, Vec3 const& vec);

		Vec3& operator+=(Vec3 const& other);
		Vec3& operator-=(Vec3 const& other);
		Vec3& operator*=(float scalar);
		Vec3& operator/=(float scalar);

		bool operator==(Vec3 const& other) const;
		bool operator!=(Vec3 const& other) const;
		
		bool IsNearlyZero() const;

		std::string ToString() const;

		float Dot(Vec3 const& other) const;
		Vec3 Cross(Vec3 const& other) const;
		float GetAngleBetweenDegrees(Vec3 const& other) const;
		float GetDistance(Vec3 const& other) const;
		float GetDistanceSquared(Vec3 const& other) const;

		float GetLength() const;
		float GetLengthSquared() const;
		float GetAngle() const;

		void Normalize();
		void Reflect(Vec3 const& normal);
		void Project(Vec3 const& axis);
		void Negate();

		IntVec3 const GetFlooredToIntVec3() const;

		Vec3 GetNormalized() const;
		Vec3 GetReflected(Vec3 const& normal) const;
		Vec3 GetProjected(Vec3 const& axis) const;
		Vec3 GetNegated() const;
		Vec3 GetAbs() const;
		Vec3 GetMin(Vec3 const& other) const;
		Vec3 GetMax(Vec3 const& other) const;
		Vec3 GetClamped(Vec3 const& min, Vec3 const& max) const;
		Vec3 GetLerped(Vec3 const& other, float t) const;
		Vec3 GetFloored() const;
		Vec3 GetCeiled() const;
		Vec3 GetRound() const;
		Vec3 GetSign() const;
		Vec3 GetReciprocal() const;

		static float DotProduct(Vec3 const& a, Vec3 const& b);
		static Vec3 CrossProduct(Vec3 const& a, Vec3 const& b);
		static float Distance(Vec3 const& a, Vec3 const& b);
		static float DistanceSquared(Vec3 const& a, Vec3 const& b);
		static float AngleBetween(Vec3 const& a, Vec3 const& b);
		static Vec3 Lerp(Vec3 const& a, Vec3 const& b, float t);
		static Vec3 Min(Vec3 const& a, Vec3 const& b);
		static Vec3 Max(Vec3 const& a, Vec3 const& b);
		static Vec3 Clamp(Vec3 const& value, Vec3 const& min, Vec3 const& max);
		static Vec3 Floor(Vec3 const& value);
		static Vec3 Ceil(Vec3 const& value);
		static Vec3 Round(Vec3 const& value);
		static Vec3 Sign(Vec3 const& value);
		static Vec3 Reciprocal(Vec3 const& value);
		static Vec3 Abs(Vec3 const& value);

		static Vec3 MakeFromPolarDegrees(float yaw, float pitch, float radius = 1.0f);
	};
}