#pragma once

#include <vma/vk_mem_alloc.h>

namespace PBE
{
	struct GPUBuffer
	{
		public:
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceSize m_Size = 0;

		void Map(VmaAllocator device);
		void Unmap(VmaAllocator device);

		bool operator==(GPUBuffer const& other) const
		{
			return m_Buffer == other.m_Buffer;
		}

		void* m_Data = nullptr;
	};
}