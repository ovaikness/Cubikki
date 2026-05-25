#include "Cubikki/Net/CubikkiServer.hpp"
#include "Cubikki/Blocks/BlockWorld.hpp"
#include "Cubikki/Actors/ActorSystem.hpp"
#include "Cubikki/Actors/ActorClasses/APlayerController.hpp"

CubikkiServer::CubikkiServer(uint16_t port)
	: PBE::NetServerInterface<CubikkiMessageType>(port)
{
}

bool CubikkiServer::OnClientConnect(PBE::NetConnection<CubikkiMessageType>& client)
{
	PBE::NetMessage<CubikkiMessageType> msg;
	msg.m_Header.m_ID = CubikkiMessageType::ServerAccept;
	client.Send(msg);
	return true;
}

void CubikkiServer::OnClientDisconnect(PBE::NetConnection<CubikkiMessageType>& client)
{
	PBE::NetMessage<CubikkiMessageType> msg;
	msg.m_Header.m_ID = CubikkiMessageType::ServerDeny;
	PBE::NamedProperties props;
	m_ClientDisconnectEvent.Invoke(client, props);
	m_ClientInfos.erase(client.GetId());

	client.Send(msg);
}

void CubikkiServer::OnMessage(PBE::NetConnection<CubikkiMessageType>& client, PBE::NetMessage<CubikkiMessageType>& msg)
{
	switch (msg.m_Header.m_ID)
	{
		case CubikkiMessageType::ServerPing:
		{
			std::cout << "[" << client.GetId() << "]: Server Ping\n";
			client.Send(msg);
		}
		break;
		case CubikkiMessageType::ClientSetup:
		{
			PBE::BufferReader reader(msg.m_Body);
			PBE::NamedProperties clientInfo;
			clientInfo.LoadFromBufferReader(reader);
			m_ClientInfos[client.GetId()] = clientInfo;
			m_ClientSetupEvent.Invoke(client, m_ClientInfos[client.GetId()]);
			g_BlockWorld->m_ActorSystem->RemoteSpawnAllActorsToClient(client.GetId());

			// Now that the client is set up, we can spawn the player controller and fly cam
			PBE::NamedProperties props;
			props["Position"] = PBE::Vec3(0.f, 0.f, 0.f);
			ActorUID flyCam = g_BlockWorld->SpawnActor("AFlyCam", props);
			PBE::NamedProperties propsController;
			propsController["ClientID"] = (int)client.GetId();
			ActorUID controller = g_BlockWorld->SpawnActor("APlayerController", propsController);
			APlayerController* controllerActor = g_BlockWorld->GetActor<APlayerController>(controller);
			controllerActor->Possess(flyCam);

		}
		break;
		case CubikkiMessageType::RequestChunk:
		{
			PBE::BufferReader reader(msg.m_Body);
			PBE::IntVec3 chunkPos;
			reader.Read(chunkPos);
			BlockWorld* world = g_BlockWorld;
			if (world)
			{
				world->QueueSendChunkToClient(client, chunkPos);
			}

		}
		break;
		case CubikkiMessageType::SpawnActor:
		{
			ActorType type;
			msg >> type;
			PBE::BufferReader reader(msg.m_Body);
			PBE::NamedProperties properties;
			properties.LoadFromBufferReader(reader);
			g_BlockWorld->m_ActorSystem->SpawnActor(type, properties);
		}
		break;
		case CubikkiMessageType::RemoteEvent:
		{
			PBE::BufferReader reader(msg.m_Body);
			std::string eventName;
			reader.ReadString(eventName);
			PBE::NamedProperties properties;
			properties.LoadFromBufferReader(reader);
			ExecuteRemoteEventLocally(eventName, client.GetId(), properties);
		}
		break;
		case CubikkiMessageType::SetPlayerPosition:
		{
			std::string playerName;
			PBE::BufferReader reader(msg.m_Body);
			std::string name;
			PBE::IntVec3 pos;

			reader.ReadString(name);
			reader.Read(pos);

			g_BlockWorld->SetPlayerPosition(name, pos);
		}
		break;
		case CubikkiMessageType::SetBlock:
		{
			PBE::IntVec3 worldPos;
			Block block;

			PBE::BufferReader stream(msg.m_Body);
			stream.Read(block);
			stream.Read(worldPos);

			std::cout << "Set Block: " << block.m_BlockID << "\n";
			g_BlockWorld->SetBlockLocally(worldPos, block);

			MessageAllClients(msg, &client);
		}
		break;
	}
}