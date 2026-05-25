#pragma once
#include "Cubikki/Actors/GameTag/GameTagToken.hpp"
#include "Cubikki/Actors/GameTag/GameTagLibrary.hpp"

#include <unordered_set>

class GameTagCollection
{
public:
	std::unordered_set<GameTagToken> m_Tags;
public:
	void AddTag(std::string_view tag);
	void RemoveTag(std::string_view tag);

	bool HasTag(GameTagToken token) const;
};

GameTagToken ConvertToTagToken(std::string_view tag);