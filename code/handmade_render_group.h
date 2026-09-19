#if !defined(HANDMADE_RENDER_GROUP_H)
#define HANDMADE_RENDER_GROUP_H

struct render_basis
{
	vec3 P;
};

struct entity_visible_piece
{
	render_basis *Basis;
	loaded_bitmap *Bitmap;
	vec2 Offset;
	real32 OffsetZ;
	real32 EntityZC;
	real32 R, G, B, A;
	vec2 Dim;
};

struct render_group
{
	render_basis *DefaultBasis;
	uint32 PieceCount;
	real32 MetersToPixels;

	uint32 MaxPushBufferSize;
	uint32 PushBufferSize;	
	uint8 *PushBufferBase;
};

inline void *
PushRenderElement(render_group *Group, uint32 Size)
{
	void *Result = 0;

	if (Group->PushBufferSize + Size < Group->MaxPushBufferSize)
	{
		Result = Group->PushBufferBase + Group->PushBufferSize;
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
	entity_visible_piece *Piece = (entity_visible_piece *)PushRenderElement(Group, sizeof(entity_visible_piece));
	Piece->Basis = Group->DefaultBasis;
	Piece->Bitmap = Bitmap;
	Piece->Offset = Group->MetersToPixels*Vec2(Offset.X, -Offset.Y) - Align;
	Piece->OffsetZ = OffsetZ;
	Piece->EntityZC = EntityZC;
	Piece->R = Color.R;
	Piece->G = Color.G;
	Piece->B = Color.B;
	Piece->A = Color.A;
	Piece->Dim = Dim;
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
	PushPiece(RenderGroup, 0, Offset, OffsetZ, Vec2(0, 0), Dim, Color, EntityZC);
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

#endif /* HANDMADE_RENDER_GROUP_H */
