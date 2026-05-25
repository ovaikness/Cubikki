#include "Engine/IO/FileUtils.hpp"

#include <fstream>
#include <filesystem>

namespace PBE
{
	std::vector<uint8_t> ReadFileToBuffer(std::filesystem::path path)
	{
		std::vector<uint8_t> buffer;
		std::ifstream file(path, std::ios::binary);

		uint32_t fileSize = static_cast<uint32_t>(std::filesystem::file_size(path));

		buffer.resize(fileSize);

		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

		file.close();

		return buffer;
	}

	void WriteBufferToFile(std::filesystem::path path, std::vector<uint8_t> const& buffer)
	{
		// Ensure the directory exists
		std::filesystem::create_directories(path.parent_path());

		// Create and open the file for binary writing
		std::ofstream file(path, std::ios::binary);

		// Write the buffer to the file
		file.write(reinterpret_cast<char const*>(buffer.data()), buffer.size());

		file.close();
	}

	std::vector<uint32_t> ReadFileToUint32Buffer(std::filesystem::path path)
	{
		std::vector<uint32_t> buffer;
		std::ifstream file(path,std::ios::binary);

		uint32_t fileSize = static_cast<uint32_t>(std::filesystem::file_size(path));

		buffer.resize(fileSize / sizeof(uint32_t));

		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

		file.close();

		return buffer;
	}

	void WriteUint32BufferToFile(std::filesystem::path path, std::vector<uint32_t> const& buffer)
	{
		// Ensure the directory exists
		std::filesystem::create_directories(path.parent_path());

		// Create and open the file for binary writing
		std::ofstream file(path, std::ios::binary);

		// Write the buffer to the file
		file.write(reinterpret_cast<char const*>(buffer.data()), buffer.size() * sizeof(uint32_t));

		file.close();
	}
}