#include "CubikkiClient.hpp"
#include "Cubikki/Blocks/BlockWorld.hpp"
#include "Cubikki/Actors/Component/TransformComponent.hpp"

CubikkiClient::CubikkiClient()
{
}

void CubikkiClient::OnMessage(PBE::NetMessage<CubikkiMessageType>& msg)
{
	switch (msg.m_Header.m_ID)
	{
		case CubikkiMessageType::ServerAccept:
		{
			std::cout << "Server Accepted Connection\n";
			PBE::NetMessage<CubikkiMessageType> msg;
			msg.m_Header.m_ID = CubikkiMessageType::ClientSetup;
			PBE::NamedProperties props;
			props["Name"] = "CubikkiClient";
			g_Client->Send(msg);
		}
		break;
		case CubikkiMessageType::ServerDeny:
		{
			std::cout << "Server Denied Connection\n";
		}
		break;
		case CubikkiMessageType::ServerPing:
		{
			std::cout << "Server Ping\n";
		}
		break;
		case CubikkiMessageType::ServerMessage:
		{
			std::cout << "Server Message\n";
		}
		break;
		case CubikkiMessageType::RemoteEvent:
		{
			PBE::BufferReader stream(msg.m_Body);
			std::string eventName;
			stream.ReadString(eventName);
			PBE::NamedProperties properties;
			properties.LoadFromBufferReader(stream);
			ExecuteRemoteEventLocally(eventName, SERVER_CLIENT_ID, properties);
		}
		break;
		case CubikkiMessageType::ChunkData:
		{
			g_BlockWorld->RecieveChunkFromServer(msg);
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
			g_BlockWorld->SetBlockLocally(worldPos,block);
		}
		break;
		case CubikkiMessageType::SpawnActor:
		{
			ActorType type;
			uint32_t netID;
			msg >> type;
			msg >> netID;
			PBE::BufferReader reader(msg.m_Body);
			PBE::NamedProperties properties;
			properties.LoadFromBufferReader(reader);
			g_BlockWorld->m_ActorSystem->SpawnActor(type, properties, false, netID);
		}
		break;
		case CubikkiMessageType::SyncProperties:
		{
			PBE::BufferReader reader(msg.m_Body);
			uint32_t numActors;
			reader.Read(numActors);
			for (uint32_t i = 0; i < numActors; ++i)
			{
				uint32_t netUID;
				reader.Read(netUID);
				PBE::NamedProperties properties;
				properties.LoadFromBufferReader(reader);
				auto itr = g_BlockWorld->m_ActorSystem->m_ActorsByNetUID.find(netUID);
				if (itr != g_BlockWorld->m_ActorSystem->m_ActorsByNetUID.end())
				{
					Actor* actor = itr->second;
					if (actor)
					{
						actor->m_NetProperties = properties;
					}
				}
			}
		}
		break;
		case CubikkiMessageType::DestroyActor:
		{
			uint32_t netID;
			msg >> netID;
			auto itr = g_BlockWorld->m_ActorSystem->m_ActorsByNetUID.find(netID);
			if (itr != g_BlockWorld->m_ActorSystem->m_ActorsByNetUID.end())
			{
				Actor* actor = itr->second;
				if (actor)
				{
					g_BlockWorld->m_ActorSystem->DestroyActor(actor->GetUID());
				}
			}

		}
		break;
		case CubikkiMessageType::SynchronizeTransforms:
		{
			PBE::BufferReader reader(msg.m_Body);
			while (!reader.IsEnd())
			{
				uint32_t netUID;
				reader.Read(&netUID, sizeof(uint32_t));

				std::string name;
				reader.ReadString(name);

				PBE::Vec3 position;
				PBE::EulerAngles rotation;
				PBE::Vec3 scale;
				reader.Read(&position, sizeof(PBE::Vec3));
				reader.Read(&rotation, sizeof(PBE::EulerAngles));
				reader.Read(&scale, sizeof(PBE::Vec3));

				Actor* actor = g_BlockWorld->m_ActorSystem->GetActorByNetUID(netUID);
				if (actor)
				{
					if (TransformComponent* transform = dynamic_cast<TransformComponent*>(actor->GetComponent(name)))
					{
						if (!transform->m_Replicates)
						{
							continue;
						}
						transform->SetPosition(position);
						transform->SetRotation(rotation);
						transform->SetScale(scale);
					}
				}

			}
		}
		break;
	}
}