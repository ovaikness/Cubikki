#pragma once
#include "Engine/Audio/AudioSystem.hpp"

namespace PBE
{
	class AudioEvent
	{
	private:
		friend class AudioSystem;
		FMOD::Studio::System* m_system = nullptr;
		FMOD::Studio::EventInstance* m_eventInstance = nullptr;
	public:
		AudioEvent(FMOD::Studio::System* system, FMOD::Studio::EventInstance* eventInstance)
			: m_system(system)
			, m_eventInstance(eventInstance)
		{
		}
		~AudioEvent()
		{
			m_eventInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
			m_eventInstance->release();
		}
		
		void SetParameterByName(const char* name, float value)
		{
			if (m_eventInstance)
			{
				m_eventInstance->setParameterByName(name, value);
			}
		}

		void Start()
		{
			m_eventInstance->start();
		}

		void Stop()
		{
			m_eventInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
		}
	};
}