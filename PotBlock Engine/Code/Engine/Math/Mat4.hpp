#pragma once

#include "Engine/Math/Vec4.hpp"

namespace PBE
{
	class EulerAngles;
	class Rotor3D;

	class Mat4
	{
	public:
		static size_t const Ix;
		static size_t const Iy;
		static size_t const Iz;
		static size_t const Iw;
		static size_t const Jx;
		static size_t const Jy;
		static size_t const Jz;
		static size_t const Jw;
		static size_t const Kx;
		static size_t const Ky;
		static size_t const Kz;
		static size_t const Kw;
		static size_t const Tx;
		static size_t const Ty;
		static size_t const Tz;
		static size_t const Tw;

		alignas(16) float m_Values[16];

		static Mat4 const IDENTITY;
		static Mat4 const ZERO;
	public:
		Mat4();
		Mat4(Mat4 const& other);
		Mat4(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis, Vec3 const& tBasis);
		Mat4(Vec4 const& iBasis, Vec4 const& jBasis, Vec4 const& kBasis, Vec4 const& tBasis);
		Mat4(float iBasisX, float iBasisY, float iBasisZ, float iBasisW,
			float jBasisX, float jBasisY, float jBasisZ, float jBasisW,
			float kBasisX, float kBasisY, float kBasisZ, float kBasisW,
			float translationX, float translationY, float translationZ, float translationW);

		Mat4 GetAppended(Mat4 const& other) const;
		Mat4 Append(Mat4 const& other);

		Mat4 GetOrthonormalized() const;
		Mat4 GetTransposed() const;
		Mat4 GetOrthonormalInverse() const;
		float GetDeterminant() const;
		Mat4 GetInverse() const;

		Vec2 TransformPoint(Vec2 const& point) const;
		Vec3 TransformPoint(Vec3 const& point) const;
		Vec4 TransformPoint(Vec4 const& point) const;

		static Mat4 const CreateLookAt(Vec3 const& eye, Vec3 const& center, Vec3 const& up);
		static Mat4 const CreateTranslation3D(Vec3 const& translation);
		static Mat4 const CreateOrthographicProjection(float left, float right, float bottom, float top, float near, float far);
		static Mat4 const CreatePerspectiveProjection(float fov, float aspectRatio, float near, float far);
		static Mat4 const CreateScale(float scale);
		static Mat4 const CreateScale(Vec3 const& scale);
		static Mat4 const Interpolate(Mat4 const& start, Mat4 const& end, float t);
		static Mat4 const CreateTRS(Vec3 const& translation, Rotor3D const& rotation, Vec3 const& scale);
		static Mat4 const CreateTRS(Vec3 const& translation, EulerAngles const& rotation, Vec3 const& scale);

		Mat4 const Translate3D(Vec3 const& translation);
		float& operator[](size_t index);
		
		bool IsValid() const;
		bool operator==(Mat4 const& other) const;
		bool operator!=(Mat4 const& other) const;

		Mat4 operator+(Mat4 const& other) const;
		Mat4 operator+=(Mat4 const& other);
		Mat4 operator-(Mat4 const& other) const;
		Mat4 operator-=(Mat4 const& other);

		Mat4 operator/(float scalar) const;
		Mat4 operator/=(float scalar);
		Mat4 operator*(float scalar) const;
		Mat4 operator*=(float scalar);

		void SetValue(size_t column, size_t row, float value);

		void SetIBasis3D(Vec3 const& basis);
		void SetJBasis3D(Vec3 const& basis);
		void SetKBasis3D(Vec3 const& basis);
		void SetTranslation3D(Vec3 const& translation);

		Rotor3D GetRotationAsQuaternion() const;
		EulerAngles GetRotationAsEulerAngles() const;

		Vec2 GetIBasis2D() const;
		Vec2 GetJBasis2D() const;
		Vec2 GetTranslation2D() const;

		Vec3 GetIBasis3D() const;
		Vec3 GetJBasis3D() const;
		Vec3 GetKBasis3D() const;

		Vec2 GetScale2D() const;
		Vec3 GetScale3D() const;
		Vec3 GetTranslation3D() const;

		Vec4 GetIBasis4D() const;
		Vec4 GetJBasis4D() const;
		Vec4 GetKBasis4D() const;
		Vec4 GetTranslation4D() const;

		std::string ToString() const;
	};
}