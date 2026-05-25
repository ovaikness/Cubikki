#include "Engine/Audio/SoundPlayback.hpp"

namespace PBE
{
	SoundPlayback::~SoundPlayback()
	{
		m_channel->stop();
	}

	void SoundPlayback::SetVelocity(Vec3 const& vel)
	{
		m_config.m_velocity = vel;
		m_channel->set3DAttributes((FMOD_VECTOR*)&m_config.m_position, (FMOD_VECTOR*)&m_config.m_velocity);
	}

	PBE::Vec3 SoundPlayback::GetVelocity() const
	{
		return m_config.m_velocity;
	}

	void SoundPlayback::SetLocalPosition(Vec3 const& pos)
	{
		m_config.m_position = pos;
		m_channel->set3DAttributes((FMOD_VECTOR*)&m_config.m_position, (FMOD_VECTOR*)&m_config.m_velocity);
	}

	void SoundPlayback::SetVolume(float volume)
	{
		m_config.m_volume = volume;
		m_channel->setVolume(m_config.m_volume);
	}

	float SoundPlayback::GetVolume() const
	{
		return m_config.m_volume;
	}

	void SoundPlayback::SetPitch(float pitch)
	{
		m_config.m_pitch = pitch;
		m_channel->setPitch(m_config.m_pitch);
	}

	float SoundPlayback::GetPitch() const
	{
		return m_config.m_pitch;
	}

	void SoundPlayback::SetLooping(bool looping)
	{
		m_config.m_looping = looping;
		m_channel->setMode(m_config.m_looping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
	}

	bool SoundPlayback::IsLooping() const
	{
		return m_config.m_looping;
	}

	void SoundPlayback::Pause()
	{
		m_config.m_paused = true;
		m_channel->setPaused(m_config.m_paused);
	}

	void SoundPlayback::Resume()
	{
		m_config.m_paused = false;
		m_channel->setPaused(m_config.m_paused);
	}

	bool SoundPlayback::IsPlaying() const
	{
		return !m_config.m_paused;
	}

	void SoundPlayback::UpdatePlaybackFromConfig()
	{
		m_channel->set3DAttributes((FMOD_VECTOR*)&m_config.m_position, (FMOD_VECTOR*)&m_config.m_velocity);
		m_channel->setVolume(m_config.m_volume);
		m_channel->setPitch(m_config.m_pitch);
		m_channel->setMode(m_config.m_looping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
		m_channel->setPaused(m_config.m_paused);
	}

	SoundPlayback::SoundPlayback(FMOD::Channel* channel, SoundPlaybackConfig const& config)
		: m_channel(channel)
		, m_config(config)
	{
		UpdatePlaybackFromConfig();
	}
}
