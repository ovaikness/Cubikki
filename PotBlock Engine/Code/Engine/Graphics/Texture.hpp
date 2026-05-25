#pragma once

#include <filesystem>

#include <vector>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace PBE
{
	enum class TextureType
	{
		Texture2D,
		Texture2DArray,
		TextureCube
	};

	class Texture_T
	{
	public:
		virtual ~Texture_T();
		VkFormat m_Format{};
		VkImage m_Image{ VK_NULL_HANDLE };
		VkImageLayout m_ImageLayout{};
		VkDeviceMemory m_ImageMemory { VK_NULL_HANDLE };
		VmaAllocation m_Allocation { VK_NULL_HANDLE };
		VkImageView m_ImageView { VK_NULL_HANDLE };
		uint32_t m_Width = 0, m_Height = 0;
		uint32_t m_MipLevels = 0;
		uint32_t m_LayerCount = 0;
		VkDescriptorImageInfo m_Descriptor{};
		VkSampler m_Sampler{ VK_NULL_HANDLE };
	};

	class Texture2DArray_T : public Texture_T
	{
	public:
		virtual ~Texture2DArray_T();
	
	};

	class Texture2D_T : public Texture_T
	{
	public:
		virtual ~Texture2D_T();
	};

	class TextureCube_T : public Texture_T
	{
	public:
		virtual ~TextureCube_T();
	};

	using Texture = Texture_T*;
	using Texture2DArray = Texture2DArray_T*;
	using Texture2D = Texture2D_T*;
	using TextureCube = TextureCube_T*;
}