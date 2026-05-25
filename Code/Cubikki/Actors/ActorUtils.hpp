#pragma once

#include "Cubikki/Actors/Actor.hpp"

#include "Engine/Core/NamedProperties.hpp"

#include <filesystem>
#include <string>
#include <map>
#include <cinttypes>
#include <string_view>
#include <functional>

extern std::map<std::string, ActorType> g_ActorTypesByName;
extern std::vector<std::function<Actor* ( PBE::NamedProperties const& streamSpawnInfo)>> g_ActorTypeConstructors;

void RegisterLuaActorsFromJson(std::filesystem::path jsonFilePath);

ActorType GetActorTypeFromName(std::string_view name);

void RegisterActor(std::string_view name, std::function<Actor* (PBE::NamedProperties const& streamSpawnInfo)> spawnCallback);
void RegisterLuaActor(
	std::string_view name, 
	std::string_view baseActorName, 
	std::vector<std::string> luaScriptsToLoad
);

Actor* CreateActor(ActorType id, PBE::NamedProperties const& props);

//Shortcut for registering a C++ actor. Maintains class name consistency, so it's less error-prone and easier to refactor (For example ctrl-r will still maintain naming conventions).
#define REGISTER_CPP_ACTOR(name) RegisterActor(#name, [](PBE::NamedProperties const& props) -> Actor* { return (Actor*)(new name(props)); });