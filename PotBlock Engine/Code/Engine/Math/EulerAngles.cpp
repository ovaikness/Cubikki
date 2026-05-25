#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/MathUtils.hpp"
namespace PBE
{
	EulerAngleBasisMode EulerAngles::s_currentBasisMode = EULER_ANGLE_BASIS_MODE_IFWD_JLEFT_KUP;
	EulerAngles const EulerAngles::ZERO = EulerAngles(0, 0, 0);

	EulerAngles::EulerAngles() : EulerAngles(0, 0, 0)
	{
	}

	EulerAngles::EulerAngles(float yaw, float pitch, float roll)
		: m_Yaw(yaw)
		, m_Pitch(pitch)
		, m_Roll(roll)
	{
	}

	EulerAngles::EulerAngles(EulerAngles const& other)
		: m_Yaw(other.m_Yaw)
		, m_Pitch(other.m_Pitch)
		, m_Roll(other.m_Roll)
	{
	}

	EulerAngles EulerAngles::CreateFromLookRotation(Vec3 const& forward, Vec3 const& up)
	{
		//#TODO : This doesnt work. Needs a relative basis to function.

		Vec3 normalizedForward = forward.GetNormalized();
		Vec3 right = Vec3::CrossProduct(up, normalizedForward).GetNormalized();
		Vec3 newUp = Vec3::CrossProduct(normalizedForward, right);

		float Ix = right.x;
		//float Iy = right.y;
		//float Iz = right.z;

		float Jx = newUp.x;
		//float Jy = newUp.y;
		float Jz = newUp.z;

		float Kx = normalizedForward.x;
		//float Ky = normalizedForward.y;
		float Kz = normalizedForward.z;

		float pitch = Asin(-Kx);
		float roll  = Atan2(Jx, Ix);
		float yaw   = Atan2(Jz, Kz);

		return EulerAngles(yaw, pitch, roll);
	}

	void EulerAngles::SetBasisMode(EulerAngleBasisMode mode)
	{
		s_currentBasisMode = mode;
	}

	void EulerAngles::GetAsVectors(Vec3* out_iBasis, Vec3* out_jBasis, Vec3* out_kBasis) const
	{
		switch (s_currentBasisMode)
		{
			case EULER_ANGLE_BASIS_MODE_IRIGHT_JDOWN_KFORWARD:
			{
				float cosYaw = CosDegrees(m_Yaw);
				float sinYaw = SinDegrees(m_Yaw);
				float cosPitch = CosDegrees(m_Pitch);
				float sinPitch = SinDegrees(m_Pitch);
				float cosRoll = CosDegrees(m_Roll);
				float sinRoll = SinDegrees(m_Roll);

				// Applying yaw first (around the y-axis), then pitch (around the x-axis), then roll (around the z-axis)
				if (out_iBasis)
				{
					out_iBasis->x = cosYaw * cosRoll + sinYaw * sinPitch * sinRoll;
					out_iBasis->y = sinRoll * cosPitch;
					out_iBasis->z = -sinYaw * cosRoll + cosYaw * sinPitch * sinRoll;
				}

				if (out_jBasis)
				{
					out_jBasis->x = -cosYaw * sinRoll + sinYaw * sinPitch * cosRoll;
					out_jBasis->y = cosRoll * cosPitch;
					out_jBasis->z = sinRoll * sinYaw + cosYaw * sinPitch * cosRoll;
				}

				if (out_kBasis)
				{
				
					out_kBasis->x = sinYaw * cosPitch;
					out_kBasis->y = -sinPitch;
					out_kBasis->z = cosYaw * cosPitch;
				}
				break;
			}

			case EULER_ANGLE_BASIS_MODE_IFWD_JLEFT_KUP:
			{
				float cosYaw = CosDegrees(m_Yaw);
				float sinYaw = SinDegrees(m_Yaw);
				float cosPitch = CosDegrees(m_Pitch);
				float sinPitch = SinDegrees(m_Pitch);
				float cosRoll = CosDegrees(m_Roll);
				float sinRoll = SinDegrees(m_Roll);

				// Yaw: Rotation around Z (up)
				// Pitch: Rotation around X (forward)
				// Roll: Rotation around Y (left)

				// iBasis (forward, X)
				if (out_iBasis)
				{
					out_iBasis->x = cosPitch * cosYaw;
					out_iBasis->y = -cosPitch * sinYaw;
					out_iBasis->z = sinPitch;
				}

				// jBasis (left, Y)
				if (out_jBasis)
				{
					out_jBasis->x = sinRoll * sinPitch * cosYaw + cosRoll * sinYaw;
					out_jBasis->y = -sinRoll * sinPitch * sinYaw + cosRoll * cosYaw;
					out_jBasis->z = -sinRoll * cosPitch;
				}

				// kBasis (up, Z)
				if (out_kBasis)
				{
					out_kBasis->x = -cosRoll * sinPitch * cosYaw + sinRoll * sinYaw;
					out_kBasis->y = cosRoll * sinPitch * sinYaw + sinRoll * cosYaw;
					out_kBasis->z = cosRoll * cosPitch;
				}

				break;
			}
		}
	}

	PBE::Mat4 EulerAngles::GetAsMatrix() const
	{
		Vec3 iBasis, jBasis, kBasis;
		GetAsVectors(&iBasis, &jBasis, &kBasis);
		return Mat4(iBasis,jBasis,kBasis, Vec3(0.f,0.f,0.f));
	}

	PBE::EulerAngles EulerAngles::operator+(EulerAngles const& other) const
	{
		EulerAngles angles;
		angles.m_Yaw = m_Yaw + other.m_Yaw;
		angles.m_Pitch = m_Pitch + other.m_Pitch;
		angles.m_Roll = m_Roll + other.m_Roll;
		return angles;
	}

	PBE::EulerAngles& EulerAngles::operator+=(EulerAngles const& other)
	{
		*this = *this + other;
		return *this;
	}

	PBE::EulerAngles EulerAngles::operator-(EulerAngles const& other) const
	{
		EulerAngles angles;
		angles.m_Yaw = m_Yaw - other.m_Yaw;
		angles.m_Pitch = m_Pitch - other.m_Pitch;
		angles.m_Roll = m_Roll - other.m_Roll;
		return angles;
	}

	PBE::EulerAngles& EulerAngles::operator-=(EulerAngles const& other)
	{
		*this = *this - other;
		return *this;
	}

	EulerAngles EulerAngles::operator-() const
	{
		EulerAngles angles;
		angles.m_Yaw = -m_Yaw;
		angles.m_Pitch = -m_Pitch;
		angles.m_Roll = -m_Roll;

		return angles;
	}

}

float PBE::AngleBetween(EulerAngles const& a, EulerAngles const& b)
{
	PBE::Vec3 forwardA, forwardB;

	a.GetAsVectors(&forwardA, nullptr, nullptr);
	b.GetAsVectors(&forwardB, nullptr, nullptr);

	float dotProduct = Vec3::DotProduct(forwardA, forwardB);
	float angle = Acos(dotProduct);

	// Ensure the angle is in the range [0, 180] degrees
	if (angle < 0.f)
	{
		angle = -angle;
	}

	if (angle > 180.f)
	{
		angle = 360.f - angle;
	}

	if (angle < 0.f)
	{
		angle = 0.f;
	}

	if (angle > 180.f)
	{
		angle = 180.f;
	}

	// Return the angle in degrees
	return angle * (180.f / PBE::PI); // Convert radians to degrees
}