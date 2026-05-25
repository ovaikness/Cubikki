#pragma once
#include "Engine/Net/NetCommon.hpp"
#include "Engine/Net/NetSystem.hpp"
#include "Engine/Net/NetConnection.hpp"
#include "Engine/Threads/AtomicQueue.hpp"

#include <iostream>
#include <string_view>
#include <vector>

namespace PBE
{
	template <typename T_ValueType>
	class NetClientInterface : public NetSystem<T_ValueType>
	{
	public:
		NetConnection<T_ValueType>* m_Connection;
	public:
		NetClientInterface()
			: m_Connection(nullptr)
		{
			this->m_Mode = NetMode::CLIENT;
		}

		~NetClientInterface()
		{
			Disconnect();
		}

		void Update()
		{
			if (!IsConnected())
			{
				return;
			}
			while (!m_Connection->m_MessagesIn.Empty())
			{
				NetMessage<T_ValueType> message = m_Connection->m_MessagesIn.PopFront();
				OnMessage(message);
			}
		}

		bool Connect(std::string_view host, uint16_t port)
		{
			try
			{
				asio::ip::tcp::resolver resolver(this->m_Context);
				asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

				m_Connection = new NetConnection<T_ValueType>(this, asio::ip::tcp::socket(this->m_Context));

				m_Connection->ConnectToServer(endpoints);

				this->m_ContextThread = std::thread([this]() { this->m_Context.run(); });
			}
			catch (std::exception& e)
			{
				Disconnect();
				std::cerr << "[CLIENT] Exception: " << e.what() << "\n";
				return false;
			}

			return true;
		}

		void Disconnect()
		{
			if (IsConnected())
			{
				m_Connection->Disconnect();
			}

			this->m_Context.stop();

			if (this->m_ContextThread.joinable())
			{
				this->m_ContextThread.join();
			}

			delete m_Connection;
			m_Connection = nullptr;
		}

		bool IsConnected()
		{
			if (m_Connection)
			{
				return m_Connection->IsConnected();
			}
			return false;
		}

		void Send(NetMessage<T_ValueType> const& message)
		{
			if (IsConnected())
			{
				m_Connection->Send(message);
			}
		}

		uint32_t GetID() const
		{
			if (m_Connection)
			{
				return m_Connection->GetId();
			}
			return 0;
		}

		virtual void OnMessage(PBE::NetMessage<T_ValueType>& msg) = 0;
	};
}