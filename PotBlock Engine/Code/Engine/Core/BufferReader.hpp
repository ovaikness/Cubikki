#pragma once

#include "Engine/Core/Result.hpp"

#include <span>
#include <string>
#include <cstdint>
#include <vector>

namespace PBE
{
	class BufferReader
	{
	public:
		size_t m_Offset = 0;
		std::span<uint8_t> m_Buffer;
	public:
		BufferReader(std::span<uint8_t> buffer)
			: m_Offset(0)
			, m_Buffer(buffer)
		{
		}

		template<typename T_ValueType>
		PBE::Result Read(T_ValueType& value)
		{
			if (m_Offset + sizeof(T_ValueType) > m_Buffer.size())
			{
				return PBE::RESULT_BUFFER_TOO_SMALL;
			}
			value = *(T_ValueType*)&m_Buffer[m_Offset];
			m_Offset += sizeof(T_ValueType);
			return PBE::RESULT_SUCCESS;
		}

		template<typename T_ValueType>
		PBE::Result ReadDynamicArray(std::vector<T_ValueType>& data)
		{
			uint32_t size;
			if (Read(size) != PBE::RESULT_SUCCESS)
			{
				return PBE::RESULT_BUFFER_TOO_SMALL;
			}
			if (m_Offset + size / sizeof(T_ValueType) > m_Buffer.size())
			{
				return PBE::RESULT_BUFFER_TOO_SMALL;
			}
			data.resize(size / sizeof(T_ValueType));
			memcpy(data.data(), &m_Buffer[m_Offset], size / sizeof(T_ValueType));
			m_Offset += size / sizeof(T_ValueType);
			return PBE::RESULT_SUCCESS;
		}

		PBE::Result Read(void* data, size_t size)
		{
			if (m_Offset + size > m_Buffer.size())
			{
				return PBE::RESULT_BUFFER_TOO_SMALL;
			}
			memcpy(data, &m_Buffer[m_Offset], size);
			m_Offset += size;
			return PBE::RESULT_SUCCESS;
		}

		PBE::Result ReadString(std::string& str)
		{
			uint32_t length;
			if (Read(length) != PBE::RESULT_SUCCESS)
			{
				return PBE::RESULT_BUFFER_TOO_SMALL;
			}
			if (m_Offset + length > m_Buffer.size())
			{
				return PBE::RESULT_BUFFER_TOO_SMALL;
			}
			str = std::string((char*)&m_Buffer[m_Offset], length);
			m_Offset += length;
			return PBE::RESULT_SUCCESS;
		}

		bool IsEnd() const
		{
			return m_Offset >= m_Buffer.size();
		}
	};
}