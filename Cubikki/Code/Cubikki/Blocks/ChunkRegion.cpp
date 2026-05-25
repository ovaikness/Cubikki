#include "Cubikki/Blocks/ChunkRegion.hpp"
#include "Cubikki/Blocks/BlockWorld.hpp"

#include "Engine/IO/FileUtils.hpp"


ChunkRegion::ChunkRegion(BlockWorld* blockWorld, std::filesystem::path path)
	: m_BlockWorld(blockWorld)
	, m_Path(path)
{
	if (m_BlockWorld->m_NetMode != BlockWorldNetMode::SERVER)
	{
		return;
	}

	if (std::filesystem::exists(path))
	{
		std::fstream fileStream;
		fileStream.open(path, std::ios::in | std::ios::binary);

		PBE::ReadFromFstream<ChunkRegionHeader>(fileStream, m_Header);

		for (size_t chunkIndex = 0; chunkIndex < CHUNK_REGION_CHUNK_COUNT; ++chunkIndex)
		{
			bool chunkExists;
			PBE::ReadFromFstream<bool>(fileStream, chunkExists);
			if (!chunkExists)
			{
				continue;
			}

			PBE::IntVec3 chunkPos;
			PBE::ReadFromFstream<PBE::IntVec3>(fileStream, chunkPos);

			ChunkCompressedData* chunkData = new ChunkCompressedData();
			chunkData->m_ChunkPos = chunkPos;
			PBE::ReadVectorFromFStream<uint8_t>(fileStream, chunkData->m_CompressedBlockIndices);
			PBE::ReadVectorFromFStream<uint8_t>(fileStream, chunkData->m_CompressedExtendedBlockIndices);
			PBE::ReadVectorFromFStream<uint8_t>(fileStream, chunkData->m_CompressedBlockData);

			int index = GetChunkIndexFromGlobalChunkCoords(chunkPos);
			m_ChunkData[index] = chunkData;
		}
		fileStream.close();
	}
}

ChunkCompressedData* ChunkRegion::GetChunkCompressedDataPointer(PBE::IntVec3 chunkPos)
{
	int index = GetChunkIndexFromGlobalChunkCoords(chunkPos);
	return m_ChunkData[index];
}

void ChunkRegion::SetChunkCompressedDataPointer(PBE::IntVec3 chunkPos, ChunkCompressedData* data)
{
	int index = GetChunkIndexFromGlobalChunkCoords(chunkPos);
	m_ChunkData[index] = data;
}

void ChunkRegion::SaveData()
{
	std::filesystem::path tempPath = m_Path;
	//tempPath = tempPath.replace_extension("tempcbr");
	std::fstream fileStream;
	fileStream.open(tempPath, std::ios::out | std::ios::binary);
	m_Header.m_Header[0] = 'C';
	m_Header.m_Header[1] = 'H';
	m_Header.m_Header[2] = 'N';
	m_Header.m_Header[3] = 'K';
	m_Header.m_Header[4] = '0';
	m_Header.m_Header[5] = '0';
	m_Header.m_Header[6] = '0';
	m_Header.m_Header[7] = '1';

	PBE::WriteToFstream<ChunkRegionHeader>(fileStream, m_Header);

	for (int i = 0; i < CHUNK_REGION_CHUNK_COUNT; ++i)
	{
		if (m_ChunkData[i] == nullptr)
		{
			PBE::WriteToFstream(fileStream, false);
			continue;
		}
		PBE::WriteToFstream(fileStream, true);
		PBE::WriteToFstream<PBE::IntVec3>(fileStream, m_ChunkData[i]->m_ChunkPos);
		PBE::WriteVectorToFstream<uint8_t>(fileStream, m_ChunkData[i]->m_CompressedBlockIndices);
		PBE::WriteVectorToFstream<uint8_t>(fileStream, m_ChunkData[i]->m_CompressedExtendedBlockIndices);
		PBE::WriteVectorToFstream<uint8_t>(fileStream, m_ChunkData[i]->m_CompressedBlockData);
	}
	    
	fileStream.close();
	//
	//if (std::filesystem::exists(m_Path)) {
	//	std::filesystem::remove(m_Path);
	//}
	//
	//try {
	//	std::filesystem::rename(tempPath, m_Path);
	//}
	//catch (const std::filesystem::filesystem_error& e) {
	//	// Probably cross-device or locked — fall back to copy + remove
	//	std::filesystem::copy_file(
	//		tempPath, m_Path,
	//		std::filesystem::copy_options::overwrite_existing
	//	);
	//	std::filesystem::remove(tempPath);
	//}

}

ChunkRegion::~ChunkRegion()
{
	if (m_BlockWorld->m_NetMode == BlockWorldNetMode::SERVER)
	{
		SaveData();
	}

	for (auto chunkData : m_ChunkData)
	{
		delete chunkData;
	}
}