#pragma once

#include "ThirdParty/fmod.hpp"
#include "ThirdParty/fmod_studio.hpp"
#include "ThirdParty/fmod_errors.h"
#include "ThirdParty/fmod_common.h"

#include "Engine/Core/Result.hpp"
#include "Engine/Math/Vec3.hpp"

#include "Engine/Audio/Sound.hpp"
#include "Engine/Audio/SoundPlayback.hpp"
#include "Engine/Audio/ChannelGroup.hpp"
#include "Engine/Audio/AudioListener.hpp"

#include <vector>
#include <map>
#include <string>
#include <filesystem>

namespace PBE
{
	enum AudioMode
	{
		AUDIO_MODE_DEFAULT = 0,
		AUDIO_MODE_2D = FMOD_2D,
		AUDIO_MODE_3D = FMOD_3D,
	};
	struct AudioSystemCreateInfo
	{
	};

	class AudioSystem
	{
	public:
		AudioSystem(AudioSystemCreateInfo const& config);
		~AudioSystem();

		void Startup();
		void BeginFrame() {
		};
		void EndFrame();
		void Shutdown();

		Result SetNumListeners(int numListeners);
		int GetNumListeners() const;

		Result LoadBank(std::string_view bankName);
		Result UnloadBank(std::string_view bankName);

		Result SetListenerAttributes(int listenerIndex, Vec3 const& pos, Vec3 const& vel, Vec3 const& fwd, Vec3 const& up);

		Result CreateSound(std::filesystem::path path, AudioMode mode, Sound** out_sound);
		Result FreeSound(Sound* sound);

		Result PlaySound2D(Sound* sound, SoundPlaybackConfig const& config, std::string_view channel, SoundPlayback** out_playback = nullptr);
		Result PlaySound3D(Sound* sound, SoundPlaybackConfig const& config, std::string_view channel, SoundPlayback** out_playback = nullptr);

	protected:
		ChannelGroup* GetChannelGroup(std::string_view name) const;
		void CreateChannelGroup(std::string_view name);
		ChannelGroup* CreateOrGetChannelGroup(std::string_view name);

		AudioListener* m_lastListener = nullptr;
		FMOD::Studio::System* m_system = nullptr;
		FMOD::System* m_CoreSystem = nullptr;
		std::vector<SoundPlayback*> m_playbacks;

		std::vector<FMOD::Studio::EventInstance*> m_EventsAwaitingRelease;

		std::map <std::string, FMOD::Studio::Bank*> m_LoadedBanks;
		std::map <std::string, PBE::ChannelGroup*> m_channelGroups;
	};
}