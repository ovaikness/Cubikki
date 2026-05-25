#pragma once
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat4.hpp"

namespace PBE
{
	class Rotor3D
	{
	public:
		static Rotor3D const IDENTITY;

		float i = 0.f;
		float j = 0.f;
		float k = 0.f;
		float w = 1.f;
	public:
		Rotor3D() = default;
		Rotor3D(float i, float j, float k, float w)
			: i(i), j(j), k(k), w(w) {}

		static Rotor3D CreateFromVectors(Vec3 const& a, Vec3 const& b);
		static Rotor3D CreateFromEulerAngles(float pitch, float yaw, float roll);
		static Rotor3D CreateFromEulerAngles(EulerAngles const& angles);
		static Rotor3D Slerp(Rotor3D const& start, Rotor3D const& end, float t);
		Rotor3D(Vec3 const& a, Vec3 const& b);

		Rotor3D operator*(Rotor3D const& rhs) const;
		Rotor3D operator*(float scalar) const;
		Rotor3D operator+(Rotor3D const& rhs) const;
		Rotor3D operator-(Rotor3D const& rhs) const;

		Rotor3D& operator*=(Rotor3D const& rhs);
		Rotor3D& operator*=(float scalar);
		Rotor3D& operator+=(Rotor3D const& rhs);
		Rotor3D& operator-=(Rotor3D const& rhs);

		Rotor3D operator-() const;

		float GetMagnitude() const;
		Rotor3D GetNormalized() const;
		Rotor3D GetConjugate() const;
		Rotor3D GetInverse() const;
		Mat4 GetMatrix() const;

		Vec3 GetRotatedVector(Vec3 const& vector) const;
		EulerAngles GetEulerAngles() const;
	};
}
