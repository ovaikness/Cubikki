#pragma once
#include "Engine/Scene/Node.hpp"
#include "Engine/Graphics/Mesh.hpp"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/Animation.hpp"
#include "Engine/Math/Mat4.hpp"

#include "Engine/Graphics/Renderer.hpp"
#include "Engine/Graphics/Camera.hpp"

#include <vector>
#include <filesystem>

namespace PBE
{
	struct SkeletalModelCreateInfo
	{
		std::string m_Name;
		Renderer* m_Renderer;
	};
	struct Bone
	{
		std::string m_Name;
		uint32_t m_Index;
		Mat4 m_InverseBindMatrix;
	};
	class SkeletalModel
	{
	public:
		SkeletalModelCreateInfo m_CreateInfo;
		Node* m_RootNode;
		std::vector<GPUMesh*> m_Meshes;
		std::vector<Material*> m_Materials;
		std::map<std::string, Bone> m_Bones;
		std::map<std::string, Animation*> m_Animations;
	public:
		SkeletalModel(SkeletalModelCreateInfo const& createInfo);
		~SkeletalModel();
		void LoadFromFile(std::filesystem::path path);

		void CmdDraw(Camera* camera, Mat4 const& transform, VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex);
	};
}