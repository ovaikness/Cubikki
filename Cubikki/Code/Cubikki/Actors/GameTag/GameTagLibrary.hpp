#pragma once

#include "Cubikki/Actors/GameTag/GameTagToken.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>


class GameTagLibrary
{
public:
	static GameTagLibrary s_GameTagLibrary;
	uint32_t m_TokenCounter = 0;
	std::unordered_map<std::string, GameTagToken> m_TagTokens;
	std::unordered_map<GameTagToken, std::string> m_TokenNames;
public:
	GameTagLibrary();
	~GameTagLibrary();
	void AddTag(std::string_view tag);
	void RemoveTag(std::string_view tag);
	bool HasTag(std::string_view tag) const;

	GameTagToken GetToken(std::string_view tag) const;
};