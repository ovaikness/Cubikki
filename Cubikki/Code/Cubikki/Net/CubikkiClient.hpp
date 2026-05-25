#pragma once

#include "Engine/Net/NetClientInterface.hpp"
#include "Cubikki/Net/CubikkiMessage.hpp"
#include "Cubikki/Net/RemoteEventUtils.hpp"

#include <iostream>

class CubikkiClient : public PBE::NetClientInterface<CubikkiMessageType>
{
public:
	CubikkiClient();

protected:
	virtual void OnMessage(PBE::NetMessage<CubikkiMessageType>& msg) override;
};