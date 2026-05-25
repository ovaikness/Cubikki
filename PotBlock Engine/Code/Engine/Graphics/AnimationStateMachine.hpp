#pragma once

#include "Engine/Graphics/Animation.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Time/Clock.hpp"
namespace PBE
{
	struct AnimationState
	{
		~AnimationState()
		{
			delete m_Clock;
		}
		PBE::Clock* m_Clock{ nullptr };
		std::string m_AnimationName;
		float m_Weight{ 0.f };
	};

	struct AnimationStateTransition
	{
		std::vector<std::string> m_Conditions;
		std::string m_From;
		std::string m_To;
		float m_TransitionTime{ 0.f };
	};

	struct AnimationStateMachineCreateInfo
	{
		std::vector<AnimationStateTransition> m_Transitions;
		std::vector<AnimationState> m_States;
		Animator* m_Animator{ nullptr };
	};

	class AnimationStateMachine
	{
	public:
		AnimationStateMachineCreateInfo m_CreateInfo;
		std::map<std::string, bool> m_Conditions;
	public:
		AnimationStateMachine(AnimationStateMachineCreateInfo const& info);
		~AnimationStateMachine();
		void Update(float deltaTime);
	};
}