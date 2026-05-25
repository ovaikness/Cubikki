#include "Engine/Graphics/Animation.hpp"
#include "Engine/Scene/SkeletalModel.hpp"
#include "Engine/Time/Clock.hpp"

PBE::Animator::Animator(AnimatorCreateInfo const& info)
	: m_CreateInfo(info)
{
	auto& defs = m_CreateInfo.m_Model->m_Animations;
	m_Nodes.resize(m_CreateInfo.m_Model->m_Bones.size(), nullptr);
	
	std::function<void(Node*)> processNode;
	processNode = [&](Node* node) -> void
		{
			std::string nodeName = node->m_Name;

			for (auto& entry : m_CreateInfo.m_Model->m_Bones)
			{
				Bone& bone = entry.second;
				if (bone.m_Name == nodeName)
				{
					m_Nodes[bone.m_Index] = node;
					break;
				}
			}

			for (Node* child : node->GetChildren())
			{
				processNode(child);
			}
		};

	processNode(m_CreateInfo.m_Model->m_RootNode);

	VmaAllocator allocator = m_CreateInfo.m_Renderer->m_Allocator;
	VkDevice device = m_CreateInfo.m_Renderer->m_Device;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		m_AnimationDescriptorSets[i] = Animation::AllocateDescriptorSet(device);

		m_BoneTransforms.resize(m_CreateInfo.m_Model->m_Bones.size(), Mat4::IDENTITY);
		if (m_CreateInfo.m_Model->m_Bones.size() == 0)
		{
			continue;
		}

		m_BoneTransformBuffers[i] = CreateGPUBuffer(allocator, sizeof(Mat4) * m_CreateInfo.m_Model->m_Bones.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		m_BoneTransformBuffers[i].Map(allocator);

		vmaSetAllocationName(info.m_Renderer->m_Allocator, m_BoneTransformBuffers[i].m_Allocation, "Animator Buffer Allocation");

		VkDescriptorSet set = m_AnimationDescriptorSets[i];
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_BoneTransformBuffers[i].m_Buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(Mat4) * m_CreateInfo.m_Model->m_Bones.size();

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = set;
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		write.descriptorCount = 1;
		write.pBufferInfo = &bufferInfo;

		VkWriteDescriptorSet writes[] = { write };

		vkUpdateDescriptorSets(device, 1, writes, 0, nullptr);
	}


	for (uint32_t i = 0; i < m_Nodes.size(); ++i)
	{
		if (m_Nodes[i] == nullptr)
		{
			continue;
		}

		m_Nodes[i]->SetLocalTransform(Mat4::IDENTITY);
	}

	for (size_t i = 0; i < m_Nodes.size(); ++i)
	{
		if (m_Nodes[i])
		{
			Mat4 inverseBind = m_Nodes[i]->m_InverseBindMatrix;
			m_BoneTransforms[i] = inverseBind.GetAppended(m_Nodes[i]->GetWorldTransform());
		}
	}
}

void PBE::Animator::SetAnimationState(std::string name, float time, float weight)
{
	Animation const* anim = m_CreateInfo.m_Model->m_Animations[name];
	
	if (anim == nullptr)
	{
		return;
	}

	if (time > anim->m_Duration)
	{
		time = fmodf(time, anim->m_Duration);
	}

	auto itr = m_AnimationStates.find(name);
	if (itr != m_AnimationStates.end())
	{
		itr->second.m_Time = time;
		itr->second.m_Weight = weight;
	}
	else
	{
		m_AnimationStates[name] = AnimatorAnimationState{ time, weight };
	}
	UpdateBoneTransforms();
}


void PBE::Animator::SetWeight(std::string name, float weight)
{
	auto itr = m_AnimationStates.find(name);
	if (itr != m_AnimationStates.end())
	{
		itr->second.m_Weight = weight;
	}
	else
	{
		m_AnimationStates[name] = AnimatorAnimationState{ 0.f, weight };
	}

	UpdateBoneTransforms();
}

void PBE::Animator::SetTime(std::string name, float time)
{
	Animation const* anim = m_CreateInfo.m_Model->m_Animations[name];

	if (anim == nullptr)
	{
		return;
	}

	if (time > anim->m_Duration)
	{
		time = fmodf(time, anim->m_Duration);
	}

	auto itr = m_AnimationStates.find(name);
	if (itr != m_AnimationStates.end())
	{
		itr->second.m_Time = time;
	}
	else
	{
		m_AnimationStates[name] = AnimatorAnimationState{ time, 1.f };
	}

	UpdateBoneTransforms();
}

void PBE::Animator::GetWeight(std::string name, float& out_weight) const
{
	auto itr = m_AnimationStates.find(name);
	if (itr != m_AnimationStates.end())
	{
		out_weight = itr->second.m_Weight;
	}
	else
	{
		out_weight = 0.f;
	}
}

void PBE::Animator::GetTime(std::string name, float& out_time) const
{
	auto itr = m_AnimationStates.find(name);
	if (itr != m_AnimationStates.end())
	{
		out_time = itr->second.m_Time;
	}
	else
	{
		out_time = 0.f;
	}
}

void PBE::Animator::UpdateBoneTransforms()
{
	float weightTotal = 0.f;
	for (auto& entry : m_AnimationStates)
	{
		auto& state = entry.second;
		if (state.m_Weight > 0.f)
		{
			weightTotal += state.m_Weight;
		}
	}

	if (weightTotal == 0.f)
	{
		return;
	}

	float weightInverse = 1.f / weightTotal;

	for (uint32_t i = 0; i < m_Nodes.size(); ++i)
	{
		if (m_Nodes[i] == nullptr)
		{
			continue;
		}

		Vec3 translation = Vec3::ZERO;
		Rotor3D rotation = Rotor3D::IDENTITY;
		Vec3 scale = Vec3::ZERO;
		Mat4 localTransform = Mat4::IDENTITY;

		bool first = true;

		for (auto& entry : m_AnimationStates)
		{
			auto& state = entry.second;
			if (state.m_Weight > 0.f)
			{
				auto itr = m_CreateInfo.m_Model->m_Animations.find(entry.first);
				if (itr == m_CreateInfo.m_Model->m_Animations.end())
				{
					continue;
				}

				Animation const* anim = itr->second;
				if (!anim)
				{
					continue;
				}

				Vec3 animTranslation = anim->GetTranslation(m_Nodes[i]->m_Name, state.m_Time);
				Rotor3D animRotation = anim->GetRotation(m_Nodes[i]->m_Name, state.m_Time);
				Vec3 animScale = anim->GetScale(m_Nodes[i]->m_Name, state.m_Time);

				translation += animTranslation * (state.m_Weight * weightInverse);
				scale += animScale * (state.m_Weight * weightInverse);

				if (first)
				{
					rotation = animRotation;
					first = false;
				}
				else
				{
					rotation = Rotor3D::Slerp(rotation, animRotation, state.m_Weight * weightInverse);
				}
			}
		}

		if (scale == Vec3::ZERO)
		{
			scale = Vec3::ONE;
		}

		rotation = rotation.GetNormalized();

		localTransform = Mat4::CreateTRS(translation, rotation, scale);

		m_Nodes[i]->SetLocalTransform(localTransform);
		m_Nodes[i]->UpdateLocalTransform();
	}

	//Solve once by propagating the world transform from the root node. 
	//Since all nodes are effected this makes it a single pass as opposed to updating each node every time they change.
	m_CreateInfo.m_Model->m_RootNode->UpdateWorldTransform();
	RemapAnimatorTransforms();
}

void PBE::Animator::RemapAnimatorTransforms()
{
	for (size_t i = 0; i < m_Nodes.size(); ++i)
	{
		if (m_Nodes[i])
		{
			Mat4 inverseBind = m_Nodes[i]->m_InverseBindMatrix;
			m_BoneTransforms[i] = inverseBind.GetAppended(m_Nodes[i]->GetWorldTransform());
		}
	}
}

void PBE::Animator::UpdateDescriptorSets(uint32_t frameIndex)
{
	memcpy(m_BoneTransformBuffers[frameIndex].m_Data, m_BoneTransforms.data(), sizeof(Mat4) * m_BoneTransforms.size());
}

void PBE::Animator::CmdBind(VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex)
{
	vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, static_cast<uint32_t>(DescriptorSetType::ANIMATION), 1, &m_AnimationDescriptorSets[frameIndex], 0, nullptr);
}

void PBE::Animator::DebugAddVerts(CPUMesh& mesh)
{
	for (size_t i = 0; i < m_Nodes.size(); ++i)
	{
		if (m_Nodes[i])
		{
			Vec3 position = m_Nodes[i]->GetWorldTransform().GetTranslation3D();

			Rgba8 color = Rgba8::Lerp(Rgba8::GREEN, Rgba8::RED, (float)i / (float)m_Nodes.size());
			mesh.AddVertsForSphere(position, 5.f, 8, 16, color);
		}
	}
}

void PBE::Animator::Free(VmaAllocator allocator)
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		m_BoneTransformBuffers[i].Unmap(allocator);
		vmaDestroyBuffer(allocator,m_BoneTransformBuffers[i].m_Buffer, m_BoneTransformBuffers[i].m_Allocation);
	}
}

std::vector<std::string> PBE::Animator::GetBoneNames() const
{
	std::vector<std::string> names;
	for (auto pair : m_CreateInfo.m_Model->m_Bones)
	{
		names.push_back(pair.second.m_Name);
	}
	return names;
}

PBE::Mat4 PBE::Animator::GetBoneTransform(std::string_view name) const
{
	if(m_CreateInfo.m_Model == nullptr)
	{
		return Mat4::IDENTITY;
	}

	if(m_CreateInfo.m_Model->m_Bones.empty())
	{
		return Mat4::IDENTITY;
	}

	auto itr = m_CreateInfo.m_Model->m_Bones.find(std::string{ name });
	if(itr == m_CreateInfo.m_Model->m_Bones.end())
	{
		return Mat4::IDENTITY;
	}

	PBE::Bone const& bone = itr->second;
	size_t index = bone.m_Index;
	if (index < 0 || index >= static_cast<int>(m_BoneTransforms.size()))
	{
		return Mat4::IDENTITY;
	}
	return m_BoneTransforms[index];
}

void PBE::Animator::GetAnimationNames(std::vector<std::string>& names) const
{
	names.reserve(m_AnimationStates.size());
	for (auto& animation : m_CreateInfo.m_Model->m_Animations)
	{
		names.push_back(animation.first);
	}
}

PBE::Vec3 PBE::Animation::GetTranslation(std::string animationName, float time) const
{
	Vec3 translation = Vec3::ZERO;
	auto itr = m_Channels.find(animationName);
	if (itr != m_Channels.end())
	{
		AnimationChannel const& boneAnim = itr->second;
		translation = boneAnim.m_Translations.GetInterpolatedValue(time);
		return translation;
	}
	return translation;
}

PBE::Rotor3D PBE::Animation::GetRotation(std::string animationName, float time) const
{
	Rotor3D rotation = Rotor3D::IDENTITY;
	auto itr = m_Channels.find(animationName);
	if (itr != m_Channels.end())
	{
		AnimationChannel const& boneAnim = itr->second;
		rotation = boneAnim.m_Rotations.GetInterpolatedValue(time);
		return rotation;
	}
	return rotation;
}

PBE::Vec3 PBE::Animation::GetScale(std::string animationName, float time) const
{
	Vec3 scale = Vec3::ONE;
	auto itr = m_Channels.find(animationName);
	if (itr != m_Channels.end())
	{
		AnimationChannel const& boneAnim = itr->second;
		scale = boneAnim.m_Scales.GetInterpolatedValue(time);
		return scale;
	}
	return scale;
}

PBE::Mat4 PBE::Animation::GetLocalTransform(std::string animationName, float time) const
{
	Mat4 transform = Mat4::IDENTITY;

	auto itr = m_Channels.find(animationName);
	if (itr != m_Channels.end())
	{
		AnimationChannel const& boneAnim = itr->second;
		Rotor3D rotation = boneAnim.m_Rotations.GetInterpolatedValue(time);
		Vec3 translation = boneAnim.m_Translations.GetInterpolatedValue(time);
		Vec3 scale = boneAnim.m_Scales.GetInterpolatedValue(time);
		transform = Mat4::CreateTRS(translation, rotation, scale);

		assert(transform.IsValid());
		return transform;
	}
	return transform;
}

VkDescriptorPool PBE::Animation::s_DescriptorPool;
VkDescriptorSetLayout PBE::Animation::s_DescriptorSetLayout;

void PBE::Animation::CreateDescriptorSetLayout(VkDevice device, uint32_t count)
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = count;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = count;

	VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &s_DescriptorPool);

	VkErrorCheck(result, "Failed to create descriptor pool");

	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &layoutBinding;

	result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &s_DescriptorSetLayout);

	VkErrorCheck(result, "Failed to create descriptor set layout");
}

void PBE::Animation::DestroyDescriptorSetLayout(VkDevice device)
{
	vkDestroyDescriptorPool(device, s_DescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, s_DescriptorSetLayout, nullptr);
}

VkDescriptorSet PBE::Animation::AllocateDescriptorSet(VkDevice device)
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = s_DescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &s_DescriptorSetLayout;

	VkDescriptorSet descriptorSet;
	VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
	VkErrorCheck(result, "Failed to allocate descriptor set");

	return descriptorSet;
}
