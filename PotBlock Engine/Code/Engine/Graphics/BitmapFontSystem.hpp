#pragma once

#include "Engine/Graphics/Renderer.hpp"
#include "Engine/Graphics/Texture.hpp"
#include "Engine/Graphics/Mesh.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

#include <filesystem>
#include <string>
#include <array>
#include <unordered_map>
#include <map>

namespace PBE
{
	struct BitmapFontCreateInfo
	{
		std::filesystem::path m_Filename;
		int m_Height;
		std::string m_Glyphs;

		bool m_DropShadow = false;
		Vec2 m_ShadowOffset = Vec2(1.f, -1.f);
		Rgba8 m_ShadowColor = Rgba8(0,0,0,80);
	};
	
	struct BitmapFontGlyph
	{
		AABB2 m_TextureRegion{};
		Vec2 m_Size{32.f, 32.f};
		Vec2 m_Bearing{0.f,0.f};
		float m_Advance = 32.f;
	};

	class BitmapFontSystem;

	class BitmapFont;

	class BitmapFont
	{
	public:
		BitmapFont(BitmapFontSystem* system);
		~BitmapFont();

		bool HasGlyph(wchar_t c) const;
		BitmapFontGlyph GetGlyph(wchar_t c) const;

		float GetTextWidth(std::string_view text, float scale = 1.f) const;
		float GetTextHeight(std::string_view text, float scale = 1.f) const;
		float GetFontSize() const;

		void AddVertsForTextInBox
		(
			CPUMesh& mesh, 
			std::string const& text, 
			AABB2 const& bounds,
			Vec2 const& alignment = Vec2(0.5f,0.5f),
			Rgba8 const& tint = Rgba8::WHITE,
			float scale = 1.f
		) const;

		Material* GetMaterial() const;
		Texture2D GetTexture() const;
	protected:
		void AddVertsForTextInBoxNoShadow
		(
			CPUMesh& mesh,
			std::string const& text,
			AABB2 const& bounds,
			Vec2 const& alignment,
			Rgba8 const& tint,
			float scale = 1.f
		) const;

		friend class BitmapFontSystem;
		BitmapFontSystem* m_System;
		std::unordered_map<char, BitmapFontGlyph> m_Glyphs;
		Material* m_Material;
		Texture2D m_Texture{ nullptr };

		float m_Size = 32.f;
		bool m_DropShadow = false;
		Vec2 m_ShadowOffset = Vec2(2.f, -2.f);
		Rgba8 m_ShadowColor = Rgba8(0,0,0,80);
	};

	struct BitmapFontSystemCreateInfo
	{
		Renderer* m_pRenderer;
	};


	class BitmapFontSystem
	{
	public:
		BitmapFontSystemCreateInfo m_CreateInfo;
	public:
		BitmapFontSystem(BitmapFontSystemCreateInfo const& info);
		BitmapFont* CreateBitmapFont(BitmapFontCreateInfo const& info);
	protected:
		FT_Library m_Library;
	};
}