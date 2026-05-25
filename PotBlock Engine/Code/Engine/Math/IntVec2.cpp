#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"

namespace PBE
{
	IntVec2 const IntVec2::ONE = IntVec2(1, 1);
	IntVec2 const IntVec2::ZERO = IntVec2(0, 0);
	IntVec2 const IntVec2::YNEG = IntVec2(0, 1);
	IntVec2 const IntVec2::YPOS = IntVec2(0, -1);
	IntVec2 const IntVec2::LEFT = IntVec2(-1, 0);
	IntVec2 const IntVec2::RIGHT = IntVec2(1, 0);

	IntVec2::IntVec2()
		: x(0)
		, y(0)
	{

	}

	IntVec2::IntVec2(int x, int y)
		: x(x)
		, y(y)
	{

	}

	IntVec2::IntVec2(IntVec2 const& other) noexcept
		: x(other.x)
		, y(other.y)
	{
	}

	IntVec2 const IntVec2::operator+(IntVec2 const& other) const
	{
		return IntVec2(x + other.x, y + other.y);
	}

	IntVec2 const IntVec2::operator-(IntVec2 const& other) const
	{
		return IntVec2(x - other.x, y - other.y);
	}

	IntVec2 const IntVec2::operator*(IntVec2 const& other) const
	{
		return IntVec2(x * other.x, y * other.y);
	}

	IntVec2 const IntVec2::operator/(IntVec2 const& other) const
	{
		return IntVec2(x / other.x, y / other.y);
	}

	IntVec2 const IntVec2::operator*(int scalar) const
	{
		return IntVec2(x * scalar, y * scalar);
	}

	IntVec2 const IntVec2::operator/(int scalar) const
	{
		return IntVec2(x / scalar, y / scalar);
	}

	IntVec2& IntVec2::operator/=(int scalar)
	{
		x /= scalar;
		y /= scalar;
		return *this;
	}

	bool IntVec2::operator!=(IntVec2 const& other) const
	{
		return x != other.x || y != other.y;
	}

	bool IntVec2::operator==(IntVec2 const& other) const
	{
		return x == other.x && y == other.y;
	}

	IntVec2& IntVec2::operator*=(int scalar)
	{
		x *= scalar;
		y *= scalar;
		return *this;
	}

	IntVec2& IntVec2::operator-=(IntVec2 const& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}

	IntVec2& IntVec2::operator+=(IntVec2 const& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	std::string IntVec2::ToString() const
	{
		return std::string("IntVec2(") + std::to_string(x) + ", " + std::to_string(y) + ")";
	}


	int IntVec2::GetLengthSquared() const
	{
		return x * x + y * y;
	}

	IntVec2 IntVec2::GetRotated90Degrees() const
	{
		return IntVec2(-y, x);
	}

	IntVec2 IntVec2::GetRotatedMinus90Degrees() const
	{
		return IntVec2(y, -x);
	}

	IntVec2 IntVec2::GetFlipped() const
	{
		return IntVec2(-x, -y);
	}

	IntVec2 IntVec2::GetNegated() const
	{
		return IntVec2(-x, -y);
	}

	IntVec2 IntVec2::GetAbs() const
	{
		return IntVec2(PBE::Abs(x), PBE::Abs(y));
	}

	IntVec2 IntVec2::GetMin(IntVec2 const& other) const
	{
		return IntVec2(PBE::Min(x, other.x), PBE::Min(y, other.y));
	}

	IntVec2 IntVec2::GetMax(IntVec2 const& other) const
	{
		return IntVec2(PBE::Max(x, other.x), PBE::Max(y, other.y));
	}

	IntVec2 IntVec2::GetClamped(IntVec2 const& min, IntVec2 const& max) const
	{
		return IntVec2(PBE::Clamp(x, min.x, max.x), PBE::Clamp(y, min.y, max.y));
	}

	IntVec2 IntVec2::GetSign() const
	{
		return IntVec2(PBE::Sign(x), PBE::Sign(y));
	}

	float IntVec2::Distance(IntVec2 const& a, IntVec2 const& b)
	{
		Vec2 const diff = Vec2(b) - Vec2(a);
		return diff.GetLength();
	}

	int IntVec2::DistanceSquared(IntVec2 const& a, IntVec2 const& b)
	{
		IntVec2 const diff = b - a;
		return diff.GetLengthSquared();

	}

	float IntVec2::AngleBetween(IntVec2 const& a, IntVec2 const& b)
	{
		return Atan2(static_cast<float>(b.y - a.y), static_cast<float>(b.x - a.x));
	}

	IntVec2 IntVec2::Min(IntVec2 const& a, IntVec2 const& b)
	{
		return IntVec2(PBE::Min(a.x, b.x), PBE::Min(a.y, b.y));
	}

	IntVec2 IntVec2::Max(IntVec2 const& a, IntVec2 const& b)
	{
		return IntVec2(PBE::Max(a.x, b.x), PBE::Max(a.y, b.y));
	}

	IntVec2 IntVec2::Clamp(IntVec2 const& value, IntVec2 const& min, IntVec2 const& max)
	{
		return IntVec2(PBE::Clamp(value.x, min.x, max.x), PBE::Clamp(value.y, min.y, max.y));
	}

	IntVec2 IntVec2::Sign(IntVec2 const& value)
	{
		return IntVec2(PBE::Sign(value.x), PBE::Sign(value.y));
	}

	IntVec2 IntVec2::Abs(IntVec2 const& value)
	{
		return IntVec2(PBE::Abs(value.x), PBE::Abs(value.y));
	}

	IntVec2 const operator*(int scalar, IntVec2 const& vec)
	{
		return IntVec2(vec.x * scalar, vec.y * scalar);
	}

}