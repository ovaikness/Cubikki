#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat4.hpp"

namespace PBE
{
	enum EulerAngleBasisMode
	{
		EULER_ANGLE_BASIS_MODE_IFWD_JLEFT_KUP,
		EULER_ANGLE_BASIS_MODE_IRIGHT_JDOWN_KFORWARD
	};

	class EulerAngles
	{
	public:
		static EulerAngles const ZERO;

		float m_Yaw;
		float m_Pitch;
		float m_Roll;
	public:
		EulerAngles();
		EulerAngles(float yaw, float pitch, float roll);
		EulerAngles(EulerAngles const& other);

		static EulerAngles CreateFromLookRotation(Vec3 const& forward, Vec3 const& up = Vec3::YNEG);
		static void SetBasisMode(EulerAngleBasisMode mode);
		void GetAsVectors(Vec3* out_iBasis, Vec3* out_jBasis, Vec3* outkBasis) const;
		Mat4 GetAsMatrix() const;

		EulerAngles operator+(EulerAngles const& other) const;
		EulerAngles& operator+=(EulerAngles const& other);
		EulerAngles operator-(EulerAngles const& other) const;
		EulerAngles& operator-=(EulerAngles const& other);

		EulerAngles operator-() const;
	protected:
		static EulerAngleBasisMode s_currentBasisMode;
	};

	float AngleBetween(EulerAngles const& a, EulerAngles const& b);
}