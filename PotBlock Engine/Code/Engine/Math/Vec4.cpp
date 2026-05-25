#include "Vec4.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"

namespace PBE
{
	Vec4 const Vec4::ZERO = Vec4(0.f,0.f,0.f,0.f);
	Vec4 const Vec4::ONE  = Vec4(1.f,1.f,1.f,1.f);

	Vec4::Vec4()
		: x(0.0f)
		, y(0.0f)
		, z(0.0f)
		, w(0.0f)
	{
	}

	Vec4::Vec4(Vec4 const& other) noexcept
		: x(other.x)
		, y(other.y)
		, z(other.z)
		, w(other.w)
	{
	}

	Vec4::Vec4(float x, float y, float z, float w)
		: x(x)
		, y(y)
		, z(z)
		, w(w)
	{
	}

	Vec4::Vec4(float scalar)
		: x(scalar)
		, y(scalar)
		, z(scalar)
		, w(scalar)
	{

	}

	Vec4::Vec4(Vec2 const& other, float z /*= 0.f*/, float w /*= 1.f*/)
		: x(other.x)
		, y(other.y)
		, z(z)
		, w(w)
	{

	}

	Vec4::Vec4(float x, Vec2 const& other, float w /*= 1.f*/)
		: x(x)
		, y(other.x)
		, z(other.y)
		, w(w)
	{

	}

	Vec4::Vec4(float x, float y, Vec2 const& other)
		: x(x)
		, y(y)
		, z(other.x)
		, w(other.y)
	{

	}

	Vec4::Vec4(Vec2 const& other1, Vec2 const& other2)
		: x(other1.x)
		, y(other1.y)
		, z(other2.x)
		, w(other2.y)
	{

	}

	Vec4::Vec4(Vec3 const& other, float w /*= 1.f*/)
		: x(other.x)
		, y(other.y)
		, z(other.z)
		, w(w)
	{

	}

	Vec4::Vec4(float x, Vec3 const& other)
		: x(x)
		, y(other.x)
		, z(other.y)
		, w(other.z)
	{

	}

	Vec4 const Vec4::operator+(Vec4 const& other) const
	{
		return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	Vec4 const Vec4::operator-(Vec4 const& other) const
	{
		return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	Vec4 const Vec4::operator*(Vec4 const& other) const
	{
		return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
	}

	Vec4 const Vec4::operator/(Vec4 const& other) const
	{
		return Vec4(x / other.x, y / other.y, z / other.z, w / other.w);
	}

	Vec4 const Vec4::operator*(float scalar) const
	{
		return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
	}

	Vec4 const Vec4::operator/(float scalar) const
	{
		float scale = scalar;
		return Vec4(x / scale, y / scale, z / scale, w / scale);
	}

	std::string Vec4::ToString() const
	{
		return std::string("Vec4(") + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w) + ")";
	}

	float Vec4::GetLength() const
	{
		return PBE::Sqrt(GetLengthSquared());
	}

	float Vec4::GetLengthSquared() const
	{
		return x * x + y * y + z * z + w * w;
	}

	void Vec4::Normalize()
	{
		float length = GetLength();
		float scale = 1.f / length;
		x *= scale;
		y *= scale;
		z *= scale;
		w *= scale;
	}

	void Vec4::Negate()
	{
		x = -x;
		y = -y;
		z = -z;
		w = -w;
	}

	Vec4 Vec4::GetNormalized() const
	{
		float length = GetLength();
		float scale = 1.f / length;
		return Vec4(x * scale, y * scale, z * scale, w * scale);
	}

	Vec4 Vec4::GetNegated() const
	{
		Vec4 result = *this;
		result.Negate();
		return result;
	}

	Vec4 Vec4::GetAbs() const
	{
		Vec4 result = *this;
		result.x = PBE::Abs(result.x);
		result.y = PBE::Abs(result.y);
		result.z = PBE::Abs(result.z);
		result.w = PBE::Abs(result.w);
		return result;
	}

	Vec4 Vec4::GetMin(Vec4 const& other) const
	{
		return Vec4(PBE::Min(x, other.x), PBE::Min(y, other.y), PBE::Min(z, other.z), PBE::Min(w, other.w));
	}

	Vec4 Vec4::GetMax(Vec4 const& other) const
	{
		return Vec4(PBE::Max(x, other.x), PBE::Max(y, other.y), PBE::Max(z, other.z), PBE::Max(w, other.w));
	}

	Vec4 Vec4::GetClamped(Vec4 const& min, Vec4 const& max) const
	{
		return Vec4(PBE::Clamp(x, min.x, max.x), PBE::Clamp(y, min.y, max.y), PBE::Clamp(z, min.z, max.z), PBE::Clamp(w, min.w, max.w));
	}

	Vec4 Vec4::GetLerped(Vec4 const& other, float t) const
	{
		return Vec4(PBE::Lerp(x, other.x, t), PBE::Lerp(y, other.y, t), PBE::Lerp(z, other.z, t), PBE::Lerp(w, other.w, t));
	}

	Vec4 Vec4::GetFloored() const
	{
		return Vec4(static_cast<float>(PBE::Floor(x)), static_cast<float>(PBE::Floor(y)), static_cast<float>(PBE::Floor(z)), static_cast<float>(PBE::Floor(w)));
	}

	Vec4 Vec4::GetCeiled() const
	{
		return Vec4(static_cast<float>(PBE::Ceil(x)), static_cast<float>(PBE::Ceil(y)), static_cast<float>(PBE::Ceil(z)), static_cast<float>(PBE::Ceil(w)));
	}

	Vec4 Vec4::GetRound() const
	{
		return Vec4(static_cast<float>(PBE::Round(x)), static_cast<float>(PBE::Round(y)), static_cast<float>(PBE::Round(z)), static_cast<float>(PBE::Round(w)));
	}

	Vec4 Vec4::GetSign() const
	{
		return Vec4(PBE::Sign(x), PBE::Sign(y), PBE::Sign(z), PBE::Sign(w));
	}

	Vec4 Vec4::GetReciprocal() const
	{
		return Vec4(1.f / x, 1.f / y, 1.f / z, 1.f / w);
	}

	Vec4 Vec4::Lerp(Vec4 const& a, Vec4 const& b, float t)
	{
		return Vec4(PBE::Lerp(a.x, b.x, t), PBE::Lerp(a.y, b.y, t), PBE::Lerp(a.z, b.z, t), PBE::Lerp(a.w, b.w, t));
	}

	Vec4 Vec4::Min(Vec4 const& a, Vec4 const& b)
	{
		return Vec4(PBE::Min(a.x, b.x), PBE::Min(a.y, b.y), PBE::Min(a.z, b.z), PBE::Min(a.w, b.w));
	}

	Vec4 Vec4::Max(Vec4 const& a, Vec4 const& b)
	{
		return Vec4(PBE::Max(a.x, b.x), PBE::Max(a.y, b.y), PBE::Max(a.z, b.z), PBE::Max(a.w, b.w));
	}

	Vec4 Vec4::Clamp(Vec4 const& value, Vec4 const& min, Vec4 const& max)
	{
		return Vec4(PBE::Clamp(value.x, min.x, max.x), PBE::Clamp(value.y, min.y, max.y), PBE::Clamp(value.z, min.z, max.z), PBE::Clamp(value.w, min.w, max.w));
	}

	Vec4 Vec4::Floor(Vec4 const& value)
	{
		return Vec4(static_cast<float>(PBE::Floor(value.x)), static_cast<float>(PBE::Floor(value.y)), static_cast<float>(PBE::Floor(value.z)), static_cast<float>(PBE::Floor(value.w)));
	}

	Vec4 Vec4::Ceil(Vec4 const& value)
	{
		return Vec4(static_cast<float>(PBE::Ceil(value.x)), static_cast<float>(PBE::Ceil(value.y)), static_cast<float>(PBE::Ceil(value.z)), static_cast<float>(PBE::Ceil(value.w)));
	}

	Vec4 Vec4::Round(Vec4 const& value)
	{
		return Vec4(static_cast<float>(PBE::Round(value.x)), static_cast<float>(PBE::Round(value.y)), static_cast<float>(PBE::Round(value.z)), static_cast<float>(PBE::Round(value.w)));
	}

	Vec4 Vec4::Sign(Vec4 const& value)
	{
		return Vec4(PBE::Sign(value.x), PBE::Sign(value.y), PBE::Sign(value.z), PBE::Sign(value.w));
	}

	Vec4 Vec4::Reciprocal(Vec4 const& value)
	{
		return Vec4(1.f / value.x, 1.f / value.y, 1.f / value.z, 1.f / value.w);
	}

	Vec4 Vec4::Abs(Vec4 const& value)
	{
		return Vec4(PBE::Abs(value.x), PBE::Abs(value.y), PBE::Abs(value.z), PBE::Abs(value.w));
	}

	bool Vec4::operator!=(Vec4 const& other) const
	{
		return !(*this == other);
	}

	bool Vec4::operator==(Vec4 const& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	Vec4& Vec4::operator/=(float scalar)
	{
		float scale = 1.f / scalar;
		x *= scale;
		y *= scale;
		z *= scale;
		w *= scale;

		return *this;
	}

	Vec4& Vec4::operator*=(float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;

		return *this;
	}

	Vec4& Vec4::operator-=(Vec4 const& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;

		return *this;
	}

	Vec4& Vec4::operator*=(Vec4 const& other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;

		return *this;
	}

	Vec4& Vec4::operator/=(Vec4 const& other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;

		return *this;
	}

	Vec4& Vec4::operator+=(Vec4 const& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;

		return *this;
	}

	Vec4 const operator*(float scalar, Vec4 const& vec)
	{
		return Vec4(scalar * vec.x, scalar * vec.y, scalar * vec.z, scalar * vec.w);
	}
}
