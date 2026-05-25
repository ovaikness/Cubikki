#include "Engine/Math/MathUtils.hpp"
namespace PBE
{
	float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
	{
		return RadToDeg(atan2f(a.Cross(b), a.Dot(b)));
	}

	float ComputeCubicBezier1D(float A, float B, float C, float D, float t)
	{
		float AB = Interpolate(A, B, t);
		float BC = Interpolate(B, C, t);
		float CD = Interpolate(C, D, t);

		float ABC = Interpolate(AB, BC, t);
		float BCD = Interpolate(BC, CD, t);

		return Interpolate(ABC, BCD, t);
	}

	float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t)
	{
		float AB = Interpolate(A, B, t);
		float BC = Interpolate(B, C, t);
		float CD = Interpolate(C, D, t);
		float DE = Interpolate(D, E, t);
		float EF = Interpolate(E, F, t);

		float ABC = Interpolate(AB, BC, t);
		float BCD = Interpolate(BC, CD, t);
		float CDE = Interpolate(CD, DE, t);
		float DEF = Interpolate(DE, EF, t);

		float ABCD = Interpolate(ABC, BCD, t);
		float BCDE = Interpolate(BCD, CDE, t);
		float CDEF = Interpolate(CDE, DEF, t);

		float ABCDE = Interpolate(ABCD, BCDE, t);
		float BCDEF = Interpolate(BCDE, CDEF, t);

		return Interpolate(ABCDE, BCDEF, t);
	}

	Vec2 SphericalInterpolate2D(Vec2 const& start, Vec2 const& end, float t)
	{
		float deg = GetAngleDegreesBetweenVectors2D(start, end);
		return (CosDegrees(deg * t) / CosDegrees(deg)) * start + (CosDegrees(deg * (1.f - t)) / CosDegrees(deg)) * end;
	}

	bool IsNearlyEqual(float a, float b, float epsilon)
	{
		return Abs(a - b) < epsilon;
	}

	bool IsNearlyZero(float value, float epsilon)
	{
		return Abs(value) < epsilon;
	}

	float RangeMap(float value, float origin, float originEnd, float destination, float destinationEnd)
	{
		float t = (value - origin) / (originEnd - origin);
		return Lerp(destination, destinationEnd, t);
	}

	float RangeMapClamped(float value, float origin, float originEnd, float destination, float destinationEnd)
	{
		return Clamp(RangeMap(value,origin,originEnd,destination,destinationEnd),destination,destinationEnd);
	}

	float PBE::NormalizeByte(unsigned char c)
	{
		return static_cast<float>(c) / 255.f;
	}

	unsigned char PBE::DenormalizeByte(float f)
	{
		// Ensure f is in the range [0.0, 1.0]
		if (f < 0.f)
		{
			f = 0.f;
		}
		else if (f > 1.f)
		{
			f = 1.f;
		}

		return static_cast<unsigned char>(f * 255.f + 0.5f);
	}

	void NormalizeBytes(unsigned char* bytes, int count, float* out_floats)
	{
		for (int i = 0; i < count; ++i)
		{
			out_floats[i] = NormalizeByte(bytes[i]);
		}
	}

	void DenormalizeBytes(float* floats, int count, unsigned char* out_bytes)
	{
		for (int i = 0; i < count; ++i)
		{
			out_bytes[i] = DenormalizeByte(floats[i]);
		}
	}
}