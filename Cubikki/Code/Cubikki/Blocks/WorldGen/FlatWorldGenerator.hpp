#pragma once
#include "Cubikki/Blocks/WorldGenerator.hpp"
#include <FastNoise/FastNoise.h>

class FlatWorldGenerator : public WorldGenerator
{
public:
	FastNoise::SmartNode<FastNoise::Simplex> fndSimplex = FastNoise::New<FastNoise::Simplex>();
	FastNoise::SmartNode<FastNoise::CellularDistance> m_ButteNoise = FastNoise::New<FastNoise::CellularDistance>();

	FastNoise::SmartNode<FastNoise::FractalFBm> m_HeightNoise = FastNoise::New<FastNoise::FractalFBm>();
	FastNoise::SmartNode<FastNoise::FractalFBm> m_ContinentalNoise = FastNoise::New<FastNoise::FractalFBm>();
	FastNoise::SmartNode<FastNoise::FractalFBm> m_RiverNoise = FastNoise::New<FastNoise::FractalFBm>();
	FastNoise::SmartNode<FastNoise::FractalFBm> m_PullNoise = FastNoise::New<FastNoise::FractalFBm>();
	FastNoise::SmartNode<FastNoise::FractalFBm> m_SpacialDensityNoise = FastNoise::New<FastNoise::FractalFBm>();
public:
	FlatWorldGenerator();
	~FlatWorldGenerator();
	void GenerateChunk(Chunk* chunk) override;
};