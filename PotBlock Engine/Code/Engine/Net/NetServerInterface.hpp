#pragma once

#include "Engine/Net/NetCommon.hpp"
#include "Engine/Net/NetSystem.hpp"
#include "Engine/Net/NetConnection.hpp"
#include "Engine/Threads/AtomicQueue.hpp"

#include <iostream>
#include <vector>

namespace PBE
{
	template <typename T_ValueType>
	class NetServerInterface : public NetSystem<T_ValueType>
	{
	public:
		std::vector<NetConnection<T_ValueType>*> m_ClientConnections;
		asio::ip::tcp::acceptor m_Acceptor;
	public:
		NetServerInterface(uint16_t port)
			: m_Acceptor(this->m_Context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{
			this->m_Mode = NetMode::SERVER;
		}

		~NetServerInterface()
		{
			Stop();
		}

		bool RemoveClientConnection(int clientId)
		{
			if (m_ClientConnections[clientId] != nullptr)
			{
				delete m_ClientConnections[clientId];
				m_ClientConnections[clientId] = nullptr;
				return true;
			}
			return false;
		}
		int AddClientConnection(NetConnection<T_ValueType>* client)
		{
			for (int i = 0; i < m_ClientConnections.size(); ++i)
			{
				if (m_ClientConnections[i] == nullptr)
				{
					m_ClientConnections[i] = client;
					return i;
				}
			}
			m_ClientConnections.push_back(client);
			return (int)(m_ClientConnections.size() - 1);
		}

		bool Start()
		{
			try
			{
				WaitForClientConnection();
				this->m_ContextThread = std::thread([this]() { this->m_Context.run(); });
			}
			catch (std::exception& e)
			{
				std::cerr << "[SERVER] Exception: " << e.what() << "\n";
				return false;
			}
			std::cout << "[SERVER] Started!\n";
			return true;
		}
		void Stop()
		{
			this->m_Context.stop();
			if (this->m_ContextThread.joinable())
			{
				this->m_ContextThread.join();
			}

			for (int i = 0; i < m_ClientConnections.size(); ++i)
			{
				if (m_ClientConnections[i])
				{
					m_ClientConnections[i]->Disconnect();
					delete m_ClientConnections[i];
					m_ClientConnections[i] = nullptr;
				}
			}

			std::cout << "[SERVER] Stopped!\n";
		}
		void Update()
		{
			for (int i = 0; i < m_ClientConnections.size(); ++i)
			{
				if (m_ClientConnections[i])
				{
					if (m_ClientConnections[i]->IsConnected())
					{
						while (!m_ClientConnections[i]->m_MessagesIn.Empty())
						{
							NetMessage<T_ValueType> message = m_ClientConnections[i]->m_MessagesIn.PopFront();
							OnMessage(*m_ClientConnections[i], message);
						};
					}
				}
			}
		}

		void WaitForClientConnection()
		{
			m_Acceptor.async_accept(
				[this](std::error_code ec, asio::ip::tcp::socket socket)
				{
					if (!ec)
					{
						std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";
						NetConnection<T_ValueType>* newClient = new NetConnection<T_ValueType>(this, std::move(socket));
						if (OnClientConnect(*newClient))
						{
							int clientId = AddClientConnection(newClient);
							newClient->ConnectToClient(clientId);
							std::cout << "[" << newClient->GetId() << "] Connection Approved\n";
						}
						else
						{
							std::cout << "[-----] Connection Denied\n";
							delete newClient;
						}
					}
					else
					{
						std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
					}
					WaitForClientConnection();
				}
			);
		}

		void MessageClient(NetConnection<T_ValueType>& client, NetMessage<T_ValueType> const& message)
		{
			if (client.IsConnected())
			{
				client.Send(message);
			}
			else
			{
				OnClientDisconnect(client);
				RemoveClientConnection(client.GetId());
			}
		}
		void MessageClient(uint32_t clientId, NetMessage<T_ValueType> const& message)
		{
			if (m_ClientConnections[clientId] != nullptr)
			{
				MessageClient(*m_ClientConnections[clientId], message);
			}
		}

		void MessageAllClients(NetMessage<T_ValueType> const& message, NetConnection<T_ValueType>* ignoreClient = nullptr)
		{
			for (int i = 0; i < m_ClientConnections.size(); ++i)
			{
				if (m_ClientConnections[i] != nullptr)
				{
					MessageClient(*m_ClientConnections[i], message);
				}
			}
		}
		void MessageAllClients(NetMessage<T_ValueType> const& message, uint32_t ignoreCount, uint32_t* ignoreClientIds)
		{
			for (int i = 0; i < m_ClientConnections.size(); ++i)
			{
				if (m_ClientConnections[i] != nullptr)
				{
					bool ignore = false;
					for (int j = 0; j < ignoreCount; ++j)
					{
						if (m_ClientConnections[i]->GetId() == ignoreClientIds[j])
						{
							ignore = true;
							goto foundIgnore;
						}
					}

				foundIgnore: 
					if (!ignore)
					{
						MessageClient(*m_ClientConnections[i], message);
					}
				}
			}
		}
	protected:
		virtual bool OnClientConnect(NetConnection<T_ValueType>& client)
		{
			return true;
		}

		virtual void OnClientDisconnect(NetConnection<T_ValueType>& client)
		{
		}

		virtual void OnMessage(NetConnection<T_ValueType>& client, NetMessage<T_ValueType>& message) = 0;
	};
}