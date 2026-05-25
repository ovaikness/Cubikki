#pragma once

#include <vector>

template<typename T>
class FillList
{
	struct FillListEntry
	{
		bool m_IsAvailable = false;
		char m_Item[sizeof(T)];

		bool operator()
		{
			return m_IsAvailable;
		}
	};
private:
	std::vector<FillListEntry> m_Entries;
public:
	FillList() = default;
	~FillList() = default;



	void Clear()
	{
		m_Entries.clear();
	}

	std::vector<T> const& GetItems() const
	{
		return m_Entries;
	}
};