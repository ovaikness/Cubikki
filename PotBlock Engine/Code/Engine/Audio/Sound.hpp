#pragma once
#include "fmod.hpp"

#include "Engine/Audio/Sound.hpp"

namespace PBE
{
	struct SoundConfig
	{
	};

	class Sound
	{
		friend class AudioSystem;
	public:
		~Sound();
	protected:
		Sound(FMOD::Sound* sound);
		FMOD::Sound* m_sound;
	};
}