#include "Engine/Graphics/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"

namespace PBE
{
	Rgba8 const Rgba8::WHITE		   = Rgba8(255, 255, 255);
	Rgba8 const Rgba8::BLACK		   = Rgba8(0, 0, 0);
	Rgba8 const Rgba8::RED			   = Rgba8(255, 0, 0);
	Rgba8 const Rgba8::GREEN		   = Rgba8(0, 255, 0);
	Rgba8 const Rgba8::BLUE			   = Rgba8(0, 0, 255);
	Rgba8 const Rgba8::YELLOW		   = Rgba8(255, 255, 0);
	Rgba8 const Rgba8::MAGENTA		   = Rgba8(255, 0, 255);
	Rgba8 const Rgba8::CYAN			   = Rgba8(0, 255, 255);
	Rgba8 const Rgba8::GRAY			   = Rgba8(128, 128, 128);
	Rgba8 const Rgba8::ORANGE		   = Rgba8(255, 128, 0);
	Rgba8 const Rgba8::PURPLE		   = Rgba8(128, 0, 255);
	Rgba8 const Rgba8::BROWN		   = Rgba8(128, 64, 0);
	Rgba8 const Rgba8::PINK			   = Rgba8(255, 128, 128);
	Rgba8 const Rgba8::CORNFLOWER_BLUE = Rgba8(100, 149, 237);
	Rgba8 const Rgba8::FOREST_GREEN	   = Rgba8(34, 139, 34);
	Rgba8 const Rgba8::LIME_GREEN	   = Rgba8(50, 205, 50);
	Rgba8 const Rgba8::SEA_GREEN	   = Rgba8(46, 139, 87);
	Rgba8 const Rgba8::SKY_BLUE		   = Rgba8(135, 206, 235);
	Rgba8 const Rgba8::TAN			   = Rgba8(210, 180, 140);
	Rgba8 const Rgba8::DARK_GRAY	   = Rgba8(64, 64, 64);
	Rgba8 const Rgba8::LIGHT_GRAY	   = Rgba8(192, 192, 192);
	Rgba8 const Rgba8::DARK_RED		   = Rgba8(128, 0, 0);
	Rgba8 const Rgba8::DARK_GREEN	   = Rgba8(0, 128, 0);
	Rgba8 const Rgba8::DARK_BLUE	   = Rgba8(0, 0, 128);
	Rgba8 const Rgba8::LIGHT_RED	   = Rgba8(255, 128, 128);
	Rgba8 const Rgba8::LIGHT_GREEN	   = Rgba8(128, 255, 128);
	Rgba8 const Rgba8::LIGHT_BLUE	   = Rgba8(128, 128, 255);

	Rgba8::Rgba8()
		: r(0)
		, g(0)
		, b(0)
		, a(0)
	{
	}

	Rgba8::Rgba8(unsigned char r, unsigned char g, unsigned char b, unsigned char a /*= 255*/)
		: r(r)
		, g(g)
		, b(b)
		, a(a)
	{
	}

	Rgba8::Rgba8(unsigned char all)
		: r(all)
		, g(all)
		, b(all)
		, a(all)
	{
	}

	Rgba8 Rgba8::Lerp(const Rgba8& start, const Rgba8& end, float fractionTowardEnd)
	{
		Rgba8 result;
		result.r = (unsigned char)PBE::Lerp((float)start.r, (float)end.r, fractionTowardEnd);
		result.g = (unsigned char)PBE::Lerp((float)start.g, (float)end.g, fractionTowardEnd);
		result.b = (unsigned char)PBE::Lerp((float)start.b, (float)end.b, fractionTowardEnd);
		result.a = (unsigned char)PBE::Lerp((float)start.a, (float)end.a, fractionTowardEnd);
		return result;
	}

	Rgba8 Rgba8::LinearToSRGB() const
	{
		float rr = (float)this->r / 255.f;
		float gg = (float)this->g / 255.f;
		float bb = (float)this->b / 255.f;
		float aa = (float)this->a / 255.f;

		rr = (rr <= 0.04045f) ? rr / 12.92f : powf((rr + 0.055f) / 1.055f, 2.4f);
		gg = (gg <= 0.04045f) ? gg / 12.92f : powf((gg + 0.055f) / 1.055f, 2.4f);
		bb = (bb <= 0.04045f) ? bb / 12.92f : powf((bb + 0.055f) / 1.055f, 2.4f);
		aa = (aa <= 0.04045f) ? aa / 12.92f : powf((aa + 0.055f) / 1.055f, 2.4f);

		rr *= 255.f;
		gg *= 255.f;
		bb *= 255.f;
		aa *= 255.f;

		return Rgba8((unsigned char)rr, (unsigned char)gg, (unsigned char)bb, (unsigned char)aa);
	}

	Vec4 Rgba8::ToVec4() const
	{
		return Vec4(NormalizeByte(r),NormalizeByte(g),NormalizeByte(b),NormalizeByte(a));
	}
}
