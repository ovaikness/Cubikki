#pragma once
#include "Engine/Net/NetCommon.hpp"
#include "Engine/Net/NetMessage.hpp"
#include "Engine/Net/NetSystem.hpp"

#include "Engine/Threads/AtomicQueue.hpp"

namespace PBE
{
	template<typename T_ValueType>
	class NetConnection
	{
	public:
		NetSystem<T_ValueType>* m_System;
		asio::ip::tcp::socket m_Socket;

		NetMessage<T_ValueType> m_ConstructingMessage;
		ThreadSafeQueue<NetMessage<T_ValueType>> m_MessagesOut;
		ThreadSafeQueue<NetMessage<T_ValueType>> m_MessagesIn;
	public:
		NetConnection(NetSystem<T_ValueType>* system, asio::ip::tcp::socket&& socket)
			: m_System(system)
			, m_Socket(std::move(socket))
		{
		}

		uint32_t GetId() const {
			return m_Id;
		}


		void Send(NetMessage<T_ValueType> const& message)
		{
			asio::post(m_System->m_Context,
				[this, message]()
				{
					bool writingMessage = !m_MessagesOut.Empty();
					m_MessagesOut.PushBack(message);
					if (!writingMessage)
					{
						WriteHeader();
					}
				}
			);
		}

		void ConnectToServer(asio::ip::tcp::resolver::results_type const& endpoints)
		{
			if (m_System->m_Mode == NetMode::SERVER)
			{
				return;
			}

			std::error_code ec;
			asio::connect(m_Socket, endpoints, ec);

			if (!ec)
			{
				ReadHeader();
			}
		}

		void ConnectToClient(uint32_t uid = 0)
		{
			if (m_System->m_Mode == NetMode::CLIENT)
			{
				return;
			}
			if (m_Socket.is_open())
			{
				m_Id = uid;
				ReadHeader();
			}
		}

		void Disconnect()
		{
			if (IsConnected())
			{
				asio::post(m_System->m_Context,
					[this]()
					{
						m_Socket.close();
					}
				);
			}
		}

		bool IsConnected() const
		{
			return m_Socket.is_open();
		}

	protected:
		uint32_t m_Id;
	private:
		void ReadHeader()
		{
			asio::async_read(m_Socket, asio::buffer(&m_ConstructingMessage.m_Header, sizeof(NetMessageHeader<T_ValueType>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_ConstructingMessage.m_Header.m_Size > 0)
						{
							m_ConstructingMessage.m_Body.resize(m_ConstructingMessage.m_Header.m_Size);
							ReadBody();
						}
						else
						{
							AddToIncomingMessageQueue();
						}
					}
					else
					{
						m_Socket.close();
					}
				}
			);
		}

		void ReadBody()
		{
			asio::async_read(m_Socket, asio::buffer(m_ConstructingMessage.m_Body.data(), m_ConstructingMessage.m_Header.m_Size),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						AddToIncomingMessageQueue();
					}
					else
					{
						m_Socket.close();
					}
				}
			);
		}

		void WriteHeader()
		{
			// Cycle the write operations until the message queue is empty
			asio::async_write(m_Socket, asio::buffer(&m_MessagesOut.Front().m_Header, sizeof(NetMessageHeader<T_ValueType>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_MessagesOut.Front().m_Header.m_Size > 0)
						{
							WriteBody();
						}
						else
						{
							m_MessagesOut.PopFront();
							if (!m_MessagesOut.Empty())
							{
								WriteHeader();
							}
						}
					}
					else
					{
						m_Socket.close();
					}
				}
			);
		}

		void WriteBody()
		{
			asio::async_write(m_Socket, asio::buffer(m_MessagesOut.Front().m_Body.data(), m_MessagesOut.Front().m_Header.m_Size),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						m_MessagesOut.PopFront();
						if (!m_MessagesOut.Empty())
						{
							WriteHeader();
						}
					}
					else
					{
						m_Socket.close();
					}
				}
			);
		}

		void AddToIncomingMessageQueue()
		{
			if (m_System->m_Mode == NetMode::SERVER)
			{
				m_MessagesIn.PushBack(m_ConstructingMessage);
			}
			else
			{
				m_MessagesIn.PushBack(m_ConstructingMessage);
			}
			ReadHeader();
		}
	};
}