#include "Engine/Graphics/Mesh.hpp"
#include "Engine/Graphics/GPUBuffer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Graphics/GraphicsUtils.hpp"
#include <array>

PBE::CPUMesh::CPUMesh()
{

}

PBE::CPUMesh::~CPUMesh()
{

}

void PBE::CPUMesh::AddVertsForAABB2(AABB2 bounds, AABB2 texCoords, Rgba8 color)
{
	InitializeArrays();

	Vec2 mins = bounds.m_Mins;
	Vec2 maxs = bounds.m_Maxs;

	Vec2 minsUV = texCoords.m_Mins;
	Vec2 maxsUV = texCoords.m_Maxs;

	Vec3 p0 = Vec3(mins.x, mins.y, 1.f);
	Vec3 p1 = Vec3(maxs.x, mins.y, 1.f);
	Vec3 p2 = Vec3(maxs.x, maxs.y, 1.f);
	Vec3 p3 = Vec3(mins.x, maxs.y, 1.f);

	Vec2 uv0 = Vec2(minsUV.x, maxsUV.y);
	Vec2 uv1 = Vec2(maxsUV.x, maxsUV.y);
	Vec2 uv2 = Vec2(maxsUV.x, minsUV.y);
	Vec2 uv3 = Vec2(minsUV.x, minsUV.y);

	Vec3 normal = Vec3::FORWARD;

	m_Positions->push_back(p0);
	m_Positions->push_back(p1);
	m_Positions->push_back(p2);
	m_Positions->push_back(p3);

	m_UVs->push_back(uv0);
	m_UVs->push_back(uv1);
	m_UVs->push_back(uv2);
	m_UVs->push_back(uv3);

	m_Colors->push_back(color);
	m_Colors->push_back(color);
	m_Colors->push_back(color);
	m_Colors->push_back(color);

	m_BoneIndices->push_back({ -1, -1, -1, -1 });
	m_BoneIndices->push_back({ -1, -1, -1, -1 });
	m_BoneIndices->push_back({ -1, -1, -1, -1 });
	m_BoneIndices->push_back({ -1, -1, -1, -1 });

	m_BoneWeights->push_back({ 1.f, 0.f, 0.f, 0.f });
	m_BoneWeights->push_back({ 1.f, 0.f, 0.f, 0.f });
	m_BoneWeights->push_back({ 1.f, 0.f, 0.f, 0.f });
	m_BoneWeights->push_back({ 1.f, 0.f, 0.f, 0.f });

	uint32_t startIndex = (uint32_t)m_Positions->size() - 4;
	m_Indices.push_back(startIndex + 0);
	m_Indices.push_back(startIndex + 1);
	m_Indices.push_back(startIndex + 2);
	m_Indices.push_back(startIndex + 0);
	m_Indices.push_back(startIndex + 2);
	m_Indices.push_back(startIndex + 3);
}

void PBE::CPUMesh::AddVertsForOBB2(OBB2 bounds, AABB2 texCoords, Rgba8 color /* = Rgba8::WHITE*/)
{
	InitializeArrays();

	Vec2 iBasis = bounds.GetIBasis();
	Vec2 jBasis = bounds.GetJBasis();
	Vec2 center = bounds.GetCenter();
	Vec2 halfExtents = bounds.m_Extents * 0.5f;

	Vec2 minsUV = texCoords.m_Mins;
	Vec2 maxsUV = texCoords.m_Maxs;

	Vec3 p0 = iBasis * -halfExtents.x + jBasis * -halfExtents.y + center;
	Vec3 p1 = iBasis * halfExtents.x + jBasis * -halfExtents.y + center;
	Vec3 p2 = iBasis * halfExtents.x + jBasis * halfExtents.y + center;
	Vec3 p3 = iBasis * -halfExtents.x + jBasis * halfExtents.y + center;

	Vec2 uv0 = Vec2(minsUV.x, maxsUV.y);
	Vec2 uv1 = Vec2(maxsUV.x, maxsUV.y);
	Vec2 uv2 = Vec2(maxsUV.x, minsUV.y);
	Vec2 uv3 = Vec2(minsUV.x, minsUV.y);

	Vec3 normal = Vec3::FORWARD;

	m_Positions->push_back(p0);
	m_Positions->push_back(p1);
	m_Positions->push_back(p2);
	m_Positions->push_back(p3);

	m_UVs->push_back(uv0);
	m_UVs->push_back(uv1);
	m_UVs->push_back(uv2);
	m_UVs->push_back(uv3);

	m_Colors->push_back(color);
	m_Colors->push_back(color);
	m_Colors->push_back(color);
	m_Colors->push_back(color);

	uint32_t startIndex = (uint32_t)m_Positions->size() - 4;
	m_Indices.push_back(startIndex + 0);
	m_Indices.push_back(startIndex + 1);
	m_Indices.push_back(startIndex + 2);
	m_Indices.push_back(startIndex + 0);
	m_Indices.push_back(startIndex + 2);
	m_Indices.push_back(startIndex + 3);
}

void PBE::CPUMesh::AddVertsForQuad(
	Vec3 const& p0, 
	Vec3 const& p1, 
	Vec3 const& p2, 
	Vec3 const& p3, 
	Vec3 const& normal,
	Vec3 const& tangent,
	Vec3 const& bitangent,
	AABB2 const& texCoords, 
	Rgba8 const& color
)
{
	Vec2 minsUV = texCoords.m_Mins;
	Vec2 maxsUV = texCoords.m_Maxs;

	Vec2 uv0 = Vec2(minsUV.x, maxsUV.y);
	Vec2 uv1 = Vec2(minsUV.x, minsUV.y);
	Vec2 uv2 = Vec2(maxsUV.x, maxsUV.y);
	Vec2 uv3 = Vec2(maxsUV.x, minsUV.y);

	Vec3 e0 = p1 - p0;
	Vec3 e1 = p2 - p1;

	m_Positions->push_back(p0);
	m_Positions->push_back(p1);
	m_Positions->push_back(p2);
	m_Positions->push_back(p3);

	m_Normals->push_back(normal);
	m_Normals->push_back(normal);
	m_Normals->push_back(normal);
	m_Normals->push_back(normal);

	m_Tangents->push_back(tangent);
	m_Tangents->push_back(tangent);
	m_Tangents->push_back(tangent);
	m_Tangents->push_back(tangent);

	m_Bitangents->push_back(bitangent);
	m_Bitangents->push_back(bitangent);
	m_Bitangents->push_back(bitangent);
	m_Bitangents->push_back(bitangent);

	m_BoneIndices->push_back({ -1, -1, -1, -1 });
	m_BoneIndices->push_back({ -1, -1, -1, -1 });
	m_BoneIndices->push_back({ -1, -1, -1, -1 });
	m_BoneIndices->push_back({ -1, -1, -1, -1 });

	m_BoneWeights->push_back({ 1.f, 0.f, 0.f, 0.f });
	m_BoneWeights->push_back({ 1.f, 0.f, 0.f, 0.f });
	m_BoneWeights->push_back({ 1.f, 0.f, 0.f, 0.f });
	m_BoneWeights->push_back({ 1.f, 0.f, 0.f, 0.f });

	m_UVs->push_back(uv0);
	m_UVs->push_back(uv1);
	m_UVs->push_back(uv2);
	m_UVs->push_back(uv3);

	m_Colors->push_back(color);
	m_Colors->push_back(color);
	m_Colors->push_back(color);
	m_Colors->push_back(color);

	uint32_t startIndex = (uint32_t)m_Positions->size() - 4;

	m_Indices.push_back(startIndex + 0);
	m_Indices.push_back(startIndex + 1);
	m_Indices.push_back(startIndex + 2);
	m_Indices.push_back(startIndex + 1);
	m_Indices.push_back(startIndex + 3);
	m_Indices.push_back(startIndex + 2);
}

void PBE::CPUMesh::AddVertsForSphere(Vec3 const& center, float radius, uint32_t numSlices, uint32_t numWedges, Rgba8 color)
{
	InitializeArrays();

	size_t startIndex = m_Positions->size();
	float wedgeAngle = 360.f / numWedges;
	float sliceAngle = 180.f / numSlices;

	for (uint32_t sliceIndex = 0; sliceIndex <= numSlices; ++sliceIndex)
	{
		float phi = sliceAngle * sliceIndex;
		for (uint32_t wedgeIndex = 0; wedgeIndex <= numWedges; ++wedgeIndex)
		{
			float theta = wedgeAngle * wedgeIndex;

			Vec3 position;
			position.x = center.x + radius * SinDegrees(phi) * CosDegrees(theta);
			position.y = center.y + radius * SinDegrees(phi) * SinDegrees(theta);
			position.z = center.z + radius * CosDegrees(phi);

			Vec3 normal = (position - center).GetNormalized();

			Vec2 uv;
			uv.x = theta / 360.f;
			uv.y = phi / 180.f;

			m_Positions->push_back(position);
			m_Normals->push_back(normal);
			m_UVs->push_back(uv);
			m_Colors->push_back(color);

			Vec3 bitangent = Vec3::CrossProduct(normal, Vec3::UP);
			if (bitangent.GetLengthSquared() == 0.f) {
				bitangent = Vec3::RIGHT; // Fallback for edge cases
			}
			bitangent = bitangent.GetNormalized();

			Vec3 tangent = Vec3::CrossProduct(bitangent, normal).GetNormalized();
			m_Bitangents->push_back(bitangent);
			m_Tangents->push_back(tangent);

			BoneWeights weights = { 0.f, 0.f, 0.f, 0.f };
			BoneIndices indices = { -1, -1, -1, -1 };

			m_BoneWeights->push_back(weights);
			m_BoneIndices->push_back(indices);
		}
	}

	// Generate indices
	for (uint32_t sliceIndex = 0; sliceIndex < numSlices; ++sliceIndex)
	{
		for (uint32_t wedgeIndex = 0; wedgeIndex < numWedges; ++wedgeIndex)
		{
			uint32_t i0 = (uint32_t)startIndex + sliceIndex * (numWedges + 1) + wedgeIndex;
			uint32_t i1 = (uint32_t)startIndex + (sliceIndex + 1) * (numWedges + 1) + wedgeIndex;
			uint32_t i2 = (uint32_t)startIndex + (sliceIndex + 1) * (numWedges + 1) + (wedgeIndex + 1) % (numWedges + 1);
			uint32_t i3 = (uint32_t)startIndex + sliceIndex * (numWedges + 1) + (wedgeIndex + 1) % (numWedges + 1);

			m_Indices.push_back(i0);
			m_Indices.push_back(i1);
			m_Indices.push_back(i2);

			m_Indices.push_back(i0);
			m_Indices.push_back(i2);
			m_Indices.push_back(i3);
		}
	}
}

void PBE::CPUMesh::AddVertsForCylinder(Vec3 const& start, Vec3 const& end, float thickness, uint32_t numSlices)
{
	InitializeArrays();

	Vec3 center = (start + end) * 0.5f;
	Vec3 up = end - start;
	Vec3 right = Vec3::CrossProduct(up, Vec3::UP);
	if (right.GetLengthSquared() == 0.f)
	{
		right = Vec3::RIGHT;
	}

	Vec3 forward = Vec3::CrossProduct(right, up).GetNormalized();

	size_t startIndex = m_Positions->size();
	float wedgeAngle = 360.f / numSlices;

	for (uint32_t sliceIndex = 0; sliceIndex < 2; ++sliceIndex)
	{
		Vec3 normal = sliceIndex == 0 ? -up : up;
		Vec3 tangent = right;
		Vec3 bitangent = forward;

		for (uint32_t wedgeIndex = 0; wedgeIndex < numSlices; ++wedgeIndex)
		{
			float theta = wedgeAngle * wedgeIndex;

			Vec3 position = center + (right * CosDegrees(theta) + forward * SinDegrees(theta)) * thickness * (sliceIndex == 0 ? -0.5f : 0.5f);

			m_Positions->push_back(position);
			m_Normals->push_back(normal);
			m_UVs->push_back(Vec2::ZERO);
			m_Colors->push_back(Rgba8::WHITE);
			m_Tangents->push_back(tangent);
			m_Bitangents->push_back(bitangent);

			BoneWeights weights = { 0.f, 0.f, 0.f, 0.f };
			BoneIndices indices = { -1, -1, -1, -1 };

			m_BoneWeights->push_back(weights);
			m_BoneIndices->push_back(indices);
		}
	}

	// Generate indices

	for (uint32_t sliceIndex = 0; sliceIndex < 1; ++sliceIndex)
	{
		for (uint32_t wedgeIndex = 0; wedgeIndex < numSlices; ++wedgeIndex)
		{
			uint32_t i0 = (uint32_t)startIndex + sliceIndex * numSlices + wedgeIndex;
			uint32_t i1 = (uint32_t)startIndex + (sliceIndex + 1) * numSlices + wedgeIndex;
			uint32_t i2 = (uint32_t)startIndex + (sliceIndex + 1) * numSlices + (wedgeIndex + 1) % 16;
			uint32_t i3 = (uint32_t)startIndex + sliceIndex * numSlices + (wedgeIndex + 1) % 16;

			m_Indices.push_back(i0);
			m_Indices.push_back(i1);
			m_Indices.push_back(i2);

			m_Indices.push_back(i0);
			m_Indices.push_back(i2);
			m_Indices.push_back(i3);
		}
	}

	// Generate caps

	for (uint32_t sliceIndex = 0; sliceIndex < numSlices; ++sliceIndex)
	{
		float theta = wedgeAngle * sliceIndex;
		Vec3 position = center + (right * CosDegrees(theta) + forward * SinDegrees(theta)) * thickness * 0.5f;

		m_Positions->push_back(position);
		m_Normals->push_back(up);
		m_UVs->push_back(Vec2::ZERO);
		m_Colors->push_back(Rgba8::WHITE);
		m_Tangents->push_back(right);
		m_Bitangents->push_back(forward);

		BoneWeights weights = { 0.f, 0.f, 0.f, 0.f };
		BoneIndices indices = { -1, -1, -1, -1 };

		m_BoneWeights->push_back(weights);
		m_BoneIndices->push_back(indices);
	}

	uint32_t capStartIndex = (uint32_t)startIndex + numSlices * 2;

	for (uint32_t sliceIndex = 0; sliceIndex < numSlices; ++sliceIndex)
	{
		uint32_t i0 = capStartIndex + sliceIndex;
		uint32_t i1 = capStartIndex + (sliceIndex + 1) % numSlices;
		uint32_t i2 = (uint32_t)startIndex + sliceIndex;
		uint32_t i3 = (uint32_t)startIndex + (sliceIndex + 1) % numSlices;

		m_Indices.push_back(i0);
		m_Indices.push_back(i1);
		m_Indices.push_back(i2);

		m_Indices.push_back(i0);
		m_Indices.push_back(i2);
		m_Indices.push_back(i3);
	}
}

PBE::GPUMesh::GPUMesh()
{
	m_VertexBuffers.resize((size_t)VertexAttribute::COUNT);
	m_DescriptorSets = VK_NULL_HANDLE;
	m_IndexBuffer = {};
	m_IndexCount = 0;
}

PBE::GPUMesh::~GPUMesh()
{
}

void PBE::GPUMesh::CmdBind(VkCommandBuffer command)
{
	if (m_IndexCount == 0)
	{
		return;
	}
	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceSize> offsets;

	for (size_t i = 0; i < m_VertexBuffers.size(); ++i)
	{
		if (m_VertexBuffers[i].has_value())
		{
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(command, (uint32_t)i, 1, &m_VertexBuffers[i]->m_Buffer, &offset);
		}
	}

	vkCmdBindIndexBuffer(command, m_IndexBuffer.m_Buffer, 0, VK_INDEX_TYPE_UINT32);
}

void PBE::GPUMesh::CmdDraw(VkCommandBuffer buffer)
{
	if (m_IndexCount == 0)
	{
		return;
	}
	vkCmdDrawIndexed(buffer, m_IndexCount, 1, 0, 0, 0);
}

std::array<VkVertexInputAttributeDescription, (size_t)PBE::VertexAttribute::COUNT> PBE::GPUMesh::s_VertexAttributeDescriptions;
std::array<VkVertexInputBindingDescription, (size_t)PBE::VertexAttribute::COUNT> PBE::GPUMesh::s_VertexBindingDescription;

void PBE::GPUMesh::CreateVertexInputDescriptions()
{
	for (uint32_t i = 0; i < static_cast<uint32_t>(VertexAttribute::COUNT); ++i)
	{
		s_VertexAttributeDescriptions[i].binding = i;
		s_VertexAttributeDescriptions[i].location = i;
		s_VertexAttributeDescriptions[i].offset = 0;

		s_VertexBindingDescription[i].binding = i;
		s_VertexBindingDescription[i].stride = 0;
		s_VertexBindingDescription[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}
	s_VertexAttributeDescriptions[(size_t)VertexAttribute::POSITION3D].format	= VK_FORMAT_R32G32B32_SFLOAT;
	s_VertexAttributeDescriptions[(size_t)VertexAttribute::NORMAL].format		= VK_FORMAT_R32G32B32_SFLOAT;
	s_VertexAttributeDescriptions[(size_t)VertexAttribute::UV].format			= VK_FORMAT_R32G32_SFLOAT;
	s_VertexAttributeDescriptions[(size_t)VertexAttribute::COLOR].format		= VK_FORMAT_R8G8B8A8_UNORM;
	s_VertexAttributeDescriptions[(size_t)VertexAttribute::TANGENT].format		= VK_FORMAT_R32G32B32_SFLOAT;
	s_VertexAttributeDescriptions[(size_t)VertexAttribute::BITANGENT].format	= VK_FORMAT_R32G32B32_SFLOAT;
	s_VertexAttributeDescriptions[(size_t)VertexAttribute::BONE_WEIGHTS].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	s_VertexAttributeDescriptions[(size_t)VertexAttribute::BONE_INDICES].format = VK_FORMAT_R32G32B32A32_SINT;

	s_VertexBindingDescription[(size_t)VertexAttribute::POSITION3D].stride		= sizeof(Vec3);
	s_VertexBindingDescription[(size_t)VertexAttribute::NORMAL].stride			= sizeof(Vec3);
	s_VertexBindingDescription[(size_t)VertexAttribute::UV].stride				= sizeof(Vec2);
	s_VertexBindingDescription[(size_t)VertexAttribute::COLOR].stride			= sizeof(Rgba8);
	s_VertexBindingDescription[(size_t)VertexAttribute::TANGENT].stride			= sizeof(Vec3);
	s_VertexBindingDescription[(size_t)VertexAttribute::BITANGENT].stride		= sizeof(Vec3);
	s_VertexBindingDescription[(size_t)VertexAttribute::BONE_WEIGHTS].stride	= sizeof(BoneWeights);
	s_VertexBindingDescription[(size_t)VertexAttribute::BONE_INDICES].stride	= sizeof(BoneIndices);
}

void PBE::CPUMesh::InitializeArrays()
{
	if (m_Positions == std::nullopt) m_Positions = std::vector<Vec3>();
	if (m_Normals == std::nullopt) m_Normals = std::vector<Vec3>();
	if (m_UVs == std::nullopt) m_UVs = std::vector<Vec2>();
	if (m_Colors == std::nullopt) m_Colors = std::vector<Rgba8>();
	if (m_Tangents == std::nullopt) m_Tangents = std::vector<Vec3>();
	if (m_Bitangents == std::nullopt) m_Bitangents = std::vector<Vec3>();
	if (m_BoneWeights == std::nullopt) m_BoneWeights = std::vector<BoneWeights>();
	if (m_BoneIndices == std::nullopt) m_BoneIndices = std::vector<BoneIndices>();

}
