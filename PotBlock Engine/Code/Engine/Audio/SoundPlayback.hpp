#pragma once
#include "fmod.hpp"
#include "Engine/Math/Vec3.hpp"

namespace PBE
{
	struct SoundPlaybackConfig
	{
		Vec3 m_position = Vec3(0.0f);
		Vec3 m_velocity = Vec3(0.0f);
		float m_volume = 1.0f;
		float m_pitch = 1.0f;
		bool m_looping = false;
		bool m_paused = false;
	};
	class SoundPlayback
	{
		friend class AudioSystem;
	public:
		~SoundPlayback();

		void SetVelocity(Vec3 const& vel);
		Vec3 GetVelocity() const;

		void SetLocalPosition(Vec3 const& pos);
		Vec3 GetPosition() const { return m_config.m_position; }

		void SetVolume(float volume);
		float GetVolume() const;

		void SetPitch(float pitch);
		float GetPitch() const;

		void SetLooping(bool looping);
		bool IsLooping() const;

		void Pause();
		void Resume();

		bool IsPlaying() const;
	protected:
		void UpdatePlaybackFromConfig();
		SoundPlayback(FMOD::Channel* channel, SoundPlaybackConfig const& config);
		FMOD::Channel* m_channel;
		SoundPlaybackConfig m_config;
	};
}