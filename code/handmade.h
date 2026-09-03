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

internal void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
	Arena->Size = Size;
	Arena->Base = (uint8 *)Base;
	Arena->Used = 0;
}

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

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
inline void
ZeroSize(memory_index Size, void *Ptr)
{
	uint8 *Byte = (uint8 *)Ptr;
	while (Size--)
	{
		*Byte++ = 0;
	}
}

#include "handmade_intrinsics.h"
#include "handmade_math.h"
#include "handmade_world.h"
#include "handmade_sim_region.h"
#include "handmade_entity.h"

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

struct low_entity
{
	world_position P;
	sim_entity Sim;	
};

struct controlled_hero
{
	uint32 EntityIndex;

	vec2 ddP;
	vec2 dSword;
	real32 dZ;
};

struct game_state
{
	memory_arena WorldArena;
	world *World;
	real32 MetersToPixels;
	
	uint32 CameraFollowingEntityIndex;
	world_position CameraP;
	
	controlled_hero ControlledHeros[ArrayCount(((game_input *)0)->Controllers)];
	
	uint32 LowEntityCount;
	low_entity LowEntities[100000];
	
	loaded_bitmap Backdrop;
	loaded_bitmap Shadow;
	hero_bitmaps HeroBitmaps[4];

	loaded_bitmap Tree;
	loaded_bitmap Sword;
};

struct entity_visible_piece
{
	loaded_bitmap *Bitmap;
	vec2 Offset;
	real32 OffsetZ;
	real32 EntityZC;
	real32 R, G, B, A;
	vec2 Dim;
};

struct entity_visible_piece_group
{
	uint32 PieceCount;
	entity_visible_piece Pieces[8];

	game_state *GameState;
};

inline low_entity *
GetLowEntity(game_state *GameState, uint32 Index)
{
	low_entity *EntityLow = 0;

	if ((Index > 0) && (Index < GameState->LowEntityCount))
	{
		EntityLow = GameState->LowEntities + Index;
	}

	return EntityLow;
}

#define HANDMADE_H
#endif
