#pragma once
#include <string>

namespace PBE
{
	class Vec3;
	class IntVec3
	{
	public:
		int x;
		int y;
		int z;

		static IntVec3 const ZERO;
		static IntVec3 const ONE;
		static IntVec3 const YNEG;
		static IntVec3 const YPOS;
		static IntVec3 const LEFT;
		static IntVec3 const RIGHT;
		static IntVec3 const FORWARD;
		static IntVec3 const BACKWARD;
	public:
		IntVec3();
		IntVec3(int x, int y, int z);
		explicit IntVec3(Vec3 const& other);
		IntVec3(IntVec3 const& other);

		IntVec3 const operator*(float scalar) const;
		IntVec3 const operator+(IntVec3 const& other) const;
		IntVec3 const operator-(IntVec3 const& other) const;
		IntVec3 const operator*(int scalar) const;
		IntVec3 const operator/(int scalar) const;
		IntVec3 const operator-() const;

		friend IntVec3 const operator*(int scalar, IntVec3 const& vec);

		IntVec3& operator+=(IntVec3 const& other);
		IntVec3& operator-=(IntVec3 const& other);
		IntVec3& operator*=(int scalar);
		IntVec3& operator/=(int scalar);

		bool operator==(IntVec3 const& other) const;
		bool operator!=(IntVec3 const& other) const;

		std::string ToString() const;

		int GetLengthSquared() const;
		int GetAngle() const;

		void Negate();

		IntVec3 GetNegated() const;
		IntVec3 GetAbs() const;
		IntVec3 GetMin(IntVec3 const& other) const;
		IntVec3 GetMax(IntVec3 const& other) const;
		IntVec3 GetClamped(IntVec3 const& min, IntVec3 const& max) const;
		IntVec3 GetLerped(IntVec3 const& other, float t) const;
		IntVec3 GetSign() const;

		static int DotProduct(IntVec3 const& a, IntVec3 const& b);
		static IntVec3 CrossProduct(IntVec3 const& a, IntVec3 const& b);
		static int DistanceSquared(IntVec3 const& a, IntVec3 const& b);

		static IntVec3 Lerp(IntVec3 const& a, IntVec3 const& b, float t);
		static IntVec3 Min(IntVec3 const& a, IntVec3 const& b);
		static IntVec3 Max(IntVec3 const& a, IntVec3 const& b);
		static IntVec3 Clamp(IntVec3 const& value, IntVec3 const& min, IntVec3 const& max);
		static IntVec3 Sign(IntVec3 const& value);
		static IntVec3 Abs(IntVec3 const& value);
	};


	class IntVec3Comparator
	{
	public:
		bool operator()(PBE::IntVec3 const& a, PBE::IntVec3 const& b) const
		{
			if (a.x < b.x)
				return true;
			if (a.x > b.x)
				return false;

			if (a.y < b.y)
				return true;
			if (a.y > b.y)
				return false;

			if (a.z < b.z)
				return true;
			if (a.z > b.z)
				return false;

			return false;
		}
	};
}

namespace std {
	template<>
	struct hash<PBE::IntVec3>
	{
		std::size_t operator()(PBE::IntVec3 const& vec) const noexcept
		{
			std::size_t h1 = std::hash<int>{}(vec.x);
			std::size_t h2 = std::hash<int>{}(vec.y);
			std::size_t h3 = std::hash<int>{}(vec.z);
			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};
}