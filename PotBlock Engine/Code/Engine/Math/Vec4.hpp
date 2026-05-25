#pragma once

#include <string>

namespace PBE
{
	class Vec3;
	class Vec2;

	class Vec4
	{
	public:
		float x;
		float y;
		float z;
		float w;

		static Vec4 const ZERO;
		static Vec4 const ONE;
	public:
		Vec4();
		Vec4(float scalar);

		Vec4(Vec2 const& other, float z = 0.f, float w = 1.f);
		Vec4(float x, Vec2 const& other, float w = 1.f);
		Vec4(float x, float y, Vec2 const& other);
		Vec4(Vec2 const& other1, Vec2 const& other2);
		Vec4(Vec3 const& other, float w = 1.f);
		Vec4(float x, Vec3 const& other);
		Vec4(float x, float y, float z, float w);
		Vec4(Vec4 const& other) noexcept;

		Vec4 const operator+(Vec4 const& other) const;
		Vec4 const operator-(Vec4 const& other) const;
		Vec4 const operator*(Vec4 const& other) const;
		Vec4 const operator/(Vec4 const& other) const;
		Vec4 const operator*(float scalar) const;
		Vec4 const operator/(float scalar) const;

		friend Vec4 const operator*(float scalar, Vec4 const& vec);

		Vec4& operator+=(Vec4 const& other);
		Vec4& operator-=(Vec4 const& other);
		Vec4& operator*=(Vec4 const& other);
		Vec4& operator/=(Vec4 const& other);
		Vec4& operator*=(float scalar);
		Vec4& operator/=(float scalar);

		bool operator==(Vec4 const& other) const;
		bool operator!=(Vec4 const& other) const;

		std::string ToString() const;

		float GetLength() const;
		float GetLengthSquared() const;

		void Normalize();
		void Negate();

		Vec4 GetNormalized() const;
		Vec4 GetNegated() const;
		Vec4 GetAbs() const;
		Vec4 GetMin(Vec4 const& other) const;
		Vec4 GetMax(Vec4 const& other) const;
		Vec4 GetClamped(Vec4 const& min, Vec4 const& max) const;
		Vec4 GetLerped(Vec4 const& other, float t) const;
		Vec4 GetFloored() const;
		Vec4 GetCeiled() const;
		Vec4 GetRound() const;
		Vec4 GetSign() const;
		Vec4 GetReciprocal() const;

		static Vec4 Lerp(Vec4 const& a, Vec4 const& b, float t);
		static Vec4 Min(Vec4 const& a, Vec4 const& b);
		static Vec4 Max(Vec4 const& a, Vec4 const& b);
		static Vec4 Clamp(Vec4 const& value, Vec4 const& min, Vec4 const& max);
		static Vec4 Floor(Vec4 const& value);
		static Vec4 Ceil(Vec4 const& value);
		static Vec4 Round(Vec4 const& value);
		static Vec4 Sign(Vec4 const& value);
		static Vec4 Reciprocal(Vec4 const& value);
		static Vec4 Abs(Vec4 const& value);
	};
}