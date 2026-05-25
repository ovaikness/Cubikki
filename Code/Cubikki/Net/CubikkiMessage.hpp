#pragma once

#include <cstdint>

enum class CubikkiMessageType : uint32_t
{
	ClientSetup,
	ServerAccept,
	ServerDeny,
	ServerPing,
	ServerMessage,
	TextChat,
	RemoteEvent,
	RequestChunk,
	SendChunk,
	ChunkData,
	SetBlock,
	SpawnActor,
	DestroyActor,
	SyncProperties,
	SetPlayerPosition,
	SynchronizeTransforms,
	COUNT
};