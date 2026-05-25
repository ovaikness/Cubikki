#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Graphics/Mesh.hpp"
#include "Engine/Graphics/Rgba8.hpp"

namespace PBE
{
	class Painter
	{
	public:
		virtual void AddVerts(CPUMesh& mesh, AABB2 const& size, Rgba8 const& color) = 0;
	};
}