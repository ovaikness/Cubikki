#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Graphics/Rgba8.hpp"

#include "Engine/Graphics/GPUBuffer.hpp"
#include "Engine/Graphics/Renderer.hpp"

#include <vector>

struct ChunkVertex
{
	static VkVertexInputAttributeDescription s_AttributeDescriptions[4];
	static VkVertexInputBindingDescription s_BindingDescription;

	PBE::Vec3 m_Position;
	uint32_t m_ByteNormal;
	PBE::Vec2 m_UV;
	PBE::Rgba8 m_Color;
};

class ChunkMeshManager
{
public:
	PBE::Renderer m_Renderer;
	std::vector<PBE::GPUBuffer> m_StagingBufferPool;
	std::vector<PBE::GPUBuffer> m_InFlightStagingBuffers;

	std::vector<PBE::GPUBuffer> m_VertexBufferPool;
	std::vector<PBE::GPUBuffer> m_IndexBufferPool;
public:
	ChunkMeshManager();
	~ChunkMeshManager();
};

class CPUChunkMesh
{
public:
	std::vector<ChunkVertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
public:
	CPUChunkMesh();
	void AddVertsForQuad(
		PBE::Vec3 const& p0,
		PBE::Vec3 const& p1,
		PBE::Vec3 const& p2,
		PBE::Vec3 const& p3,
		PBE::Vec3 const& normal,
		PBE::AABB2 const& texCoords,
		PBE::Rgba8 const& color
	);
};

class GPUChunkMesh
{
	PBE::GPUBuffer m_VertexBuffer;
	PBE::GPUBuffer m_IndexBuffer;
};