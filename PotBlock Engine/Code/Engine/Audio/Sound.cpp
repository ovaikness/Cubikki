#include "Engine/Audio/Sound.hpp"

namespace PBE
{
	Sound::~Sound()
	{
		m_sound->release();
	}

	Sound::Sound(FMOD::Sound* sound)
		: m_sound(sound)
	{

	}
}