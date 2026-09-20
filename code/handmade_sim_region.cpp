internal sim_entity_hash *
GetHashFromStorageIndex(sim_region *SimRegion, uint32 StorageIndex)
{
	Assert(StorageIndex);

	sim_entity_hash *Result = 0;

	uint32 HashValue = StorageIndex;
	for (uint32 Offset = 0; Offset < ArrayCount(SimRegion->Hash); ++Offset)
	{
		uint32 HashMask = ArrayCount(SimRegion->Hash) - 1;
		uint32 HashIndex = (HashValue + Offset) & HashMask;
		sim_entity_hash *Entry = SimRegion->Hash + HashIndex;
		if ((Entry->Index == 0) || (Entry->Index == StorageIndex))
		{
			Result = Entry;
			break;
		}
	}

	return Result;
}

inline sim_entity *
GetEntityByStorageIndex(sim_region *SimRegion, uint32 StorageIndex)
{
	sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, StorageIndex);
	sim_entity *Result = Entry->Ptr;
	return Result;
}

inline vec3
GetSimSpaceP(sim_region *Region, low_entity *Stored)
{
	vec3 Result = InvalidP;
	if (!IsSet(&Stored->Sim, EntityFlag_Nonspatial))
	{
		Result = Subtract(Region->World, &Stored->P, &Region->Origin);
	}
	
	return Result;
}

internal sim_entity *
AddEntity(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source, vec3 *SimP);
inline void
LoadEntityReference(game_state *GameState, sim_region *SimRegion, entity_reference *Ref)
{
	if (Ref->Index)
	{
		sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, Ref->Index);
		if (Entry->Ptr == 0)
		{
			Entry->Index = Ref->Index;
			low_entity *LowEntity = GetLowEntity(GameState, Ref->Index);
			vec3 P = GetSimSpaceP(SimRegion, LowEntity);
			Entry->Ptr = AddEntity(GameState, SimRegion, Ref->Index, LowEntity, &P);
		}

		Ref->Ptr = Entry->Ptr;
	}
}

inline void
StoreEntityReference(entity_reference *Ref)
{
	if (Ref->Ptr != 0)
	{
		Ref->Index = Ref->Ptr->StorageIndex;
	}
}

internal sim_entity *
AddEntityRaw(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source)
{
	Assert(StorageIndex);
	sim_entity *Entity = 0;
	
	sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, StorageIndex);
	if (Entry->Ptr == 0)
	{
		if (SimRegion->EntityCount < SimRegion->MaxEntityCount)
		{
			Entity = SimRegion->Entities + SimRegion->EntityCount++;

			Entry->Index = StorageIndex;
			Entry->Ptr = Entity;

			if (Source)
			{
				*Entity = Source->Sim;
				LoadEntityReference(GameState, SimRegion, &Entity->Sword);

				Assert(!IsSet(&Source->Sim, EntityFlag_Simming));
				AddFlags(&Source->Sim, EntityFlag_Simming);
			}

			Entity->StorageIndex = StorageIndex;
			Entity->Updatable = false;
		}
		else
		{
			InvalidCodePath;
		}
	}

	return Entity;
}

inline bool32
EntityOverlapsRectangle(vec3 P, sim_entity_collision_volume Volume, rectangle3 Rect)
{
	rectangle3 Grown = AddRadiusTo(Rect, 0.5f*Volume.Dim);
	bool32 Result = IsInRectangle(Grown, P + Volume.OffsetP);

	return Result;
}

internal sim_entity * //NOTE: only set updatable when give SimP
AddEntity(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source, vec3 *SimP)
{
	sim_entity *Dest = AddEntityRaw(GameState, SimRegion, StorageIndex, Source);
	if (Dest)
	{
		if (SimP)
		{
			Dest->P = *SimP;
			Dest->Updatable = EntityOverlapsRectangle(Dest->P, Dest->Collision->TotalVolume, SimRegion->UpdatableBounds);
		}
		else
		{
			Dest->P = GetSimSpaceP(SimRegion, Source);
		}
	}

	return Dest;
}

internal sim_region *
BeginSim(memory_arena *SimArena, game_state *GameState, world *World, world_position Origin, rectangle3 Bounds, real32 dt)
{
	sim_region *SimRegion = PushStruct(SimArena, sim_region);
	ZeroStruct(SimRegion->Hash);
	
	SimRegion->MaxEntityRadius = 5.0f;
	SimRegion->MaxEntityVelocity = 30.0f;
	real32 UpdateSafetyMargin = SimRegion->MaxEntityRadius + dt*SimRegion->MaxEntityVelocity;
	real32 UpdateSafetyMarginZ = 1.0f;

	SimRegion->World = World;
	SimRegion->Origin = Origin;
	SimRegion->UpdatableBounds = AddRadiusTo(Bounds, Vec3(SimRegion->MaxEntityRadius,
														 SimRegion->MaxEntityRadius,
														 SimRegion->MaxEntityRadius));
	SimRegion->Bounds = AddRadiusTo(SimRegion->UpdatableBounds,
									Vec3(UpdateSafetyMargin, UpdateSafetyMargin, UpdateSafetyMarginZ));

	SimRegion->MaxEntityCount = 4096;
	SimRegion->EntityCount = 0;
	SimRegion->Entities = PushArray(SimArena, SimRegion->MaxEntityCount, sim_entity);

	world_position MinChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMinCorner(SimRegion->Bounds));
	world_position MaxChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMaxCorner(SimRegion->Bounds));
	
	for (int32 ChunkZ = MinChunkP.ChunkZ; ChunkZ <= MaxChunkP.ChunkZ; ++ChunkZ)
	{
		for (int32 ChunkY = MinChunkP.ChunkY; ChunkY <= MaxChunkP.ChunkY; ++ChunkY)
		{
			for (int32 ChunkX = MinChunkP.ChunkX; ChunkX <= MaxChunkP.ChunkX; ++ChunkX)
			{
				world_chunk *Chunk = GetWorldChunk(World, ChunkX, ChunkY, ChunkZ);
				if (Chunk)
				{
					world_entity_block *FirstBlock = &Chunk->FirstBlock;
					for (world_entity_block *Block = FirstBlock; Block; Block = Block->Next)
					{
						for (uint32 EntityIndexIndex = 0; EntityIndexIndex < Block->EntityCount; ++EntityIndexIndex)
						{
							uint32 LowEntityIndex = Block->LowEntityIndex[EntityIndexIndex];
							low_entity *Low = GameState->LowEntities + LowEntityIndex;
							if (!IsSet(&Low->Sim, EntityFlag_Nonspatial))
							{
								vec3 SimSpaceP = GetSimSpaceP(SimRegion, Low);
								if (EntityOverlapsRectangle(SimSpaceP, Low->Sim.Collision->TotalVolume, SimRegion->Bounds))
								{
									AddEntity(GameState, SimRegion, LowEntityIndex, Low, &SimSpaceP);
								}
							}
						}					
					}
				}
			}
		}
	}

	return SimRegion;
}

internal void
EndSim(sim_region *Region, game_state *GameState)
{
	sim_entity *Entity = Region->Entities;
	for (uint32 EntityIndex = 0; EntityIndex < Region->EntityCount;
		 ++EntityIndex, ++Entity)
	{
		low_entity *Stored = GameState->LowEntities + Entity->StorageIndex;

		Assert(IsSet(&Stored->Sim, EntityFlag_Simming));
		Stored->Sim = *Entity;
		Assert(!IsSet(&Stored->Sim, EntityFlag_Simming));

		StoreEntityReference(&Stored->Sim.Sword);
		
		world_position NewP = IsSet(Entity, EntityFlag_Nonspatial) ?
			                      NullPosition() :
			                      MapIntoChunkSpace(GameState->World, Region->Origin, Entity->P);		
		ChangeEntityLocation(&GameState->WorldArena, GameState->World, Entity->StorageIndex,
							 Stored, NewP);
				
		if (Entity->StorageIndex == GameState->CameraFollowingEntityIndex)
		{
			world_position NewCameraP = GameState->CameraP;

			NewCameraP.ChunkZ = Stored->P.ChunkZ;

#if 0 
			if (CameraFollowingEntity.High->P.x > (9.0f*World->TileSideInMeters))
			{
				NewCameraP.AbsTileX += 17;
			}
			if (CameraFollowingEntity.High->P.x < -(9.0f*World->TileSideInMeters))
			{
				NewCameraP.AbsTileX -= 17;
			}
			if (CameraFollowingEntity.High->P.y > (5.0f*World->TileSideInMeters))
			{
				NewCameraP.AbsTileY += 9;
			}
			if (CameraFollowingEntity.High->P.y < -(5.0f*World->TileSideInMeters))
			{
				NewCameraP.AbsTileY -= 9;
			}
#else
			real32 CamZOffset = NewCameraP._Offset.z;
			NewCameraP = Stored->P;
			NewCameraP._Offset.z = CamZOffset;
#endif
			GameState->CameraP = NewCameraP;
		}		
	}
}

struct test_wall
{
	real32 X;
	real32 RelX;
	real32 RelY;
	real32 DeltaX;
	real32 DeltaY;	
	real32 MinY;
	real32 MaxY;
	vec3 Normal;
};

internal bool32
TestWall(real32 WallX, real32 RelX, real32 RelY, real32 PlayerDeltaX, real32 PlayerDeltaY,
		 real32 *tMin, real32 MinY, real32 MaxY)
{
	bool32 Hit = false;
	
	real32 tEpsilon = 0.001f; // casey sets to 0.00001f;
	if (PlayerDeltaX != 0.0f)
	{
		real32 tResult = (WallX - RelX) / PlayerDeltaX;
		real32 Y = RelY + PlayerDeltaY*tResult;
		if ((tResult >= 0.0f) && (tResult < *tMin))
		{
			if ((Y >= MinY) && (Y <= MaxY))
			{
				*tMin = Maximum(0.0f, tResult - tEpsilon);
				Hit = true;
			}
		}
	}

	return Hit;
}

internal bool32
CanCollide(game_state *GameState, sim_entity *A, sim_entity *B)
{
	bool32 Result = false;

	if (A != B)
	{
		if (A->StorageIndex > B->StorageIndex)
		{
			sim_entity *Temp = A;
			A = B;
			B = Temp;
		}

		if (IsSet(A, EntityFlag_Collides) && IsSet(B, EntityFlag_Collides))
		{
			if ((!IsSet(A, EntityFlag_Nonspatial)) &&
				(!IsSet(B, EntityFlag_Nonspatial)))
			{
				Result = true;
			}

			uint32 HashBucket = A->StorageIndex & (ArrayCount(GameState->CollisionRuleHash) - 1);
			for (pairwise_collision_rule *Rule = GameState->CollisionRuleHash[HashBucket];
				 Rule;
				 Rule = Rule->NextInHash)
			{
				if ((Rule->StorageIndexA == A->StorageIndex) &&
					(Rule->StorageIndexB == B->StorageIndex))
				{
					Result = Rule->CanCollide;
					break;
				}
			}			
		}
	}

	return Result;
}	   

internal bool32
HandleCollision(game_state *GameState, sim_entity *A, sim_entity *B)
{
	bool32 StopsOnCollision = false;

	if (A->Type == EntityType_Sword)
	{
		AddCollisionRule(GameState, A->StorageIndex, B->StorageIndex, false);
		StopsOnCollision = false;
	}
	else
	{
		StopsOnCollision = true;
	}
	
	if (A->Type > B->Type)
	{
		sim_entity *Temp = A;
		A = B;
		B = Temp;
	}

	if ((A->Type == EntityType_Monster) &&
		(B->Type == EntityType_Sword))
	{
		if (A->HitPointMax > 0)
		{
			--A->HitPointMax;
		}
	}

	return StopsOnCollision;
}

internal bool32
CanOverlap(game_state *GameState, sim_entity *Mover, sim_entity *Region)
{
	bool32 Result = false;

	if (Mover != Region)
	{
		if (Region->Type == EntityType_Stairwell)
		{
			Result = true;
		}
	}
	
	return Result;
}	   

internal void
HandleOverlap(game_state *GameState, sim_entity *Mover, sim_entity *Region, real32 dt, real32 *Ground)	
{
	if (Region->Type == EntityType_Stairwell)
	{
		*Ground = GetStairGround(Region, GetEntityGroundPoint(Mover));
	}
}

internal bool32
SpeculativeCollide(sim_entity *Mover, sim_entity *Region, vec3 TestP)
{
	bool32 Result = true;
	if (Region->Type == EntityType_Stairwell)
	{
		real32 StepHeight = 0.1f;
#if 0
		Result = ((AbsoluteValue(GetEntityGroundPoint(Mover).z - Ground) > StepHeight) ||
				  ((Bary.y > 0.1f) && (Bary.y < 0.9f)));
#endif
		vec3 MoverGroundPoint = GetEntityGroundPoint(Mover, TestP);
		real32 Ground = GetStairGround(Region, MoverGroundPoint);
		Result = (AbsoluteValue(MoverGroundPoint.z - Ground) > StepHeight);		
	}

	return Result;
}

internal bool32
EntitiesOverlap(sim_entity *Entity, sim_entity *TestEntity, vec3 Epsilon = Vec3(0, 0, 0))
{
	bool32 Result = false;
	for (uint32 VolumeIndex = 0;
		 VolumeIndex < Entity->Collision->VolumeCount;
		 ++VolumeIndex)
	{
		sim_entity_collision_volume *EntityVolume = Entity->Collision->Volumes + VolumeIndex;
							
		for (uint32 TestVolumeIndex = 0;
			 TestVolumeIndex < TestEntity->Collision->VolumeCount;
			 ++TestVolumeIndex)
		{
			sim_entity_collision_volume *TestVolume = TestEntity->Collision->Volumes + TestVolumeIndex;

			rectangle3 EntityRect = RectCenterDim(Entity->P + EntityVolume->OffsetP, EntityVolume->Dim + Epsilon);
			rectangle3 TestRect = RectCenterDim(TestEntity->P + TestVolume->OffsetP, TestVolume->Dim);
			Result = RectanglesIntersect(EntityRect, TestRect);
		}
	}

	return Result;
}

internal void
MoveEntity(game_state *GameState, sim_region *SimRegion, sim_entity *Entity, real32 dt, move_spec *MoveSpec, vec3 ddP)
{
	Assert(!IsSet(Entity, EntityFlag_Nonspatial));
	
	world *World = SimRegion->World;

	if (MoveSpec->UnitMaxAccelVector)
	{
		real32 ddPLength = LengthSq(ddP);
		if (ddPLength > 1.0f)
		{
			ddP *= (1.0f / SquareRoot(ddPLength));
		}
	}
	
	ddP *= MoveSpec->Speed;

	vec3 Drag = -MoveSpec->Drag*Entity->dP;
	Drag.z = 0;
	ddP += Drag;
	
	if (!IsSet(Entity, EntityFlag_ZSupported))
	{
		ddP += Vec3(0, 0, -9.8f);
	}
	
	vec3 OldPlayerP = Entity->P;
	vec3 PlayerDelta = (0.5f*ddP*Square(dt) + Entity->dP*dt);
	Entity->dP = Entity->dP + ddP*dt;			

	Assert(LengthSq(Entity->dP) <= Square(SimRegion->MaxEntityVelocity));	
	vec3 NewPlayerP = OldPlayerP + PlayerDelta;				
		
	real32 DistanceRemaining = Entity->DistanceLimit;
	if (DistanceRemaining == 0.0f)
	{
		DistanceRemaining = 10000.0f;
	}

	for (uint32 Iteration = 0; Iteration < 4; ++Iteration)
	{
		real32 tMin = 1.0f;
		real32 tMax = 0.0f;
		
		real32 PlayerDeltaLength = Length(PlayerDelta);
		if (PlayerDeltaLength > 0.0f)
		{
			if (PlayerDeltaLength > DistanceRemaining)
			{
				tMin = DistanceRemaining / PlayerDeltaLength;
			}
			
			vec3 WallNormalMin = {};
			vec3 WallNormalMax = {};
			sim_entity *HitEntityMin = 0;
			sim_entity *HitEntityMax = 0;

			vec3 DesiredPosition = Entity->P + PlayerDelta;

			if (!IsSet(Entity, EntityFlag_Nonspatial))
			{
				for (uint32 TestHighEntityIndex = 0; TestHighEntityIndex < SimRegion->EntityCount; ++TestHighEntityIndex)
				{
					sim_entity *TestEntity = SimRegion->Entities + TestHighEntityIndex;
					real32 OverlapEpsilon = 0.001f;
					   
					if ((IsSet(TestEntity, EntityFlag_Traversable) &&
						 EntitiesOverlap(Entity, TestEntity, OverlapEpsilon*Vec3(1, 1, 1))) ||
						CanCollide(GameState, Entity, TestEntity))
					{
						for (uint32 VolumeIndex = 0;
							 VolumeIndex < Entity->Collision->VolumeCount;
							 ++VolumeIndex)
						{
							sim_entity_collision_volume *EntityVolume = Entity->Collision->Volumes + VolumeIndex;
							
							for (uint32 TestVolumeIndex = 0;
								 TestVolumeIndex < TestEntity->Collision->VolumeCount;
								 ++TestVolumeIndex)
							{
								sim_entity_collision_volume *TestVolume = TestEntity->Collision->Volumes + TestVolumeIndex;

								vec3 MinkowskiDiameter = {TestVolume->Dim.x + EntityVolume->Dim.x,
									                      TestVolume->Dim.y + EntityVolume->Dim.y,
									                      TestVolume->Dim.z + EntityVolume->Dim.z};
				
								vec3 MinCorner = -0.5f*MinkowskiDiameter;
								vec3 MaxCorner = 0.5f*MinkowskiDiameter;

								vec3 Rel = (Entity->P + EntityVolume->OffsetP)
									       - (TestEntity->P + TestVolume->OffsetP);
								
								if ((Rel.z >= MinCorner.z) && (Rel.z < MaxCorner.z))
								{
									test_wall Walls[] = {
										{MinCorner.x, Rel.x, Rel.y, PlayerDelta.x, PlayerDelta.y, MinCorner.y, MaxCorner.y, Vec3(-1.0f, 0, 0)},
										{MaxCorner.x, Rel.x, Rel.y, PlayerDelta.x, PlayerDelta.y, MinCorner.y, MaxCorner.y, Vec3(1.0f, 0, 0)},
										{MinCorner.y, Rel.y, Rel.x, PlayerDelta.y, PlayerDelta.x, MinCorner.x, MaxCorner.x, Vec3(0, -1.0f, 0)},
										{MaxCorner.y, Rel.y, Rel.x, PlayerDelta.y, PlayerDelta.x, MinCorner.x, MaxCorner.x, Vec3(0, 1.0f, 0)}
									};
									
									if (IsSet(TestEntity, EntityFlag_Traversable))
									{
										real32 tMaxTest = tMax;
										vec3 TestWallNormal = {};

										bool32 HitThis = false;									
										for (uint32 WallIndex = 0; WallIndex < ArrayCount(Walls); ++WallIndex)
										{
											test_wall *Wall = Walls + WallIndex;
											real32 tEpsilon = 0.001f;
											if (Wall->DeltaX != 0)
											{
												real32 tResult = (Wall->X - Wall->RelX) / Wall->DeltaX;
												real32 Y = Wall->RelY + Wall->DeltaY*tResult;
												if ((tResult >= 0.0f) && (tResult > tMaxTest))
												{
													if ((Y >= Wall->MinY) && (Y <= Wall->MaxY))
													{
														tMaxTest = Maximum(0.0f, tResult - tEpsilon);
														TestWallNormal = Wall->Normal;
														HitThis = true;
													}
												}											
											}
										}

										if (HitThis)
										{
											tMax = tMaxTest;
											WallNormalMax = TestWallNormal;
											HitEntityMax = TestEntity;
										}
									}
									else
									{
										real32 tMinTest = tMin;
										vec3 TestWallNormal = {};

										bool32 HitThis = false;									
										for (uint32 WallIndex = 0; WallIndex < ArrayCount(Walls); ++WallIndex)
										{
											test_wall *Wall = Walls + WallIndex;
											real32 tEpsilon = 0.001f;
											if (Wall->DeltaX != 0)
											{
												real32 tResult = (Wall->X - Wall->RelX) / Wall->DeltaX;
												real32 Y = Wall->RelY + Wall->DeltaY*tResult;
												if ((tResult >= 0.0f) && (tResult < tMinTest))
												{
													if ((Y >= Wall->MinY) && (Y <= Wall->MaxY))
													{
														tMinTest = Maximum(0.0f, tResult - tEpsilon);
														TestWallNormal = Wall->Normal;
														HitThis = true;
													}
												}											
											}
										}

										if (HitThis)
										{
											vec3 TestP = Entity->P + tMinTest*PlayerDelta;
											if (SpeculativeCollide(Entity, TestEntity, TestP))
											{
												tMin = tMinTest;
												WallNormalMin = TestWallNormal;
												HitEntityMin = TestEntity;
											}
										}										
									}
								}
							}
						}
					}
				}
			}

			vec3 WallNormal;
			sim_entity *HitEntity;
			real32 tStop;			
			if (tMin < tMax)
			{
				tStop = tMin;
				WallNormal = WallNormalMin;
				HitEntity = HitEntityMin;
			}
			else
			{
				tStop = tMax;
				WallNormal = WallNormalMax;
				HitEntity = HitEntityMax;
			}
			
			Entity->P += tStop*PlayerDelta;
			DistanceRemaining -= tStop*PlayerDeltaLength;
			if (HitEntity)
			{
				PlayerDelta = DesiredPosition - Entity->P;
				
				bool32 StopsOnCollision = HandleCollision(GameState, Entity, HitEntity);
				if (StopsOnCollision)
				{
					PlayerDelta = PlayerDelta - 1*Inner(PlayerDelta, WallNormal)*WallNormal;
					Entity->dP = Entity->dP - 1*Inner(Entity->dP, WallNormal)*WallNormal;
				}
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	real32 Ground = 0.0f;
	
	{
		for (uint32 TestHighEntityIndex = 0; TestHighEntityIndex < SimRegion->EntityCount; ++TestHighEntityIndex)
		{
			sim_entity *TestEntity = SimRegion->Entities + TestHighEntityIndex;
			if (CanOverlap(GameState, Entity, TestEntity) &&
				EntitiesOverlap(Entity, TestEntity))
			{
				HandleOverlap(GameState, Entity, TestEntity, dt, &Ground);
			}
		}
	}

	Ground += Entity->P.z - GetEntityGroundPoint(Entity).z;
    if((Entity->P.z <= Ground) ||
	   (IsSet(Entity, EntityFlag_ZSupported) &&
		Entity->dP.z == 0))
    {
        Entity->P.z = Ground;
		Entity->dP.z = 0;
		AddFlags(Entity, EntityFlag_ZSupported);
    }
	else
	{
		ClearFlags(Entity, EntityFlag_ZSupported);
	}

	if (Entity->DistanceLimit != 0.0f)
	{
		Entity->DistanceLimit = DistanceRemaining;
	}
	
	if ((Entity->dP.x == 0.0f) && (Entity->dP.y == 0.0f))
	{
		// Leave it unchanged
	}
	else if (AbsoluteValue(Entity->dP.x) > AbsoluteValue(Entity->dP.y))
	{
		if (Entity->dP.x > 0)
		{
			Entity->FacingDirection = 0;
		}
		else
		{
			Entity->FacingDirection = 2;
		}
	}
	else
	{
		if (Entity->dP.y > 0)
		{
			Entity->FacingDirection = 1;	
		}
		else
		{
			Entity->FacingDirection = 3;
		}
	}
}
