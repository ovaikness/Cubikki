#pragma once

namespace PBE
{
	template <typename T_ValueType>
	struct NetMessageHeader
	{
		T_ValueType m_ID;
		uint32_t m_Size = 0;
	};

	template <typename T_ValueType>
	struct NetMessage
	{
		NetMessageHeader<T_ValueType> m_Header;
		std::vector<uint8_t> m_Body;
		size_t Size() const
		{
			return m_Body.size();
		}
		friend std::ostream& operator << (std::ostream& os, const NetMessage<T_ValueType>& msg)
		{
			os << "ID: " << int(msg.m_Header.m_ID) << " Size: " << msg.m_Header.m_Size;
			return os;
		}

		template<typename DataType>
		friend NetMessage<T_ValueType>& operator << (NetMessage<T_ValueType>& msg, const DataType& data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");
			size_t i = msg.m_Body.size();
			msg.m_Body.resize(msg.m_Body.size() + sizeof(DataType));
			std::memcpy(msg.m_Body.data() + i, &data, sizeof(DataType));
			msg.m_Header.m_Size = (uint32_t)msg.Size();
			return msg;
		}
		template<typename DataType>
		friend NetMessage<T_ValueType>& operator >> (NetMessage<T_ValueType>& msg, DataType& data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");
			size_t i = msg.m_Body.size() - sizeof(DataType);
			std::memcpy(&data, msg.m_Body.data() + i, sizeof(DataType));
			msg.m_Body.resize(i);
			msg.m_Header.m_Size = (uint32_t)msg.Size();
			return msg;
		}
	};
}