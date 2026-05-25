#include "Engine/Math/Rotor2D.hpp"
#include "Engine/Math/MathUtils.hpp"

PBE::Rotor2D const PBE::Rotor2D::IDENTITY { 0.f , 1.f };

PBE::Rotor2D PBE::Rotor2D::operator*(Rotor2D const& rhs) const
{
	return Rotor2D(w * rhs.xy + xy * rhs.w, w * rhs.w - xy * rhs.xy);
}

PBE::Rotor2D PBE::Rotor2D::operator+(Rotor2D const& rhs) const
{
	return Rotor2D(xy + rhs.xy, w + rhs.w);
}

PBE::Rotor2D& PBE::Rotor2D::operator+=(Rotor2D const& rhs)
{
	w += rhs.w;
	xy += rhs.xy;

	return *this;
}

PBE::Rotor2D& PBE::Rotor2D::operator-=(Rotor2D const& rhs)
{
	w -= rhs.w;
	xy -= rhs.xy;

	return *this;
}

float PBE::Rotor2D::GetMagnitude() const
{
	return Sqrt(w * w + xy * xy);
}

PBE::Rotor2D PBE::Rotor2D::GetNormalized() const
{
	return *this * (1.f / GetMagnitude());
}

PBE::Rotor2D PBE::Rotor2D::GetConjugate() const
{
	Rotor2D result;
	result.w = w;
	result.xy = -xy;

	return result;
}

PBE::Rotor2D PBE::Rotor2D::GetInverse() const
{
	float sqrMag = w * w + xy * xy;

	Rotor2D result;
	result.w = w / sqrMag;
	result.xy = -xy / sqrMag;

	return result;
}

PBE::Vec2 PBE::Rotor2D::GetRotatedVector(Vec2 const& vector) const
{
	Rotor2D vecRotor(vector.x, vector.y);
	Rotor2D result = *this * vecRotor * GetConjugate();

	return Vec2(result.xy, result.w);
}

float PBE::Rotor2D::GetAngle() const
{
	return 2.f * Atan2(xy, w);
}

PBE::Rotor2D PBE::Rotor2D::operator-() const
{
	return Rotor2D(-xy, -w);
}

PBE::Rotor2D PBE::Rotor2D::operator-(Rotor2D const& rhs) const
{
	Rotor2D result;
	result.w = w - rhs.w;
	result.xy = xy - rhs.xy;

	return result;
}

PBE::Rotor2D& PBE::Rotor2D::operator*=(float scalar)
{
	w *= scalar;
	xy *= scalar;

	return *this;
}

PBE::Rotor2D& PBE::Rotor2D::operator*=(Rotor2D const& rhs)
{
	*this = *this * rhs;
	return *this;
}

PBE::Rotor2D PBE::Rotor2D::operator*(float scalar) const
{
	Rotor2D result;
	result.w = w * scalar;
	result.xy = xy * scalar;

	return result;
}
