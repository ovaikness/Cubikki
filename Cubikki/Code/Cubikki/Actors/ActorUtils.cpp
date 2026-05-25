#include "Cubikki/Actors/ActorUtils.hpp"
#include "Cubikki/Actors/ActorSystem.hpp"
#include "nlohmann/json.hpp"

std::map<std::string, ActorType> g_ActorTypesByName;
std::vector<std::function<Actor* ( PBE::NamedProperties const& streamSpawnInfo)>> g_ActorTypeConstructors;

void RegisterLuaActorsFromJson(std::filesystem::path jsonFilePath)
{
	std::vector<std::filesystem::path> jsonFiles;
	if (std::filesystem::exists(jsonFilePath) && std::filesystem::is_regular_file(jsonFilePath))
	{
		jsonFiles.push_back(jsonFilePath);
	}
	else if (std::filesystem::exists(jsonFilePath) && std::filesystem::is_directory(jsonFilePath))
	{
		for (auto const& entry : std::filesystem::directory_iterator(jsonFilePath))
		{
			if (entry.path().extension() == ".json")
			{
				jsonFiles.push_back(entry.path());
			}
		}
	}
	else
	{
		return; // No valid JSON files found
	}

	for (auto const& jsonFile : jsonFiles)
	{
		std::ifstream file(jsonFile);
		if (!file.is_open())
		{
			continue; // Skip if file cannot be opened
		}

		nlohmann::json jsonData;
		file >> jsonData;
		if (!jsonData.is_array())
		{
			continue; // Skip if JSON data is not an array
		}

		for (auto const& actorJson : jsonData)
		{
			if (!actorJson.contains("name") || !actorJson.contains("cpp-class") || !actorJson.contains("lua-scripts"))
			{
				continue; // Skip if required fields are missing
			}
			std::string name = actorJson["name"].get<std::string>();
			std::string baseActorName = actorJson["cpp-class"].get<std::string>();
			std::vector<std::string> luaScriptsToLoad = actorJson["lua-scripts"].get<std::vector<std::string>>();
			RegisterLuaActor(name, baseActorName, luaScriptsToLoad);
		}
	}
}

ActorType GetActorTypeFromName(std::string_view name)
{
	std::string nameStr = name.data();
	std::map<std::string, ActorType>::iterator found = g_ActorTypesByName.find(nameStr);

	if (found == g_ActorTypesByName.end())
	{
		return INVALID_ACTOR_TYPE;
	}
	else
	{
		return found->second;
	}

}

void RegisterActor(std::string_view name, std::function<Actor* (PBE::NamedProperties const& properties)> spawnCallback)
{
	std::string nameStr = name.data();
	g_ActorTypeConstructors.push_back(spawnCallback);
	ActorType id = g_ActorTypeConstructors.size() - 1;
	g_ActorTypesByName[nameStr] = id;
}

void RegisterLuaActor(std::string_view name, std::string_view baseActorName, std::vector<std::string> luaScriptsToLoad)
{
	std::string nameStr = name.data();
	std::string baseActorNameStr = baseActorName.data();
	std::vector<std::string> luaScriptsToLoadStr = luaScriptsToLoad;
	std::vector<PBE::ScriptID> scriptIDs;
	for (std::string& scriptName : luaScriptsToLoadStr)
	{
		PBE::ScriptID id = g_Engine->m_pLuaSystem->GetScriptID(scriptName);
		scriptIDs.push_back(id);
	}
	RegisterActor(name, [nameStr, baseActorNameStr, scriptIDs]( PBE::NamedProperties const& properties)
		{
			ActorType baseType = GetActorTypeFromName(baseActorNameStr);
			if (baseType == INVALID_ACTOR_TYPE)
			{
				return (Actor*)nullptr;
			}

			Actor* actor = CreateActor(baseType, properties);
			if (!actor)
			{
				return (Actor*)nullptr;
			}

			actor->m_ActorType = GetActorTypeFromName(nameStr);
			actor->m_Environment = g_Engine->m_pLuaSystem->CreateEnvironment();
			actor->m_Environment["self"] = actor;
			actor->m_Environment["Actor"] = actor;
			//actor->m_Environment["SpawnInfo"] = properties;

			//actor->Startup(properties);

			for (PBE::ScriptID id : scriptIDs)
			{
				actor->BindLuaScriptToActor(g_Engine->m_pLuaSystem->GetScript(id));
			}

			return actor;
		});
}

Actor* CreateActor(ActorType id, PBE::NamedProperties const& properties)
{
	if (id == INVALID_ACTOR_TYPE)
	{
		return nullptr;
	}

	if (id > g_ActorTypeConstructors.size() - 1)
	{
		return nullptr;
	}

	Actor* actor = g_ActorTypeConstructors[id](properties);
	actor->m_ActorType = id;

	return actor;
}

