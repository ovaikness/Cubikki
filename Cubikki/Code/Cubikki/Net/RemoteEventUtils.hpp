#pragma once

#include "Engine/Core/GrowBuffer.hpp"
#include "Engine/Core/BufferReader.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EventDispatcher.hpp"

#include <string>
#include <string_view>
#include <unordered_map>

// <clientid, properties/args>
extern std::unordered_map<std::string, PBE::EventDispatcher<uint32_t,PBE::NamedProperties&>> g_RemoteEvents;

void ExecuteRemoteEventLocally(std::string_view eventName, uint32_t id, PBE::NamedProperties& properties);

PBE::SubscriberHandle<uint32_t,PBE::NamedProperties&> SubscribeToRemoteEvent(std::string_view eventName, PBE::EventDispatcher<uint32_t, PBE::NamedProperties&>::Delegate delegate);
void UnsubscribeFromRemoteEvent(std::string_view eventName, PBE::SubscriberHandle<uint32_t, PBE::NamedProperties&> delegate);

void SendRemoteEventToServer(std::string_view eventName, PBE::NamedProperties& properties);
void SendRemoteEventToAllClients(std::string_view eventName, PBE::NamedProperties& properties);
void SendRemoteEventToClients(std::string_view eventName, PBE::NamedProperties& properties, std::vector<uint32_t> clientIds);