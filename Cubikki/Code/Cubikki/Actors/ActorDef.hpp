#pragma once

#include "Engine/Scripting/LuaSystem.hpp"

#include "Cubikki/Actors/Actor.hpp"
#include "Cubikki/Net/CubikkiMessage.hpp"

struct ActorDef
{
public:
	ActorType m_Type;
	std::vector<PBE::ScriptID> m_Scripts;
};