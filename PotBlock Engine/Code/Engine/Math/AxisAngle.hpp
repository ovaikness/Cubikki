#pragma once

#include "Engine/Math/Vec3.hpp"

namespace PBE
{
	class AxisAngle
	{
	public:
		float m_Angle;
		Vec3 m_Axis;
	public:
		AxisAngle() = default;
		AxisAngle(float angle, Vec3 const& axis)
			: m_Angle(angle)
			, m_Axis(axis)
		{
		}

		void GetBasisAsVectors(Vec3* outIBasis, Vec3* outJBasis, Vec3* outKBasis) const
		{
			float cosTheta = cos(m_Angle);
			float sinTheta = sin(m_Angle);
			float oneMinusCosTheta = 1.f - cosTheta;
			float iix = oneMinusCosTheta * m_Axis.x * m_Axis.x + cosTheta;
			float iiy = oneMinusCosTheta * m_Axis.x * m_Axis.y - m_Axis.z * sinTheta;
			float iiz = oneMinusCosTheta * m_Axis.x * m_Axis.z + m_Axis.y * sinTheta;
			float jix = oneMinusCosTheta * m_Axis.x * m_Axis.y + m_Axis.z * sinTheta;
			float jiy = oneMinusCosTheta * m_Axis.y * m_Axis.y + cosTheta;
			float jiz = oneMinusCosTheta * m_Axis.y * m_Axis.z - m_Axis.x * sinTheta;
			float kix = oneMinusCosTheta * m_Axis.x * m_Axis.z - m_Axis.y * sinTheta;
			float kiy = oneMinusCosTheta * m_Axis.y * m_Axis.z + m_Axis.x * sinTheta;
			float kiz = oneMinusCosTheta * m_Axis.z * m_Axis.z + cosTheta;

			if (outIBasis)
			{
				*outIBasis = Vec3(iix, iiy, iiz);
			}

			if (outJBasis)
			{
				*outJBasis = Vec3(jix, jiy, jiz);
			}

			if (outKBasis)
			{
				*outKBasis = Vec3(kix, kiy, kiz);
			}
		}
	};
}