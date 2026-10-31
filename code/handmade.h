#if !defined(HANDMADE_H)

#include "handmade_platform.h"

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

struct memory_arena
{
	memory_index Size;
	uint8 *Base;
	memory_index Used;

	int32 TempCount;
};

struct temporary_memory
{
	memory_arena *Arena;
	memory_index Used;
};

inline void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
	Arena->Size = Size;
	Arena->Base = (uint8 *)Base;
	Arena->Used = 0;
	Arena->TempCount = 0;
}

inline memory_index
GetAlignmentOffset(memory_arena *Arena, memory_index Alignment)
{
	memory_index AlignmentOffset = 0;

	memory_index ResultPointer = (memory_index)Arena->Base + Arena->Used;
	memory_index AlignmentMask = Alignment - 1;
	if (ResultPointer & AlignmentMask)
	{
		AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);
	}

	return AlignmentOffset;
}

inline memory_index
GetArenaSizeRemaining(memory_arena *Arena, memory_index Alignment=4)
{
	memory_index Result = Arena->Size - (Arena->Used + GetAlignmentOffset(Arena, Alignment));

	return Result;
}

#define PushStruct(Arena, type, ...) (type *)_PushSize(Arena, sizeof(type), ## __VA_ARGS__)
#define PushArray(Arena, Count, type, ...) (type *)_PushSize(Arena, Count*sizeof(type), ## __VA_ARGS__)
#define PushSize(Arena, Size, ...) _PushSize(Arena, Size, ## __VA_ARGS__)
inline void *
_PushSize(memory_arena *Arena, memory_index SizeInit, memory_index Alignment=4)
{
	memory_index Size = SizeInit;

	memory_index AlignmentOffset = GetAlignmentOffset(Arena, Alignment);
	Size += AlignmentOffset;

	Assert((Arena->Used + Size) <= Arena->Size);
	void *Result = Arena->Base + Arena->Used + AlignmentOffset;
	Arena->Used += Size;

	Assert(Size >= SizeInit);

	return Result;
}

inline void
SubArena(memory_arena *Result, memory_arena *Arena, memory_index Size, memory_index Alignment=16)
{
	Result->Size = Size;
	Result->Base = (uint8 *)PushSize(Arena, Size, Alignment);
	Result->Used = 0;
	Result->TempCount = 0;
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

inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
	temporary_memory Result;
	Result.Arena = Arena;
	Result.Used = Arena->Used;

	++Arena->TempCount;
	
	return Result;
}

inline void
EndTemporaryMemory(temporary_memory TempMem)
{
	memory_arena *Arena = TempMem.Arena;
	Assert(Arena->Used >= TempMem.Used);
	Arena->Used = TempMem.Used;
	Assert(Arena->TempCount > 0);
	--Arena->TempCount;
}

inline void
CheckArena(memory_arena *Arena)
{
	Assert(Arena->TempCount == 0);
}

#include "handmade_intrinsics.h"
#include "handmade_math.h"
#include "handmade_world.h"
#include "handmade_sim_region.h"
#include "handmade_entity.h"
#include "handmade_render_group.h"

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

struct pairwise_collision_rule
{
	bool32 CanCollide;
	uint32 StorageIndexA;
	uint32 StorageIndexB;

	pairwise_collision_rule *NextInHash;
};

struct ground_buffer
{
	world_position P; // NOTE: invalid P means hasnt been filled, valid P is center of bitmap
	loaded_bitmap Bitmap;
};

enum game_asset_id
{
	GAI_Backdrop,
	GAI_Shadow,
	GAI_Tree,
	GAI_Sword,
	GAI_Stairwell,

	GAI_Count,
};

enum asset_state
{
	AssetState_Unloaded,
	AssetState_Queued,
	AssetState_Loaded,
};
struct asset_handle
{
	asset_state State;
	loaded_bitmap *Bitmap;
};

struct game_assets
{
	struct transient_state *TranState;
	memory_arena Arena;
	debug_platform_read_entire_file *ReadEntireFile;

	loaded_bitmap *Bitmaps[GAI_Count];

	loaded_bitmap Grass[2];
	loaded_bitmap Tuft[3];
	loaded_bitmap Stone[4];

	hero_bitmaps HeroBitmaps[4];
};
inline loaded_bitmap *
GetBitmap(game_assets *Assets, game_asset_id ID)
{
	loaded_bitmap *Bitmap = Assets->Bitmaps[ID];

	return Bitmap;
}

struct game_state
{
	memory_arena WorldArena;	
	world *World;
	
	real32 TypicalFloorHeight;
	
	uint32 CameraFollowingEntityIndex;
	world_position CameraP;
	
	controlled_hero ControlledHeros[ArrayCount(((game_input *)0)->Controllers)];
	
	uint32 LowEntityCount;
	low_entity LowEntities[100000];
	
	pairwise_collision_rule *CollisionRuleHash[256];
	pairwise_collision_rule *FirstFreeCollisionRule;

	sim_entity_collision_volume_group *NullCollision;
	sim_entity_collision_volume_group *SwordCollision;
	sim_entity_collision_volume_group *StairCollision;
	sim_entity_collision_volume_group *PlayerCollision;
	sim_entity_collision_volume_group *MonsterCollision;
	sim_entity_collision_volume_group *FamiliarCollision;
	sim_entity_collision_volume_group *WallCollision;
	sim_entity_collision_volume_group *StandardRoomCollision;

	real32 Time;
	real32 ZOffset;
	
	loaded_bitmap TestDiffuse;
	loaded_bitmap TestNormal;
};

struct task_with_memory
{
	bool32 BeginUsed;
	memory_arena Arena;

	temporary_memory MemoryFlush;
};

struct transient_state
{
	bool32 IsInitialized;
	memory_arena TranArena;

	task_with_memory Tasks[4];
	
	uint32 GroundBufferCount;
	ground_buffer *GroundBuffers;
	platform_work_queue *HighPriorityQueue;
	platform_work_queue *LowPriorityQueue;
	
	uint32 EnvMapWidth;
	uint32 EnvMapHeight;
	environment_map EnvMaps[3];

	game_assets Assets;
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

internal void AddCollisionRule(game_state *GameState, uint32 StorageIndexA, uint32 StorageIndexB, bool32 ShouldCollide);
internal void ClearCollisionRulesFor(game_state *GameState, uint32 StorageIndex);

internal void LoadAsset(game_assets *Assets, game_asset_id ID);

global_variable platform_add_entry *PlatformAddEntry;
global_variable platform_complete_all_work *PlatformCompleteAllWork;

#define HANDMADE_H
#endif
