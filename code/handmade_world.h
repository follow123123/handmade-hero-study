#if !defined(HANDMADE_TILE_H)

struct world_difference
{
	vec2 dXY;
	real32 dZ;
};

struct world_position
{
	int32 AbsTileX;
	int32 AbsTileY;
	int32 AbsTileZ;
	
	vec2 _Offset;
};

struct world_entity_block
{
	uint32 EntityCount;
	uint32 LowEntityIndex[16];
	world_entity_block *Next;
};

struct world_chunk
{
	int32 ChunkX;
	int32 ChunkY;
	int32 ChunkZ;
	
	world_entity_block FirstBlock;
	
	world_chunk *NextInHash;
};

struct world
{
	real32 TileSideInMeters;
	
    int32 ChunkShift;
	int32 ChunkMask;
	int32 ChunkDim;
	world_chunk ChunkHash[4096];
};

#define HANDMADE_TILE_H
#endif
