#include "Cubikki/Actors/ActorClasses/APlayer.hpp"
#include "Cubikki/Actors/ActorClasses/AController.hpp"
#include "Cubikki/Actors/ActorClasses/APlayerController.hpp"
#include "Cubikki/Blocks/BlockWorld.hpp"
#include "Cubikki/Blocks/Physics/BlockPhysicsWorld.hpp"
#include "Cubikki/Blocks/Physics/BlockPhysicsBody.hpp"
#include "Cubikki/Game/Game.hpp"
#include "Cubikki/Common.hpp"

#include "imgui.h"

APlayer::APlayer(PBE::NamedProperties const& properties)
	: APawn(properties)
{
	m_IsNetActor = true;
	m_name = "FlyCam";

	m_Model = AddComponent(g_Game->m_SkeletalModelSystem->CreateComponentFromFilepath("Female Character Model", "Assets/Models/player_female_model.glb"));
	BlockPhysicsBodyCreateInfo info;
	info.m_Mass = 1.f;
	info.m_OBB = PBE::OBB3(PBE::Vec3(0.f), PBE::Vec3(1.f), PBE::Mat4::IDENTITY);
	info.m_Restitution = 0.5f;
	info.m_AngularDrag = 0.5f;
	info.m_Drag = 0.5f;
	info.m_Friction = 0.5f;
	info.m_HasGravity = true;
	info.m_OBB.SetCenter(PBE::Vec3(0.f, 0.f, 1.f));

	m_Collider = AddComponent(g_BlockWorld->m_PhysicsWorld->CreateComponent("Collider", info, g_BlockWorld->m_PhysicsWorld));
}

APlayer::~APlayer()
{
}

void APlayer::OnPossessed(ActorUID actor)
{
	APawn::OnPossessed(actor);

	if (AController* controller = GetController())
	{
		PBE::InputAction action;
		action.m_Type = PBE::INPUT_ACTION_VALUE_TYPE_VEC2;
		action.m_Name = "Move";
		action.m_Held = [this](PBE::InputActionValue const& value)
			{
				PBE::Vec2 axis = std::get<PBE::Vec2>(value);
				PBE::Vec3 move = m_Transform.GetAs<TransformComponent>()->GetForward() * axis.x + m_Transform.GetAs<TransformComponent>()->GetRight() * axis.y;
				move.z = 0.f;
				move.Normalize();

				PBE::Vec3 translation = move * 8.f * PBE::Clock::GetSystemClock()->GetDeltaSeconds();
				Translate(translation);
			};

		controller->m_ActionContext.BindAction("Move", action);

		action.m_Name = "Look";
		action.m_Held = [this](PBE::InputActionValue const& value)
			{
				PBE::Vec2 axis = std::get<PBE::Vec2>(value);
				PBE::EulerAngles rotation = PBE::EulerAngles(axis.x, -axis.y, 0.f);
				Rotate(rotation);
			};

		controller->m_ActionContext.BindAction("Look", action);

		action.m_Type = PBE::INPUT_ACTION_VALUE_TYPE_FLOAT;
		action.m_Name = "Elevate/Descend";
		action.m_Held = [this](PBE::InputActionValue const& value)
			{
				float elevation = std::get<float>(value);
				PBE::Vec3 translation = PBE::Vec3(0.f, 0.f, elevation) * 8.f * PBE::Clock::GetSystemClock()->GetDeltaSeconds();
				Translate(translation);
			};

		controller->m_ActionContext.BindAction("Elevate/Descend", action);

		action.m_Type = PBE::INPUT_ACTION_VALUE_TYPE_BOOL;
		action.m_Name = "PrimaryAction";
		action.m_Started = [this](PBE::InputActionValue const& value)
			{
				PBE::Vec3 position = m_Transform.GetAs<TransformComponent>()->GetPosition();
				PBE::Vec3 forward = m_Transform.GetAs<TransformComponent>()->GetForward();
				PBE::RaycastResult3D result = g_BlockWorld->Raycast(position, forward, 100.f);

				if (result.m_Hit)
				{
					Block block{ 0, m_blockIndex };
					g_BlockWorld->SetBlock(result.m_BlockPos + PBE::IntVec3(result.m_HitNormal), block);
				}
			};

		controller->m_ActionContext.BindAction("PrimaryAction", action);

		action.m_Type = PBE::INPUT_ACTION_VALUE_TYPE_BOOL;
		action.m_Name = "SecondaryAction";
		action.m_Started = [this](PBE::InputActionValue const& value)
			{
				PBE::Vec3 position = m_Transform.GetAs<TransformComponent>()->GetPosition();
				PBE::Vec3 forward = m_Transform.GetAs<TransformComponent>()->GetForward();
				PBE::RaycastResult3D result = g_BlockWorld->Raycast(position, forward, 100.f);

				if (result.m_Hit)
				{
					Block block{ 0, 0 };
					g_BlockWorld->SetBlock(result.m_BlockPos, block);
				}
			};

		controller->m_ActionContext.BindAction("SecondaryAction", action);

		action.m_Type = PBE::INPUT_ACTION_VALUE_TYPE_FLOAT;
		action.m_Name = "SelectBlock";
		action.m_Started = [this](PBE::InputActionValue const& value)
			{
				m_blockIndex = (int)std::get<float>(value);
			};

		controller->m_ActionContext.BindAction("SelectBlock", action);
	}
}

void APlayer::OnUnpossessed()
{
	APawn::OnUnpossessed();
}

void APlayer::BeginFrame()
{
	APawn::BeginFrame();
	TransformComponent* netTrans = m_NetTransform.GetAs<TransformComponent>();
	if (TransformComponent* transform = m_Transform.GetAs<TransformComponent>())
	{
		if (BlockPhysicsBodyComponent* collider = m_Collider.GetAs<BlockPhysicsBodyComponent>())
		{
			AController* controller = GetController();
			APlayerController* playerController = dynamic_cast<APlayerController*>(controller);
			if (!playerController)
			{
				return;
			}
			if (netTrans && g_BlockWorld->m_NetMode != BlockWorldNetMode::SERVER && playerController->m_ClientID != g_Client->GetID())
			{
				collider->SetPosition(netTrans->GetPosition());
				transform->SetRotation(netTrans->GetRotation());
			}
			transform->SetPosition(collider->m_Position);
		}
	}
}

void APlayer::Update()
{
	APawn::Update();
}

void APlayer::EndFrame()
{
	APawn::EndFrame();
	AController* controller = GetController();
	APlayerController* playerController = dynamic_cast<APlayerController*>(controller);

	bool visible = true;
	if (playerController)
	{
		if (playerController->m_ClientID == SERVER_CLIENT_ID && g_BlockWorld->m_NetMode == BlockWorldNetMode::SERVER)
		{
			visible = false;
		}
		if (playerController->m_ClientID == g_Client->GetID() && g_BlockWorld->m_NetMode != BlockWorldNetMode::SERVER)
		{
			visible = false;
		}
	}

	if (SkeletalModelComponent* model = dynamic_cast<SkeletalModelComponent*>(m_Model.Get()))
	{
		TransformComponent* transform = m_Transform.GetAs<TransformComponent>();
		model->m_Visible = visible;
		model->m_Transform = m_Transform;
	}
}

void APlayer::ImGui()
{
}

void APlayer::Translate(PBE::Vec3 const& translation)
{
	if (BlockPhysicsBodyComponent* collider = m_Collider.GetAs<BlockPhysicsBodyComponent>())
	{
		collider->SetPosition(collider->GetPosition() + translation);
	}
}

void APlayer::Rotate(PBE::EulerAngles const& rotation)
{
	if (TransformComponent* transform = m_Transform.GetAs<TransformComponent>())
	{
		transform->SetRotation(transform->GetRotation() + rotation);
	}
}

