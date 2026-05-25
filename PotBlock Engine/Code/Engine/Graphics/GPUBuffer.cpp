#include "Engine/Graphics/GPUBuffer.hpp"

void PBE::GPUBuffer::Map(VmaAllocator allocator)
{
	vmaMapMemory(allocator, m_Allocation, &m_Data);
}

void PBE::GPUBuffer::Unmap(VmaAllocator allocator)
{
	if (m_Data)
	{
		vmaUnmapMemory(allocator, m_Allocation);
		m_Data = nullptr;
	}
}
