#pragma  once
#include <string>
#include <locale>
#include <algorithm>

namespace PBE
{
	class HCIString
	{
	public:
		HCIString() = default;
		HCIString(char const* cString);
		HCIString(std::string const& string);
		HCIString(std::string&& string);
		HCIString(HCIString const& other);
		HCIString(HCIString&& other) noexcept;

		HCIString& operator=(std::string const& string);
		HCIString& operator=(char const* cString);
		HCIString& operator=(std::string&& string) noexcept;
		HCIString& operator=(HCIString const& other);
		HCIString& operator=(HCIString&& other) noexcept;

		bool operator==(HCIString const& other) const;
		bool operator!=(HCIString const& other) const;

		bool operator <(HCIString const& other) const;

		uint32_t GetHash() const;

		std::string const& GetCasedString() const;
	private:
		uint32_t m_Hash = 0;
		std::string m_CasedString;
	private:
		void Rehash();
	};
}

namespace std
{
	template<>
	struct hash<PBE::HCIString>
	{
		size_t operator()(PBE::HCIString const& str) const
		{
			return str.GetHash();
		}
	};
}