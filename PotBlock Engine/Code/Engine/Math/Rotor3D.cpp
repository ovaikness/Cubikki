#include "Engine/Math/Rotor3D.hpp"
#include "Engine/Math/MathUtils.hpp"

PBE::Rotor3D const PBE::Rotor3D::IDENTITY = PBE::Rotor3D(0.f, 0.f, 0.f, 1.f);

PBE::Rotor3D PBE::Rotor3D::CreateFromVectors(Vec3 const& a, Vec3 const& b)
{
	Vec3 aNormalized = a.GetNormalized();
	Vec3 bNormalized = b.GetNormalized();

	Vec3 halfBNorm = (aNormalized + bNormalized).GetNormalized();

	return Rotor3D(aNormalized, halfBNorm);
}

PBE::Rotor3D PBE::Rotor3D::CreateFromEulerAngles(float pitch, float yaw, float roll)
{
	float cosYaw = CosDegrees(yaw * 0.5f);
	float sinYaw = SinDegrees(yaw * 0.5f);
	float cosPitch = CosDegrees(pitch * 0.5f);
	float sinPitch = SinDegrees(pitch * 0.5f);
	float cosRoll = CosDegrees(roll * 0.5f);
	float sinRoll = SinDegrees(roll * 0.5f);

	Rotor3D result;
	result.w = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;
	result.i = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
	result.j = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
	result.k = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;

	return result.GetNormalized();
}

PBE::Rotor3D PBE::Rotor3D::CreateFromEulerAngles(EulerAngles const& angles)
{
	float cosPitch = CosDegrees(angles.m_Pitch * 0.5f);
	float sinPitch = SinDegrees(angles.m_Pitch * 0.5f);
	float cosYaw = CosDegrees(-angles.m_Yaw * 0.5f);
	float sinYaw = SinDegrees(-angles.m_Yaw * 0.5f);
	float cosRoll = CosDegrees(angles.m_Roll * 0.5f);
	float sinRoll = SinDegrees(angles.m_Roll * 0.5f);

	Rotor3D result;
	result.w = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;
	result.i = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
	result.j = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
	result.k = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;

	return result.GetNormalized();
}

PBE::Rotor3D PBE::Rotor3D::Slerp(Rotor3D const& start, Rotor3D const& end, float t)
{
	//Fallback try linear
	
	if (t == 0.f)
	{
		return start;
	}

	if (t == 1.f)
	{
		return end;
	}

	// Compute the dot product
	float dot = start.w * end.w + start.i * end.i + start.j * end.j + start.k * end.k;
	dot = Clamp(dot, -1.f, 1.f);

	// Ensure shortest path by flipping the end quaternion if necessary
	Rotor3D endCopy = end;
	if (dot < 0.f)
	{
		dot = -dot;
		endCopy = Rotor3D(-end.i, -end.j, -end.k, -end.w);
	}

	// Calculate angle and sine
	float theta = AcosDegrees(dot);
	float sinTheta = SinDegrees(theta);

	// If theta is very small, fall back to linear interpolation
	if (sinTheta < 0.01f)
	{
		return (start * (1.f - t) + endCopy * t).GetNormalized();
	}

	// Compute interpolation factors
	float a = SinDegrees((1.f - t) * theta) / sinTheta;
	float b = SinDegrees(t * theta) / sinTheta;

	// Interpolate and normalize the result
	Rotor3D result = start * a + endCopy * b;
	return result.GetNormalized();
}

PBE::Rotor3D::Rotor3D(Vec3 const& a, Vec3 const& b)
{
	w = Vec3::DotProduct(a, b);
	i = a.y * b.z - a.z * b.y;
	j = a.z * b.x - a.x * b.z;
	k = a.x * b.y - a.y * b.x;
}

PBE::Rotor3D PBE::Rotor3D::operator*(Rotor3D const& rhs) const
{
	Rotor3D result;
	result.w = w * rhs.w - i * rhs.i - j * rhs.j - k * rhs.k;
	result.i = w * rhs.i + i * rhs.w + j * rhs.k - k * rhs.j;
	result.j = w * rhs.j - i * rhs.k + j * rhs.w + k * rhs.i;
	result.k = w * rhs.k + i * rhs.j - j * rhs.i + k * rhs.w;

	return result;
}

PBE::Rotor3D PBE::Rotor3D::operator+(Rotor3D const& rhs) const
{
	Rotor3D result;
	result.w = w + rhs.w;
	result.i = i + rhs.i;
	result.j = j + rhs.j;
	result.k = k + rhs.k;

	return result;
}

PBE::Rotor3D& PBE::Rotor3D::operator+=(Rotor3D const& rhs)
{
	w = w + rhs.w;
	i = i + rhs.i;
	j = j + rhs.j;
	k = k + rhs.k;

	return *this;

}

PBE::Rotor3D PBE::Rotor3D::operator-() const
{
	return Rotor3D(i, j, k, -w);
}

PBE::Rotor3D PBE::Rotor3D::operator-(Rotor3D const& rhs) const
{
	return *this + -rhs;
}

PBE::Rotor3D& PBE::Rotor3D::operator*=(float scalar)
{
	w *= scalar;
	i *= scalar;
	j *= scalar;
	k *= scalar;

	return *this;
}

PBE::Rotor3D& PBE::Rotor3D::operator*=(Rotor3D const& rhs)
{
	*this = *this * rhs;
	return *this;

}

PBE::Rotor3D PBE::Rotor3D::operator*(float scalar) const
{
	Rotor3D result;
	result.w = w * scalar;
	result.i = i * scalar;
	result.j = j * scalar;
	result.k = k * scalar;

	return result;
}

PBE::Rotor3D& PBE::Rotor3D::operator-=(Rotor3D const& rhs)
{
	w = w - rhs.w;
	i = i - rhs.i;
	j = j - rhs.j;
	k = k - rhs.k;

	return *this;
}

float PBE::Rotor3D::GetMagnitude() const
{
	return Sqrt(w * w + i * i + j * j + k * k);
}

PBE::Rotor3D PBE::Rotor3D::GetNormalized() const
{
	float mag = GetMagnitude();

	if (mag == 0.f)
	{
		return Rotor3D(0.f, 0.f, 0.f, 0.f);
	}

	return *this * (1.f / mag);
}

PBE::Rotor3D PBE::Rotor3D::GetConjugate() const
{
	return Rotor3D(-i, -j, -k, w);
}

PBE::Rotor3D PBE::Rotor3D::GetInverse() const
{
	return GetConjugate() * (1.f / (w * w + i * i + j * j + k * k));
}

PBE::Mat4 PBE::Rotor3D::GetMatrix() const
{
	float xx = i * i;
	float yy = j * j;
	float zz = k * k;

	float xy = i * j;
	float xz = i * k;
	float xw = i * w;

	float yz = j * k;
	float yw = j * w;

	float zw = k * w;

	Vec4 iBasis = Vec4(1 - 2 * (yy + zz), 2 * (xy + zw), 2 * (xz - yw), 0);
	Vec4 jBasis = Vec4(2 * (xy - zw), 1 - 2 * (xx + zz), 2 * (yz + xw), 0);
	Vec4 kBasis = Vec4(2 * (xz + yw), 2 * (yz - xw), 1 - 2 * (xx + yy), 0);
	Vec4 tBasis = Vec4(0, 0, 0, 1);

	Mat4 result
	{
		iBasis,
		jBasis,
		kBasis,
		tBasis
	};

	return result;
}

PBE::Vec3 PBE::Rotor3D::GetRotatedVector(Vec3 const& vector) const
{
	Rotor3D q = *this * Rotor3D(vector.x, vector.y, vector.z, 0) * GetInverse();
	return Vec3(q.i, q.j, q.k);

}

PBE::EulerAngles PBE::Rotor3D::GetEulerAngles() const
{
	float sinPitch = 2.f * (w * j - k * i);
	float pitch = AsinDegrees(sinPitch);

	float yaw = 0.f;
	float roll = 0.f;

	if (sinPitch < 0.99999f)
	{
		yaw = Atan2Degrees(2.f * (w * k + i * j), 1.f - 2.f * (j * j + k * k));
		roll = Atan2Degrees(2.f * (w * i + j * k), 1.f - 2.f * (i * i + j * j));
	}
	else
	{
		yaw = Atan2Degrees(2.f * (i * j - w * k), 2.f * (w * w + i * i) - 1.f);
	}

	return EulerAngles(pitch, yaw, roll);
}
