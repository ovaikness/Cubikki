#pragma once
#include "Engine/Graphics/Texture.hpp"
#include "Engine/Graphics/Renderer.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"

#include <map>
#include <string>
#include <string_view>
#include <filesystem>

namespace PBE
{
	struct TextureRegion
	{
		AABB2 m_UV;
		Vec2 m_Size;
		Vec2 m_NormalizedPivot;
	};

	struct TextureAtlasCreateInfo
	{
		Texture2D m_Texture;
		std::map<std::string, TextureRegion> m_TextureCoords;
	};

	class TextureAtlas
	{
	public:
		TextureAtlas() = default;
		TextureAtlas(TextureAtlasCreateInfo const& info);
		~TextureAtlas();

		static TextureAtlas LoadOrGetTextureAtlas(Renderer* renderer, std::filesystem::path path, PBE::SamplerType sampler = PBE::SamplerType::LINEAR_CLAMP);
		TextureRegion GetTextureRegion(std::string_view name) const;

		Texture2D GetTexture() const { return m_Texture; }
	public:
		Texture2D m_Texture = nullptr;
		std::map<std::string, TextureRegion> m_TextureCoords{};
	};
}