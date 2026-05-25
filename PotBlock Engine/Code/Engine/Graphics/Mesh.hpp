#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Graphics/GPUBuffer.hpp"
#include "Engine/Graphics/Rgba8.hpp"

#include <vulkan/vulkan.h>
#include <filesystem>
#include <optional>
#include <vector>
#include <variant>

namespace PBE
{
	enum class VertexAttribute : uint32_t
	{
		POSITION3D = 0,
		NORMAL = 1,
		UV = 2,
		COLOR = 3,
		TANGENT = 4,
		BITANGENT = 5,
		BONE_WEIGHTS = 6,
		BONE_INDICES = 7,
		COUNT
	};

	struct BoneWeights
	{
		float m_Weights[4];
	};

	struct BoneIndices
	{
		int32_t m_Indices[4];
	};

	class CPUMesh
	{
	public:
		CPUMesh();
		~CPUMesh();

		std::optional<std::vector<Vec3>> m_Positions;
		std::optional<std::vector<Vec3>> m_Normals;
		std::optional<std::vector<Vec2>> m_UVs;
		std::optional<std::vector<Rgba8>> m_Colors;
		std::optional<std::vector<Vec3>> m_Tangents;
		std::optional<std::vector<Vec3>> m_Bitangents;
		std::optional<std::vector<BoneWeights>> m_BoneWeights;
		std::optional<std::vector<BoneIndices>> m_BoneIndices;
		std::vector<uint32_t> m_Indices;

	public:
		void AddVertsForAABB2(AABB2 bounds, AABB2 texCoords, Rgba8 color = Rgba8::WHITE);
		void AddVertsForOBB2(OBB2 bounds, AABB2 texCoords, Rgba8 color = Rgba8::WHITE);
		void AddVertsForQuad(
			Vec3 const& p0, 
			Vec3 const& p1, 
			Vec3 const& p2, 
			Vec3 const& p3, 
			Vec3 const& normal,
			Vec3 const& tangent,
			Vec3 const& bitangent,
			AABB2 const& texCoords = AABB2(0,0,1,1),
			Rgba8 const& color = Rgba8::WHITE
		);
		void AddVertsForSphere(Vec3 const& center, float radius, uint32_t numSlices, uint32_t numWedges, Rgba8 color);
		void AddVertsForCylinder(Vec3 const& start, Vec3 const& end, float thickness, uint32_t numSlices);
		void InitializeArrays();
	};

	class GPUMesh
	{
	public:
		static std::array<VkVertexInputAttributeDescription, (size_t)VertexAttribute::COUNT> s_VertexAttributeDescriptions;
		static std::array<VkVertexInputBindingDescription, (size_t)VertexAttribute::COUNT> s_VertexBindingDescription;
	public:
		GPUMesh();
		~GPUMesh();

		void CmdBind(VkCommandBuffer buffer);

		void CmdDraw(VkCommandBuffer buffer);
		uint32_t GetIndexCount() const { return m_IndexCount; }
	protected:
		friend class Renderer;
		static void CreateVertexInputDescriptions();

		std::vector<std::optional<GPUBuffer>> m_VertexBuffers;
		GPUBuffer m_IndexBuffer;

		uint32_t m_IndexCount;

		VkDescriptorSet m_DescriptorSets { VK_NULL_HANDLE };
	};
}