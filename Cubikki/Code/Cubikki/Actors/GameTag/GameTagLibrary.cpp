#include "Cubikki/Actors/GameTag/GameTagLibrary.hpp"

GameTagToken const GameTagToken::INVALID_TOKEN = GameTagToken(0xFFFFFFFF);
GameTagLibrary GameTagLibrary::s_GameTagLibrary;

GameTagLibrary::GameTagLibrary()
{
}

GameTagLibrary::~GameTagLibrary()
{

}

void GameTagLibrary::AddTag(std::string_view tag)
{
	if (!HasTag(tag))
	{
		m_TagTokens[tag.data()] = GameTagToken(m_TokenCounter++);
		m_TokenNames[m_TagTokens[tag.data()]] = tag.data();
	}
}

void GameTagLibrary::RemoveTag(std::string_view tag)
{
	auto iter = m_TagTokens.find(tag.data());
	if (iter != m_TagTokens.end())
	{
		m_TagTokens.erase(iter);
	}

	auto nameIter = m_TokenNames.find(iter->second);
	if (nameIter != m_TokenNames.end())
	{
		m_TokenNames.erase(nameIter);
	}
}

bool GameTagLibrary::HasTag(std::string_view tag) const
{
	auto iter = m_TagTokens.find(tag.data());
	return iter != m_TagTokens.end();
}

GameTagToken GameTagLibrary::GetToken(std::string_view tag) const
{
	auto iter = m_TagTokens.find(tag.data());
	if (iter != m_TagTokens.end())
	{
		return iter->second;
	}
	return GameTagToken::INVALID_TOKEN;
}

