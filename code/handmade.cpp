#include "handmade.h"
#include "handmade_world.cpp"
#include "handmade_random.h"
#include "handmade_sim_region.cpp"
#include "handmade_entity.cpp"

internal void
GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{    
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

    int16 *SampleOut = SoundBuffer->Samples;
    for (int SampleIndex = 0;
		 SampleIndex < SoundBuffer->SampleCount;
		 ++SampleIndex)
    {
#if 0
		real32 SineValue = sinf(GameState->tSine);
		int16 SampleValue = (int16)(SineValue * ToneVolume);
#else
		int16 SampleValue = 0;
#endif
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;

#if 0
		GameState->tSine += 2.0f * Pi32 / (real32)WavePeriod;
		if (GameState->tSine > 2.0f * Pi32)
		{
			GameState->tSine -= 2.0f * Pi32;
		}
#endif
    }
}

internal void
DrawRectangle(game_offscreen_buffer *Buffer,
			  vec2 vMin, vec2 vMax,
			  real32 R, real32 G, real32 B)
{
    int MinX = RoundReal32ToInt32(vMin.X);
    int MinY = RoundReal32ToInt32(vMin.Y);
    int MaxX = RoundReal32ToInt32(vMax.X);
    int MaxY = RoundReal32ToInt32(vMax.Y);

    if (MinX < 0)
    {
		MinX = 0;
    }
    if (MinY < 0)
    {
		MinY = 0;
    }
    if (MaxX > Buffer->Width)
    {
		MaxX = Buffer->Width;
    }
    if (MaxY > Buffer->Height)
    {
		MaxY = Buffer->Height;
    }

    uint32 Color = ((RoundReal32ToUInt32(R * 255.0f) << 16) |
					(RoundReal32ToUInt32(G * 255.0f) << 8) |
					(RoundReal32ToUInt32(B * 255.0f)));			    

    uint8 *Row = ((uint8 *)Buffer->Memory
				  + MinX * Buffer->BytesPerPixel
				  + MinY * Buffer->Pitch);
    
    for (int Y = MinY; Y < MaxY; ++Y)    
    {
		uint32 *Pixel = (uint32 *)Row;
		for (int X = MinX; X < MaxX; ++X)    
		{
			*Pixel++ = Color;		
		}
	
		Row += Buffer->Pitch;
    }
}

internal void
DrawBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap,
		   real32 RealX, real32 RealY,
		   real32 CAlpha = 1.0f)
{
    int MinX = RoundReal32ToInt32(RealX);
    int MinY = RoundReal32ToInt32(RealY);
    int MaxX = (int)(RealX + Bitmap->Width);
    int MaxY = (int)(RealY + Bitmap->Height);

	int32 SourceOffsetX = 0;
    if (MinX < 0)
    {
		SourceOffsetX -= MinX;
		MinX = 0;
    }

	int32 SourceOffsetY = 0;
    if (MinY < 0)
    {
		SourceOffsetY -= MinY;
		MinY = 0;
    }
	
    if (MaxX > Buffer->Width)
    {
		MaxX = Buffer->Width;
    }
	
    if (MaxY > Buffer->Height)
    {
		MaxY = Buffer->Height;
    }

	uint32 *SourceRow = Bitmap->Pixels + Bitmap->Width*(Bitmap->Height - 1);
	SourceRow += SourceOffsetX - Bitmap->Width*SourceOffsetY;
    uint8 *DestRow = ((uint8 *)Buffer->Memory
				  + MinX * Buffer->BytesPerPixel
				  + MinY * Buffer->Pitch);

	for (int32 Y = MinY; Y < MaxY; ++Y)
	{
		uint32 *Dest = (uint32 *)DestRow;
		uint32 *Source = SourceRow;
		for (int32 X = MinX; X < MaxX; ++X)
		{
			real32 A = (real32)((*Source >> 24) & 0xFF) / 255.0f;
			A *= CAlpha;
			
			real32 SR = (real32)((*Source >> 16) & 0xFF);
			real32 SG = (real32)((*Source >> 8) & 0xFF);
			real32 SB = (real32)((*Source >> 0) & 0xFF);

			real32 DR = (real32)((*Dest >> 16) & 0xFF);
			real32 DG = (real32)((*Dest >> 8) & 0xFF);
			real32 DB = (real32)((*Dest >> 0) & 0xFF);

			real32 R = (1.0f-A)*DR + A*SR;			
			real32 G = (1.0f-A)*DG + A*SG;
			real32 B = (1.0f-A)*DB + A*SB;

			*Dest = (((uint32)(R + 0.5f) << 16) |
					 ((uint32)(G + 0.5f) << 8) |
					 ((uint32)(B + 0.5f) << 0));

			++Dest;
			++Source;
		}

		DestRow += Buffer->Pitch;
		SourceRow -= Bitmap->Width;
	}
}

inline void
PushPiece(entity_visible_piece_group *Group, loaded_bitmap *Bitmap,
		  vec2 Offset, real32 OffsetZ, vec2 Align, vec2 Dim, vec4 Color, real32 EntityZC)
{
	Assert(Group->PieceCount < ArrayCount(Group->Pieces));
	entity_visible_piece *Piece = Group->Pieces + Group->PieceCount++;

	Piece->Bitmap = Bitmap;
	Piece->Offset = Group->GameState->MetersToPixels*Vec2(Offset.X, -Offset.Y) - Align;
	Piece->OffsetZ = Group->GameState->MetersToPixels*-OffsetZ;
	Piece->EntityZC = EntityZC;
	Piece->R = Color.R;
	Piece->G = Color.G;
	Piece->B = Color.B;
	Piece->A = Color.A;
	Piece->Dim = Dim;
}

inline void
PushBitmap(entity_visible_piece_group *PieceGroup, loaded_bitmap *Bitmap,
		   vec2 Offset, real32 OffsetZ, vec2 Align, real32 Alpha=1.0f, real32 EntityZC=1.0f)
{
	PushPiece(PieceGroup, Bitmap, Offset, OffsetZ, Align, Vec2(0, 0), Vec4(1.0f, 1.0f, 1.0f, Alpha), EntityZC);
}

inline void
PushRect(entity_visible_piece_group *PieceGroup,
		 vec2 Offset, real32 OffsetZ, vec2 Dim, vec4 Color, real32 EntityZC=1.0f)
{
	PushPiece(PieceGroup, 0, Offset, OffsetZ, Vec2(0, 0), Dim, Color, EntityZC);
}

#pragma pack(push, 1)
struct bitmap_header
{
	uint16 FileType;
	uint32 FileSize;
	uint16 Reserved1;
	uint16 Reserved2;
	uint32 BitmapOffset;
	uint32 Size;
	int32 Width;
	int32 Height;
	uint16 Planes; 
	uint16 BitsPerPixel;
    uint32 Compression;
    uint32 SizeOfBitmap;
    int32 HorzResolution;
    int32 VertResolution;
    uint32 ColorsUsed;
    uint32 ColorsImportant;

    uint32 RedMask;
    uint32 GreenMask;
    uint32 BlueMask;
	
};
#pragma pack(pop)

internal loaded_bitmap 
DEBUGLoadBMP(thread_context *Thread, debug_platform_read_entire_file *ReadEntireFile, char *Filename)
{
	loaded_bitmap Result = {};	

	debug_read_file_result ReadResult = ReadEntireFile(Thread, Filename);
	if (ReadResult.ContentsSize != 0)
	{
		bitmap_header *Header = (bitmap_header *)ReadResult.Contents;
		uint32 *Pixels = (uint32 *)((uint8 *)ReadResult.Contents + Header->BitmapOffset);		
		Result.Pixels = Pixels;
		Result.Width = Header->Width;
		Result.Height = Header->Height;

		Assert(Header->Compression == 3);
		
		uint32 RedMask = Header->RedMask;
		uint32 GreenMask = Header->GreenMask;
		uint32 BlueMask = Header->BlueMask;
		uint32 AlphaMask = ~(RedMask | GreenMask | BlueMask);

		bit_scan_result RedScan = FindLeastSignificantSetBit(RedMask);
		bit_scan_result GreenScan = FindLeastSignificantSetBit(GreenMask);
		bit_scan_result BlueScan = FindLeastSignificantSetBit(BlueMask);
		bit_scan_result AlphaScan = FindLeastSignificantSetBit(AlphaMask);		

		Assert(RedScan.Found);
		Assert(GreenScan.Found);
		Assert(BlueScan.Found);
		Assert(AlphaScan.Found);

		int32 RedShift = 16 - (int32)RedScan.Index;
		int32 GreenShift = 8 - (int32)GreenScan.Index;
		int32 BlueShift = 0 - (int32)BlueScan.Index;
		int32 AlphaShift = 24 - (int32)AlphaScan.Index;
					
		uint32 *SourceDest = Pixels;
		for (int32 Y = 0; Y < Header->Height; ++Y)
		{
			for (int32 X = 0; X < Header->Width; ++X)
			{
				uint32 C = *SourceDest;

                *SourceDest++ = (RotateLeft(C & RedMask, RedShift) |
                                 RotateLeft(C & GreenMask, GreenShift) |
                                 RotateLeft(C & BlueMask, BlueShift) |
                                 RotateLeft(C & AlphaMask, AlphaShift));
			}
		}
	}

	return Result;
}

inline vec2
GetCameraSpaceP(game_state *GameState, low_entity *EntityLow)
{
	world_difference Diff = Subtract(GameState->World, &EntityLow->P, &GameState->CameraP);
	vec2 Result = Diff.dXY;

	return Result;
}

struct add_low_entity_result
{
	uint32 LowIndex;
	low_entity *Low;
};

internal add_low_entity_result
AddLowEntity(game_state *GameState, entity_type Type, world_position *P)
{
	Assert(GameState->LowEntityCount < ArrayCount(GameState->LowEntities));
	uint32 LowIndex = GameState->LowEntityCount++;	
	
	low_entity *EntityLow = GameState->LowEntities + LowIndex;
	*EntityLow = {};
	EntityLow->Sim.Type = Type;

	ChangeEntityLocation(&GameState->WorldArena, GameState->World, LowIndex, EntityLow, 0, P);	

	add_low_entity_result Result = {};
	Result.LowIndex = LowIndex;
	Result.Low = EntityLow;
	
	return Result;
}

internal void
InitHitPoints(low_entity *EntityLow, uint32 HitPointCount)
{
	Assert(HitPointCount <= ArrayCount(EntityLow->Sim.HitPoint));
	EntityLow->Sim.HitPointMax = HitPointCount;
	for (uint32 HitPointIndex = 0; HitPointIndex < HitPointCount; ++HitPointIndex)
	{
		hit_point *HitPoint = EntityLow->Sim.HitPoint + HitPointIndex;
		HitPoint->Flags = 0;
		HitPoint->FilledAmount = HIT_POINT_SUB_COUNT;
	}
}

internal void
DrawHitPoints(sim_entity *SimEntity, entity_visible_piece_group *Group)
{
	if (SimEntity->HitPointMax >= 1)
	{
		vec2 HealthDim = {0.2f, 0.2f};
		real32 SpacingX = 1.5f*HealthDim.X;
		vec2 HitP = {-0.5f*(SimEntity->HitPointMax - 1)*SpacingX, -0.25f};
		vec2 dHitP = {SpacingX, 0};
		for (uint32 HealthIndex = 0; HealthIndex < SimEntity->HitPointMax; ++HealthIndex)
		{
			hit_point HitPoint = SimEntity->HitPoint[HealthIndex];
			vec4 Color = {1.0f, 0, 0, 1.0f};
			if (HitPoint.FilledAmount == 0)
			{
				Color = {0.2f, 0.2f, 0.2f, 1.0f};
			}

			PushRect(Group, HitP, 0, HealthDim, Color, 0.0f);
			HitP += dHitP;
		}
	}
}

internal add_low_entity_result
AddSword(game_state *GameState)
{
	add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Sword, 0);

	Entity.Low->Sim.Height = 0.5f;
	Entity.Low->Sim.Width = 1.0f;			
	Entity.Low->Sim.Collides = false;

	return Entity;
}

internal add_low_entity_result
AddPlayer(game_state *GameState)
{
	add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Hero, &GameState->CameraP); 

	Entity.Low->Sim.Height = 0.5f;
	Entity.Low->Sim.Width = 1.0f;			
	Entity.Low->Sim.Collides = true;
	
	InitHitPoints(Entity.Low, 3);

	add_low_entity_result Sword = AddSword(GameState);
	Entity.Low->Sim.Sword.Index = Sword.LowIndex; 
	
	if (GameState->CameraFollowingEntityIndex == 0)
	{
		GameState->CameraFollowingEntityIndex = Entity.LowIndex;
	}

	return Entity;
}

internal add_low_entity_result
AddWall(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
	world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
	add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Wall, &P);	

	Entity.Low->Sim.Height = GameState->World->TileSideInMeters;
	Entity.Low->Sim.Width = Entity.Low->Sim.Height;			
	Entity.Low->Sim.Collides = true;

	return Entity;
}

internal add_low_entity_result
AddMonster(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
	world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
	add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Monster, &P);	

	Entity.Low->Sim.Height = GameState->World->TileSideInMeters;
	Entity.Low->Sim.Width = Entity.Low->Sim.Height;			
	Entity.Low->Sim.Collides = true;

	return Entity;
}

internal add_low_entity_result
AddFamiliar(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
	world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
	add_low_entity_result Result = AddLowEntity(GameState, EntityType_Familiar, &P);	

	Result.Low->Sim.Height = GameState->World->TileSideInMeters;
	Result.Low->Sim.Width = Result.Low->Sim.Height;			
	Result.Low->Sim.Collides = false;

	return Result;
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) 
		   == (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if (!Memory->IsInitialized)
    {
		AddLowEntity(GameState, EntityType_Null, 0);
		
		GameState->Backdrop =
			DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_background.bmp");
		GameState->Shadow =
			DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_shadow.bmp");
		GameState->Tree =
			DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test2/tree00.bmp");
		GameState->Sword =
			DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test2/rock03.bmp");

		hero_bitmaps *Bitmap = GameState->HeroBitmaps;
		
		Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_right_head.bmp");
		Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_right_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_right_torso.bmp");
        Bitmap->Align = Vec2(72, 182);
		++Bitmap;

		Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_back_head.bmp");
		Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_back_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_back_torso.bmp");
        Bitmap->Align = Vec2(72, 182);
		++Bitmap;

		Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_left_head.bmp");
		Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_left_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_left_torso.bmp");
        Bitmap->Align = Vec2(72, 182);
		++Bitmap;

		Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_front_head.bmp");
		Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_front_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_front_torso.bmp");
        Bitmap->Align = Vec2(72, 182);
		++Bitmap;

		InitializeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state),
						(uint8 *)Memory->PermanentStorage + sizeof(game_state));

		GameState->World = PushStruct(&GameState->WorldArena, world);
		world *World = GameState->World;
		InitializeWorld(World, 1.4f);
			
		uint32 RandomNumberIndex = 0;
		uint32 TilesPerWidth = 17;
		uint32 TilesPerHeight = 9;

		uint32 ScreenBaseX = 0;
		uint32 ScreenBaseY = 0;
		uint32 ScreenBaseZ = 0;
		uint32 ScreenX = ScreenBaseX;
		uint32 ScreenY = ScreenBaseY;
		uint32 AbsTileZ = ScreenBaseZ;
		
		bool32 DoorLeft = false;
		bool32 DoorRight = false;
		bool32 DoorTop = false;
		bool32 DoorBottom = false;
		bool32 DoorUp = false;
		bool32 DoorDown = false;		
		for (uint32 ScreenIndex = 0; ScreenIndex < 2000; ++ScreenIndex)
		{
			Assert(RandomNumberIndex < ArrayCount(RandomNumberTable));

			uint32 RandomChoice;
// 			if (DoorUp || DoorDown)
			{
				RandomChoice = RandomNumberTable[RandomNumberIndex++] % 2;								
			}
#if 0
			else
			{
				RandomChoice = RandomNumberTable[RandomNumberIndex++] % 3;
			}
#endif
			
			bool32 CreatedZDoor = false;
			if (RandomChoice == 2)
			{
				CreatedZDoor = true;
				if (AbsTileZ == 0)
				{
					DoorUp = true;
				}
				else
				{
					DoorDown = true;
				}
			}
			else if (RandomChoice == 1)
			{
				DoorRight = true;
			}
			else
			{
				DoorTop = true;
			}
			
			for (uint32 TileY = 0; TileY < TilesPerHeight; ++TileY)
			{
				for (uint32 TileX = 0; TileX < TilesPerWidth; ++TileX)
				{					
					uint32 AbsTileX = ScreenX*TilesPerWidth + TileX;
					uint32 AbsTileY = ScreenY*TilesPerHeight + TileY;

					uint32 TileValue = 1;
					if ((TileX == 0) && (!DoorLeft || (TileY != TilesPerHeight/2)))
					{
						TileValue = 2;
					}

					if ((TileX == TilesPerWidth - 1) && (!DoorRight || (TileY != TilesPerHeight/2)))
					{
						TileValue = 2;
					}

					if ((TileY == 0) && (!DoorBottom || (TileX != TilesPerWidth/2)))
					{
						TileValue = 2;
					}

					if ((TileY == TilesPerHeight - 1) && (!DoorTop || (TileX != TilesPerWidth/2)))
					{
						TileValue = 2;
					}

					if ((TileX == 10) && (TileY == 6))
					{
						if (DoorUp)
						{
							TileValue = 3;
						}

						if (DoorDown)
						{
							TileValue = 4;
						}
					}
						
					if (TileValue == 2)
					{
						AddWall(GameState, AbsTileX, AbsTileY, AbsTileZ);
					}
				}
			}

			DoorLeft = DoorRight;
			DoorBottom = DoorTop;

			if (CreatedZDoor)
			{
				DoorDown = !DoorDown;
				DoorUp = !DoorUp;
			}
			else
			{
				DoorUp = false;
				DoorDown = false;
			}
			
			DoorRight = false;
			DoorTop = false;

			if (RandomChoice == 2)
			{
				if (AbsTileZ == ScreenBaseZ)
				{
					AbsTileZ = ScreenBaseZ + 1;
				}
				else
				{
					AbsTileZ = ScreenBaseZ;
				}
			}
			else if (RandomChoice == 1)
			{
				ScreenX += 1;
			}
			else
			{
				ScreenY += 1;
			}
		}


#if 0
        while(GameState->LowEntityCount < (ArrayCount(GameState->LowEntities) - 16))
        {
            uint32 Coordinate = 1024 + GameState->LowEntityCount;
            AddWall(GameState, Coordinate, Coordinate, Coordinate);
        }
#endif

		world_position NewCameraP = {};
        uint32 CameraTileX = ScreenBaseX*TilesPerWidth + 17/2;
        uint32 CameraTileY = ScreenBaseY*TilesPerHeight + 9/2;
        uint32 CameraTileZ = ScreenBaseZ;
        NewCameraP = ChunkPositionFromTilePosition(GameState->World,
                                                   CameraTileX,
                                                   CameraTileY,
                                                   CameraTileZ);
		AddMonster(GameState, CameraTileX + 2, CameraTileY + 2, CameraTileZ);
		for (int FamiliarIndex = 0; FamiliarIndex < 1; ++FamiliarIndex)
		{
            int32 FamiliarOffsetX = (RandomNumberTable[RandomNumberIndex++] % 10) - 7;
            int32 FamiliarOffsetY = (RandomNumberTable[RandomNumberIndex++] % 10) - 3;
			if ((FamiliarOffsetX != 0) || (FamiliarOffsetY != 0))
			{
				AddFamiliar(GameState, CameraTileX + FamiliarOffsetX, CameraTileY + FamiliarOffsetY, CameraTileZ);
			}

		}

		Memory->IsInitialized = true;		
    }

	world *World = GameState->World;

	int32 TileSideInPixels = 60;
	real32 MetersToPixels = (real32)TileSideInPixels / (real32)World->TileSideInMeters;
	GameState->MetersToPixels = MetersToPixels;

	real32 LowerLeftX = -(real32)TileSideInPixels/2;
	real32 LowerLeftY = (real32)Buffer->Height;

    for (int ControllerIndex = 0;
		 ControllerIndex < ArrayCount(Input->Controllers);
		 ++ControllerIndex)
    {
		game_controller_input *Controller = GetController(Input, ControllerIndex);
		controlled_hero *ConHero = GameState->ControlledHeros + ControllerIndex;
		if (ConHero->EntityIndex == 0)
		{
			if (Controller->Start.EndedDown)
			{
				*ConHero = {};
				ConHero->EntityIndex = AddPlayer(GameState).LowIndex;
			}
		}
		else
		{
			ConHero->ddP = {};

			if (Controller->IsAnalog)
			{
			    ConHero->ddP = vec2{Controller->StickAverageX, Controller->StickAverageY};
			}
			else
			{
				if (Controller->MoveUp.EndedDown)
				{
					ConHero->ddP.Y = 1.0f;
				}
				if (Controller->MoveDown.EndedDown)
				{
					ConHero->ddP.Y = -1.0f;
				}
				if (Controller->MoveLeft.EndedDown)
				{
					ConHero->ddP.X = -1.0f;
				}
				if (Controller->MoveRight.EndedDown)
				{
					ConHero->ddP.X = 1.0f;
				}
			}

			if (Controller->Start.EndedDown)
			{
				ConHero->dZ = 3.0f;
			}

			vec2 dSword = {};
			if (Controller->ActionUp.EndedDown)
			{
				dSword = {0.0f, 1.0f};
			}
			if (Controller->ActionDown.EndedDown)
			{
				dSword = {0.0f, -1.0f};
			}
			if (Controller->ActionRight.EndedDown)
			{
				dSword = {1.0f, 0.0f};
			}
			if (Controller->ActionLeft.EndedDown)
			{
				dSword = {-1.0f, 0.0f};
			}
		}
	}	

	uint32 TileSpanX = 17*3;
	uint32 TileSpanY = 9*3;
	rectangle2 CameraBounds = RectCenterDim(Vec2(0, 0),
											World->TileSideInMeters*Vec2((real32)TileSpanX,
																		 (real32)TileSpanY));

	memory_arena SimArena;
	InitializeArena(&SimArena, Memory->TransientStorageSize, Memory->TransientStorage);
	sim_region *SimRegion = BeginSim(&SimArena, GameState, GameState->World, GameState->CameraP, CameraBounds);
	
#if 0
	DrawBitmap(Buffer, &GameState->Backdrop, 0, 0); 
#else
	DrawRectangle(Buffer, vec2{0, 0}, vec2{(real32)Buffer->Width, (real32)Buffer->Height}, 0.5f, 0.5f, 0.5f);
#endif
	
	real32 ScreenCenterX = 0.5f*(real32)Buffer->Width;	
	real32 ScreenCenterY = 0.5f*(real32)Buffer->Height;
		
	entity_visible_piece_group PieceGroup = {};
	PieceGroup.GameState = GameState;
	
	sim_entity *Entity = SimRegion->Entities;
	for (uint32 EntityIndex = 0; EntityIndex < SimRegion->EntityCount;
		 ++EntityIndex, ++Entity)
	{
		PieceGroup.PieceCount = 0;
	
		real32 dt = Input->dtForFrame;
		
		real32 ShadowAlpha = 1.0f - 0.5f*Entity->Z;
		if (ShadowAlpha < 0)
		{
			ShadowAlpha = 0.0f;
		}

		hero_bitmaps *HeroBitmaps = &GameState->HeroBitmaps[Entity->FacingDirection];						
		switch (Entity->Type)
		{
		    case EntityType_Hero:
		    {
				for (uint32 ControlIndex = 0; ControlIndex < ArrayCount(GameState->ControlledHeros); ++ControlIndex)
				{
					controlled_hero *ConHero = GameState->ControlledHeros + ControlIndex;

					if (ConHero->EntityIndex == Entity->StorageIndex)
					{
						move_spec MoveSpec = DefaultMoveSpec();
						MoveSpec.UnitMaxAccelVector = true;
						MoveSpec.Speed = 50.0f;
						MoveSpec.Drag = 8.0f;
						MoveEntity(SimRegion, Entity, Input->dtForFrame, &MoveSpec, ConHero->ddP);
						if ((ConHero->dSword.X != 0.0f) || (ConHero->dSword.Y != 0.0f))
						{
							sim_entity *Sword = Entity->Sword.Ptr;
							if (Sword)
							{
								Sword->P = Entity->P;								
								Sword->DistanceRemaining = 5.0f;
								Sword->dP = 5.0f*ConHero->dSword;
							}
						}
					}
				}
				
				PushBitmap(&PieceGroup, &GameState->Shadow, Vec2(0, 0), 0, HeroBitmaps->Align, ShadowAlpha, 0.0f);
		    	PushBitmap(&PieceGroup, &HeroBitmaps->Torso, Vec2(0, 0), 0, HeroBitmaps->Align);
		    	PushBitmap(&PieceGroup, &HeroBitmaps->Cape, Vec2(0, 0), 0, HeroBitmaps->Align);
		    	PushBitmap(&PieceGroup, &HeroBitmaps->Head, Vec2(0, 0), 0, HeroBitmaps->Align);
				
				DrawHitPoints(Entity, &PieceGroup);
		    } break;
			
		    case EntityType_Monster:
		    {
				UpdateMonster(SimRegion, Entity, dt);
		    	PushBitmap(&PieceGroup, &GameState->Shadow, Vec2(0, 0), 0, HeroBitmaps->Align, ShadowAlpha, 0.0f);
		    	PushBitmap(&PieceGroup, &HeroBitmaps->Torso, Vec2(0, 0), 0, HeroBitmaps->Align);
		    } break;
			
		    case EntityType_Familiar:
		    {
				UpdateFamiliar(SimRegion, Entity, dt);
				Entity->tBob += dt;
				if (Entity->tBob > (2.0f*Pi32))
				{
					Entity->tBob -= (2.0f*Pi32);
				}
				real32 BobSin = Sin(2.0f*Entity->tBob);
		    	PushBitmap(&PieceGroup, &GameState->Shadow, Vec2(0, 0), 0, HeroBitmaps->Align, (0.5f*ShadowAlpha) - 0.2f*BobSin, 0.0f);
		    	PushBitmap(&PieceGroup, &HeroBitmaps->Head, Vec2(0, 0), 0.25f*BobSin, HeroBitmaps->Align);
		    } break;
			
		    case EntityType_Wall:
		    {
		    	PushBitmap(&PieceGroup, &GameState->Tree, Vec2(0, 0), 0, Vec2(40, 80));
		    } break;

		    case EntityType_Sword:
		    {
				UpdateSword(SimRegion, Entity, dt);
		    	PushBitmap(&PieceGroup, &GameState->Shadow, Vec2(0, 0), 0, HeroBitmaps->Align, ShadowAlpha, 0.0f);
				PushBitmap(&PieceGroup, &GameState->Sword, Vec2(0, 0), 0, Vec2(29, 10));
    		} break;
			
		    default:
		    {
		    	InvalidCodePath;
		    } break;
		}

		real32 ddZ = -9.8f;
		Entity->Z = 0.5f*ddZ*Square(dt) + Entity->dZ*dt + Entity->Z;
		Entity->dZ = ddZ*dt + Entity->dZ;
		if (Entity->Z < 0)
		{
			Entity->Z =0;
		}
		
        real32 EntityGroundPointX = ScreenCenterX + MetersToPixels*Entity->P.X;
        real32 EntityGroundPointY = ScreenCenterY - MetersToPixels*Entity->P.Y;            
        real32 EntityZ = -MetersToPixels*Entity->Z;
		for (uint32 PieceIndex = 0; PieceIndex < PieceGroup.PieceCount; ++PieceIndex)
		{
			entity_visible_piece *Piece = PieceGroup.Pieces + PieceIndex;
			vec2 Center = {EntityGroundPointX + Piece->Offset.X,
				           EntityGroundPointY + Piece->Offset.Y + Piece->OffsetZ + Piece->EntityZC*EntityZ};
			if (Piece->Bitmap)
			{
				DrawBitmap(Buffer, Piece->Bitmap, Center.X, Center.Y, Piece->A);
			}
			else
			{
				vec2 HalfDim = 0.5f*Piece->Dim*MetersToPixels;;
				DrawRectangle(Buffer, Center - HalfDim, Center + HalfDim, Piece->R, Piece->G, Piece->B);				
			}
		}
	}

	EndSim(SimRegion, GameState);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    GameOutputSound(GameState, SoundBuffer, 400);
}
