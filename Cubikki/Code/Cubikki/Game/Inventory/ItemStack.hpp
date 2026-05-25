#pragma once

class ItemDef;
//Nullptr is used to indicate an empty slot
class ItemSlot
{
public:
	ItemDef const* m_Definition{ nullptr };
	int m_StackSize{ 0 };
public:
	ItemSlot(ItemDef const* definition, int stackSize)
		: m_Definition(definition)
		, m_StackSize(stackSize)
	{
	}

	bool SwapIfDifferent(ItemSlot* otherStack)
	{
		if (!otherStack)
		{
			return false;
		}
		if (m_Definition == otherStack->m_Definition)
		{
			return false;
		}
		std::swap(m_Definition, otherStack->m_Definition);
		std::swap(m_StackSize, otherStack->m_StackSize);
		return true;
	};

	bool TakeFromUntilFull(ItemSlot* otherStack)
	{
		if (!otherStack)
		{
			return false;
		}

		if (m_Definition == nullptr)
		{
			m_Definition = otherStack->m_Definition;
		}

		if (m_Definition != otherStack->m_Definition)
		{
			return false;
		}

		int spaceLeft = m_Definition->m_StackSize - m_StackSize;
		if (spaceLeft <= 0)
		{
			return false;
		}

		if (otherStack->m_StackSize > spaceLeft)
		{
			m_StackSize += spaceLeft;
			otherStack->m_StackSize -= spaceLeft;

			return true;
		}
		else
		{
			m_StackSize += otherStack->m_StackSize;
			otherStack->m_StackSize = 0;
			otherStack->m_Definition = nullptr;

			return true;
		}
	};

	bool TakeOneFrom(ItemSlot* otherStack)
	{
		if (!otherStack)
		{
			return false;
		}
		if (m_Definition == nullptr)
		{
			m_Definition = otherStack->m_Definition;
		}
		if (m_Definition != otherStack->m_Definition)
		{
			return false;
		}
		if (otherStack->m_StackSize <= 0)
		{
			return false;
		}
		if (m_StackSize >= m_Definition->m_StackSize)
		{
			return false;
		}

		m_StackSize++;
		otherStack->m_StackSize--;
		if (otherStack->m_StackSize <= 0)
		{
			otherStack->m_Definition = nullptr;
			return true;
		}

		return true;
	};

	bool TakeHalfIfEmpty(ItemSlot* otherStack)
	{
		if (!otherStack)
		{
			return false;
		}

		if (m_Definition != nullptr)
		{
			return false;
		}
		m_Definition = otherStack->m_Definition;

		if (otherStack->m_StackSize <= 0)
		{
			return false;
		}

		if (m_StackSize >= m_Definition->m_StackSize)
		{
			return false;
		}

		int half;
		if (otherStack->m_StackSize == 1)
		{
			half = 1;
		}
		else
		{
			half = otherStack->m_StackSize / 2;
		}

		m_StackSize += half;
		otherStack->m_StackSize -= half;

		if (otherStack->m_StackSize <= 0)
		{
			otherStack->m_Definition = nullptr;
			return true;
		}
		return true;
	};
};