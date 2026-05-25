#include "RemoteEventUtils.hpp"
#include "CubikkiClient.hpp"
#include "CubikkiServer.hpp"
#include "CubikkiMessage.hpp"

#include "Cubikki/Common.hpp"

std::unordered_map<std::string, PBE::EventDispatcher<uint32_t, PBE::NamedProperties&>> g_RemoteEvents;

void ExecuteRemoteEventLocally(std::string_view eventName, uint32_t id, PBE::NamedProperties& props)
{
	auto itr = g_RemoteEvents.find(std::string(eventName));
	if (itr != g_RemoteEvents.end())
	{
		itr->second.Invoke(id, props);
	}
}

PBE::SubscriberHandle<uint32_t,PBE::NamedProperties&> SubscribeToRemoteEvent(std::string_view eventName, PBE::EventDispatcher<uint32_t, PBE::NamedProperties&>::Delegate delegate)
{
	return g_RemoteEvents[std::string(eventName)].AddSubscriber(delegate);
}

void UnsubscribeFromRemoteEvent(std::string_view eventName, PBE::SubscriberHandle<uint32_t, PBE::NamedProperties&> handle)
{
	g_RemoteEvents[std::string(eventName)].RemoveSubscriber(handle);
}

void SendRemoteEventToServer(std::string_view eventName, PBE::NamedProperties& properties)
{
	PBE::NetMessage<CubikkiMessageType> msg;
	msg.m_Header.m_ID = CubikkiMessageType::RemoteEvent;

	PBE::GrowBuffer outBuffer;
	outBuffer.WriteString(eventName);
	properties.WriteToGrowBuffer(outBuffer);

	msg.m_Body = outBuffer.m_Buffer;
	msg.m_Header.m_Size = (uint32_t)msg.m_Body.size();

	g_Client->Send(msg);
}

void SendRemoteEventToAllClients(std::string_view eventName, PBE::NamedProperties& properties)
{
	PBE::NetMessage<CubikkiMessageType> msg;
	msg.m_Header.m_ID = CubikkiMessageType::RemoteEvent;
	PBE::GrowBuffer outBuffer;
	outBuffer.WriteString(eventName);
	properties.WriteToGrowBuffer(outBuffer);
	msg.m_Body = outBuffer.m_Buffer;
	msg.m_Header.m_Size = (uint32_t)msg.m_Body.size();
	g_Server->MessageAllClients(msg);
}

void SendRemoteEventToClients(std::string_view eventName, PBE::NamedProperties& properties, std::vector<uint32_t> clientIds)
{
	PBE::NetMessage<CubikkiMessageType> msg;
	msg.m_Header.m_ID = CubikkiMessageType::RemoteEvent;

	PBE::GrowBuffer outBuffer;
	outBuffer.WriteString(eventName);
	properties.WriteToGrowBuffer(outBuffer);

	msg.m_Body = outBuffer.m_Buffer;
	msg.m_Header.m_Size = (uint32_t)msg.m_Body.size();

	for (uint32_t id : clientIds)
	{
		g_Server->MessageClient(id,msg);
	}
}
