#include "Engine/Audio/AudioListener.hpp"

const PBE::Vec3 fwd = PBE::Vec3{ 0.0f, 0.0f, 1.0f };
const PBE::Vec3 up = PBE::Vec3{ 0.0f, -1.0f, 0.0f };

PBE::AudioListener::~AudioListener()
{
	if (m_next)
	{
		m_next->m_listenerIndex = m_listenerIndex;
	}
	if (m_prev)
	{
		m_prev->m_next = m_next;
	}
}

void PBE::AudioListener::SetLocalPosition(Vec3 const& pos)
{
	m_config.m_position = pos;
	m_system->set3DListenerAttributes(m_listenerIndex, (FMOD_VECTOR*)&pos, (FMOD_VECTOR*)&m_config.m_velocity, (FMOD_VECTOR*)&fwd, (FMOD_VECTOR*)&up);

}

void PBE::AudioListener::SetVelocity(Vec3 const& vel)
{
	m_config.m_velocity = vel;
	m_system->set3DListenerAttributes(m_listenerIndex, (FMOD_VECTOR*)&m_config.m_position, (FMOD_VECTOR*)&vel, (FMOD_VECTOR*)&fwd, (FMOD_VECTOR*)&up);
}

PBE::AudioListener::AudioListener(FMOD::System* system, AudioListener* prev, int index)
	: m_system(system), m_prev(prev), m_listenerIndex(index)
{
	m_system->set3DNumListeners(index + 1);
	if (prev)
	{
		prev->m_next = this;
	}
}
