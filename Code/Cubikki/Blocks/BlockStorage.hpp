#pragma once
#include "Cubikki/Blocks/BlockCommon.hpp"
#include "Cubikki/Blocks/Block.hpp"
#include "Engine/Core/GrowBuffer.hpp"

#include <vector>

class BlockStorage
{
public:
	unsigned char m_BlockIndices[CHUNK_BLOCK_COUNT]{ 0 };
	std::vector<unsigned char> m_ExtendedBlockIndices{};
	std::vector<Block> m_BlockEntries{};
public:
	BlockStorage();
	~BlockStorage();

	void Clear();

	Block const& GetBlock(int blockIndex) const;
	void SetBlock(int blockIndex, Block const& block);

	int GetEntryIndexFromBlockIndex(int blockIndex) const;
	int GetSameBlockIndex(Block const& block);

	void ShrinkToSize();

	void PushCompressedBlockIndices(PBE::GrowBuffer& stream) const;
	void PushCompressedExtendedBlockIndices(PBE::GrowBuffer& stream) const;
	void PushCompressedBlockEntries(PBE::GrowBuffer& stream) const;

	std::vector<uint8_t> GetCompressedBlockIndices() const;
	std::vector<uint8_t> GetCompressedExtendedBlockIndices() const;
	std::vector<uint8_t> GetCompressedBlockData() const;

	void LoadBlockIndices(std::vector<unsigned char> const& blockIndices);
	void LoadExtendedBlockIndices(std::vector<unsigned char> const& extendedBlockIndices);
	void LoadBlockEntries(std::vector<unsigned char> const& blockEntries);
};