#include "Vec3.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/MathUtils.hpp"

namespace PBE
{
	Vec3 const Vec3::ZERO	  = Vec3(0.f, 0.f, 0.f);
	Vec3 const Vec3::ONE	  = Vec3(1.f, 1.f, 1.f);
	Vec3 const Vec3::YNEG		  = Vec3(0.f, 1.f, 0.f);
	Vec3 const Vec3::YPOS	  = Vec3(0.f, -1.f, 0.f);
	Vec3 const Vec3::LEFT	  = Vec3(0.f, 1.f, 0.f);
	Vec3 const Vec3::RIGHT	  = Vec3(0.f, -1.f, 0.f);
	Vec3 const Vec3::FORWARD  = Vec3(1.f, 0.f, 0.f);
	Vec3 const Vec3::BACKWARD = Vec3(-1.f, 0.f,0.f);
	Vec3 const Vec3::UP		  = Vec3(0.f, 0.f, 1.f);
	Vec3 const Vec3::DOWN	  = Vec3(0.f, 0.f, -1.f);

	Vec3::Vec3()
		: x(0.f)
		, y(0.f)
		, z(0.f)
	{

	}

	Vec3::Vec3(Vec2 const& other, float z)
		: x(other.x)
		, y(other.y)
		, z(z)
	{
	}

	Vec3::Vec3(float x, Vec2 const& other)
		: x(x)
		, y(other.x)
		, z(other.y)
	{
	}

	Vec3::Vec3(float x, float y, float z)
		: x(x)
		, y(y)
		, z(z)
	{

	}

	Vec3::Vec3(float all)
		:x(all)
		,y(all)
		,z(all)
	{
	}

	Vec3::Vec3(IntVec3 const& other)
	{
		x = static_cast<float>(other.x);
		y = static_cast<float>(other.y);
		z = static_cast<float>(other.z);
	}

	Vec3::Vec3(Vec3 const& other) noexcept
		: x(other.x)
		, y(other.y)
		, z(other.z)
	{
	}

	Vec3 const Vec3::operator+(Vec3 const& other) const
	{
		return Vec3(x + other.x, y + other.y, z + other.z);
	}

	Vec3 const Vec3::operator-(Vec3 const& other) const
	{
		return Vec3(x - other.x, y - other.y, z - other.z);
	}

	Vec3 const Vec3::operator*(float scalar) const
	{
		return Vec3(x * scalar, y * scalar, z * scalar);
	}

	Vec3 const Vec3::operator/(float scalar) const
	{
		float scale = 1.f / scalar;
		return Vec3(x * scale, y * scale, z * scale);
	}

	Vec3 const Vec3::operator-() const
	{
		return Vec3(-x,-y,-z);
	}

	Vec3& Vec3::operator/=(float scalar)
	{
		float scale = 1.f / scalar;
		x *= scale;
		y *= scale;
		z *= scale;
		return *this;
	}

	Vec3& Vec3::operator*=(float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	Vec3& Vec3::operator-=(Vec3 const& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;

		return *this;
	}

	Vec3& Vec3::operator+=(Vec3 const& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;

		return *this;
	}

	bool Vec3::operator!=(Vec3 const& other) const
	{
		return x != other.x || y != other.y || z != other.z;
	}

	bool Vec3::IsNearlyZero() const
	{
		if (PBE::IsNearlyZero(x) && PBE::IsNearlyZero(y) && PBE::IsNearlyZero(z))
		{
			return true;
		}

		return false;
	}

	bool Vec3::operator==(Vec3 const& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}	

	std::string Vec3::ToString() const
	{
		return std::string("Vec3(") + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
	}

	float Vec3::GetLength() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	float Vec3::GetLengthSquared() const
	{
		return x * x + y * y + z * z;
	}

	float Vec3::GetAngle() const
	{
		return PBE::Atan2(y, x);
	}

	void Vec3::Normalize()
	{
		float length = GetLength();

		if (length == 0.f)
		{
			return;
		}

		float scale = 1.f / length;
		x *= scale;
		y *= scale;
		z *= scale;
	}

	void Vec3::Reflect(Vec3 const& normal)
	{
		float dot = DotProduct(*this, normal);
		*this -= 2.f * dot * normal;
	}

	void Vec3::Project(Vec3 const& axis)
	{
		float dot = DotProduct(*this, axis);
		float lengthSquared = axis.GetLengthSquared();

		if (lengthSquared == 0.f)
		{
			*this = Vec3::ZERO;
			return;
		}

		*this = axis * (dot / lengthSquared);
	}

	void Vec3::Negate()
	{
		x = -x;
		y = -y;
		z = -z;
	}

	IntVec3 const Vec3::GetFlooredToIntVec3() const
	{
		return IntVec3(PBE::Floor(x), PBE::Floor(y), PBE::Floor(z));
	}

	Vec3 Vec3::GetNormalized() const
	{
		float length = GetLength();
		if (length == 0.f)
		{
			return Vec3::ZERO;
		}
		float scale = 1.f / length;
		return Vec3(x * scale, y * scale, z * scale);
	}

	Vec3 Vec3::GetReflected(Vec3 const& normal) const
	{
		float dot = DotProduct(*this, normal);
		return *this - 2.f * dot * normal;
	}

	Vec3 Vec3::GetProjected(Vec3 const& axis) const
	{
		float dot = DotProduct(*this, axis);
		float lengthSquared = axis.GetLengthSquared();
		return axis * (dot / lengthSquared);
	}

	Vec3 Vec3::GetNegated() const
	{
		return Vec3(-x, -y, -z);
	}

	Vec3 Vec3::GetAbs() const
	{
		return Vec3(PBE::Abs(x), PBE::Abs(y), PBE::Abs(z));
	}

	Vec3 Vec3::GetMin(Vec3 const& other) const
	{
		return Vec3(PBE::Min(x, other.x), PBE::Min(y, other.y), PBE::Min(z, other.z));
	}

	Vec3 Vec3::GetMax(Vec3 const& other) const
	{
		return Vec3(PBE::Max(x, other.x), PBE::Max(y, other.y), PBE::Max(z, other.z));
	}

	Vec3 Vec3::GetClamped(Vec3 const& min, Vec3 const& max) const
	{
		return Vec3(PBE::Clamp(x, min.x, max.x), PBE::Clamp(y, min.y, max.y), PBE::Clamp(z, min.z, max.z));
	}

	Vec3 Vec3::GetLerped(Vec3 const& other, float t) const
	{
		return Vec3(PBE::Lerp(x, other.x, t), PBE::Lerp(y, other.y, t), PBE::Lerp(z, other.z, t));
	}

	Vec3 Vec3::GetFloored() const
	{
		return Vec3(static_cast<float>(PBE::Floor(x)), static_cast<float>(PBE::Floor(y)), static_cast<float>(PBE::Floor(z)));
	}

	Vec3 Vec3::GetCeiled() const
	{
		return Vec3(static_cast<float>(PBE::Ceil(x)), static_cast<float>(PBE::Ceil(y)), static_cast<float>(PBE::Ceil(z)));
	}

	Vec3 Vec3::GetRound() const
	{
		return Vec3(static_cast<float>(PBE::Round(x)), static_cast<float>(PBE::Round(y)), static_cast<float>(PBE::Round(z)));
	}

	Vec3 Vec3::GetSign() const
	{
		return Vec3(PBE::Sign(x), PBE::Sign(y), PBE::Sign(z));
	}

	Vec3 Vec3::GetReciprocal() const
	{
		return Vec3(1.f / x, 1.f / y, 1.f / z);
	}

	float Vec3::Dot(Vec3 const& other) const
	{
		return x * other.x + y * other.y + z * other.z;
	}

	Vec3 Vec3::Cross(Vec3 const& other) const
	{
		return Vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
	}

	float Vec3::GetAngleBetweenDegrees(Vec3 const& other) const
	{
		return PBE::AcosDegrees(DotProduct(*this, other) / (GetLength() * other.GetLength()));
	}

	float Vec3::GetDistance(Vec3 const& other) const
	{
		return (other - *this).GetLength();
	}

	float Vec3::GetDistanceSquared(Vec3 const& other) const
	{
		return (other - *this).GetLengthSquared();
	}

	float Vec3::DotProduct(Vec3 const& a, Vec3 const& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	Vec3 Vec3::CrossProduct(Vec3 const& a, Vec3 const& b)
	{
		Vec3 result;
		result.x = a.y * b.z - a.z * b.y;
		result.y = a.z * b.x - a.x * b.z;
		result.z = a.x * b.y - a.y * b.x;
		return result;
	}

	float Vec3::Distance(Vec3 const& a, Vec3 const& b)
	{
		return (a - b).GetLength();
	}

	float Vec3::DistanceSquared(Vec3 const& a, Vec3 const& b)
	{
		return (a - b).GetLengthSquared();
	}

	float Vec3::AngleBetween(Vec3 const& a, Vec3 const& b)
	{
		float dot = DotProduct(a, b);
		float length = a.GetLength() * b.GetLength();
		if (length == 0.f)
		{
			return 0.f;
		}
		float cosTheta = PBE::Clamp(dot / length, -1.f, 1.f);
		return PBE::Acos(cosTheta);
	}

	Vec3 Vec3::Lerp(Vec3 const& a, Vec3 const& b, float t)
	{
		return a + (b - a) * t;
	}

	Vec3 Vec3::Min(Vec3 const& a, Vec3 const& b)
	{
		return Vec3(PBE::Min(a.x, b.x), PBE::Min(a.y, b.y), PBE::Min(a.z, b.z));
	}

	Vec3 Vec3::Max(Vec3 const& a, Vec3 const& b)
	{
		return Vec3(PBE::Max(a.x, b.x), PBE::Max(a.y, b.y), PBE::Max(a.z, b.z));
	}

	Vec3 Vec3::Clamp(Vec3 const& value, Vec3 const& min, Vec3 const& max)
	{
		return Vec3(PBE::Clamp(value.x, min.x, max.x), PBE::Clamp(value.y, min.y, max.y), PBE::Clamp(value.z, min.z, max.z));
	}

	Vec3 Vec3::Floor(Vec3 const& value)
	{
		return Vec3(static_cast<float>(PBE::Floor(value.x)), static_cast<float>(PBE::Floor(value.y)), static_cast<float>(PBE::Floor(value.z)));
	}

	Vec3 Vec3::Ceil(Vec3 const& value)
	{
		return Vec3(static_cast<float>(PBE::Ceil(value.x)), static_cast<float>(PBE::Ceil(value.y)), static_cast<float>(PBE::Ceil(value.z)));
	}

	Vec3 Vec3::Round(Vec3 const& value)
	{
		return Vec3(static_cast<float>(PBE::Round(value.x)), static_cast<float>(PBE::Round(value.y)), static_cast<float>(PBE::Round(value.z)));
	}

	Vec3 Vec3::Sign(Vec3 const& value)
	{
		return Vec3(PBE::Sign(value.x), PBE::Sign(value.y), PBE::Sign(value.z));
	}

	Vec3 Vec3::Reciprocal(Vec3 const& value)
	{
		return Vec3(1.f / value.x, 1.f / value.y, 1.f / value.z);
	}

	Vec3 Vec3::Abs(Vec3 const& value)
	{
		return Vec3(PBE::Abs(value.x), PBE::Abs(value.y), PBE::Abs(value.z));
	}

	Vec3 Vec3::MakeFromPolarDegrees(float yaw, float pitch, float radius)
	{
		float x = CosDegrees(yaw) * SinDegrees(pitch);
		float y = CosDegrees(pitch);
		float z = SinDegrees(yaw) * SinDegrees(pitch);

		return Vec3(x,y,z) * radius;
	}

	Vec3 const operator*(float scalar, Vec3 const& vec)
	{
		return Vec3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
	}
}