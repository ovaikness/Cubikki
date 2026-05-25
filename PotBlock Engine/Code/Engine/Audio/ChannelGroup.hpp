#pragma once

#include "fmod.hpp"

namespace PBE
{
	class ChannelGroup
	{
		friend class AudioSystem;
	public:
		~ChannelGroup();
	protected:
		ChannelGroup(FMOD::ChannelGroup* channelgroup);
		FMOD::ChannelGroup* m_channelgroup;
	};
}