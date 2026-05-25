#include "Engine/Audio/ChannelGroup.hpp"

PBE::ChannelGroup::~ChannelGroup()
{
	m_channelgroup->release();
}

PBE::ChannelGroup::ChannelGroup(FMOD::ChannelGroup* channelgroup)
{
	m_channelgroup = channelgroup;
}
