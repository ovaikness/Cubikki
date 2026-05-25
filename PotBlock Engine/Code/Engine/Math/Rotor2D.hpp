#pragma once
#include "Engine/Math/Mat4.hpp"
#include "Engine/Math/Vec2.hpp"

namespace PBE
{
	class Rotor2D
	{
	public:
		static Rotor2D const IDENTITY;

		float w = 1.f;
		float xy = 0.f;
	public:
		Rotor2D() = default;
		Rotor2D(float xy, float w)
			: xy(xy), w(w) {}

		Rotor2D operator*(Rotor2D const& rhs) const;
		Rotor2D operator*(float scalar) const;
		Rotor2D operator+(Rotor2D const& rhs) const;
		Rotor2D operator-(Rotor2D const& rhs) const;

		Rotor2D& operator*=(Rotor2D const& rhs);
		Rotor2D& operator*=(float scalar);
		Rotor2D& operator+=(Rotor2D const& rhs);
		Rotor2D& operator-=(Rotor2D const& rhs);

		Rotor2D operator-() const;

		float GetMagnitude() const;
		Rotor2D GetNormalized() const;
		Rotor2D GetConjugate() const;
		Rotor2D GetInverse() const;

		Vec2 GetRotatedVector(Vec2 const& vector) const;

		float GetAngle() const;
	};
}