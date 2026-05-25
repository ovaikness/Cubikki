#pragma once

#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/BitmapFontSystem.hpp"
#include "Engine/Graphics/Painters/Painter.hpp"

namespace PBE
{
	struct Theme
	{
		Material* m_DefaultWhiteMaterial;
		Material* m_Material;
		BitmapFont* m_Font;
		Painter* m_Backer;
		Painter* m_TextBacker;
	};
}