#pragma once

#include "Engine/Math/IntVec3.hpp"

class Chunk;

class WorldGenerator
{
public:
	virtual void GenerateChunk(Chunk* chunk) = 0;
};