#if !defined(HANDMADE_H)

/*
 * HANDMADE_INTERNAL:
 *	0 - Build for public release
 *	1 = Build for developer only
 * 
 * HANDMADE_SLOW:
 *	0 - No slow code allowed
 *	1 - Slow code allowed
 *
 */

#include "handmade_platform.h"

struct memory_arena
{
	memory_index Size;
	uint8 *Base;
	memory_index Used;
};

#define PushStruct(Arena, type) _PushSize(Arena, sizeof(type))
#define PushArray(Arena, Count, type) _PushSize(Arena, Count*sizeof(type))
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
#include "handmade_tile.h"

struct world
{
	tile_map *TileMap;
};

struct loaded_bitmap
{
	int32 Width;
	int32 Height;
	uint32 *Pixels;
};

struct hero_bitmaps
{
	int32 AlignX;
	int32 AlignY;
	loaded_bitmap Head;
	loaded_bitmap Cape;
	loaded_bitmap Torso; 
};

struct game_state
{
	memory_arena WorldArena;
	world *World;
	
	tile_map_position PlayerP;
	tile_map_position CameraP;
	vec2 dPlayerP;
	
	loaded_bitmap Backdrop;
	hero_bitmaps HeroBitmaps[4];
	int HeroFacingDirection;	
};

#define HANDMADE_H
#endif
