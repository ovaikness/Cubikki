#include "Cubikki/Actors/GameTag/GameTagCollection.hpp"

GameTagToken ConvertToTagToken(std::string_view tag)
{
	return GameTagLibrary::s_GameTagLibrary.GetToken(tag);
}

void GameTagCollection::AddTag(std::string_view tag)
{
	GameTagLibrary::s_GameTagLibrary.AddTag(tag);
	m_Tags.insert(GameTagLibrary::s_GameTagLibrary.GetToken(tag));
}

void GameTagCollection::RemoveTag(std::string_view tag)
{
	GameTagToken token = GameTagLibrary::s_GameTagLibrary.GetToken(tag);
	if (token != GameTagToken::INVALID_TOKEN)
	{
		m_Tags.erase(token);
	}
}

bool GameTagCollection::HasTag(GameTagToken token) const
{
	return m_Tags.find(token) != m_Tags.end();
}

