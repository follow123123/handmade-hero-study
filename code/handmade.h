#if !defined(HANDMADE_H)

#include "handmade_platform.h"

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

struct memory_arena
{
	memory_index Size;
	uint8 *Base;
	memory_index Used;
};

#define PushStruct(Arena, type) (type *)_PushSize(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)_PushSize(Arena, Count*sizeof(type))
void *
_PushSize(memory_arena *Arena, memory_index Size)
{
	Assert((Arena->Used + Size) <= Arena->Size);
	void *Result = Arena->Base + Arena->Used;
	Arena->Used += Size;

	return Result;
}

internal void
InitializeArena(memory_arena *Arena, memory_index Size, uint8 *Base)
{
	Arena->Size = Size;
	Arena->Base = Base;
	Arena->Used = 0;
}

#include "handmade_math.h"
#include "handmade_intrinsics.h"
#include "handmade_world.h"

struct loaded_bitmap
{
	int32 Width;
	int32 Height;
	uint32 *Pixels;
};

struct hero_bitmaps
{
	vec2 Align;
	loaded_bitmap Head;
	loaded_bitmap Cape;
	loaded_bitmap Torso; 
};

struct high_entity
{
	vec2 P;
	vec2 dP;
	uint32 ChunkZ;
	uint32 FacingDirection;

	real32 tBob;
	
	real32 Z;
	real32 dZ;

	uint32 LowEntityIndex;
};

enum entity_type
{
	EntityType_Null,
	EntityType_Hero,
	EntityType_Wall,
	EntityType_Monster,
	EntityType_Familiar,
};

struct low_entity
{
	entity_type Type;

	world_position P;
	real32 Width, Height;

	bool32 Collides;
	int32 dAbsTileZ;

	uint32 HighEntityIndex;
};

struct entity
{
	uint32 LowIndex;
	low_entity *Low;
	high_entity *High;
};

struct entity_visible_piece
{
	loaded_bitmap *Bitmap;
	vec2 Offset;
	real32 OffsetZ;
	real32 Alpha;
};

struct entity_visible_piece_group
{
	uint32 PieceCount;
	entity_visible_piece Pieces[8];
};

struct game_state
{
	memory_arena WorldArena;
	world *World;
	
	uint32 CameraFollowingEntityIndex;
	world_position CameraP;
	
	uint32 PlayerIndexForController[ArrayCount(((game_input *)0)->Controllers)];
	
	uint32 LowEntityCount;
	low_entity LowEntities[100000];

	uint32 HighEntityCount;
	high_entity _HighEntities[256];
	
	loaded_bitmap Backdrop;
	loaded_bitmap Shadow;
	hero_bitmaps HeroBitmaps[4];

	loaded_bitmap Tree;
};

#define HANDMADE_H
#endif
