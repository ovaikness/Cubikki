#pragma once

#include <vector>
#include <string_view>

namespace PBE
{
	// A buffer that can grow in size as needed to store arbitrary data.
	class GrowBuffer
	{
	public:
		std::vector<uint8_t> m_Buffer;
		size_t m_Head = 0;
	public:
		GrowBuffer(GrowBuffer&& other) noexcept;
		GrowBuffer(std::vector<uint8_t>&& buffer);
		GrowBuffer();
		~GrowBuffer();

		void Resize(size_t newSize);
		void Reserve(size_t size);

		template<typename T_ValueType>
		void Write(T_ValueType const& value)
		{
			Write(&value, sizeof(T_ValueType));
		}
		void WriteOffset(size_t offset, void const* data, size_t size);
		void WriteDynamicArrayOffset(size_t offset, void const* data, size_t size, size_t count);
		void Write(void const* data, size_t size);
		void WriteString(std::string_view str);
		void WriteDynamicArray(void const* data, size_t size);

		void* GetData();
		void const* GetData() const;
		size_t GetSize() const;

		std::vector<uint8_t>& GetBuffer();
		std::vector<uint8_t> const& GetBuffer() const;
	};
}