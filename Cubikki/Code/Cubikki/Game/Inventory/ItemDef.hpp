#pragma once

#include "Cubikki/Actors/GameTag/GameTagCollection.hpp"
#include "Cubikki/Actors/Actor.hpp"

#include <string>
#include <vector>

class ItemDef
{
public:
	std::string m_Name;
	std::string m_Description;

	std::string m_IconPath;
	std::string m_ModelPath;

	int m_StackSize;

	std::function<void(Actor*)> m_Equipped;
	std::function<void(Actor*)> m_Dropped;
	std::function<void(Actor*)> m_Activated;
	std::function<void(Actor*)> m_Deactivated;


	GameTagCollection m_Tags;
};

extern std::vector<ItemDef> g_ItemDefinitions;