#include "Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

namespace PBE
{
	Vec2 const Vec2::ONE = Vec2(1.f, 1.f);
	Vec2 const Vec2::ZERO = Vec2(0.f, 0.f);
	Vec2 const Vec2::YNEG = Vec2(0.f, 1.f);
	Vec2 const Vec2::YPOS = Vec2(0.f, -1.f);
	Vec2 const Vec2::LEFT = Vec2(-1.f, 0.f);
	Vec2 const Vec2::RIGHT = Vec2(1.f, 0.f);

	Vec2::Vec2()
		: x(0.f)
		, y(0.f)
	{
	}

	Vec2::Vec2(float x, float y)
		: x(x)
		, y(y)
	{
	}

	Vec2::Vec2(Vec2 const& other) noexcept
		: x(other.x)
		, y(other.y)
	{
	}

	Vec2::Vec2(IntVec2 const& other) noexcept
		: x(static_cast<float>(other.x))
		, y(static_cast<float>(other.y))
	{
	}

	Vec2 const Vec2::operator+(Vec2 const& other) const
	{
		return Vec2(x + other.x, y + other.y);
	}

	Vec2 const Vec2::operator-(Vec2 const& other) const
	{
		return Vec2(x - other.x, y - other.y);
	}

	Vec2 const Vec2::operator*(Vec2 const& other) const
	{
		return Vec2(x * other.x, y * other.y);
	}

	Vec2 const Vec2::operator/(Vec2 const& other) const
	{
		return Vec2(x / other.x, y / other.y);
	}

	Vec2 const Vec2::operator*(float scalar) const
	{
		return Vec2(x * scalar, y * scalar);
	}

	Vec2 const Vec2::operator/(float scalar) const
	{
		float scale = 1.f / scalar;
		return Vec2(x * scale, y * scale);
	}

	Vec2& Vec2::operator/=(float scalar)
	{
		float scale = 1.f / scalar;
		x *= scale;
		y *= scale;
		return *this;
	}

	Vec2& Vec2::operator-()
	{
		x = -x;
		y = -y;
		return *this;
	}

	bool Vec2::operator!=(Vec2 const& other) const
	{
		return x != other.x || y != other.y;
	}

	bool Vec2::operator==(Vec2 const& other) const
	{
		return x == other.x && y == other.y;
	}

	Vec2& Vec2::operator*=(float scalar)
	{
		x *= scalar;
		y *= scalar;
		return *this;
	}

	Vec2& Vec2::operator-=(Vec2 const& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}

	Vec2& Vec2::operator+=(Vec2 const& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	Vec2 const operator*(float scalar, Vec2 const& vec)
	{
		return Vec2(vec.x * scalar, vec.y * scalar);
	}

	std::string Vec2::ToString() const
	{
		return std::string("Vec2(") + std::to_string(x) + ", " + std::to_string(y) + ")";
	}

	float Vec2::Dot(Vec2 const& other) const
	{
		return x * other.x + y * other.y;
	}

	float Vec2::Cross(Vec2 const& other) const
	{
		return x * other.y - y * other.x;
	}

	float Vec2::GetAngleBetweenDegrees(Vec2 const& other) const
	{
		return AcosDegrees(Dot(other) / (GetLength() * other.GetLength()));
	}

	float Vec2::GetDistance(Vec2 const& other) const
	{
		return (other - *this).GetLength();
	}

	float Vec2::GetDistanceSquared(Vec2 const& other) const
	{
		return (other - *this).GetLengthSquared();
	}

	float Vec2::GetLength() const
	{
		return Sqrt(x * x + y * y);
	}

	float Vec2::GetLengthSquared() const
	{
		return x * x + y * y;
	}

	float Vec2::GetAngle() const
	{
		return Atan2(y, x);
	}

	float Vec2::GetAngleDegrees() const
	{
		return Atan2Degrees(y, x);
	}

	void Vec2::Normalize()
	{
		float length = GetLength();

		if (length == 0.f)
		{
			return;
		}

		float scale = 1.f / length;
		x *= scale;
		y *= scale;
	}

	void Vec2::Rotate(float angle)
	{
		float radians = GetAngle() + angle;
		float length = GetLength();
		x = length * Cos(radians);
		y = length * Sin(radians);
	}

	void Vec2::RotateAroundPivot(float angle, Vec2 const& pivot)
	{
		Vec2 diff = *this - pivot;
		diff.Rotate(angle);
		*this = pivot + diff;
	}

	void Vec2::Reflect(Vec2 const& normal)
	{
		float dot = DotProduct(*this, normal);
		Vec2 const& projection = dot * normal;
		*this = *this - 2.f * projection;
	}

	void Vec2::Project(Vec2 const& axis)
	{
		float dot = DotProduct(*this, axis);
		float lengthSquared = axis.GetLengthSquared();
		if (lengthSquared == 0.f)
		{
			*this = Vec2::ZERO;
			return;
		}
		*this = dot * (axis / lengthSquared);
	}

	void Vec2::Rotate90Degrees()
	{
		std::swap(x, y);
		x = -x;
	}

	void Vec2::RotateMinus90Degrees()
	{
		std::swap(x, y);
		y = -y;
	}

	void Vec2::Flip()
	{
		x = -x;
		y = -y;
	}

	void Vec2::Negate()
	{
		x = -x;
		y = -y;
	}

	Vec2 Vec2::GetNormalized() const
	{
		float length = GetLength();
		if (length == 0.f)
		{
			return Vec2::ZERO;
		}
		float scale = 1.f / length;
		return Vec2(x * scale, y * scale);
	}

	Vec2 Vec2::GetRotated(float angle) const
	{
		return Vec2(x * Cos(angle) - y * Sin(angle), x * Sin(angle) + y * Cos(angle));
	}


	Vec2 Vec2::GetRotatedAroundPivot(float angle, Vec2 const& pivot) const
	{
		Vec2 diff = *this - pivot;
		return diff.GetRotated(angle) + pivot;
	}

	Vec2 Vec2::GetReflected(Vec2 const& normal) const
	{
		float dot = DotProduct(*this, normal);
		return *this - 2.f * dot * normal;
	}

	Vec2 Vec2::GetProjected(Vec2 const& axis) const
	{
		float dot = DotProduct(*this, axis);
		float lengthSquared = axis.GetLengthSquared();
		if (lengthSquared == 0.f)
		{
			return Vec2::ZERO;
		}
		return axis * (dot / lengthSquared);
	}

	float Vec2::GetProjectedLength(Vec2 const& normal) const
	{
		float dot = DotProduct(*this, normal);
		return dot;
	}

	Vec2 Vec2::GetRotated90Degrees() const
	{
		return Vec2(-y, x);
	}

	Vec2 Vec2::GetRotatedMinus90Degrees() const
	{
		return Vec2(y, -x);
	}

	Vec2 Vec2::GetFlipped() const
	{
		return Vec2(-x, -y);
	}

	Vec2 Vec2::GetNegated() const
	{
		return Vec2(-x, -y);
	}

	Vec2 Vec2::GetAbs() const
	{
		return Vec2(PBE::Abs(x), PBE::Abs(y));
	}

	Vec2 Vec2::GetMin(Vec2 const& other) const
	{
		return Vec2(PBE::Min(x, other.x), PBE::Min(y, other.y));
	}

	Vec2 Vec2::GetMax(Vec2 const& other) const
	{
		return Vec2(PBE::Max(x, other.x), PBE::Max(y, other.y));
	}

	Vec2 Vec2::GetClamped(Vec2 const& min, Vec2 const& max) const
	{
		return GetMax(min).GetMin(max);
	}

	Vec2 Vec2::GetLerped(Vec2 const& other, float t) const
	{
		return *this + (other - *this) * t;
	}

	Vec2 Vec2::GetFloored() const
	{
		return Vec2(static_cast<float>(PBE::Floor(x)), static_cast<float>(PBE::Floor(y)));
	}

	Vec2 Vec2::GetCeiled() const
	{
		return Vec2(static_cast<float>(PBE::Ceil(x)), static_cast<float>(PBE::Ceil(y)));
	}

	Vec2 Vec2::GetRound() const
	{
		return Vec2(static_cast<float>(PBE::Round(x)), static_cast<float>(PBE::Round(y)));
	}

	Vec2 Vec2::GetSign() const
	{
		return Vec2(static_cast<float>(PBE::Sign(x)), static_cast<float>(PBE::Sign(y)));
	}

	Vec2 Vec2::GetReciprocal() const
	{
		return Vec2(1.f / x, 1.f / y);
	}

	Vec2 const Vec2::MakeFromPolarDegrees(float angleDegrees, float length)
	{
		return Vec2(PBE::CosDegrees(angleDegrees) * length, PBE::SinDegrees(angleDegrees) * length);
	}

	float Vec2::DotProduct(Vec2 const& a, Vec2 const& b)
	{
		return a.x * b.x + a.y * b.y;
	}

	float Vec2::CrossProduct(Vec2 const& a, Vec2 const& b)
	{
		return a.x * b.y - a.y * b.x;
	}

	float Vec2::Distance(Vec2 const& a, Vec2 const& b)
	{
		return (a - b).GetLength();
	}

	float Vec2::DistanceSquared(Vec2 const& a, Vec2 const& b)
	{
		return (a - b).GetLengthSquared();
	}

	float Vec2::AngleBetween(Vec2 const& a, Vec2 const& b)
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

	Vec2 Vec2::Lerp(Vec2 const& a, Vec2 const& b, float t)
	{
		return a + (b - a) * t;
	}

	Vec2 Vec2::Min(Vec2 const& a, Vec2 const& b)
	{
		return Vec2(PBE::Min(a.x, b.x), PBE::Min(a.y, b.y));
	}

	Vec2 Vec2::Max(Vec2 const& a, Vec2 const& b)
	{
		return Vec2(PBE::Max(a.x, b.x), PBE::Max(a.y, b.y));
	}

	Vec2 Vec2::Clamp(Vec2 const& value, Vec2 const& min, Vec2 const& max)
	{
		return value.GetMax(min).GetMin(max);
	}

	Vec2 Vec2::Floor(Vec2 const& value)
	{
		return Vec2(static_cast<float>(PBE::Floor(value.x)), static_cast<float>(PBE::Floor(value.y)));
	}

	Vec2 Vec2::Ceil(Vec2 const& value)
	{
		return Vec2(static_cast<float>(PBE::Ceil(value.x)), static_cast<float>(PBE::Ceil(value.y)));
	}

	Vec2 Vec2::Round(Vec2 const& value)
	{
		return Vec2(static_cast<float>(PBE::Round(value.x)), static_cast<float>(PBE::Round(value.y)));
	}

	Vec2 Vec2::Sign(Vec2 const& value)
	{
		return Vec2(static_cast<float>(PBE::Sign(value.x)), static_cast<float>(PBE::Sign(value.y)));
	}

	Vec2 Vec2::Reciprocal(Vec2 const& value)
	{
		return Vec2(1.f / value.x, 1.f / value.y);
	}

	Vec2 Vec2::Abs(Vec2 const& value)
	{
		return Vec2(PBE::Abs(value.x), PBE::Abs(value.y));
	}

}