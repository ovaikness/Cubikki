#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
namespace PBE
{
	IntVec3 const IntVec3::ZERO = IntVec3(0, 0, 0);
	IntVec3 const IntVec3::ONE = IntVec3(1, 1, 1);
	IntVec3 const IntVec3::YNEG = IntVec3(0, 1, 0);
	IntVec3 const IntVec3::YPOS = IntVec3(0, -1, 0);
	IntVec3 const IntVec3::LEFT = IntVec3(-1, 0, 0);
	IntVec3 const IntVec3::RIGHT = IntVec3(1, 0, 0);
	IntVec3 const IntVec3::FORWARD = IntVec3(0, 0, 1);
	IntVec3 const IntVec3::BACKWARD = IntVec3(0, 0, -1);

	IntVec3::IntVec3()
		: x(0)
		, y(0)
		, z(0)
	{

	}

	IntVec3::IntVec3(int x, int y, int z)
		: x(x)
		, y(y)
		, z(z)
	{

	}

	IntVec3::IntVec3(Vec3 const& other)
		: x(static_cast<int>(other.x))
		, y(static_cast<int>(other.y))
		, z(static_cast<int>(other.z))
	{
	}

	IntVec3::IntVec3(IntVec3 const& other)
		: x(other.x)
		, y(other.y)
		, z(other.z)
	{
	}

	IntVec3 const IntVec3::operator*(float scalar) const
	{
		float newX = static_cast<float>(this->x) * scalar;
		float newY = static_cast<float>(this->y) * scalar;
		float newZ = static_cast<float>(this->z) * scalar;
		return IntVec3(static_cast<int>(newX), static_cast<int>(newY), static_cast<int>(newZ));
	}

	IntVec3 const IntVec3::operator+(IntVec3 const& other) const
	{
		return IntVec3(x + other.x, y + other.y, z + other.z);
	}

	IntVec3 const IntVec3::operator-(IntVec3 const& other) const
	{
		return IntVec3(x - other.x, y - other.y, z - other.z);
	}

	IntVec3 const IntVec3::operator*(int scalar) const
	{
		return IntVec3(x * scalar, y * scalar, z * scalar);
	}

	IntVec3 const IntVec3::operator/(int scalar) const
	{
		return IntVec3(x / scalar, y / scalar, z / scalar);
	}

	IntVec3 const IntVec3::operator-() const
	{
		return IntVec3(-x, -y, -z);
	}

	IntVec3& IntVec3::operator/=(int scalar)
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}

	IntVec3& IntVec3::operator*=(int scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	IntVec3& IntVec3::operator-=(IntVec3 const& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;

		return *this;
	}

	IntVec3& IntVec3::operator+=(IntVec3 const& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;

		return *this;
	}

	bool IntVec3::operator!=(IntVec3 const& other) const
	{
		return x != other.x || y != other.y || z != other.z;
	}

	bool IntVec3::operator==(IntVec3 const& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	std::string IntVec3::ToString() const
	{
		return std::string("IntVec3(") + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
	}

	int IntVec3::GetLengthSquared() const
	{
		return x * x + y * y + z * z;
	}

	int IntVec3::GetAngle() const
	{
		return PBE::Atan2(y, x);
	}

	void IntVec3::Negate()
	{
		x = -x;
		y = -y;
		z = -z;
	}

	IntVec3 IntVec3::GetNegated() const
	{
		return IntVec3(-x, -y, -z);
	}

	IntVec3 IntVec3::GetAbs() const
	{
		return IntVec3(PBE::Abs(x), PBE::Abs(y), PBE::Abs(z));
	}

	IntVec3 IntVec3::GetMin(IntVec3 const& other) const
	{
		return IntVec3(PBE::Min(x, other.x), PBE::Min(y, other.y), PBE::Min(z, other.z));
	}

	IntVec3 IntVec3::GetMax(IntVec3 const& other) const
	{
		return IntVec3(PBE::Max(x, other.x), PBE::Max(y, other.y), PBE::Max(z, other.z));
	}

	IntVec3 IntVec3::GetClamped(IntVec3 const& min, IntVec3 const& max) const
	{
		return IntVec3(PBE::Clamp(x, min.x, max.x), PBE::Clamp(y, min.y, max.y), PBE::Clamp(z, min.z, max.z));
	}

	IntVec3 IntVec3::GetLerped(IntVec3 const& other, float t) const
	{
		return IntVec3(PBE::Lerp(x, other.x, t), PBE::Lerp(y, other.y, t), PBE::Lerp(z, other.z, t));
	}

	IntVec3 IntVec3::GetSign() const
	{
		return IntVec3(PBE::Sign(x), PBE::Sign(y), PBE::Sign(z));
	}

	int IntVec3::DotProduct(IntVec3 const& a, IntVec3 const& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	IntVec3 IntVec3::CrossProduct(IntVec3 const& a, IntVec3 const& b)
	{
		IntVec3 result;
		result.x = a.y * b.z - a.z * b.y;
		result.y = a.z * b.x - a.x * b.z;
		result.z = a.x * b.y - a.y * b.x;
		return result;
	}

	int IntVec3::DistanceSquared(IntVec3 const& a, IntVec3 const& b)
	{
		return (a - b).GetLengthSquared();
	}

	IntVec3 IntVec3::Lerp(IntVec3 const& a, IntVec3 const& b, float t)
	{
		return a + (b - a) * t;
	}

	IntVec3 IntVec3::Min(IntVec3 const& a, IntVec3 const& b)
	{
		return IntVec3(PBE::Min(a.x, b.x), PBE::Min(a.y, b.y), PBE::Min(a.z, b.z));
	}

	IntVec3 IntVec3::Max(IntVec3 const& a, IntVec3 const& b)
	{
		return IntVec3(PBE::Max(a.x, b.x), PBE::Max(a.y, b.y), PBE::Max(a.z, b.z));
	}

	IntVec3 IntVec3::Clamp(IntVec3 const& value, IntVec3 const& min, IntVec3 const& max)
	{
		return IntVec3(PBE::Clamp(value.x, min.x, max.x), PBE::Clamp(value.y, min.y, max.y), PBE::Clamp(value.z, min.z, max.z));
	}

	IntVec3 IntVec3::Sign(IntVec3 const& value)
	{
		return IntVec3(PBE::Sign(value.x), PBE::Sign(value.y), PBE::Sign(value.z));
	}

	IntVec3 IntVec3::Abs(IntVec3 const& value)
	{
		return IntVec3(PBE::Abs(value.x), PBE::Abs(value.y), PBE::Abs(value.z));
	}

	IntVec3 const operator*(int scalar, IntVec3 const& vec)
	{
		return IntVec3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
	}
}