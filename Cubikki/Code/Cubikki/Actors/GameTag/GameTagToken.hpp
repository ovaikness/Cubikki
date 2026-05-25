#pragma once

#include <cstdint>
#include <functional>

class GameTagToken
{
	uint32_t m_Token;
public:
	static GameTagToken const INVALID_TOKEN;
	GameTagToken() = default;
	GameTagToken(uint32_t token)
		: m_Token(token)
	{
	};

	bool operator==(GameTagToken const& other) const
	{
		return m_Token == other.m_Token;
	}

	bool operator!=(GameTagToken const& other) const
	{
		return m_Token != other.m_Token;
	}

	bool operator<(GameTagToken const& other) const
	{
		return m_Token < other.m_Token;
	}

	bool operator>(GameTagToken const& other) const
	{
		return m_Token > other.m_Token;
	}

	bool operator<=(GameTagToken const& other) const
	{
		return m_Token <= other.m_Token;
	}

	bool operator>=(GameTagToken const& other) const
	{
		return m_Token >= other.m_Token;
	}

	uint32_t GetToken() const
	{
		return m_Token;
	}
};

namespace std
{
	template <>
	class hash<GameTagToken>
	{
	public:
		std::uint64_t operator()(GameTagToken const& token) const
		{
			return token.GetToken();
		}
	};
}