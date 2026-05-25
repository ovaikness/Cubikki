#pragma once
#include "Engine/Math/Vec4.hpp"

namespace PBE
{
	struct Rgba8
	{
	public:
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;

		static Rgba8 const WHITE;
		static Rgba8 const BLACK;
		static Rgba8 const RED;
		static Rgba8 const GREEN;
		static Rgba8 const BLUE;
		static Rgba8 const YELLOW;
		static Rgba8 const MAGENTA;
		static Rgba8 const CYAN;
		static Rgba8 const GRAY;
		static Rgba8 const ORANGE;
		static Rgba8 const PURPLE;
		static Rgba8 const BROWN;
		static Rgba8 const PINK;
		static Rgba8 const CORNFLOWER_BLUE;
		static Rgba8 const FOREST_GREEN;
		static Rgba8 const LIME_GREEN;
		static Rgba8 const SEA_GREEN;
		static Rgba8 const SKY_BLUE;
		static Rgba8 const TAN;
		static Rgba8 const DARK_GRAY;
		static Rgba8 const LIGHT_GRAY;
		static Rgba8 const DARK_RED;
		static Rgba8 const DARK_GREEN;
		static Rgba8 const DARK_BLUE;
		static Rgba8 const LIGHT_RED;
		static Rgba8 const LIGHT_GREEN;
		static Rgba8 const LIGHT_BLUE;
	public:
		Rgba8();
		Rgba8(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
		Rgba8(unsigned char all);

		static Rgba8 Lerp(const Rgba8& start, const Rgba8& end, float fractionTowardEnd);
		Rgba8 LinearToSRGB() const;
		Vec4 ToVec4() const;
	};
}