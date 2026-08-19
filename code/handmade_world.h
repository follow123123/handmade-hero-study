#if !defined(HANDMADE_TILE_H)

struct world_difference
{
	vec2 dXY;
	real32 dZ;
};

struct world_position
{
	int32 ChunkX;
	int32 ChunkY;
	int32 ChunkZ;
	
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
	real32 ChunkSideInMeters;

	world_entity_block *FirstFree;
	
	world_chunk ChunkHash[4096];
};

#define HANDMADE_TILE_H
#endif
