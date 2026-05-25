#include "Engine/Graphics/BitmapFontSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <stdexcept>
#include <cmath>

PBE::BitmapFontSystem::BitmapFontSystem(BitmapFontSystemCreateInfo const& info)
	: m_CreateInfo(info)
{
	if (FT_Init_FreeType(&m_Library))
	{
		throw std::runtime_error("Failed to initialize FreeType library");
	}
}

PBE::BitmapFont* PBE::BitmapFontSystem::CreateBitmapFont(BitmapFontCreateInfo const& info)
{
	BitmapFont* font = new BitmapFont(this);
	FT_Face face;

	std::string filename = info.m_Filename.string();
	FT_New_Face(m_Library, filename.c_str(), 0, &face);

	FT_Set_Pixel_Sizes(face, 0, info.m_Height);

	int totalArea = 0;
	int padding = 1; // Space between glyphs

	// Calculate total area required for glyphs
	for (wchar_t c : info.m_Glyphs) 
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) 
		{
			throw std::runtime_error("Failed to load glyph");
		}
		totalArea += (face->glyph->bitmap.width + padding) * (face->glyph->bitmap.rows + padding);
	}

	// Estimate the atlas dimensions (nearest power of 2)
	int atlasSize = (int)(std::ceil(std::sqrt(totalArea)) * 1.5);
	atlasSize = (int)std::pow(2, std::ceil(std::log2(atlasSize))); // Round to the nearest power of 2

	std::vector<Rgba8> atlasBuffer(atlasSize * atlasSize, 0);

	int xOffset = 0;
	int yOffset = 0;
	int rowHeight = 0;

	for (char c : info.m_Glyphs) 
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) 
		{
			throw std::runtime_error("Failed to load glyph");
		}

		FT_Bitmap& bitmap = face->glyph->bitmap;

		// Check if the glyph fits in the current row
		if (xOffset + (int)bitmap.width + padding > atlasSize) 
		{
			// Move to the next row
			xOffset = 0;
			yOffset += rowHeight + padding;
			rowHeight = 0;
		}

		// Copy glyph bitmap to the atlas buffer
		for (int y = 0; y < (int)bitmap.rows; ++y) 
		{
			for (int x = 0; x < (int)bitmap.width; ++x) 
			{
				char intensity = bitmap.buffer[y * bitmap.pitch + x];
				if (intensity == 0)
				{
					continue;
				}
				atlasBuffer[(yOffset + y) * atlasSize + (xOffset + x)].r = 255;
				atlasBuffer[(yOffset + y) * atlasSize + (xOffset + x)].g = 255;
				atlasBuffer[(yOffset + y) * atlasSize + (xOffset + x)].b = 255;
				atlasBuffer[(yOffset + y) * atlasSize + (xOffset + x)].a = intensity;
			}
		}

		// Store glyph metadata
		BitmapFontGlyph glyph;
		glyph.m_Size = Vec2((float)bitmap.width, (float)bitmap.rows);
		glyph.m_Bearing = Vec2((float)face->glyph->bitmap_left, (float)bitmap.rows - (float)face->glyph->bitmap_top);
		glyph.m_Advance = (float)(face->glyph->advance.x >> 6); // Convert from 1/64 pixels

		// UV Coordinates in the atlas
		float u1 = xOffset / (float)atlasSize;
		float v1 = yOffset / (float)atlasSize;
		float u2 = (xOffset + bitmap.width) / (float)atlasSize;
		float v2 = (yOffset + bitmap.rows) / (float)atlasSize;
		glyph.m_TextureRegion = AABB2(Vec2(u1, v1), Vec2(u2, v2));

		font->m_Glyphs[c] = glyph;

		// Update offsets
		xOffset += bitmap.width + padding;
		rowHeight = Max(rowHeight, bitmap.rows);
	}

	VkExtent2D extent = { (uint32_t)atlasSize, (uint32_t)atlasSize };

	font->m_Texture = m_CreateInfo.m_pRenderer->LoadFromMemoryTexture2D(
		"Font Atlas",
		extent,
		atlasBuffer.data(),
		SamplerType::LINEAR_CLAMP
	);

	MaterialCreateInfo materialInfo{};
	materialInfo.m_Allocator = m_CreateInfo.m_pRenderer->m_Allocator;
	materialInfo.m_Device = m_CreateInfo.m_pRenderer->m_Device;
	materialInfo.m_DiffuseTexture = font->m_Texture;
	materialInfo.m_NormalTexture = m_CreateInfo.m_pRenderer->GetWhiteTexture();
	materialInfo.m_ParallaxTexture = m_CreateInfo.m_pRenderer->GetWhiteTexture();
	materialInfo.m_SpecularGlossEmitTexture = m_CreateInfo.m_pRenderer->GetWhiteTexture();

	font->m_Material = m_CreateInfo.m_pRenderer->CreateMaterial(materialInfo);

	if (font->m_Glyphs.find(' ') == font->m_Glyphs.end())
	{
		BitmapFontGlyph spaceGlyph;
		spaceGlyph.m_Size = Vec2(0.f, 0.f);
		spaceGlyph.m_Bearing = Vec2(0.f, 0.f);
		spaceGlyph.m_Advance = info.m_Height * 0.2f;
		spaceGlyph.m_TextureRegion = AABB2(Vec2(0.f, 0.f), Vec2(0.f, 0.f));
		font->m_Glyphs[' '] = spaceGlyph;
	}

	font->m_DropShadow = info.m_DropShadow;
	font->m_ShadowOffset = info.m_ShadowOffset;
	font->m_ShadowColor = info.m_ShadowColor;
	font->m_Size = (float)info.m_Height;

	return font;
}

PBE::BitmapFont::BitmapFont(BitmapFontSystem* system)
	: m_System(system)
{
}

PBE::BitmapFont::~BitmapFont()
{
	auto renderer = m_System->m_CreateInfo.m_pRenderer;
	delete m_Material;
	renderer->FreeTexture(m_Texture);


}

bool PBE::BitmapFont::HasGlyph(wchar_t c) const
{
	return m_Glyphs.find((char)c) != m_Glyphs.end();
}

PBE::BitmapFontGlyph PBE::BitmapFont::GetGlyph(wchar_t c) const
{
	auto it = m_Glyphs.find((char)c);
	if (it != m_Glyphs.end())
	{
		return it->second;
	}

	return {};
}

float PBE::BitmapFont::GetTextWidth(std::string_view text, float scale) const
{
	float width = 0.f;

	for (wchar_t c : text)
	{
		if (c == L'\n')
		{
			break;
		}

		if (!HasGlyph(c))
		{
			continue;
		}

		BitmapFontGlyph glyph = GetGlyph(c);
		width += glyph.m_Advance * scale;
	}

	return width;
}

float PBE::BitmapFont::GetTextHeight(std::string_view text, float /*scale*/) const
{
	// Calculate text box dimensions
	float textHeight = 0.f;
	float maxLineHeight = 0.f;

	for (wchar_t c : text)
	{
		if (c == L'\n')
		{
			textHeight += maxLineHeight * 1.5f; // Move to the next line
			maxLineHeight = 0.f;
			continue;
		}

		if (!HasGlyph(c))
		{
			continue; // Skip empty or missing glyphs
		}
		BitmapFontGlyph glyph = GetGlyph(c);

		maxLineHeight = Max(maxLineHeight, glyph.m_Size.y);
	}
	textHeight += maxLineHeight; // Include the last line height
	return textHeight;
}

float PBE::BitmapFont::GetFontSize() const
{
	return m_Size;
}

void PBE::BitmapFont::AddVertsForTextInBox(CPUMesh& mesh, std::string const& text, AABB2 const& bounds, Vec2 const& alignment, Rgba8 const& tint, float scale) const
{
	if (m_DropShadow)
	{
		AABB2 shadowBounds = bounds;
		shadowBounds.Translate(m_ShadowOffset);
		AddVertsForTextInBoxNoShadow(mesh, text, shadowBounds, alignment, m_ShadowColor, scale);
	}

	AddVertsForTextInBoxNoShadow(mesh, text, bounds, alignment, tint, scale);
}

PBE::Material* PBE::BitmapFont::GetMaterial() const
{
	return m_Material;
}

PBE::Texture2D PBE::BitmapFont::GetTexture() const
{
	return m_Texture;
}

void PBE::BitmapFont::AddVertsForTextInBoxNoShadow(CPUMesh& mesh, std::string const& text, AABB2 const& bounds, Vec2 const& alignment, Rgba8 const& tint, float scale) const
{
	// Calculate text box dimensions
	Vec2 textSize(0.f, 0.f);
	float maxLineHeight = m_Size * 1.15f * scale;
	std::vector<float> lineLengths;
	std::vector<std::pair<wchar_t, float>> advances; // Store glyph advances for alignment calculation
	float maxLineWidth = 0.f;

	for (wchar_t c : text)
	{
		if (c == L'\n')
		{
			textSize.y += maxLineHeight; // Move to the next line
			lineLengths.push_back(textSize.x);
			textSize.x = 0.f; // Reset horizontal size for the new line
			continue;
		}

		if (!HasGlyph(c))
		{
			continue; // Skip empty or missing glyphs
		}
		BitmapFontGlyph glyph = GetGlyph(c);

		advances.emplace_back(c, glyph.m_Advance * scale);
		textSize.x += glyph.m_Advance * scale; // Accumulate horizontal size
		maxLineWidth = Max(maxLineWidth, textSize.x);
	}

	lineLengths.push_back(textSize.x); // Store the last line length
	float lastLineHeight = maxLineHeight;

	// Calculate initial pen position based on alignment
	Vec2 penOrigin = bounds.m_Mins;
	penOrigin.x += (bounds.GetDimensions().x - maxLineWidth) * alignment.x - maxLineWidth * alignment.x;
	penOrigin.y += (bounds.GetDimensions().y) * alignment.y + textSize.y - (textSize.y + lastLineHeight) * alignment.y;

	Vec2 pen = penOrigin;
	int lineIndex = 0;

	for (wchar_t c : text)
	{
		if (c == L'\n')
		{
			pen.x = penOrigin.x;
			pen.y -= maxLineHeight;
			++lineIndex;
			continue;
		}

		if (c == L'\r')
		{
			continue; // Skip carriage returns
		}

		if (c == L'\0')
		{
			break; // Stop at null terminator
		}

		if (!HasGlyph(c))
		{
			continue;
		}

		float lineLengthOffset = (maxLineWidth - lineLengths[lineIndex]) * alignment.x;

		BitmapFontGlyph glyph = GetGlyph(c);
		Vec2 offset = Vec2(glyph.m_Bearing.x * scale, -glyph.m_Bearing.y * scale); // Adjust Y bearing to match top-left origin
		Vec2 size = glyph.m_Size * scale;

		Vec2 min = pen + offset + Vec2(lineLengthOffset, 0.f);
		Vec2 max = min + size;

		Vec2 uvMin = glyph.m_TextureRegion.m_Mins;
		Vec2 uvMax = glyph.m_TextureRegion.m_Maxs;

		if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '\0')
		{
			// Add vertices for the current glyph
			mesh.AddVertsForAABB2(AABB2(min, max), AABB2(uvMin, uvMax), tint);
		}

		// Advance pen position
		pen.x += glyph.m_Advance * scale;
	}
}
