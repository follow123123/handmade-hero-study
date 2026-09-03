inline move_spec
DefaultMoveSpec()
{
	move_spec Result;
	Result.UnitMaxAccelVector = false;
	Result.Speed = 1.0f;
	Result.Drag = 0.0;
	
	return Result;
}

inline void
UpdateFamiliar(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{
	sim_entity *ClosestHero = 0;
	real32 ClosestHeroDSq = Square(10.0f);

	sim_entity *TestEntity = SimRegion->Entities;
	for (uint32 TestEntityIndex = 0; TestEntityIndex < SimRegion->EntityCount; ++TestEntityIndex)
	{
		if (TestEntity->Type == EntityType_Hero)
		{
			real32 TestDSq = LengthSq(TestEntity->P - Entity->P);
			if (TestEntity->Type == EntityType_Hero)
			{
				TestDSq *= 0.75f;
			}
			
			if (TestDSq < ClosestHeroDSq)
			{
				ClosestHero = TestEntity;
				ClosestHeroDSq = TestDSq;
			}
		}
	}

	vec2 ddP = {};
	if (ClosestHero && (ClosestHeroDSq > Square(3.0f)))
	{
		real32 Acceleration = 0.5f;
		real32 OneOverLength = Acceleration / SquareRoot(ClosestHeroDSq);
		ddP = OneOverLength*(ClosestHero->P - Entity->P);
	}

	move_spec MoveSpec = DefaultMoveSpec();
	MoveSpec.UnitMaxAccelVector = true;
	MoveSpec.Speed = 50.0f;
	MoveSpec.Drag = 8.0f;
	MoveEntity(SimRegion, Entity, dt, &MoveSpec, ddP);
}

inline void
UpdateSword(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{
	move_spec MoveSpec = DefaultMoveSpec();
	MoveSpec.UnitMaxAccelVector = false;
	MoveSpec.Speed = 0.0f;
	MoveSpec.Drag = 0.0f;

	vec2 OldP = Entity->P;
	MoveEntity(SimRegion, Entity, dt, &MoveSpec, Vec2(0, 0));
	real32 DistanceTravelled = Length(Entity->P - OldP);

	Entity->DistanceRemaining -= DistanceTravelled;
	if (Entity->DistanceRemaining < 0.0f)
	{
		Assert(!"NEED TO MAKE ENTITIES BE ABLE TO NOT BE THERE!");
	}
}

inline void
UpdateMonster(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{
}
