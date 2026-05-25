#pragma once
#include "Cubikki/Blocks/BlockCommon.hpp"

class Block
{
public:
	uint32_t m_BlockCount;
	BlockTypeID  m_BlockID;

	bool operator==(const Block& other) const
	{
		return m_BlockID == other.m_BlockID;
	}
};