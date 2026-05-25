#pragma once
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Rotor3D.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat4.hpp"
#include "Engine/Graphics/GPUBuffer.hpp"
#include "Engine/Graphics/GraphicsUtils.hpp"
#include "Engine/Scene/Node.hpp"

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <array>
#include <map>
#include <vector>

namespace PBE
{
	constexpr uint32_t MAX_BONE_COUNT = 255;

	class SkeletalModel;
	class Model;
	class Renderer;

	struct MeshMorphKey
	{
		float m_Time;
		std::vector<float> m_Weights;
		std::vector<int> m_Values;
	};
	struct MeshMorphTarget
	{
		std::string m_Name;
		std::vector<MeshMorphKey> m_Keys;
	};
	struct AnimationChannelVec3
	{
		public:
		std::map<float, Vec3> m_Keys;

		Vec3 GetInterpolatedValue(float time) const
		{
			if (m_Keys.size() == 0)
			{
				return Vec3::ZERO;
			}

			if (m_Keys.size() == 1)
			{
				return m_Keys.begin()->second;
			}

			//First value greater than or equal to time
			auto upperBoundTime = m_Keys.lower_bound(time);
			if (upperBoundTime == m_Keys.end())
			{
				return (--upperBoundTime)->second;
			}

			if (upperBoundTime == m_Keys.begin())
			{
				return upperBoundTime->second;
			}

			//First value less than or equal to time
			auto lowerBoundTime = upperBoundTime--;

			float t = (time - upperBoundTime->first) / (lowerBoundTime->first - upperBoundTime->first);

			return Vec3::Lerp(upperBoundTime->second, lowerBoundTime->second, t);
		}
	};

	struct AnimationChannelQuat
	{
	public:
		std::map<float, Rotor3D> m_Keys;

		Rotor3D GetInterpolatedValue(float time) const
		{
			if (m_Keys.size() == 0)
			{
				return Rotor3D::IDENTITY;
			}

			if (m_Keys.size() == 1)
			{
				return m_Keys.begin()->second;
			}

			//First value greater than or equal to time
			auto upperBoundTime = m_Keys.lower_bound(time);
			if (upperBoundTime == m_Keys.end())
			{
				return (--upperBoundTime)->second;
			}

			if (upperBoundTime == m_Keys.begin())
			{
				return upperBoundTime->second;
			}

			//First value less than or equal to time
			auto lowerBoundTime = upperBoundTime--;

			float t = (time - upperBoundTime->first) / (lowerBoundTime->first - upperBoundTime->first);

			return Rotor3D::Slerp(upperBoundTime->second, lowerBoundTime->second, t);
		}
	};

	struct AnimationChannel
	{
		std::string m_Name;
		std::string m_NodeName;

		Mat4 m_BindPose = Mat4::IDENTITY;
		Mat4 m_InverseBindPose = Mat4::IDENTITY;
		
		AnimationChannelQuat m_Rotations;
		AnimationChannelVec3 m_Translations;
		AnimationChannelVec3 m_Scales;

		bool m_isRoot = false;
		std::vector<std::string> m_Children;
	};

	class Animator;
	struct AnimatorCreateInfo;

	class Animation
	{
		public:
		static VkDescriptorPool s_DescriptorPool;
		static VkDescriptorSetLayout s_DescriptorSetLayout;

		static void CreateDescriptorSetLayout(VkDevice device, uint32_t count);
		static void DestroyDescriptorSetLayout(VkDevice device);

		std::string m_Name;
		float m_Duration;
		float m_TicksPerSecond;

		std::map<std::string, AnimationChannel> m_Channels;
		std::vector<MeshMorphTarget> m_MorphTargets;

		static VkDescriptorSet AllocateDescriptorSet(VkDevice device);
		Vec3 GetTranslation(std::string animationName, float time) const;
		Rotor3D GetRotation(std::string animationName, float time) const;
		Vec3 GetScale(std::string animationName, float time) const;
		Mat4 GetLocalTransform(std::string animationName, float time) const;
	};

	struct AnimatorCreateInfo
	{
		PBE::SkeletalModel* m_Model;

		Renderer* m_Renderer;
	};

	struct AnimatorAnimationState
	{
		float m_Time;
		float m_Weight;
	};

	class Animator
	{
	public:
		Animator() = default;
		Animator(AnimatorCreateInfo const& info);
		void SetAnimationState(std::string name, float time, float weight);
		void SetWeight(std::string name, float weight);
		void SetTime(std::string name, float time);
		void GetWeight(std::string name, float& out_weight) const;
		void GetTime(std::string name, float& out_time) const;

		void UpdateBoneTransforms();
		void RemapAnimatorTransforms();
		void UpdateDescriptorSets(uint32_t frameIndex);
		void CmdBind(VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex);
		void DebugAddVerts(CPUMesh& mesh);
		void Free(VmaAllocator allocator);

		std::vector<std::string> GetBoneNames() const;
		PBE::Mat4 GetBoneTransform(std::string_view name) const;

		void GetAnimationNames(std::vector<std::string>& names) const;
	public:
		AnimatorCreateInfo m_CreateInfo;
		std::map<std::string, AnimatorAnimationState> m_AnimationStates;

		std::vector<Node*> m_Nodes;
		std::vector<Mat4> m_BoneTransforms;

		std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> m_AnimationDescriptorSets{ VK_NULL_HANDLE };
		std::array<GPUBuffer,MAX_FRAMES_IN_FLIGHT> m_BoneTransformBuffers;
	};
}