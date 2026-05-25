#pragma once

#include "Engine/Net/NetCommon.hpp"
#include "Engine/Threads/AtomicQueue.hpp"

#include <thread>

namespace PBE
{
	enum class NetMode
	{
		SERVER,
		CLIENT
	};

	template <typename T_ValueType>
	class NetSystem
	{
	public:
		NetMode m_Mode = NetMode::SERVER;
		asio::io_context m_Context = {};
		std::thread m_ContextThread = {};
	}; 
}