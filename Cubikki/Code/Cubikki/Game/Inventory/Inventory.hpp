#pragma once
#include "Cubikki/Game/Inventory/ItemDef.hpp"
#include "Cubikki/Game/Inventory/ItemStack.hpp"

class Actor;

class Inventory
{
public:
	Actor* m_Owner{ nullptr };
	std::vector<ItemSlot> m_Slots;
public:
	Inventory() = default;
	Inventory(int size);
	~Inventory();

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Interface
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	void PrimaryPutItem(ItemSlot* item, int slotIndex);
	void SecondaryPutItem(ItemSlot* item, int slotIndex);

	ItemSlot* GetItemSlot(int slotIndex);
};