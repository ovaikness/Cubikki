#pragma once

#include <filesystem>
#include <vector>
#include <string_view>
#include <fstream>

namespace PBE
{
	std::vector<uint8_t> ReadFileToBuffer(std::filesystem::path path);
	void WriteBufferToFile(std::filesystem::path filename, std::vector<uint8_t> const& buffer);

	std::vector<uint32_t> ReadFileToUint32Buffer(std::filesystem::path path);
	void WriteUint32BufferToFile(std::filesystem::path filename, std::vector<uint32_t> const& buffer);

	template<typename T_ValueType>
	bool ReadVectorFromFStream(std::fstream& stream, std::vector<T_ValueType>& vec)
	{
		size_t size;
		stream.read(reinterpret_cast<char*>(&size), sizeof(size_t));
		vec.resize(size);
		stream.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T_ValueType));
		return true;
	}

	template<typename T_ValueType>
	bool ReadFromFstream(std::fstream& stream, T_ValueType& data)
	{
		stream.read(reinterpret_cast<char*>(&data), sizeof(T_ValueType));
		return true;
	}

	template<typename T_ValueType>
	bool WriteVectorToFstream(std::fstream& stream, std::vector<T_ValueType> const& vec)
	{
		size_t size = vec.size();
		stream.write(reinterpret_cast<char*>(&size), sizeof(size_t));
		stream.write(reinterpret_cast<char const*>(vec.data()), size * sizeof(T_ValueType));
		return true;
	}

	template<typename T_ValueType>
	bool WriteToFstream(std::fstream& stream, T_ValueType const& data)
	{
		stream.write(reinterpret_cast<char const*>(&data), sizeof(T_ValueType));
		return true;
	}
}