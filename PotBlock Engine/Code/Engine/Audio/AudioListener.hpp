#pragma once
#include "fmod.hpp"
#include "Engine/Math/Vec3.hpp"

namespace PBE
{
	struct AudioListenerConfig
	{
		Vec3 m_position;
		Vec3 m_velocity;
	};

	class AudioListener
	{
		friend class AudioSystem;
	public:
		~AudioListener();
		void SetLocalPosition(Vec3 const& pos);
		inline Vec3 GetPosition() const 
		{
			return m_config.m_position;
		}
		void SetVelocity(Vec3 const& vel);
		inline Vec3 GetVelocity() const 
		{ 
			return m_config.m_velocity; 
		}
	protected:
		AudioListener(FMOD::System* system, AudioListener* prev, int index);
		FMOD::System* m_system = nullptr;
		AudioListener* m_prev = nullptr;
		AudioListener* m_next = nullptr;
		int m_listenerIndex = 0;
		AudioListenerConfig m_config;
	};
}