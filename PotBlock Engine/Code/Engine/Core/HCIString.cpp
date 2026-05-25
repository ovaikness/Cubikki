#include "Engine/Core/HCIString.hpp"

PBE::HCIString::HCIString(char const* cString)
	: m_CasedString(cString)
{
	Rehash();
}

PBE::HCIString::HCIString(std::string const& string)
	: m_CasedString(string)
{
	m_CasedString = string;
	Rehash();
}

PBE::HCIString::HCIString(std::string&& string)
	: m_CasedString(std::move(string))
{
	Rehash();
}

PBE::HCIString::HCIString(HCIString const& other)
	: m_CasedString(other.m_CasedString)
	, m_Hash(other.m_Hash)
{
}
PBE::HCIString::HCIString(HCIString&& other) noexcept
	: m_CasedString(std::move(other.m_CasedString))
	, m_Hash(std::move(other.m_Hash))
{
}

PBE::HCIString& PBE::HCIString::operator=(std::string const& string)
{
	*this = PBE::HCIString(string);
	return *this;
}

PBE::HCIString& PBE::HCIString::operator=(char const* cString)
{
	*this = PBE::HCIString(cString);
	return *this;
}

PBE::HCIString& PBE::HCIString::operator=(std::string&& string) noexcept
{
	*this = PBE::HCIString(std::move(string));
	return *this;
}

PBE::HCIString& PBE::HCIString::operator=(HCIString const& other)
{
	m_Hash = other.m_Hash;
	m_CasedString = other.m_CasedString;
	return *this;
}

PBE::HCIString& PBE::HCIString::operator=(HCIString&& other) noexcept
{
	m_Hash = other.m_Hash;
	m_CasedString = std::move(other.m_CasedString);
	return *this;
}

bool PBE::HCIString::operator==(HCIString const& other) const
{
	if (m_Hash != other.m_Hash)
		return false;

	if (m_CasedString.size() != other.m_CasedString.size())
		return false;

	for (int i = 0; i < m_CasedString.size(); ++i)
	{
		if (std::tolower(m_CasedString[i]) != std::tolower(other.m_CasedString[i]))
			return false;
	}

	return true;
}

bool PBE::HCIString::operator!=(HCIString const& other) const
{
	return m_Hash != other.m_Hash;
}

bool PBE::HCIString::operator<(HCIString const& other) const
{
	if (m_Hash == other.m_Hash)
	{
		return m_CasedString < other.m_CasedString;
	}

	return m_Hash < other.m_Hash;
}

uint32_t PBE::HCIString::GetHash() const
{
	return m_Hash;
}

std::string const& PBE::HCIString::GetCasedString() const
{
	return m_CasedString;
}

void PBE::HCIString::Rehash()
{
	m_Hash = 0;
	for (char c : m_CasedString)
	{
		char lower = (char)std::tolower(c);
		m_Hash = (m_Hash << 1) + 31 * lower;
	}
}