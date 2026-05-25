#pragma once

#include "Engine/Math/Vec2.hpp"

#include <cmath>
#include <cassert>

namespace PBE
{
	constexpr float const PI = 3.1415926535897932384626433832795f;

	template<typename T_ValueType>
	inline constexpr T_ValueType const PositiveModulo(T_ValueType const& value, T_ValueType const& modulus)
	{
		return (value % modulus + modulus) % modulus;
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Min(T_ValueType const& a)
	{
		return a;
	}

	template<typename T_ValueType, typename... Args>
	inline constexpr T_ValueType const Min(T_ValueType const& first, Args const&... args)
	{
		T_ValueType result = first;
		((result = (args < result) ? args : result), ...);
		return result;
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Max(T_ValueType const& a)
	{
		return a;
	}

	template<typename T_ValueType, typename... Args>
	inline constexpr T_ValueType const Max(T_ValueType const& first, Args const&... args)
	{
		T_ValueType result = first;
		((result = ((T_ValueType)args > result) ? (T_ValueType)args : result), ...);
		return result;
	}

	template<typename T_ValueType>
	inline constexpr int Floor(T_ValueType const& value)
	{
		return static_cast<int>(value < 0.f ? value - 1.f : value);
	}

	template<typename T_ValueType>
	inline constexpr int Ceil(T_ValueType const& value)
	{
		return static_cast<int>(value < 0.f ? value : value + 1.f);
	}

	template<typename T_ValueType>
	inline constexpr int Round(T_ValueType const& value)
	{
		return static_cast<int>(value < 0 ? value - 0.5f : value + 0.5f);
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Clamp(T_ValueType const& value, T_ValueType const& min, T_ValueType const& max)
	{
		return Min(Max(value, min), max);
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Lerp(T_ValueType const& a, T_ValueType const& b, float t)
	{
		return static_cast<T_ValueType>(a + (b - a) * t);
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Abs(T_ValueType const& value)
	{
		return (value < 0) ? -value : value;
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Sign(T_ValueType const& value)
	{
		return static_cast<T_ValueType>((value < static_cast<T_ValueType>(0)) ? -1 : 1);
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Square(T_ValueType const& value)
	{
		return value * value;
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Cube(T_ValueType const& value)
	{
		return value * value * value;
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Pow(T_ValueType const& value, int power)
	{
		assert((power < 1) && "Pow does not support power values less than 1!");
		T_ValueType result = value;
		for (int i = 1; i < power; ++i)
		{
			result *= value;
		}
		return result;
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Sqrt(T_ValueType const& value)
	{
		return static_cast<T_ValueType>(sqrt(value));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const InverseSqrt(T_ValueType const& value)
	{
		return static_cast<T_ValueType>(1 / Sqrt(value));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const DegToRad(T_ValueType const& value)
	{
		return value * (PI / 180.f);
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const RadToDeg(T_ValueType const& value)
	{
		return value * (180.f / PI);
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Atan2(T_ValueType const& y, T_ValueType const& x)
	{
		return static_cast<T_ValueType>(atan2(y, x));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Asin(T_ValueType const& value)
	{
		return static_cast<T_ValueType>(asin(value));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Acos(T_ValueType const& value)
	{
		return static_cast<T_ValueType>(acos(value));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Tan(T_ValueType const& value)
	{
		return static_cast<T_ValueType>(tan(value));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Sin(T_ValueType const& value)
	{
		return static_cast<T_ValueType>(sin(value));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Cos(T_ValueType const& value)
	{
		return static_cast<T_ValueType>(cos(value));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const Atan2Degrees(T_ValueType const& y, T_ValueType const& x)
	{
		return RadToDeg(Atan2(y, x));
	}
	template<typename T_ValueType>
	inline constexpr T_ValueType const AcosDegrees(T_ValueType const& value)
	{
		return RadToDeg(Acos(value));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const AsinDegrees(T_ValueType const& value)
	{
		return RadToDeg(Asin(value));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const TanDegrees(T_ValueType const& value)
	{
		return Tan(DegToRad(value));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const SinDegrees(T_ValueType const& value)
	{
		return Sin(DegToRad(value));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const CosDegrees(T_ValueType const& value)
	{
		return Cos(DegToRad(value));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const RangeMap(T_ValueType const& value, T_ValueType const& origin, T_ValueType const& originEnd, T_ValueType const& destination, T_ValueType const& destinationEnd)
	{
		return destination + (destinationEnd - destination) * ((value - origin) / (originEnd - origin));
	}

	template<typename T_ValueType>
	inline constexpr T_ValueType const RangeMapClamped(T_ValueType const& value, T_ValueType const& origin, T_ValueType const& originEnd, T_ValueType const& destination, T_ValueType const& destinationEnd)
	{
		return Clamp(RangeMap(value, origin, originEnd, destination, destinationEnd), Min(destination, destinationEnd), Max(destination, destinationEnd));
	}

	float ComputeCubicBezier1D(float A, float B, float C, float D, float t);
	float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t);

	Vec2 SphericalInterpolate2D(Vec2 const& start, Vec2 const& end, float t);
	float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b);

	template<typename T>
	T ComputeCubicBezier(T A, T B, T C, T D, float t)
	{
		T AB = Interpolate(A, B, t);
		T BC = Interpolate(B, C, t);
		T CD = Interpolate(C, D, t);

		T ABC = Interpolate(AB, BC, t);
		T BCD = Interpolate(BC, CD, t);

		return Interpolate(ABC, BCD, t);
	}

	template<typename T>
	T ComputeQuinticBezier(T A, T B, T C, T D, T E, T F, float t)
	{
		T AB = Interpolate(A, B, t);
		T BC = Interpolate(B, C, t);
		T CD = Interpolate(C, D, t);
		T DE = Interpolate(D, E, t);
		T EF = Interpolate(E, F, t);

		T ABC = Interpolate(AB, BC, t);
		T BCD = Interpolate(BC, CD, t);
		T CDE = Interpolate(CD, DE, t);
		T DEF = Interpolate(DE, EF, t);

		T ABCD = Interpolate(ABC, BCD, t);
		T BCDE = Interpolate(BCD, CDE, t);
		T CDEF = Interpolate(CDE, DEF, t);

		T ABCDE = Interpolate(ABCD, BCDE, t);
		T BCDEF = Interpolate(BCDE, CDEF, t);

		return Interpolate(ABCDE, BCDEF, t);
	}

	template<typename T>
	T Interpolate(T start, T end, float t)
	{
		return T(start * (1.f - t) + end * t);
	}
	template <typename T>
	T SmoothStart2(T start, T end, float t)
	{
		float tPow = t * t;
		return T(start * (1.f - tPow) + end * tPow);
	}

	template <typename T>
	T SmoothStart3(T start, T end, float t)
	{
		float tPow = t * t * t;
		return T(start * (1.f - tPow) + end * tPow);
	}
	template <typename T>
	T SmoothStart4(T start, T end, float t)
	{
		float tPow = t * t * t * t;
		return T(start * (1.f - tPow) + end * tPow);
	}
	template <typename T>
	T SmoothStart5(T start, T end, float t)
	{
		float tPow = t * t * t * t * t;
		return T((start * (1.f - tPow)) + (end * tPow));
	}
	template <typename T>
	T SmoothStart6(T start, T end, float t)
	{
		float tPow = t * t * t * t * t * t;
		return T(start * (1.f - tPow) + end * tPow);
	}

	template <typename T>
	T SmoothStop2(T start, T end, float t)
	{
		float invT = 1.f - t;
		float tPow = 1.f - invT * invT;
		return T(start * (1.f - tPow) + end * tPow);
	}

	template <typename T>
	T SmoothStop3(T start, T end, float t)
	{
		float invT = 1.f - t;
		float tPow = 1.f - invT * invT * invT;
		return T(start * (1.f - tPow) + end * tPow);
	}
	template <typename T>
	T SmoothStop4(T start, T end, float t)
	{
		float invT = 1.f - t;
		float tPow = 1.f - invT * invT * invT * invT;
		return T(start * (1.f - tPow) + end * tPow);
	}
	template <typename T>
	T SmoothStop5(T start, T end, float t)
	{
		float invT = 1.f - t;
		float tPow = 1.f - invT * invT * invT * invT * invT;
		return T(start * (1.f - tPow) + end * tPow);
	}
	template <typename T>
	T SmoothStop6(T start, T end, float t)
	{
		float invT = 1.f - t;
		float tPow = 1.f - invT * invT * invT * invT * invT * invT;
		return T(start * (1.f - tPow) + end * tPow);
	}

	template <typename T>
	T SmoothStep3(T start, T end, float t)
	{
		return Interpolate(SmoothStart2(start, end, t), SmoothStop2(start, end, t), t);
	}

	template <typename T>
	T SmoothStep5(T start, T end, float t)
	{
		float tBezier = ComputeQuinticBezier1D(0.f, 0.f, 0.f, 1.f, 1.f, 1.f, t);
		return Interpolate(start, end, tBezier);
	}

	template <typename T>
	T Hesitate3(T start, T end, float t)
	{
		float tBezier = ComputeCubicBezier1D(0.f, 1.f, 0.f, 1.f, t);
		return Interpolate(start, end, tBezier);
	}

	template<typename T>
	T Hesitate5(T start, T end, float t)
	{
		float tBezier = ComputeQuinticBezier1D(0.f, 1.f, 0.f, 1.f, 0.f, 1.f, t);
		return Interpolate(start, end, tBezier);
	}

	
	bool IsNearlyEqual(float a, float b, float epsilon = 0.0001f);
	bool IsNearlyZero(float value, float epsilon = 0.0001f);

	float RangeMap(float value, float origin, float originEnd, float destination, float destinationEnd);
	float RangeMapClamped(float value, float origin, float originEnd, float destination, float destinationEnd);

	float NormalizeByte(unsigned char c);
	unsigned char DenormalizeByte(float f);

	void NormalizeBytes(unsigned char* bytes, int count, float* out_floats);
	void DenormalizeBytes(float* floats, int count, unsigned char* out_bytes);
}