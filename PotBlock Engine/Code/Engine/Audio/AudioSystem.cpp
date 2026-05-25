#include "Engine/Audio/AudioSystem.hpp"

//#TODO: Add error handling for FMOD functions
// This is just a foundation for the audio system. It is not complete and will need to be expanded upon.

PBE::AudioSystem::AudioSystem(AudioSystemCreateInfo const& /*config*/)
{
	Startup();
}

PBE::AudioSystem::~AudioSystem()
{

}

void PBE::AudioSystem::Startup()
{
	FMOD::Studio::System::create(&m_system);
	m_system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_3D_RIGHTHANDED, nullptr);
	m_system->getCoreSystem(&m_CoreSystem);
}

void PBE::AudioSystem::EndFrame()
{
	m_system->update();
	for (auto& playback : m_playbacks)
	{
		if (!playback->IsPlaying())
		{
			delete playback;
			playback = nullptr;
		}
	}
}

void PBE::AudioSystem::Shutdown()
{
	for (auto& channelGroup : m_channelGroups)
	{
		delete channelGroup.second;
	}
	m_system->release();
}

PBE::Result PBE::AudioSystem::SetNumListeners(int numListeners)
{
	FMOD_RESULT result = m_CoreSystem->set3DNumListeners(numListeners);
	if (result != FMOD_OK)
	{
		return RESULT_FAILURE;
	}
	return RESULT_SUCCESS;
}

int PBE::AudioSystem::GetNumListeners() const
{
	int numListeners = 0;
	m_CoreSystem->get3DNumListeners(&numListeners);
	return numListeners;
}

PBE::Result PBE::AudioSystem::LoadBank(std::string_view bankName)
{
	FMOD::Studio::Bank* bank = nullptr;
	FMOD_RESULT result = m_system->loadBankFile(bankName.data(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
	if (result != FMOD_OK)
	{
		return RESULT_FAILURE;
	}
	return RESULT_SUCCESS;
}

PBE::Result PBE::AudioSystem::UnloadBank(std::string_view bankName)
{
	FMOD::Studio::Bank* bank = nullptr;

	FMOD_RESULT result = m_system->getBank(bankName.data(), &bank);
	if (result != FMOD_OK)
	{
		return RESULT_FAILURE;
	}

	result = bank->unload();
	if (result != FMOD_OK)
	{
		return RESULT_FAILURE;
	}

	return RESULT_SUCCESS;
}

PBE::Result PBE::AudioSystem::SetListenerAttributes(int listenerIndex, Vec3 const& pos, Vec3 const& vel, Vec3 const& fwd, Vec3 const& up)
{
	FMOD_VECTOR fPos = { pos.x, pos.y, pos.z };
	FMOD_VECTOR fVel = { vel.x, vel.y, vel.z };
	FMOD_VECTOR fForward = { fwd.x, fwd.y, fwd.z };
	FMOD_VECTOR fUp = { up.x, up.y, up.z };
	FMOD_RESULT result = m_CoreSystem->set3DListenerAttributes(listenerIndex, &fPos, &fVel, &fForward, &fUp);
	if (result != FMOD_OK)
	{
		return RESULT_FAILURE;
	}
	return RESULT_SUCCESS;
}

PBE::Result PBE::AudioSystem::CreateSound(std::filesystem::path path, AudioMode mode, Sound** out_sound)
{
	if (out_sound == nullptr)
	{
		return RESULT_INVALID_PARAMETER;
	}
	FMOD::Sound* fSound = nullptr;
	FMOD_RESULT result = m_CoreSystem->createSound(path.string().c_str(), mode, nullptr, &fSound);
	if (result != FMOD_OK)
	{
		return RESULT_FAILURE;
	}

	Sound* sound = new Sound(fSound);
	*out_sound = sound;

	return RESULT_SUCCESS;
}

PBE::Result PBE::AudioSystem::FreeSound(Sound* sound)
{
	delete sound;
	return RESULT_SUCCESS;
}

PBE::Result PBE::AudioSystem::PlaySound2D(Sound* sound, SoundPlaybackConfig const& config, std::string_view channel, PBE::SoundPlayback** out_playback)
{
	if (sound == nullptr)
	{
		return RESULT_INVALID_PARAMETER;
	}
	FMOD::Channel* fChannel = nullptr;
	m_CoreSystem->playSound(sound->m_sound, CreateOrGetChannelGroup(channel)->m_channelgroup, false, &fChannel);
	SoundPlayback* playback = new SoundPlayback(fChannel,config);
	if (out_playback)
	{
		*out_playback = playback;
	}
	else
	{
		m_playbacks.push_back(playback);
	}
	return RESULT_SUCCESS;
}

PBE::Result PBE::AudioSystem::PlaySound3D(Sound* sound, SoundPlaybackConfig const& config, std::string_view channel, PBE::SoundPlayback** out_playback)
{
	if (sound == nullptr)
	{
		return RESULT_INVALID_PARAMETER;
	}
	FMOD::Channel* fChannel = nullptr;
	m_CoreSystem->playSound(sound->m_sound, CreateOrGetChannelGroup(channel)->m_channelgroup, false, &fChannel);
	SoundPlayback* playback = new SoundPlayback(fChannel, config);
	if (out_playback)
	{
		*out_playback = playback;
	}
	else
	{
		bool found = false;

		for (auto& pb : m_playbacks)
		{
			if (pb == nullptr)
			{
				pb = playback;
				found = true;
				break;
			}
		}

		if (!found)
		{
			m_playbacks.push_back(playback);
		}
	}
	return RESULT_SUCCESS;
}

PBE::ChannelGroup* PBE::AudioSystem::GetChannelGroup(std::string_view name) const
{
	return m_channelGroups.at(name.data());
}

void PBE::AudioSystem::CreateChannelGroup(std::string_view name)
{
	FMOD::ChannelGroup* fChannelGroup = nullptr;
	if (m_channelGroups.find(name.data()) == m_channelGroups.end())
	{
		m_CoreSystem->createChannelGroup(name.data(), &fChannelGroup);
		m_channelGroups[name.data()] = new ChannelGroup(fChannelGroup);
	}
}

PBE::ChannelGroup* PBE::AudioSystem::CreateOrGetChannelGroup(std::string_view name)
{
	CreateChannelGroup(name);
	return GetChannelGroup(name);
}

