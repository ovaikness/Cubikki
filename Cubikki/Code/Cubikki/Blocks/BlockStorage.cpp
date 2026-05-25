#include "Cubikki/Blocks/BlockStorage.hpp"
#include "Cubikki/Common.hpp"

#include <stdexcept>
#include <zstd.h>

constexpr int COMPRESSION_LEVEL = -7;
BlockStorage::BlockStorage()
{
	std::fill(std::begin(m_BlockIndices), std::end(m_BlockIndices), 0);
	m_BlockEntries.push_back(Block{ 0, AIR_BLOCK_TYPE_ID });
}

BlockStorage::~BlockStorage()
{
}

void BlockStorage::Clear()
{
	std::fill(std::begin(m_BlockIndices), std::end(m_BlockIndices), 0);
	m_ExtendedBlockIndices.clear();
	m_BlockEntries.clear();
	m_BlockEntries.push_back(Block{ 0, AIR_BLOCK_TYPE_ID });
}

Block const& BlockStorage::GetBlock(int blockIndex) const
{
	int entryIndex = GetEntryIndexFromBlockIndex(blockIndex);
	return m_BlockEntries[entryIndex];
}

void BlockStorage::SetBlock(int blockIndex, Block const& block)
{
	int entryIndex = GetEntryIndexFromBlockIndex(blockIndex);
	Block& entry = m_BlockEntries[entryIndex];
	--entry.m_BlockCount;
	int newEntryIndex = GetSameBlockIndex(block);
	if (newEntryIndex == -1)
	{
		m_BlockEntries.push_back(block);
		newEntryIndex = (int)m_BlockEntries.size() - 1;
	}
	++m_BlockEntries[newEntryIndex].m_BlockCount;

	if (newEntryIndex > 255)
	{
		if (m_ExtendedBlockIndices.size() == 0)
		{
			m_ExtendedBlockIndices.resize(CHUNK_BLOCK_COUNT, 0);
		}
		m_ExtendedBlockIndices[blockIndex] = newEntryIndex >> 8;
		m_BlockIndices[blockIndex] = newEntryIndex & 0xFF;
	}
	else
	{
		m_BlockIndices[blockIndex] = newEntryIndex;
	}
}

int BlockStorage::GetEntryIndexFromBlockIndex(int blockIndex) const
{
	if (m_ExtendedBlockIndices.size() > 0)
	{
		int index = m_ExtendedBlockIndices[blockIndex] << 8;
		index |= m_BlockIndices[blockIndex];
	}
	else
	{
		return m_BlockIndices[blockIndex];
	}
	return -1;
}

int BlockStorage::GetSameBlockIndex(Block const& block)
{
	for (int i = 0; i < m_BlockEntries.size(); ++i)
	{
		if (m_BlockEntries[i] == block)
		{
			return i;
		}
	}
	return -1;
}

void BlockStorage::ShrinkToSize()
{
	bool needsToShrink = false;
	for (Block& block : m_BlockEntries)
	{
		if (block.m_BlockCount == 0)
		{
			needsToShrink = true;
			break;
		}
	}

	if (!needsToShrink)
	{
		return;
	}

	std::vector<Block> newBlockEntries;
	std::unordered_map<int, int> oldToNewIndex;
	newBlockEntries.reserve(m_BlockEntries.size());

	for (int i = 0; i < m_BlockEntries.size(); ++i)
	{
		if (m_BlockEntries[i].m_BlockCount > 0)
		{
			newBlockEntries.push_back(m_BlockEntries[i]);
			oldToNewIndex[i] = (int)newBlockEntries.size() - 1;
		}
	}

	for (int i = 0; i < CHUNK_BLOCK_COUNT; ++i)
	{
		int oldIndex = GetEntryIndexFromBlockIndex(i);
		m_BlockIndices[i] = oldToNewIndex[oldIndex];
	}

	m_BlockEntries = std::move(newBlockEntries);
}

void BlockStorage::PushCompressedBlockIndices(PBE::GrowBuffer& stream) const
{
	std::vector<uint8_t> compressed = GetCompressedBlockIndices();
	stream.WriteDynamicArray(compressed.data(), sizeof(uint8_t) * compressed.size());
}

void BlockStorage::PushCompressedExtendedBlockIndices(PBE::GrowBuffer& stream) const
{
	std::vector<uint8_t> compressed = GetCompressedExtendedBlockIndices();
	stream.WriteDynamicArray(compressed.data(), sizeof(uint8_t) * compressed.size());
}

void BlockStorage::PushCompressedBlockEntries(PBE::GrowBuffer& stream) const
{
	std::vector<uint8_t> compressed = GetCompressedBlockData();
	stream.WriteDynamicArray(compressed.data(), sizeof(uint8_t) * compressed.size());
}

std::vector<uint8_t> BlockStorage::GetCompressedBlockIndices() const
{
	size_t const compressedSize = ZSTD_compressBound(CHUNK_BLOCK_COUNT);
	std::vector<uint8_t> compressed(compressedSize);
	size_t const compressedBytes = ZSTD_compress(compressed.data(), compressedSize, m_BlockIndices, CHUNK_BLOCK_COUNT, COMPRESSION_LEVEL);

	if (ZSTD_isError(compressedBytes))
	{
		throw std::runtime_error("Compression failed");
	}

	compressed.resize(compressedBytes);

	return compressed;
}

std::vector<uint8_t> BlockStorage::GetCompressedExtendedBlockIndices() const
{
	if (m_ExtendedBlockIndices.size() == 0)
	{
		return { 0 };
	}

	size_t const compressedSize = ZSTD_compressBound(m_ExtendedBlockIndices.size());
	std::vector<uint8_t> compressed(compressedSize + 1);
	compressed[0] = 1;
	size_t const compressedBytes = ZSTD_compress(compressed.data() + 1, compressedSize, m_ExtendedBlockIndices.data(), m_ExtendedBlockIndices.size(), COMPRESSION_LEVEL);

	compressed.resize(compressedBytes + 1);

	return compressed;
}

std::vector<uint8_t> BlockStorage::GetCompressedBlockData() const
{
    // Allocate buffer for compressed data
    size_t const compressedSize = ZSTD_compressBound(m_BlockEntries.size() * sizeof(Block));
    std::vector<uint8_t> compressed(compressedSize);

    // Compress data
    size_t const compressedBytes = ZSTD_compress(compressed.data(), compressedSize, m_BlockEntries.data(), m_BlockEntries.size() * sizeof(Block), COMPRESSION_LEVEL);

    if (ZSTD_isError(compressedBytes))
    {
        throw std::runtime_error(ZSTD_getErrorName(compressedBytes));
    }

    compressed.resize(compressedBytes);
    return compressed;
}

void BlockStorage::LoadBlockIndices(std::vector<unsigned char> const& blockIndices)
{
	size_t const decompressedSize = ZSTD_getFrameContentSize(blockIndices.data(), blockIndices.size());

	if (decompressedSize == ZSTD_CONTENTSIZE_ERROR)
	{
		throw std::runtime_error("Invalid ZSTD frame: unable to determine content size");
	}
	else if (decompressedSize == ZSTD_CONTENTSIZE_UNKNOWN)
	{
		throw std::runtime_error("Unknown content size in ZSTD frame");
	}

	std::vector<unsigned char> decompressed(decompressedSize);
	size_t const decompressedBytes = ZSTD_decompress(decompressed.data(), decompressedSize, blockIndices.data(), blockIndices.size());

	if (ZSTD_isError(decompressedBytes))
	{
		std::cout << ZSTD_getErrorName(decompressedBytes) << "\n";
		throw std::runtime_error(ZSTD_getErrorName(decompressedBytes));
	}

	if (decompressedBytes != decompressedSize)
	{
		throw std::runtime_error("Failed to decompress block indices");
	}

	for (size_t i = 0; i < CHUNK_BLOCK_COUNT; ++i)
	{
		m_BlockIndices[i] =(decompressed[i]);
	}
}


void BlockStorage::LoadExtendedBlockIndices(std::vector<unsigned char> const& extendedBlockIndices)
{
	if (extendedBlockIndices[0] == 0)
	{
		return;
	}
	size_t const decompressedSize = ZSTD_getFrameContentSize(extendedBlockIndices.data() + 1, extendedBlockIndices.size() - 1);

	std::vector<unsigned char> decompressed(decompressedSize);

	size_t const decompressedBytes = ZSTD_decompress(decompressed.data(), decompressedSize, extendedBlockIndices.data() + 1, extendedBlockIndices.size() - 1);

	if (decompressedBytes != decompressedSize)
	{
		throw std::runtime_error("Failed to decompress extended block indices");
	}

	m_ExtendedBlockIndices.clear();
	m_ExtendedBlockIndices.reserve(decompressedSize);

	for (int i = 0; i < decompressedSize; ++i)
	{
		m_ExtendedBlockIndices.emplace_back(decompressed[i]);
	}
}

void BlockStorage::LoadBlockEntries(std::vector<unsigned char> const& blockEntries)
{
	size_t const decompressedSize = ZSTD_getFrameContentSize(blockEntries.data(), blockEntries.size());
	std::vector<unsigned char> decompressed(decompressedSize);
	size_t const decompressedBytes = ZSTD_decompress(decompressed.data(), decompressedSize, blockEntries.data(), blockEntries.size());
	if (decompressedBytes != decompressedSize)
	{
		throw std::runtime_error("Failed to decompress block entries");
	}
	m_BlockEntries.clear();
	m_BlockEntries.reserve(decompressedSize / sizeof(Block));
	for (int i = 0; i < decompressedSize; i += sizeof(Block))
	{
		Block block;
		uint8_t* data = reinterpret_cast<uint8_t*>(&block);
		for (int j = 0; j < sizeof(Block); ++j)
		{
			data[j] = decompressed[i + j];
		}
		m_BlockEntries.emplace_back(block);
	}
}
