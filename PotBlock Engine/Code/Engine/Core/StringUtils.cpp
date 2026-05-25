#include "Engine/Core/StringUtils.hpp"

#include <string>
#include <cstdarg>
#include <vector>
#include <stdexcept>
#include <functional>
#include <tuple>
#include <sstream>
#include <vector>
#include <cwchar>  // for std::vsnprintf

namespace PBE
{
	std::wstring StringToWString(std::string_view s)
	{
		std::wstring wsTmp(s.begin(), s.end());
		return wsTmp;
	}

	std::string WStringToString(std::wstring_view ws)
	{
		std::string sTmp;
		for (std::wstring_view::iterator itr = ws.begin(); itr != ws.end(); ++itr)
		{
			sTmp += (char*)*itr;
		}
		return sTmp;
	}

	Strings SplitString(std::string_view str)
	{
		std::vector<std::string> parts;
		std::string part;
		bool insideQuotes = false;

		for (size_t i = 0; i < str.size(); ++i)
		{
			char currentChar = str[i];

			// Toggle insideQuotes when encountering a quote
			if (currentChar == '"')
			{
				insideQuotes = !insideQuotes;
				if (!insideQuotes)
				{
					parts.push_back(part);        // push the token (quoted part)
					part.clear();
				}
			}
			else if (insideQuotes)
			{
				part.push_back(currentChar);  // Add character to the part if inside quotes
			}
			else if (std::isspace(static_cast<unsigned char>(currentChar)))
			{
				// Ignore spaces outside quotes (split token)
				if (!part.empty())
				{
					parts.push_back(part);
					part.clear();
				}
			}
			else
			{
				// Otherwise, add to the current token
				part.push_back(currentChar);
			}
		}

		// If there's any leftover part after processing the string
		if (!part.empty())
		{
			parts.push_back(part);
		}

		return parts;
	}

	std::vector<std::string> SplitStringOnDelimiter(std::string_view str, char delimiter)
	{
		Strings result;
		int start = 0;
		for (int index = 0; index < str.size(); ++index)
		{
			if (str[index] == delimiter)
			{
				result.push_back(std::string(str.substr(start, index - start)));
				start = index + 1;
			}
		}
		if (start < str.size())
		{
			result.push_back(std::string(str.substr(start, str.size())));
		}
		return result;
	}
	std::string ToLowerCase(std::string_view str)
	{
		std::string result;
		for (int index = 0; index < str.size(); ++index)
		{
			result += static_cast<char>(std::tolower(str[index]));
		}
		return result;
	}
	std::string Stringf(const char* format, ...)
	{
		// Start variable arguments
		va_list args;
		va_start(args, format);

		// Determine required buffer size
		va_list args_copy;
		va_copy(args_copy, args);
		int size = std::vsnprintf(nullptr, 0, format, args_copy);
		va_end(args_copy);

		if (size < 0) {
			va_end(args);
			throw std::runtime_error("Error during string formatting");
		}

		// Allocate a buffer large enough to hold the formatted string
		std::vector<char> buffer(size + 1); // +1 for null terminator
		std::vsnprintf(buffer.data(), buffer.size(), format, args);
		va_end(args);

		// Return as a std::string
		return std::string(buffer.data());
	}

	std::wstring Stringf(const wchar_t* format, ...)
	{
		// Start variable arguments
		va_list args;
		va_start(args, format);

		// Determine the required buffer size
		int size = _vsnwprintf(nullptr, 0, format, args);
		va_end(args);

		if (size < 0) {
			throw std::runtime_error("Error during string formatting");
		}

		// Allocate a buffer large enough to hold the formatted string
		std::vector<wchar_t> buffer(size + 1);  // +1 for the null terminator

		// Format the string into the buffer
		va_start(args, format);
		_vsnwprintf(buffer.data(), buffer.size(), format, args);
		va_end(args);

		// Return as a std::wstring
		return std::wstring(buffer.data());
	}
}