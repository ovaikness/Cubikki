#include "Cubikki/Actors/ActorClasses/ACharacter.hpp"
#include "Cubikki/Game/Game.hpp"

ACharacter::ACharacter(PBE::NamedProperties const& properties)
	: APawn(properties)
{
	m_IsNetActor = true;

	m_WeirdTransform = AddComponent(g_Game->m_TransformSystem->CreateComponent());
	m_Model = AddComponent(g_Game->m_SkeletalModelSystem->CreateComponentFromFilepath("ProtoMesh", "Assets/Models/proto.glb"));
	m_WeirdModel = AddComponent(g_Game->m_SkeletalModelSystem->CreateComponentFromFilepath("WeirdMesh", "Assets/Models/box.glb"));

	if (SkeletalModelComponent* model = dynamic_cast<SkeletalModelComponent*>(m_Model.Get()))
	{
		model->m_Transform = m_Transform;
	}
	if (SkeletalModelComponent* model = dynamic_cast<SkeletalModelComponent*>(m_WeirdModel.Get()))
	{
		model->m_Transform = m_Transform;
	}
}

ACharacter::~ACharacter()
{

}

void ACharacter::Update()
{

	if (SkeletalModelComponent* model = dynamic_cast<SkeletalModelComponent*>(m_Model.Get()))
	{
		model->m_Transform = m_Transform;
		model->SetAnimationState("run", PBE::Clock::GetSystemClock()->GetTimeMilliseconds(), 1.f);

		if (TransformComponent* transform = dynamic_cast<TransformComponent*>(m_WeirdTransform.Get()))
		{
			PBE::Mat4 mat = model->m_Animator->GetBoneTransform("head");
			if (TransformComponent* transformRoot = dynamic_cast<TransformComponent *>(m_Transform.Get()))
			{
				transform->SetMatrix(mat.GetAppended(transformRoot->m_Transform));
			}
		}
	}
}

void ACharacter::MoveForward(float value)
{

}

void ACharacter::MoveRight(float value)
{

}

