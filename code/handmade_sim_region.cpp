internal sim_entity *
AddEntity(sim_region *SimRegion)
{
	sim_entity *Entity = 0;

	if (SimRegion->EntityCount < SimRegion->MaxEntityCount)
	{
		Entity = SimRegion->Entities + SimRegion->EntityCount++;
		*Entity = {};
	}
	else
	{
		InvalidCodePath;
	}

	return Entity;
}

inline vec2
GetSimSpaceP(sim_region *Region, low_entity *Stored)
{
	world_difference Diff = Subtract(&Region->World, Stored->P, Region->Origin);
	vec2 Result = Diff.dXY;

	return Result;
}

internal sim_entity *
AddEntity(sim_region *SimRegion, low_entity *Source, vec2 *SimP)
{
	sim_entity *Dest = AddEntity(SimRegion);
	if (Dest)
	{
		if (SimP)
		{
			Dest->P = *SimP;
		}
		else
		{
			Dest->P = GetSimSpaceP(SimRegion, Source);
		}
	}

	return Dest;
}

internal sim_region *
BeginSim(memory_arean *Arena, game_state *GameState, world *World, world_position Origin, rectangle2 Bounds)
{
	sim_region *SimRegion = PushStruct(Arena, sim_region);

	SimRegion->World = World;
	SimRegion->Origin = Origin;
	SimRegion->Bounds = Bounds;

	SimRegion->MaxEntityCount = 4096;
	SimRegion->EntityCount = 0;
	SimRegion->Entities = PushArray(Arena, SimRegion->MaxEntityCount, sim_entity);

	world_position MinChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMinCorner(SimRegion->Bounds));
	world_position MaxChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMaxCorner(SimRegion->Bounds));
	
	for (int32 ChunkY = MinChunkP.ChunkY; ChunkY <= MaxChunkP.ChunkY; ++ChunkY)
	{
		for (int32 ChunkX = MinChunkP.ChunkX; ChunkX <= MaxChunkP.ChunkX; ++ChunkX)
		{
			world_chunk *Chunk = GetWorldChunk(World, ChunkX, ChunkY, SimRegion->Origin.ChunkZ);
			if (Chunk)
			{
				world_entity_block *FirstBlock = &Chunk->FirstBlock;
				for (world_entity_block *Block = FirstBlock; Block; Block = Block->Next)
				{
					for (uint32 EntityIndexIndex = 0; EntityIndexIndex < Block->EntityCount; ++EntityIndexIndex)
					{
						uint32 LowEntityIndex = Block->LowEntityIndex[EntityIndexIndex];
						low_entity *Low = GameState->LowEntities + LowEntityIndex;
						vec2 SimSpaceP = GetSimSpaceP(GameState, Low);
						if (IsInRectangle(SimRegion->Bounds, SimSpaceP))
						{
							AddEntity(SimRegion, Low, &SimSpace);
						}
					}					
				}
			}
		}
	}
}

internal void
EndSim(sim_region *Region, game_state *GameState)
{
	sim_entity *Entity = Region->Entities;
	for (uint32 EntityIndex = 0; EntityIndex < Region->EntityCount: ++EntityIndex)
	{
		low_entity *Stored = GameState->LowEntities + Entity->StorageIndex;

		world_position NewP = MapIndexChunkSpace(GameState->World, Region->Origin, Entity->P);
		ChangeEntityLocation(&GameState->WorldArena, GameState->World, Entity->StorageIndex,
							 Stored, &Stored->P. &NewP);
		
		entity CameraFollowingEntity = ForceEntityIntoHigh(GameState, GameState->CameraFollowingEntityIndex);
		if (CameraFollowingEntity.High)
		{
			world_position NewCameraP = GameState->CameraP;

			NewCameraP.ChunkZ = CameraFollowingEntity.Low->P.ChunkZ;

#if 0 
			if (CameraFollowingEntity.High->P.X > (9.0f*World->TileSideInMeters))
			{
				NewCameraP.AbsTileX += 17;
			}
			if (CameraFollowingEntity.High->P.X < -(9.0f*World->TileSideInMeters))
			{
				NewCameraP.AbsTileX -= 17;
			}
			if (CameraFollowingEntity.High->P.Y > (5.0f*World->TileSideInMeters))
			{
				NewCameraP.AbsTileY += 9;
			}
			if (CameraFollowingEntity.High->P.Y < -(5.0f*World->TileSideInMeters))
			{
				NewCameraP.AbsTileY -= 9;
			}
#else
			NewCameraP = CameraFollowingEntity.Low->P;
#endif
		
			SetCamera(GameState, NewCameraP);
		}		
	}
}
