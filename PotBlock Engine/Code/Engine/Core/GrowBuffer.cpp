#include "Engine/Core/GrowBuffer.hpp"

PBE::GrowBuffer::GrowBuffer()
	: m_Head(0)
{

}

PBE::GrowBuffer::GrowBuffer(std::vector<uint8_t>&& buffer)
	: m_Buffer(std::move(buffer))
	, m_Head(0)
{

}

PBE::GrowBuffer::GrowBuffer(GrowBuffer&& other) noexcept
	: m_Buffer(std::move(other.m_Buffer))
	, m_Head(other.m_Head)
{

}

PBE::GrowBuffer::~GrowBuffer()
{

}

void PBE::GrowBuffer::Resize(size_t newSize)
{
	m_Buffer.resize(newSize);
}

void PBE::GrowBuffer::Reserve(size_t size)
{
	m_Buffer.reserve(size);
}

void PBE::GrowBuffer::WriteOffset(size_t offset, void const* data, size_t size)
{
	size_t newSize = offset + size;
	if (newSize > m_Buffer.size())
	{
		m_Buffer.resize(newSize);
	}
	memcpy(m_Buffer.data() + offset, data, size);
}

void PBE::GrowBuffer::WriteDynamicArrayOffset(size_t offset, void const* data, size_t size, size_t count)
{
	WriteOffset(offset, data, count);
	size_t newSize = offset + size * count;
	if (newSize > m_Buffer.size())
	{
		m_Buffer.resize(newSize);
	}
	memcpy(m_Buffer.data() + offset, data, size * count);
}

void PBE::GrowBuffer::Write(void const* data, size_t size)
{
	size_t newSize = m_Head + size;
	if (newSize > m_Buffer.size())
	{
		m_Buffer.resize(newSize);
	}
	memcpy(m_Buffer.data() + m_Head, data, size);
	m_Head = newSize;
}

void PBE::GrowBuffer::WriteString(std::string_view str)
{
	uint32_t size = (uint32_t)str.size();
	Write(&size, sizeof(size));
	Write(str.data(), size);
}

void PBE::GrowBuffer::WriteDynamicArray(void const* data, size_t size)
{
	uint32_t size32 = (uint32_t)size;
	Write(&size32, sizeof(size32));
	size_t newSize = m_Head + size;
	if (newSize > m_Buffer.size())
	{
		m_Buffer.resize(newSize);
	}
	memcpy(m_Buffer.data() + m_Head, data, size32);
	m_Head = newSize;
}

void* PBE::GrowBuffer::GetData()
{
	return m_Buffer.data();
}

void const* PBE::GrowBuffer::GetData() const
{
	return m_Buffer.data();
}

size_t PBE::GrowBuffer::GetSize() const
{
	return m_Buffer.size();
}

std::vector<uint8_t>& PBE::GrowBuffer::GetBuffer()
{
	return m_Buffer;
}

std::vector<uint8_t> const& PBE::GrowBuffer::GetBuffer() const
{
	return m_Buffer;
}

