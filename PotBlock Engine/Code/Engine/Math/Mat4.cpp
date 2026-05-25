#include "Engine/Math/Mat4.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Rotor3D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"

#include <immintrin.h>

namespace PBE
{
	size_t const Mat4::Ix = 0;

	size_t const Mat4::Iy = 1;

	size_t const Mat4::Iz = 2;

	size_t const Mat4::Iw = 3;

	size_t const Mat4::Jx = 4;

	size_t const Mat4::Jy = 5;

	size_t const Mat4::Jz = 6;

	size_t const Mat4::Jw = 7;

	size_t const Mat4::Kx = 8;

	size_t const Mat4::Ky = 9;

	size_t const Mat4::Kz = 10;

	size_t const Mat4::Kw = 11;

	size_t const Mat4::Tx = 12;

	size_t const Mat4::Ty = 13;

	size_t const Mat4::Tz = 14;

	size_t const Mat4::Tw = 15;

	Mat4 const Mat4::ZERO =
		Mat4(
			0.f, 0.f, 0.f, 0.f, 
			0.f, 0.f, 0.f, 0.f, 
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f
		);

	PBE::Mat4 const Mat4::ZERO;

	Mat4 const Mat4::IDENTITY =
		Mat4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);

	PBE::Mat4 const Mat4::IDENTITY;

	Mat4::Mat4()
		: m_Values
		{
				1.f,0.f,0.f,0.f,
				0.f,1.f,0.f,0.f,
				0.f,0.f,1.f,0.f,
				0.f,0.f,0.f,1.f
		}
	{
	}

	Mat4::Mat4(Mat4 const& other)
		: m_Values
		{
			other.m_Values[0], other.m_Values[1], other.m_Values[2], other.m_Values[3],
			other.m_Values[4], other.m_Values[5], other.m_Values[6], other.m_Values[7],
			other.m_Values[8], other.m_Values[9], other.m_Values[10], other.m_Values[11],
			other.m_Values[12], other.m_Values[13], other.m_Values[14], other.m_Values[15]
		}
	{
	}

	Mat4::Mat4(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis, Vec3 const& tBasis)
		: m_Values
		{
			iBasis.x, iBasis.y, iBasis.z,  0.f,
			jBasis.x, jBasis.y, jBasis.z,  0.f,
			kBasis.x, kBasis.y, kBasis.z , 0.f,
			tBasis.x, tBasis.y, tBasis.z , 1.f
		}
	{
	}

	Mat4::Mat4(Vec4 const& iBasis, Vec4 const& jBasis, Vec4 const& kBasis, Vec4 const& tBasis)
		: m_Values
		{
			iBasis.x,iBasis.y,iBasis.z,iBasis.w,
			jBasis.x,jBasis.y,jBasis.z,jBasis.w,
			kBasis.x,kBasis.y,kBasis.z,kBasis.w,
			tBasis.x,tBasis.y,tBasis.z,tBasis.w
		}
	{
	}

	Mat4::Mat4(float iBasisX, float iBasisY, float iBasisZ, float iBasisW,
		float jBasisX, float jBasisY, float jBasisZ, float jBasisW,
		float kBasisX, float kBasisY, float kBasisZ, float kBasisW,
		float translationX, float translationY, float translationZ, float translationW)
		: m_Values{ iBasisX, iBasisY, iBasisZ, iBasisW,
		jBasisX, jBasisY, jBasisZ, jBasisW,
		kBasisX, kBasisY, kBasisZ, kBasisW,
		translationX, translationY, translationZ, translationW }
	{
	}

	Mat4 Mat4::GetOrthonormalized() const
	{
		//grant schmidt's method
		Vec3 iBasis = GetIBasis3D();
		Vec3 jBasis = GetJBasis3D();
		Vec3 kBasis = GetKBasis3D();

		iBasis = iBasis.GetNormalized();
		jBasis = jBasis - iBasis * Vec3::DotProduct(iBasis,jBasis);
		jBasis = jBasis.GetNormalized();
		kBasis = kBasis - iBasis * Vec3::DotProduct(iBasis,kBasis) - jBasis * Vec3::DotProduct(jBasis,kBasis);
		kBasis = kBasis.GetNormalized();

		Mat4 result = Mat4(iBasis, jBasis, kBasis, GetTranslation3D());
		return result;
	
	}

	Mat4 Mat4::GetTransposed() const
	{
		Mat4 result;
		__m128 xRow = _mm_loadu_ps(&m_Values[0]);
		__m128 yRow = _mm_loadu_ps(&m_Values[4]);
		__m128 zRow = _mm_loadu_ps(&m_Values[8]);
		__m128 tRow = _mm_loadu_ps(&m_Values[12]);

		__m128 iResult = _mm_setr_ps(m_Values[Ix], m_Values[Jx], m_Values[Kx], m_Values[Tx]);
		__m128 jResult = _mm_setr_ps(m_Values[Iy], m_Values[Jy], m_Values[Ky], m_Values[Ty]);
		__m128 kResult = _mm_setr_ps(m_Values[Iz], m_Values[Jz], m_Values[Kz], m_Values[Tz]);
		__m128 tResult = _mm_setr_ps(m_Values[Iw], m_Values[Jw], m_Values[Kw], m_Values[Tw]);

		_mm_storeu_ps(&result.m_Values[0], iResult);
		_mm_storeu_ps(&result.m_Values[4], jResult);
		_mm_storeu_ps(&result.m_Values[8], kResult);
		_mm_storeu_ps(&result.m_Values[12], tResult);

		return result;
	
	}

	Mat4 Mat4::GetOrthonormalInverse() const
	{
		Mat4 copy = Mat4::IDENTITY;

		copy.m_Values[Ix] = this->m_Values[Ix];
		copy.m_Values[Iy] = this->m_Values[Jx];
		copy.m_Values[Iz] = this->m_Values[Kx];

		copy.m_Values[Jx] = this->m_Values[Iy];
		copy.m_Values[Jy] = this->m_Values[Jy];
		copy.m_Values[Jz] = this->m_Values[Ky];

		copy.m_Values[Kx] = this->m_Values[Iz];
		copy.m_Values[Ky] = this->m_Values[Jz];
		copy.m_Values[Kz] = this->m_Values[Kz];

		Vec3 translation = Vec3(-this->m_Values[Tx], -this->m_Values[Ty], -this->m_Values[Tz]);

		copy.m_Values[Tx] = Vec3::DotProduct(this->GetIBasis3D(), translation);
		copy.m_Values[Ty] = Vec3::DotProduct(this->GetJBasis3D(), translation);
		copy.m_Values[Tz] = Vec3::DotProduct(this->GetKBasis3D(), translation);
		copy.m_Values[Tw] = 1.f;
		return copy;
	}

	float Mat4::GetDeterminant() const {
		return m_Values[Ix] * (m_Values[Jy] * m_Values[Kz] - m_Values[Ky] * m_Values[Jz])
			- m_Values[Iy] * (m_Values[Jx] * m_Values[Kz] - m_Values[Kx] * m_Values[Jz])
			+ m_Values[Iz] * (m_Values[Jx] * m_Values[Ky] - m_Values[Kx] * m_Values[Jy]);
	}

	Mat4 Mat4::GetInverse() const {
		float det = GetDeterminant();
		if (det == 0.f) return Mat4::ZERO;
		float invDet = 1.f / det;

		Mat4 result;
		// Calculate inverse of the 3x3 rotation/scale part
		result.m_Values[Ix] = (m_Values[Jy] * m_Values[Kz] - m_Values[Ky] * m_Values[Jz]) * invDet;
		result.m_Values[Iy] = (m_Values[Iz] * m_Values[Ky] - m_Values[Iy] * m_Values[Kz]) * invDet;
		result.m_Values[Iz] = (m_Values[Iy] * m_Values[Jz] - m_Values[Jy] * m_Values[Iz]) * invDet;

		result.m_Values[Jx] = (m_Values[Kx] * m_Values[Jz] - m_Values[Jx] * m_Values[Kz]) * invDet;
		result.m_Values[Jy] = (m_Values[Ix] * m_Values[Kz] - m_Values[Kx] * m_Values[Iz]) * invDet;
		result.m_Values[Jz] = (m_Values[Jx] * m_Values[Iz] - m_Values[Ix] * m_Values[Jz]) * invDet;

		result.m_Values[Kx] = (m_Values[Jx] * m_Values[Ky] - m_Values[Kx] * m_Values[Jy]) * invDet;
		result.m_Values[Ky] = (m_Values[Kx] * m_Values[Iy] - m_Values[Ix] * m_Values[Ky]) * invDet;
		result.m_Values[Kz] = (m_Values[Ix] * m_Values[Jy] - m_Values[Jx] * m_Values[Iy]) * invDet;

		// Invert translation part
		result.m_Values[Tx] = -(m_Values[Tx] * result.m_Values[Ix] + m_Values[Ty] * result.m_Values[Jx] + m_Values[Tz] * result.m_Values[Kx]);
		result.m_Values[Ty] = -(m_Values[Tx] * result.m_Values[Iy] + m_Values[Ty] * result.m_Values[Jy] + m_Values[Tz] * result.m_Values[Ky]);
		result.m_Values[Tz] = -(m_Values[Tx] * result.m_Values[Iz] + m_Values[Ty] * result.m_Values[Jz] + m_Values[Tz] * result.m_Values[Kz]);

		result.m_Values[Iw] = -(result.m_Values[Ix] * m_Values[Iw] +
			result.m_Values[Iy] * m_Values[Jw] +
			result.m_Values[Iz] * m_Values[Kw]);

		result.m_Values[Jw] = -(result.m_Values[Jx] * m_Values[Iw] +
			result.m_Values[Jy] * m_Values[Jw] +
			result.m_Values[Jz] * m_Values[Kw]);

		result.m_Values[Kw] = -(result.m_Values[Kx] * m_Values[Iw] +
			result.m_Values[Ky] * m_Values[Jw] +
			result.m_Values[Kz] * m_Values[Kw]);


		return result;
	}

	Mat4 Mat4::GetAppended(Mat4 const& other) const
	{
		Mat4 result; // Temporary matrix to store the accumulated results

		__m128 xRow = _mm_loadu_ps(&other.m_Values[0]);
		__m128 yRow = _mm_loadu_ps(&other.m_Values[4]);
		__m128 zRow = _mm_loadu_ps(&other.m_Values[8]);
		__m128 tRow = _mm_loadu_ps(&other.m_Values[12]);

		// Perform the matrix multiplication
		__m128 iResult = _mm_add_ps(
			_mm_add_ps(_mm_mul_ps(xRow, _mm_set1_ps(m_Values[0])), _mm_mul_ps(yRow, _mm_set1_ps(m_Values[1]))),
			_mm_add_ps(_mm_mul_ps(zRow, _mm_set1_ps(m_Values[2])), _mm_mul_ps(tRow, _mm_set1_ps(m_Values[3])))
		);

		__m128 jResult = _mm_add_ps(
			_mm_add_ps(_mm_mul_ps(xRow, _mm_set1_ps(m_Values[4])), _mm_mul_ps(yRow, _mm_set1_ps(m_Values[5]))),
			_mm_add_ps(_mm_mul_ps(zRow, _mm_set1_ps(m_Values[6])), _mm_mul_ps(tRow, _mm_set1_ps(m_Values[7])))
		);

		__m128 kResult = _mm_add_ps(
			_mm_add_ps(_mm_mul_ps(xRow, _mm_set1_ps(m_Values[8])), _mm_mul_ps(yRow, _mm_set1_ps(m_Values[9]))),
			_mm_add_ps(_mm_mul_ps(zRow, _mm_set1_ps(m_Values[10])), _mm_mul_ps(tRow, _mm_set1_ps(m_Values[11])))
		);

		__m128 tResult = _mm_add_ps(
			_mm_add_ps(_mm_mul_ps(xRow, _mm_set1_ps(m_Values[12])), _mm_mul_ps(yRow, _mm_set1_ps(m_Values[13]))),
			_mm_add_ps(_mm_mul_ps(zRow, _mm_set1_ps(m_Values[14])), _mm_mul_ps(tRow, _mm_set1_ps(m_Values[15])))
		);

		_mm_storeu_ps(&result.m_Values[0], iResult);
		_mm_storeu_ps(&result.m_Values[4], jResult);
		_mm_storeu_ps(&result.m_Values[8], kResult);
		_mm_storeu_ps(&result.m_Values[12], tResult);
		
		return result;
	}

	Mat4 Mat4::Append(Mat4 const& other)
	{
		*this = GetAppended(other);
		return *this;
	}

	Vec2 Mat4::TransformPoint(Vec2 const& point) const
	{
		Vec2 iProjection = point.x * GetIBasis2D();
		Vec2 jProjection = point.y * GetJBasis2D();
		return iProjection + jProjection + GetTranslation2D();
	}

	Vec3 Mat4::TransformPoint(Vec3 const& point) const
	{
		Vec3 iProjection = point.x * GetIBasis3D();
		Vec3 jProjection = point.y * GetJBasis3D();
		Vec3 kProjection = point.z * GetKBasis3D();
		return iProjection + jProjection + kProjection + GetTranslation3D();
	}

	Vec4 Mat4::TransformPoint(Vec4 const& point) const
	{
		Vec4 iProjection = point.x * GetIBasis4D();
		Vec4 jProjection = point.y * GetJBasis4D();
		Vec4 kProjection = point.z * GetKBasis4D();
		Vec4 tProjection = point.w * GetTranslation4D();
		return iProjection + jProjection + kProjection + tProjection;
	}

	Mat4 const Mat4::CreateLookAt(Vec3 const& eye, Vec3 const& center, Vec3 const& up)
	{
		Vec3 displacement = center - eye;
		Vec3 f = (center - eye).GetNormalized();
		Vec3 r = Vec3::CrossProduct(f, up).GetNormalized();
		Vec3 u = Vec3::CrossProduct(r, f);

		Mat4 result(f, -r, u, Vec3(0.f));

		return result;
	}

	Mat4 const Mat4::CreateTranslation3D(Vec3 const& translation)
	{
		Mat4 result = Mat4::IDENTITY;
		result.m_Values[Tx] = translation.x;
		result.m_Values[Ty] = translation.y;
		result.m_Values[Tz] = translation.z;
		result.m_Values[Tw] = 1.f;
		return result;
	}

	PBE::Mat4 const Mat4::CreateOrthographicProjection(float left, float right, float bottom, float top, float near, float far)
	{
		Mat4 mat;
		mat.m_Values[Ix] = 2.f / (right - left);
		mat.m_Values[Jy] = -2.f / (top - bottom);
		mat.m_Values[Kz] = 1.f / (far - near);

		mat.m_Values[Tx] = -(right + left) / (right - left);
		mat.m_Values[Ty] = (top + bottom) / (top - bottom);
		mat.m_Values[Tz] = (1.f + -(far + near) / (far - near)) * 0.5f;
		mat.m_Values[Tw] = 1.f;

		return mat;
	}

	PBE::Mat4 const Mat4::CreatePerspectiveProjection(float fov, float aspectRatio, float near, float far)
	{
		Mat4 mat = Mat4::IDENTITY;

		float tanHalfFov = TanDegrees(fov * 0.5f);

		float scaleY = 1.0f / tanHalfFov;
		float scaleX = scaleY / aspectRatio;
		float scaleZ = far / (far - near);

		float tz = (near * far) / (near - far);

		mat.m_Values[Ix] = scaleX;
		mat.m_Values[Jy] = scaleY;
		mat.m_Values[Kz] = scaleZ;
		mat.m_Values[Kw] = 1.0f;
		mat.m_Values[Tz] = tz;
		mat.m_Values[Tw] = 0.0f;

		return mat;
	}

	PBE::Mat4 const Mat4::CreateScale(float scale)
	{
		Mat4 mat = Mat4::IDENTITY;
		mat.m_Values[Ix] = scale;
		mat.m_Values[Jy] = scale;
		mat.m_Values[Kz] = scale;
		return mat;
	}

	Mat4 const Mat4::CreateScale(Vec3 const& scale)
	{
		Mat4 mat = Mat4::IDENTITY;
		mat.m_Values[Ix] = scale.x;
		mat.m_Values[Jy] = scale.y;
		mat.m_Values[Kz] = scale.z;
		return mat;
	}

	Mat4 const Mat4::Interpolate(Mat4 const& start, Mat4 const& end, float t)
	{
		Rotor3D startRotation = start.GetRotationAsQuaternion();
		Rotor3D endRotation = end.GetRotationAsQuaternion();
		Vec3 startTranslation = start.GetTranslation3D();
		Vec3 endTranslation = end.GetTranslation3D();

		Rotor3D rotation = Rotor3D::Slerp(startRotation, endRotation, t);

		Mat4 result = rotation.GetMatrix();
		Vec3 translation = startTranslation + (endTranslation - startTranslation) * t;

		result.SetTranslation3D(translation);

		return result;
	}

	Mat4 const Mat4::CreateTRS(Vec3 const& translation, Rotor3D const& rotation, Vec3 const& scale)
	{
		Mat4 transMat = Mat4::CreateTranslation3D(translation);
		Mat4 rotMat = rotation.GetNormalized().GetMatrix();
		Mat4 scaleMat = Mat4::CreateScale(scale);

		return transMat.GetAppended(rotMat).GetAppended(scaleMat);
	}

	Mat4 const Mat4::CreateTRS(Vec3 const& translation, EulerAngles const& rotation, Vec3 const& scale)
	{
		Mat4 transMat = Mat4::CreateTranslation3D(translation);
		Mat4 rotMat = rotation.GetAsMatrix();
		Mat4 scaleMat = Mat4::CreateScale(scale);

		return scaleMat.GetAppended(rotMat).GetAppended(transMat);
	}


	Mat4 const Mat4::Translate3D(Vec3 const& translation)
	{
		Mat4 translationMatrix = Mat4::CreateTranslation3D(translation);
		this->Append(translationMatrix);
		return *this;
	}

	float& Mat4::operator[](size_t index)
	{
		return m_Values[index];
	}

	bool Mat4::IsValid() const
	{
		for (float value : m_Values)
		{
			if (std::isnan(value))
			{
				return false;
			}
		}
		return true;
	}

	PBE::Mat4 Mat4::operator+(Mat4 const& other) const
	{
		Mat4 result;

		__m128 xRow = _mm_loadu_ps(&other.m_Values[0]);
		__m128 yRow = _mm_loadu_ps(&other.m_Values[4]);
		__m128 zRow = _mm_loadu_ps(&other.m_Values[8]);
		__m128 tRow = _mm_loadu_ps(&other.m_Values[12]);

		__m128 iResult = _mm_add_ps(_mm_loadu_ps(&m_Values[0]), xRow);
		__m128 jResult = _mm_add_ps(_mm_loadu_ps(&m_Values[4]), yRow);
		__m128 kResult = _mm_add_ps(_mm_loadu_ps(&m_Values[8]), zRow);
		__m128 tResult = _mm_add_ps(_mm_loadu_ps(&m_Values[12]), tRow);

		_mm_storeu_ps(&result.m_Values[0], iResult);
		_mm_storeu_ps(&result.m_Values[4], jResult);
		_mm_storeu_ps(&result.m_Values[8], kResult);
		_mm_storeu_ps(&result.m_Values[12], tResult);

		return result;
	}

	Mat4 Mat4::operator+=(Mat4 const& other)
	{
		*this = *this + other;
		return *this;
	}

	PBE::Mat4 Mat4::operator-(Mat4 const& other) const
	{
		Mat4 result;

		__m128 xRow = _mm_loadu_ps(&other.m_Values[0]);
		__m128 yRow = _mm_loadu_ps(&other.m_Values[4]);
		__m128 zRow = _mm_loadu_ps(&other.m_Values[8]);
		__m128 tRow = _mm_loadu_ps(&other.m_Values[12]);

		__m128 iResult = _mm_sub_ps(_mm_loadu_ps(&m_Values[0]), xRow);
		__m128 jResult = _mm_sub_ps(_mm_loadu_ps(&m_Values[4]), yRow);
		__m128 kResult = _mm_sub_ps(_mm_loadu_ps(&m_Values[8]), zRow);
		__m128 tResult = _mm_sub_ps(_mm_loadu_ps(&m_Values[12]), tRow);

		_mm_storeu_ps(&result.m_Values[0], iResult);
		_mm_storeu_ps(&result.m_Values[4], jResult);
		_mm_storeu_ps(&result.m_Values[8], kResult);
		_mm_storeu_ps(&result.m_Values[12], tResult);

		return result;
	}

	Mat4 Mat4::operator-=(Mat4 const& other)
	{
		*this = *this - other;
		return *this;
	}

	PBE::Mat4 Mat4::operator/(float scalar) const
	{
		Mat4 result;

		__m128 scalarVector = _mm_set1_ps(scalar);
		
		__m128 iResult = _mm_div_ps(_mm_loadu_ps(&m_Values[0]), scalarVector);
		__m128 jResult = _mm_div_ps(_mm_loadu_ps(&m_Values[4]), scalarVector);
		__m128 kResult = _mm_div_ps(_mm_loadu_ps(&m_Values[8]), scalarVector);
		__m128 tResult = _mm_div_ps(_mm_loadu_ps(&m_Values[12]), scalarVector);

		_mm_storeu_ps(&result.m_Values[0], iResult);
		_mm_storeu_ps(&result.m_Values[4], jResult);
		_mm_storeu_ps(&result.m_Values[8], kResult);
		_mm_storeu_ps(&result.m_Values[12], tResult);

		return result;
	}

	Mat4 Mat4::operator/=(float scalar)
	{
		*this = *this / scalar;
		return *this;
	}

	PBE::Mat4 Mat4::operator*(float scalar) const
	{
		Mat4 result;

		__m128 scalarVector = _mm_set1_ps(scalar);

		__m128 iResult = _mm_mul_ps(_mm_loadu_ps(&m_Values[0]), scalarVector);
		__m128 jResult = _mm_mul_ps(_mm_loadu_ps(&m_Values[4]), scalarVector);
		__m128 kResult = _mm_mul_ps(_mm_loadu_ps(&m_Values[8]), scalarVector);
		__m128 tResult = _mm_mul_ps(_mm_loadu_ps(&m_Values[12]), scalarVector);

		_mm_storeu_ps(&result.m_Values[0], iResult);
		_mm_storeu_ps(&result.m_Values[4], jResult);
		_mm_storeu_ps(&result.m_Values[8], kResult);
		_mm_storeu_ps(&result.m_Values[12], tResult);

		return result;
	}

	Mat4 Mat4::operator*=(float scalar)
	{
		*this = *this * scalar;
		return *this;
	}

	bool Mat4::operator==(Mat4 const& other) const
	{
		return m_Values[Ix] == other.m_Values[Ix] &&
			m_Values[Iy] == other.m_Values[Iy] &&
			m_Values[Iz] == other.m_Values[Iz] &&
			m_Values[Iw] == other.m_Values[Iw] &&
			m_Values[Jx] == other.m_Values[Jx] &&
			m_Values[Jy] == other.m_Values[Jy] &&
			m_Values[Jz] == other.m_Values[Jz] &&
			m_Values[Jw] == other.m_Values[Jw] &&
			m_Values[Kx] == other.m_Values[Kx] &&
			m_Values[Ky] == other.m_Values[Ky] &&
			m_Values[Kz] == other.m_Values[Kz] &&
			m_Values[Kw] == other.m_Values[Kw] &&
			m_Values[Tx] == other.m_Values[Tx] &&
			m_Values[Ty] == other.m_Values[Ty] &&
			m_Values[Tz] == other.m_Values[Tz] &&
			m_Values[Tw] == other.m_Values[Tw];
	}

	bool Mat4::operator!=(Mat4 const& other) const
	{
		return !(*this == other);
	}

	void Mat4::SetValue(size_t column, size_t row, float value)
	{
		m_Values[column * 4 + row] = value;
	}

	void Mat4::SetIBasis3D(Vec3 const& basis)
	{
		m_Values[Mat4::Ix] = basis.x;
		m_Values[Mat4::Iy] = basis.y;
		m_Values[Mat4::Iz] = basis.z;
	}

	void Mat4::SetJBasis3D(Vec3 const& basis)
	{
		m_Values[Mat4::Jx] = basis.x;
		m_Values[Mat4::Jy] = basis.y;
		m_Values[Mat4::Jz] = basis.z;
	}

	void Mat4::SetKBasis3D(Vec3 const& basis)
	{
		m_Values[Mat4::Kx] = basis.x;
		m_Values[Mat4::Ky] = basis.y;
		m_Values[Mat4::Kz] = basis.z;
	}

	void Mat4::SetTranslation3D(Vec3 const& translation)
	{
		m_Values[Mat4::Tx] = translation.x;
		m_Values[Mat4::Ty] = translation.y;
		m_Values[Mat4::Tz] = translation.z;
	}

	PBE::Rotor3D Mat4::GetRotationAsQuaternion() const
	{
		Rotor3D result;
		float trace = m_Values[Ix] + m_Values[Jy] + m_Values[Kz];

		if (trace > 0.f)
		{
			float s = 0.5f / sqrtf(trace + 1.f);
			result.w = 0.25f / s;
			result.i = (m_Values[Ky] - m_Values[Jz]) * s;
			result.j = (m_Values[Iz] - m_Values[Kx]) * s;
			result.k = (m_Values[Jx] - m_Values[Iy]) * s;
		}
		else if (m_Values[Ix] > m_Values[Jy] && m_Values[Ix] > m_Values[Kz])
		{
			float s = 2.f * sqrtf(1.f + m_Values[Ix] - m_Values[Jy] - m_Values[Kz]);
			result.w = (m_Values[Ky] - m_Values[Jz]) / s;
			result.i = 0.25f * s;
			result.j = (m_Values[Iy] + m_Values[Jx]) / s;
			result.k = (m_Values[Iz] + m_Values[Kx]) / s;
		}
		else if (m_Values[Jy] > m_Values[Kz])
		{
			float s = 2.f * sqrtf(1.f + m_Values[Jy] - m_Values[Ix] - m_Values[Kz]);
			result.w = (m_Values[Iz] - m_Values[Kx]) / s;
			result.i = (m_Values[Iy] + m_Values[Jx]) / s;
			result.j = 0.25f * s;
			result.k = (m_Values[Jz] + m_Values[Ky]) / s;
		}
		else
		{
			float s = 2.f * sqrtf(1.f + m_Values[Kz] - m_Values[Ix] - m_Values[Jy]);
			result.w = (m_Values[Jx] - m_Values[Iy]) / s;
			result.i = (m_Values[Iz] + m_Values[Kx]) / s;
			result.j = (m_Values[Jz] + m_Values[Ky]) / s;
			result.k = 0.25f * s;
		}

		// Normalize the quaternion to ensure it's a unit quaternion
		float length = sqrtf(result.w * result.w + result.i * result.i + result.j * result.j + result.k * result.k);
		result.w /= length;
		result.i /= length;
		result.j /= length;
		result.k /= length;

		return result;
	}

	PBE::Vec2 Mat4::GetIBasis2D() const
	{
		Vec2 iBasis2D = Vec2(m_Values[Ix], m_Values[Iy]);
		return iBasis2D;
	}

	PBE::Vec2 Mat4::GetJBasis2D() const
	{
		Vec2 jBasis2D = Vec2(m_Values[Jx], m_Values[Jy]);
		return jBasis2D;
	}

	PBE::Vec2 Mat4::GetTranslation2D() const
	{
		Vec2 translation2D = Vec2(m_Values[Tx], m_Values[Ty]);
		return translation2D;
	}

	PBE::Vec3 Mat4::GetIBasis3D() const
	{
		Vec3 iBasis3D = Vec3(m_Values[Ix], m_Values[Iy], m_Values[Iz]);
		return iBasis3D;
	}

	PBE::Vec3 Mat4::GetJBasis3D() const
	{
		Vec3 jBasis3D = Vec3(m_Values[Jx], m_Values[Jy], m_Values[Jz]);
		return jBasis3D;
	}

	PBE::Vec3 Mat4::GetKBasis3D() const
	{
		Vec3 kBasis3D = Vec3(m_Values[Kx], m_Values[Ky], m_Values[Kz]);
		return kBasis3D;
	}

	Vec2 Mat4::GetScale2D() const
	{
		Vec2 iBasis = GetIBasis2D();
		Vec2 jBasis = GetJBasis2D();

		float scaleX = iBasis.GetLength();
		float scaleY = jBasis.GetLength();

		return Vec2(scaleX, scaleY);
	}

	Vec3 Mat4::GetScale3D() const
	{
		Vec3 iBasis = GetIBasis3D();
		Vec3 jBasis = GetJBasis3D();
		Vec3 kBasis = GetKBasis3D();

		float scaleX = iBasis.GetLength();
		float scaleY = jBasis.GetLength();
		float scaleZ = kBasis.GetLength();

		return Vec3(scaleX, scaleY, scaleZ);
	}

	EulerAngles Mat4::GetRotationAsEulerAngles() const
	{
		float yaw = atan2f(m_Values[Jx], m_Values[Ix]);
		float pitch = asinf(-m_Values[Kx]);
		float roll = atan2f(m_Values[Ky], m_Values[Kz]);

		EulerAngles rotation = EulerAngles(yaw, pitch, roll);

		return rotation;
	}

	Vec3 Mat4::GetTranslation3D() const
	{
		return Vec3(m_Values[Tx],m_Values[Ty],m_Values[Tz]);
	}

	PBE::Vec4 Mat4::GetIBasis4D() const
	{
		Vec4 iBasis4D = Vec4(m_Values[Ix], m_Values[Iy], m_Values[Iz], m_Values[Iw]);
		return iBasis4D;
	}

	PBE::Vec4 Mat4::GetJBasis4D() const
	{
		Vec4 jBasis4D = Vec4(m_Values[Jx], m_Values[Jy], m_Values[Jz], m_Values[Jw]);
		return jBasis4D;
	}

	PBE::Vec4 Mat4::GetKBasis4D() const
	{
		Vec4 kBasis4D = Vec4(m_Values[Kx], m_Values[Ky], m_Values[Kz], m_Values[Kw]);
		return kBasis4D;
	}

	PBE::Vec4 Mat4::GetTranslation4D() const
	{
		Vec4 translation4D = Vec4(m_Values[Tx], m_Values[Ty], m_Values[Tz], m_Values[Tw]);
		return translation4D;
	}

	std::string Mat4::ToString() const
	{
		std::string result = "Mat4\n\r(";
		for (int index = 0; index < 16; ++index)
		{
			result += std::to_string(m_Values[index]);
			if (index != 15)
			{
				if (index % 4 == 3)
				{
					result += "\n\r";
				}
				else
				{
					result += ", ";
				}
			}
		}
		result += ")";
		return result;
	}
}