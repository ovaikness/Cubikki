#pragma once
#include <string>
#include <vector>
namespace PBE
{
	using Strings = std::vector<std::string>;

	std::wstring StringToWString(std::string_view s);
	std::string WStringToString(std::wstring_view ws);

	Strings SplitString(std::string_view str);
	Strings SplitStringOnDelimiter(std::string_view str, char delimiter);
	std::string ToLowerCase(std::string_view str);
	std::string Stringf(const char* format, ...);
	std::wstring Stringf(const wchar_t* format, ...);
}