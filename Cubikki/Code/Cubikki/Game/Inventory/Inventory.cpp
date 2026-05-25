#include "Cubikki/Game/Inventory/Inventory.hpp"

Inventory::Inventory(int size)
{
	m_Slots.resize(size, ItemSlot(nullptr, 0));
}

Inventory::~Inventory()
{

}

void Inventory::SecondaryPutItem(ItemSlot* item, int slotIndex)
{
	ItemSlot* targetSlot = GetItemSlot(slotIndex);
	if (!targetSlot)
	{
		return;
	}
	if (item->TakeHalfIfEmpty(targetSlot))
	{
		return;
	}
	if (targetSlot->TakeOneFrom(item))
	{
		return;
	}
	if (targetSlot->SwapIfDifferent(item))
	{
		return;
	}
}

ItemSlot* Inventory::GetItemSlot(int slotIndex)
{
	if (slotIndex >= 0 && slotIndex < m_Slots.size())
	{
		return &m_Slots[slotIndex];
	}
	return nullptr;
}

void Inventory::PrimaryPutItem(ItemSlot* item, int slotIndex)
{
	ItemSlot* targetSlot = GetItemSlot(slotIndex);
	if (!targetSlot)
	{
		return;
	}
	if (item->TakeFromUntilFull(item))
	{
		return;
	}
	if (targetSlot->SwapIfDifferent(item))
	{
		return;
	}
}
