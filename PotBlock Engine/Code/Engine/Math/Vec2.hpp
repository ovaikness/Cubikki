#pragma once
#include <string>

namespace PBE
{
	class IntVec2;
	class Vec2
	{
	public:
		float x;
		float y;

		static Vec2 const ZERO;
		static Vec2 const ONE;
		static Vec2 const YNEG;
		static Vec2 const YPOS;
		static Vec2 const LEFT;
		static Vec2 const RIGHT;
	public:
		Vec2();
		Vec2(float x, float y);
		Vec2(Vec2 const& other) noexcept;
		explicit Vec2(IntVec2 const& other) noexcept;

		Vec2 const operator+(Vec2 const& other) const;
		Vec2 const operator-(Vec2 const& other) const;
		
		Vec2 const operator*(Vec2 const& other) const;
		Vec2 const operator/(Vec2 const& other) const;

		Vec2 const operator*(float scalar) const;
		Vec2 const operator/(float scalar) const;

		friend Vec2 const operator*(float scalar, Vec2 const& vec);

		Vec2& operator+=(Vec2 const& other);
		Vec2& operator-=(Vec2 const& other);
		Vec2& operator*=(float scalar);
		Vec2& operator/=(float scalar);
		Vec2& operator-();

		bool operator==(Vec2 const& other) const;
		bool operator!=(Vec2 const& other) const;
		std::string ToString() const;

		float Dot(Vec2 const& other) const;
		float Cross(Vec2 const& other) const;
		float GetAngleBetweenDegrees(Vec2 const& other) const;
		float GetDistance(Vec2 const& other) const;
		float GetDistanceSquared(Vec2 const& other) const;

		float GetLength() const;
		float GetLengthSquared() const;
		float GetAngle() const;
		float GetAngleDegrees() const;

		void Normalize();
		void Rotate(float angle);
		void RotateAroundPivot(float angle, Vec2 const& pivot);
		void Reflect(Vec2 const& normal);
		void Project(Vec2 const& axis);
		void Rotate90Degrees();
		void RotateMinus90Degrees();
		void Flip();
		void Negate();

		Vec2 GetNormalized() const;
		Vec2 GetRotated(float angle) const;
		Vec2 GetRotatedAroundPivot(float angle, Vec2 const& pivot) const;
		Vec2 GetReflected(Vec2 const& normal) const;
		Vec2 GetProjected(Vec2 const& axis) const;
		float GetProjectedLength(Vec2 const& normal) const;
		Vec2 GetRotated90Degrees() const;
		Vec2 GetRotatedMinus90Degrees() const;
		Vec2 GetFlipped() const;
		Vec2 GetNegated() const;
		Vec2 GetAbs() const;
		Vec2 GetMin(Vec2 const& other) const;
		Vec2 GetMax(Vec2 const& other) const;
		Vec2 GetClamped(Vec2 const& min, Vec2 const& max) const;
		Vec2 GetLerped(Vec2 const& other, float t) const;
		Vec2 GetFloored() const;
		Vec2 GetCeiled() const;
		Vec2 GetRound() const;
		Vec2 GetSign() const;
		Vec2 GetReciprocal() const;

		static Vec2 const MakeFromPolarDegrees(float angleDegrees, float length = 1.f);

		static float DotProduct(Vec2 const& a, Vec2 const& b);
		static float CrossProduct(Vec2 const& a, Vec2 const& b);
		static float Distance(Vec2 const& a, Vec2 const& b);
		static float DistanceSquared(Vec2 const& a, Vec2 const& b);
		static float AngleBetween(Vec2 const& a, Vec2 const& b);
		static Vec2 Lerp(Vec2 const& a, Vec2 const& b, float t);
		static Vec2 Min(Vec2 const& a, Vec2 const& b);
		static Vec2 Max(Vec2 const& a, Vec2 const& b);
		static Vec2 Clamp(Vec2 const& value, Vec2 const& min, Vec2 const& max);
		static Vec2 Floor(Vec2 const& value);
		static Vec2 Ceil(Vec2 const& value);
		static Vec2 Round(Vec2 const& value);
		static Vec2 Sign(Vec2 const& value);
		static Vec2 Reciprocal(Vec2 const& value);
		static Vec2 Abs(Vec2 const& value);
	};
}