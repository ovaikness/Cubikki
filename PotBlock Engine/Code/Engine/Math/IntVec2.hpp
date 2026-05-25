#pragma once
#include <string>

namespace PBE
{
	class IntVec2
	{
	public:
		int x;
		int y;

		static IntVec2 const ZERO;
		static IntVec2 const ONE;
		static IntVec2 const YNEG;
		static IntVec2 const YPOS;
		static IntVec2 const LEFT;
		static IntVec2 const RIGHT;
	public:
		IntVec2();
		IntVec2(int x, int y);
		IntVec2(IntVec2 const& other) noexcept;

		IntVec2 const operator+(IntVec2 const& other) const;
		IntVec2 const operator-(IntVec2 const& other) const;

		IntVec2 const operator*(IntVec2 const& other) const;
		IntVec2 const operator/(IntVec2 const& other) const;

		IntVec2 const operator*(int scalar) const;
		IntVec2 const operator/(int scalar) const;

		friend IntVec2 const operator*(int scalar, IntVec2 const& vec);

		IntVec2& operator+=(IntVec2 const& other);
		IntVec2& operator-=(IntVec2 const& other);
		IntVec2& operator*=(int scalar);
		IntVec2& operator/=(int scalar);

		bool operator==(IntVec2 const& other) const;
		bool operator!=(IntVec2 const& other) const;

		std::string ToString() const;

		int GetLengthSquared() const;
		IntVec2 GetRotated90Degrees() const;
		IntVec2 GetRotatedMinus90Degrees() const;
		IntVec2 GetFlipped() const;
		IntVec2 GetNegated() const;
		IntVec2 GetAbs() const;
		IntVec2 GetMin(IntVec2 const& other) const;
		IntVec2 GetMax(IntVec2 const& other) const;
		IntVec2 GetClamped(IntVec2 const& min, IntVec2 const& max) const;
		IntVec2 GetSign() const;

		static float Distance(IntVec2 const& a, IntVec2 const& b);
		static int DistanceSquared(IntVec2 const& a, IntVec2 const& b);
		static float AngleBetween(IntVec2 const& a, IntVec2 const& b);

		static IntVec2 Min(IntVec2 const& a, IntVec2 const& b);
		static IntVec2 Max(IntVec2 const& a, IntVec2 const& b);
		static IntVec2 Clamp(IntVec2 const& value, IntVec2 const& min, IntVec2 const& max);
		static IntVec2 Sign(IntVec2 const& value);
		static IntVec2 Abs(IntVec2 const& value);
	};


	class IntVec2Comparator
	{
	public:
		bool operator()(PBE::IntVec2 const& a, PBE::IntVec2 const& b) const
		{
			if (a.x < b.x)
				return true;
			if (a.x > b.x)
				return false;

			if (a.y < b.y)
				return true;
			if (a.y > b.y)
				return false;

			return false;
		}
	};
}