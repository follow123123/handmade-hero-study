//
//
internal void
DrawRectangle(loaded_bitmap *Buffer, vec2 vMin, vec2 vMax, real32 R, real32 G, real32 B)
{
    int32 MinX = RoundReal32ToInt32(vMin.X);
    int32 MinY = RoundReal32ToInt32(vMin.Y);
    int32 MaxX = RoundReal32ToInt32(vMax.X);
    int32 MaxY = RoundReal32ToInt32(vMax.Y);

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
				  + MinX * BITMAP_BYTES_PER_PIXEL
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
DrawRectangleOutline(loaded_bitmap *Buffer, vec2 vMin, vec2 vMax, vec3 Color, real32 R=2.0f)
{		
	DrawRectangle(Buffer, Vec2(vMin.X - R, vMin.Y - R), Vec2(vMax.X + R, vMin.Y + R), Color.R, Color.G, Color.B);
	DrawRectangle(Buffer, Vec2(vMin.X - R, vMax.Y - R), Vec2(vMax.X + R, vMax.Y + R), Color.R, Color.G, Color.B);

	DrawRectangle(Buffer, Vec2(vMin.X - R, vMin.Y - R), Vec2(vMin.X + R, vMax.Y + R), Color.R, Color.G, Color.B);
	DrawRectangle(Buffer, Vec2(vMax.X - R, vMin.Y - R), Vec2(vMax.X + R, vMax.Y + R), Color.R, Color.G, Color.B);
}

internal void
DrawBitmap(loaded_bitmap *Buffer, loaded_bitmap *Bitmap, real32 RealX, real32 RealY, real32 CAlpha = 1.0f)
{
    int32 MinX = RoundReal32ToInt32(RealX);
    int32 MinY = RoundReal32ToInt32(RealY);
    int32 MaxX = MinX + Bitmap->Width;
    int32 MaxY = MinY + Bitmap->Height;

    int32 SourceOffsetX = 0;
    if(MinX < 0)
    {
        SourceOffsetX = -MinX;
        MinX = 0;
    }

    int32 SourceOffsetY = 0;
    if(MinY < 0)
    {
        SourceOffsetY = -MinY;
        MinY = 0;
    }

    if(MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }

    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }

    uint8 *SourceRow = (uint8 *)Bitmap->Memory + SourceOffsetY*Bitmap->Pitch + BITMAP_BYTES_PER_PIXEL*SourceOffsetX;
    uint8 *DestRow = ((uint8 *)Buffer->Memory +
                      MinX*BITMAP_BYTES_PER_PIXEL +
                      MinY*Buffer->Pitch);
    for(int Y = MinY;
        Y < MaxY;
        ++Y)
    {
        uint32 *Dest = (uint32 *)DestRow;
        uint32 *Source = (uint32 *)SourceRow;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {
            real32 SA = (real32)((*Source >> 24) & 0xFF);
            real32 RSA = (SA / 255.0f) * CAlpha;            
            real32 SR = CAlpha*(real32)((*Source >> 16) & 0xFF);
            real32 SG = CAlpha*(real32)((*Source >> 8) & 0xFF);
            real32 SB = CAlpha*(real32)((*Source >> 0) & 0xFF);

            real32 DA = (real32)((*Dest >> 24) & 0xFF);
            real32 DR = (real32)((*Dest >> 16) & 0xFF);
            real32 DG = (real32)((*Dest >> 8) & 0xFF);
            real32 DB = (real32)((*Dest >> 0) & 0xFF);
            real32 RDA = (DA / 255.0f);
            
            real32 InvRSA = (1.0f-RSA);
            real32 A = 255.0f*(RSA + RDA - RSA*RDA);
            real32 R = InvRSA*DR + SR;
            real32 G = InvRSA*DG + SG;
            real32 B = InvRSA*DB + SB;

            *Dest = (((uint32)(A + 0.5f) << 24) |
                     ((uint32)(R + 0.5f) << 16) |
                     ((uint32)(G + 0.5f) << 8) |
                     ((uint32)(B + 0.5f) << 0));
            
            ++Dest;
            ++Source;
        }

        DestRow += Buffer->Pitch;
        SourceRow += Bitmap->Pitch;
    }
}

internal render_group *
AllocateRenderGroup(memory_arena *Arena, uint32 MaxPushBufferSize, real32 MetersToPixels)
{
	render_group *Result = PushStruct(Arena, render_group);
	Result->PushBufferBase = (uint8 *)_PushSize(Arena, MaxPushBufferSize);

	Result->DefaultBasis = PushStruct(Arena, render_basis);
	Result->DefaultBasis->P = Vec3(0, 0, 0);
	Result->MetersToPixels = MetersToPixels;
	
	Result->MaxPushBufferSize = MaxPushBufferSize;
	Result->PushBufferSize = 0;

	return Result;
}

inline vec2
GetRenderEntityBasisP(render_group *RenderGroup, render_entity_basis *EntityBasis, vec2 ScreenCenter)   
{
	vec3 EntityBaseP = EntityBasis->Basis->P;
	real32 ZFudge = 1.0f + 0.1f*(EntityBaseP.Z + EntityBasis->OffsetZ);
     		
	real32 EntityGroundPointX = ScreenCenter.X + ZFudge*RenderGroup->MetersToPixels*EntityBaseP.X;
	real32 EntityGroundPointY = ScreenCenter.Y - ZFudge*RenderGroup->MetersToPixels*EntityBaseP.Y;            
	real32 EntityZ = -RenderGroup->MetersToPixels*EntityBaseP.Z;
     		
	vec2 Center = {EntityGroundPointX + EntityBasis->Offset.X,
		           EntityGroundPointY + EntityBasis->Offset.Y + EntityBasis->EntityZC*EntityZ};

	return Center;
}

internal void
RenderGroupToOutput(render_group *RenderGroup, loaded_bitmap *OutputBuffer)
{
	vec2 ScreenCenter = 0.5f*Vec2i(OutputBuffer->Width, OutputBuffer->Height);
	real32 MetersToPixels = RenderGroup->MetersToPixels;
	
	for (uint32 BaseAddress = 0; BaseAddress < RenderGroup->PushBufferSize;)
	{
		render_group_entry_header *Header = (render_group_entry_header *)(RenderGroup->PushBufferBase + BaseAddress);

		switch (Header->Type)
		{
     		case RenderGroupEntryType_render_entry_clear:
     		{
     			render_entry_clear *Entry = (render_entry_clear *)Header;
     			
     			BaseAddress += sizeof(*Entry);
     		} break;
     
     		case RenderGroupEntryType_render_entry_bitmap:
     		{
     			render_entry_bitmap *Entry = (render_entry_bitmap *)Header;

				vec2 P = GetRenderEntityBasisP(RenderGroup, &Entry->EntityBasis, ScreenCenter);
     
     			DrawBitmap(OutputBuffer, Entry->Bitmap, P.X, P.Y, Entry->A);
     			BaseAddress += sizeof(*Entry);
     		} break;
     
     		case RenderGroupEntryType_render_entry_rectangle:
     		{
     			render_entry_rectangle *Entry = (render_entry_rectangle *)Header;
     
				vec2 P = GetRenderEntityBasisP(RenderGroup, &Entry->EntityBasis, ScreenCenter);
     			vec2 Dim = Entry->Dim * MetersToPixels;;
				
     			DrawRectangle(OutputBuffer, P, P + Dim, Entry->R, Entry->G, Entry->B);
     			BaseAddress += sizeof(*Entry);			
     		} break;
     
		    InvalidDefaultCase;
		}		
	}
}

#define PushRenderElement(Group, type) (type *)_PushRenderElement(Group, sizeof(type), RenderGroupEntryType_##type)
inline render_group_entry_header *
_PushRenderElement(render_group *Group, uint32 Size, render_group_entry_type Type)
{
	render_group_entry_header *Result = 0;

	if (Group->PushBufferSize + Size < Group->MaxPushBufferSize)
	{
		Result = (render_group_entry_header *)(Group->PushBufferBase + Group->PushBufferSize);
		Result->Type = Type;
		Group->PushBufferSize += Size;		
	}
	else
	{
		InvalidCodePath;
	}

	return Result;
}

inline void
PushPiece(render_group *Group, loaded_bitmap *Bitmap,
		  vec2 Offset, real32 OffsetZ, vec2 Align, vec2 Dim, vec4 Color, real32 EntityZC)
{
	render_entry_bitmap *Piece = PushRenderElement(Group, render_entry_bitmap);
	if (Piece)
	{
		Piece->Bitmap = Bitmap;
		Piece->EntityBasis.Basis = Group->DefaultBasis;
		Piece->EntityBasis.Offset = Group->MetersToPixels*Vec2(Offset.X, -Offset.Y) - Align;
		Piece->EntityBasis.OffsetZ = OffsetZ;
		Piece->EntityBasis.EntityZC = EntityZC;
		Piece->R = Color.R;
		Piece->G = Color.G;
		Piece->B = Color.B;
		Piece->A = Color.A;
	}
}
inline void
PushBitmap(render_group *RenderGroup, loaded_bitmap *Bitmap,
		   vec2 Offset, real32 OffsetZ, vec2 Align, real32 Alpha=1.0f, real32 EntityZC=1.0f)
{
	PushPiece(RenderGroup, Bitmap, Offset, OffsetZ, Align, Vec2(0, 0), Vec4(1.0f, 1.0f, 1.0f, Alpha), EntityZC);
}

inline void
PushRect(render_group *RenderGroup,
		 vec2 Offset, real32 OffsetZ, vec2 Dim, vec4 Color, real32 EntityZC=1.0f)
{
	render_entry_rectangle *Piece = PushRenderElement(RenderGroup, render_entry_rectangle);
	if (Piece)
	{
		vec2 HalfDim = 0.5f*RenderGroup->MetersToPixels*Dim;
		
		Piece->EntityBasis.Basis = RenderGroup->DefaultBasis;
		Piece->EntityBasis.Offset = RenderGroup->MetersToPixels*Vec2(Offset.X, -Offset.Y) - HalfDim;
		Piece->EntityBasis.OffsetZ = OffsetZ;
		Piece->EntityBasis.EntityZC = EntityZC;
		Piece->R = Color.R;
		Piece->G = Color.G;
		Piece->B = Color.B;
		Piece->Dim = Dim;
	}
}

inline void
PushRectOutline(render_group *RenderGroup,
				vec2 Offset, real32 OffsetZ, vec2 Dim, vec4 Color, real32 EntityZC=1.0f)
{
	real32 Thickness = 0.1f;
	
	PushPiece(RenderGroup, 0, (Offset - Vec2(0, 0.5f*Dim.Y)), OffsetZ, Vec2(0, 0), Vec2(Dim.X, Thickness), Color, EntityZC);
	PushPiece(RenderGroup, 0, (Offset + Vec2(0, 0.5f*Dim.Y)), OffsetZ, Vec2(0, 0), Vec2(Dim.X, Thickness), Color, EntityZC);

	PushPiece(RenderGroup, 0, (Offset - Vec2(0.5f*Dim.X, 0)), OffsetZ, Vec2(0, 0), Vec2(Thickness, Dim.Y), Color, EntityZC);
	PushPiece(RenderGroup, 0, (Offset + Vec2(0.5f*Dim.X, 0)), OffsetZ, Vec2(0, 0), Vec2(Thickness, Dim.Y), Color, EntityZC);
}

