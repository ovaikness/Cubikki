#include "Cubikki/Actors/ActorSystem.hpp"
#include "Cubikki/Actors/Actor.hpp"
#include "Cubikki/Net/RemoteEventUtils.hpp"
#include "Cubikki/Blocks/BlockWorld.hpp"

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/BufferReader.hpp"

#include "imgui.h"
#include "Cubikki/Game/Game.hpp"

ActorSystem::ActorSystem()
{
};

ActorSystem::~ActorSystem()
{
	Clear();
};

void ActorSystem::Startup()
{
};

PBE::Result ActorSystem::RegisterUsertypes(PBE::LuaSystem& system)
{
	return PBE::Result();
}

Actor* ActorSystem::GetActor(ActorUID uid) const
{
	if (!uid.IsValid())
	{
		return nullptr;
	}

	size_t index = uid.GetIndex();

	if (index >= m_Actors.size())
	{
		return nullptr;
	}

	Actor* actor = m_Actors[index];

	if (!actor)
	{
		return nullptr;
	}

	if (actor->GetUID() == uid)
	{
		return actor;
	}

	return nullptr;
}
Actor* ActorSystem::GetActorByNetUID(uint32_t netUID) const
{
	auto itr = m_ActorsByNetUID.find(netUID);
	if (itr != m_ActorsByNetUID.end())
	{
		return itr->second;
	}

	return nullptr;
}
;

ActorUID ActorSystem::SpawnActor(ActorType type, PBE::NamedProperties const& spawnInfo, bool isServer, uint32_t givenNetUID)
{
	Actor* actor = CreateActor(type, spawnInfo);
	ActorUID uid;
	uint32_t netUID;
	if (isServer)
	{
		netUID = m_NetUIDCounter++;
	}
	else
	{
		netUID = givenNetUID;
	}

	for (size_t i = 0; i < m_Actors.size(); ++i)
	{
		if (!m_Actors[i])
		{
			m_Actors[i] = actor;
			uid = ActorUID((uint16_t)m_SaltCounter++, (uint16_t)i);
			actor->m_UID = uid;
			actor->m_NetUID = netUID;
			actor->m_System = this;
			m_ActorSpawnedEvent.Invoke(actor);
			m_ActorsByNetUID[netUID] = actor;
			if (isServer && actor->m_IsNetActor)
			{
				PBE::NetMessage<CubikkiMessageType> msg;
				msg.m_Body = spawnInfo.GetBuffer();
				msg << netUID;
				msg << type;
				msg.m_Header.m_ID = CubikkiMessageType::SpawnActor;
				g_Server->MessageAllClients(msg);
			}
			return uid;
		}
	}

	m_Actors.push_back(actor);
	m_ActorsByNetUID[netUID] = actor;
	uid = ActorUID((uint16_t)m_SaltCounter++, (uint16_t)(m_Actors.size() - 1));
	actor->m_UID = uid;
	actor->m_NetUID = netUID;
	actor->m_System = this;
	m_ActorSpawnedEvent.Invoke(actor);
	if (isServer && actor->m_IsNetActor)
	{
		PBE::NetMessage<CubikkiMessageType> msg;
		msg.m_Body = spawnInfo.GetBuffer();
		msg << netUID;
		msg << type;
		msg.m_Header.m_ID = CubikkiMessageType::SpawnActor;
		g_Server->MessageAllClients(msg);
	}
	return uid;
}

void ActorSystem::RemoteSpawnAllActorsToClient(uint32_t clientID)
{
	if (g_BlockWorld->m_NetMode == BlockWorldNetMode::SERVER)
	{
		for (auto& pair : m_ActorsByNetUID)
		{
			Actor* actor = pair.second;
			if (actor && actor->m_IsNetActor)
			{
				PBE::NetMessage<CubikkiMessageType> msg;
				msg.m_Header.m_ID = CubikkiMessageType::SpawnActor;
				msg.m_Body = actor->m_NetProperties.GetBuffer();
				msg << actor->m_NetUID;
				msg << actor->m_ActorType;
				g_Server->MessageClient(clientID, msg);
			}
		}
	}
}

void ActorSystem::RemoteSpawnActor(ActorType type, PBE::NamedProperties const& spawnInfo)
{
	if (g_Client->IsConnected())
	{
		PBE::NetMessage<CubikkiMessageType> msg;
		msg.m_Body = spawnInfo.GetBuffer();
		msg.m_Header.m_ID = CubikkiMessageType::SpawnActor;
		msg << type;
		g_Client->Send(msg);
	}
}
;

void ActorSystem::DestroyActor(ActorUID uid)
{
	Actor* actor = GetActor(uid);
	if (!actor)
	{
		return;
	}
	if (g_BlockWorld->m_NetMode == BlockWorldNetMode::SERVER)
	{
		PBE::NetMessage<CubikkiMessageType> msg;
		msg.m_Header.m_ID = CubikkiMessageType::DestroyActor;
		msg << actor->m_NetUID;
		g_Server->MessageAllClients(msg);
	}
	m_ActorsByNetUID.erase(actor->m_NetUID);
	actor->m_AwaitingDestroy = true;
	m_ActorDestroyedEvent.Invoke(actor);
};

void ActorSystem::BeginFrame()
{
	for (int actorIndex = 0; actorIndex < m_Actors.size(); ++actorIndex)
	{
		Actor* actor = m_Actors[actorIndex];
		if (actor && actor->m_AwaitingCreate)
		{
			actor->m_AwaitingCreate = false;
			actor->Create();
			m_ActorSpawnedEvent.Invoke(actor);
		}
	}

	for (int actorIndex = 0; actorIndex < m_Actors.size(); ++actorIndex)
	{
		Actor* actor = m_Actors[actorIndex];
		if (actor)
		{
			actor->BeginFrame();
		}
	}
};

void ActorSystem::Update()
{
	for (int actorIndex = 0; actorIndex < m_Actors.size(); ++ actorIndex)
	{
		Actor* actor = m_Actors[actorIndex];
		if (actor)
		{
			actor->Update();
		}
	}
};

void ActorSystem::EndFrame()
{
	
	for (int actorIndex = 0; actorIndex < m_Actors.size(); ++actorIndex)
	{
		Actor* actor = m_Actors[actorIndex];
		if (actor)
		{
			actor->EndFrame();
		}
	}

	DestroyAwaitingActors();

	if (!g_ShowDebugInfo)
	{
		return;
	}

	ImGui::Begin("Actors in World");
	for (auto actor : m_Actors)
	{
		if (actor)
		{
			std::string destroyButtonID = PBE::Stringf("[X]##%i", actor->GetIndex());

			if (ImGui::Button(destroyButtonID.c_str()))
			{
				DestroyActor(actor->GetUID());
			}
			ImGui::SameLine();
			
			std::string label = PBE::Stringf("%s_%i", actor->m_name.c_str(), actor->m_UID.m_UID);
			if (ImGui::TreeNodeEx(label.c_str()))
			{
				if (ImGui::TreeNodeEx("Properties"))
				{
					actor->ImGui();
					for (std::pair<PBE::HCIString, PBE::Property> propertyPair : actor->m_NetProperties.m_Properties)
					{
						ImGui::Text(propertyPair.first.GetCasedString().c_str());
						PBE::Property& property = propertyPair.second;
						if (std::holds_alternative<float>(property))
						{
							float value = std::get<float>(property);
							if (ImGui::DragFloat(propertyPair.first.GetCasedString().c_str(), &value))
							{
								actor->m_NetProperties[propertyPair.first] = value;
							}
						}
						else if (std::holds_alternative<int>(property))
						{
							int value = std::get<int>(property);
							if (ImGui::DragInt(propertyPair.first.GetCasedString().c_str(), &value))
							{
								actor->m_NetProperties[propertyPair.first] = value;
							}
						}
						else if (std::holds_alternative<std::string>(property))
						{
							char buffer[256];
							std::string value = std::get<std::string>(property);
							std::memcpy(buffer, value.c_str(), value.size() + 1);  // Copy string to buffer
							if (ImGui::InputText(propertyPair.first.GetCasedString().c_str(), buffer, 256))
							{
								actor->m_NetProperties[propertyPair.first] = std::string(buffer);
							}
						}
						else if (std::holds_alternative<PBE::Vec3>(property))
						{
							PBE::Vec3 value = std::get<PBE::Vec3>(property);
							if (ImGui::DragFloat3(propertyPair.first.GetCasedString().c_str(), &value.x))
							{
								actor->m_NetProperties[propertyPair.first] = value;
							}
						}
						else if (std::holds_alternative<PBE::Vec4>(property))
						{
							PBE::Vec4 value = std::get<PBE::Vec4>(property);
							if (ImGui::DragFloat4(propertyPair.first.GetCasedString().c_str(), &value.x))
							{
								actor->m_NetProperties[propertyPair.first] = value;
							}
						}
						else if (std::holds_alternative<PBE::EulerAngles>(property))
						{
							PBE::EulerAngles value = std::get<PBE::EulerAngles>(property);
							if (ImGui::DragFloat3(propertyPair.first.GetCasedString().c_str(), &value.m_Yaw))
							{
								actor->m_NetProperties[propertyPair.first] = value;
							}
						}
					}
					ImGui::TreePop();
				}

				//Lua specific stuff
				if (actor->m_Environment)
				{
					for (auto& pair : actor->m_Environment)
					{
						if (pair.second.get_type() == sol::type::function)
						{
							if (ImGui::Button(pair.first.as<std::string>().c_str()))
							{
								sol::set_environment(actor->m_Environment, sol::protected_function(actor->m_Environment[pair.first]));
								sol::protected_function func = actor->m_Environment[pair.first];
								sol::protected_function_result result = func();

								if (!result.valid())
								{
									sol::error err = result;
									std::cerr << "Lua function error: " << err.what() << std::endl;
								}
							}
						}
						else if (pair.second.get_type() == sol::type::number)
						{
							float value = pair.second.as<float>();
							if (ImGui::DragFloat(pair.first.as<std::string>().c_str(), &value))
							{
								// Set the value in the Lua table explicitly
								actor->m_Environment[pair.first] = value;
							}
						}
						else if (pair.second.get_type() == sol::type::string)
						{
							char buffer[256];
							std::string value = pair.second.as<std::string>();
							std::memcpy(buffer, value.c_str(), value.size() + 1);  // Copy string to buffer

							if (ImGui::InputText(pair.first.as<std::string>().c_str(), buffer, 256))
							{
								actor->m_Environment[pair.first] = std::string(buffer);
							}
						}
						else if (pair.second.get_type() == sol::type::boolean)
						{
							bool value = pair.second.as<bool>();
							if (ImGui::Checkbox(pair.first.as<std::string>().c_str(), &value))
							{
								actor->m_Environment[pair.first] = value;
							}
						}
					}
				}

				for (std::pair<std::string const, UniqueComponentReference>& pair : actor->m_ComponentMap)
				{
					Component* component = pair.second.Get();
					if (!component)
					{
						continue;
					}
					if (ImGui::TreeNodeEx(component->m_Name.c_str()))
					{
						component->ImGui();
						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			}
		}
	}
	ImGui::End();

	ImGui::Begin("Spawn Actors");

	PBE::NamedProperties props;
	props["position"] = PBE::Vec3(0.0f, 0.0f, 0.0f);
	props["rotation"] = PBE::EulerAngles(0.0f, 0.0f, 0.0f);
	props["scale"] = PBE::Vec3(1.0f, 1.0f, 1.0f);
	if (g_Game)
	{
		PBE::Camera* camera = g_Game->m_Camera;
		if (camera)
		{
			PBE::Vec3 cameraPos = camera->m_Position;
			if (g_BlockWorld)
			{
				PBE::RaycastResult3D result = g_BlockWorld->Raycast(cameraPos, camera->GetForward(), 1000.f);
				if (result.m_Hit)
				{
					props["position"] = result.m_HitPoint;
				}
			}
		}
	}
	PBE::Vec3* position = std::get_if<PBE::Vec3>(&props["position"]);
	PBE::EulerAngles* rotation = std::get_if<PBE::EulerAngles>(&props["rotation"]);
	PBE::Vec3* scale = std::get_if<PBE::Vec3>(&props["scale"]);

	// Add input fields for properties
	ImGui::InputFloat3("Position", (float*)position);
	ImGui::InputFloat3("Rotation", (float*)rotation);
	ImGui::InputFloat3("Scale",	   (float*)scale);

	for (auto& pair : g_ActorTypesByName)
	{
		if (ImGui::Button(pair.first.c_str()))
		{
			if (g_BlockWorld->m_NetMode == BlockWorldNetMode::SERVER)
			{
				SpawnActor(pair.second, props);
			}
			else
			{
				RemoteSpawnActor(pair.second, props);
			}
		}
	}

	ImGui::End();

	if (g_BlockWorld->m_NetMode == BlockWorldNetMode::SERVER)
	{
		PBE::NetMessage<CubikkiMessageType> msg;
		msg.m_Header.m_ID = CubikkiMessageType::SyncProperties;
		PBE::GrowBuffer buffer;

		uint32_t numActors = 0;
		for (Actor* actor : m_Actors)
		{
			if (actor && actor->m_IsNetActor)
			{
				++numActors;
			}
		}

		buffer.Write(&numActors, sizeof(numActors));
		for (Actor* actor : m_Actors)
		{
			if (actor && actor->m_IsNetActor)
			{
				buffer.Write(&actor->m_NetUID, sizeof(actor->m_NetUID));
				actor->m_NetProperties.WriteToGrowBuffer(buffer);
			}
		}

		msg.m_Body = buffer.GetBuffer();
		msg.m_Header.m_Size = (uint32_t)msg.m_Body.size();
		g_Server->MessageAllClients(msg);
	}
}

void ActorSystem::Clear()
{
	for (Actor* actor : m_Actors)
	{
		if (actor)
		{
			DestroyActor(actor->GetUID());
		}
	}
	DestroyAwaitingActors();
};

void ActorSystem::DestroyAwaitingActors()
{
	for (int actorIndex = 0; actorIndex < m_Actors.size(); ++actorIndex)
	{
		Actor* actor = m_Actors[actorIndex];
		if (actor && actor->m_AwaitingDestroy)
		{
			actor->Destroy();
			delete actor;
			m_Actors[actorIndex] = nullptr;
		}
	}
};
