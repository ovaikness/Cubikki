#include "Cubikki/Actors/Component/TransformComponent.hpp"
#include "Cubikki/Actors/Actor.hpp"

#include "Cubikki/Blocks/BlockWorld.hpp"

#include "imgui.h"

TransformComponent::TransformComponent(std::string_view name)
	: Component(name)
{
}

TransformComponent::TransformComponent(std::string_view name, PBE::Vec3 position, PBE::EulerAngles rotation, PBE::Vec3 scale)
	: Component(name)
{
	m_Transform = PBE::Mat4::CreateTRS(position, rotation, scale);
	m_Rotation = rotation;
	m_Scale = scale;
	m_Position = position;
}

PBE::Vec3 TransformComponent::GetPosition()
{
	return m_Position;
}

PBE::EulerAngles TransformComponent::GetRotation()
{
	return m_Rotation;
}

PBE::Vec3 TransformComponent::GetScale()
{
	return m_Scale;
}

PBE::Vec3 TransformComponent::GetForward() const
{
	return m_Transform.GetIBasis3D().GetNormalized();
}

PBE::Vec3 TransformComponent::GetRight() const
{
	return -m_Transform.GetJBasis3D().GetNormalized();
}

PBE::Vec3 TransformComponent::GetUp() const
{
	return m_Transform.GetKBasis3D().GetNormalized();
}

void TransformComponent::SetPosition(PBE::Vec3 position)
{
	m_Position = position;
	m_Transform.SetTranslation3D(position);
}

void TransformComponent::SetRotation(PBE::EulerAngles rotation)
{
	m_Rotation = rotation;
	m_Transform = PBE::Mat4::CreateTRS(m_Position, m_Rotation, m_Scale);
}

void TransformComponent::SetScale(PBE::Vec3 scale)
{
	m_Scale = scale;
	m_Transform = PBE::Mat4::CreateTRS(m_Position, m_Rotation, m_Scale);
}

PBE::Mat4 TransformComponent::GetTransform()
{
	return m_Transform;
}

void TransformComponent::SetMatrix(PBE::Mat4 transform)
{
	m_Transform = transform;
}

void TransformComponent::ImGui()
{
	if (ImGui::DragFloat3("Position", &m_Position.x, 0.1f))
	{
		m_Transform.SetTranslation3D(m_Position);
	}

	if (ImGui::DragFloat3("Rotation", &m_Rotation.m_Yaw, 0.1f))
	{
		SetRotation(m_Rotation);
	}

	if (ImGui::DragFloat3("Scale", &m_Scale.x, 0.1f))
	{
		SetScale(m_Scale);
	}
}

void TransformSystem::Update()
{
	if (!g_BlockWorld)
	{
		return;
	}

	for (auto& component : m_Components)
	{
		if (component.m_InUse)
		{
			if (component.m_Component.m_Replicates)
			{
				if (Actor* actor = component.m_Component.GetOwner())
				{
					PBE::NetMessage<CubikkiMessageType> msg;
					msg.m_Header.m_ID = CubikkiMessageType::SynchronizeTransforms;

					PBE::GrowBuffer buffer;
					if (g_BlockWorld->m_NetMode == BlockWorldNetMode::SERVER)
					{
						buffer.Write(&actor->m_NetUID, sizeof(actor->m_NetUID));
						buffer.WriteString(component.m_Component.m_Name);
						PBE::Vec3 position = component.m_Component.GetPosition();
						PBE::EulerAngles rotation = component.m_Component.GetRotation();
						PBE::Vec3 scale = component.m_Component.GetScale();
						buffer.Write(&position, sizeof(PBE::Vec3));
						buffer.Write(&rotation, sizeof(PBE::EulerAngles));
						buffer.Write(&scale,    sizeof(PBE::Vec3));
					}

					msg.m_Body = buffer.GetBuffer();
					msg.m_Header.m_Size = (uint32_t)msg.m_Body.size();

					g_Server->MessageAllClients(msg);
				}
			}
		}
	}
}
