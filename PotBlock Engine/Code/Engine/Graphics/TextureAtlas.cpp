#include "Engine/Graphics/TextureAtlas.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

PBE::TextureAtlas::TextureAtlas(TextureAtlasCreateInfo const& info)
	: m_Texture(info.m_Texture)
	, m_TextureCoords(info.m_TextureCoords)
{
}

PBE::TextureAtlas::~TextureAtlas()
{

}

PBE::TextureAtlas PBE::TextureAtlas::LoadOrGetTextureAtlas(Renderer* renderer, std::filesystem::path path, PBE::SamplerType samplerType)
{
	std::ifstream f(path);

	nlohmann::json j = nlohmann::json::parse(f);

	std::map<std::string, TextureRegion> regions;
	std::filesystem::path texturePath = "Assets/" + j["meta"]["image"].get<std::string>();

	Texture2D texture = renderer->LoadOrGetTexture2D(texturePath, samplerType);

	auto meta = j["meta"];
	Vec2 size(meta["size"]["w"].get<float>(), meta["size"]["h"].get<float>());
	auto array = j["frames"];
	for (auto& element : array)
	{
		auto frame = element["frame"];
		auto pivot = element["pivot"];
		auto name = element["filename"].get<std::string>();
		auto x = frame["x"].get<float>();
		auto y = frame["y"].get<float>();
		auto w = frame["w"].get<float>();
		auto h = frame["h"].get<float>();
		auto px = pivot["x"].get<float>();
		auto py = pivot["y"].get<float>();

		TextureRegion region;
		AABB2 uvRaw = AABB2(Vec2(x / size.x, y / size.y), Vec2((x + w) / size.x, (y + h) / size.y));
		Vec2 uvDims = uvRaw.GetDimensions();
		uvRaw.m_Mins += uvDims * 0.005f;
		uvRaw.m_Maxs -= uvDims * 0.005f;
		region.m_UV = uvRaw;
		region.m_Size = Vec2(w, h);
		region.m_NormalizedPivot = Vec2(px / w, py / h);
		regions[name] = region;
	}

	TextureAtlasCreateInfo info
	{
		.m_Texture = texture,
		.m_TextureCoords = regions
	};

	return TextureAtlas(info);
}

PBE::TextureRegion PBE::TextureAtlas::GetTextureRegion(std::string_view name) const
{
	return m_TextureCoords.at(std::string(name));
}
