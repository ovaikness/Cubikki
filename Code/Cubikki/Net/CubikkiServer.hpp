#pragma once

#include "Engine/Net/NetServerInterface.hpp"
#include "Engine/Core/EventDispatcher.hpp"

#include "Cubikki/Net/CubikkiMessage.hpp"
#include "Cubikki/Net/RemoteEventUtils.hpp"
#include <iostream>
#include <map>

class CubikkiServer : public PBE::NetServerInterface <CubikkiMessageType>
{
public:
	std::map<uint32_t, PBE::NamedProperties> m_ClientInfos;
	PBE::EventDispatcher<PBE::NetConnection<CubikkiMessageType>&, PBE::NamedProperties&> m_ClientSetupEvent;
	PBE::EventDispatcher<PBE::NetConnection<CubikkiMessageType>&, PBE::NamedProperties&> m_ClientDisconnectEvent;
public:
	CubikkiServer(uint16_t port);

protected:
	virtual bool OnClientConnect(PBE::NetConnection<CubikkiMessageType>& client) override;

	virtual void OnClientDisconnect(PBE::NetConnection<CubikkiMessageType>& client) override;

	virtual void OnMessage(PBE::NetConnection<CubikkiMessageType>& client, PBE::NetMessage<CubikkiMessageType>& msg) override;
};