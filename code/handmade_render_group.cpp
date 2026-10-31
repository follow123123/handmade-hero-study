//
//
inline vec4
Unpack4x8(uint32 Color)
{
	vec4 Unpacked = {(real32)((Color >> 16) & 0xFF),
		             (real32)((Color >> 8) & 0xFF),
		             (real32)((Color >> 0) & 0xFF),
		             (real32)((Color >> 24) & 0xFF)};

	return Unpacked;
};

inline vec4
SRGB255ToLinear1(vec4 Color)
{
	real32 Inv255 = 1.0f / 255.0f;
	
	vec4 Result = {Square(Inv255*Color.r),
		           Square(Inv255*Color.g),
		           Square(Inv255*Color.b),
		           Inv255*Color.a};

	return Result;
}

inline vec4
Linear1ToSRGB255(vec4 Color)
{
	real32 One255 = 255.0f;

	vec4 Result = {One255*SquareRoot(Color.r),
 		           One255*SquareRoot(Color.g),
		           One255*SquareRoot(Color.b),
		           255.0f*Color.a};

	return Result;
}

struct bilinear_sample
{
	uint32 A, B, C, D;
};
inline bilinear_sample
BilinearSample(loaded_bitmap *Texture, int32 X, int32 Y)
{
	bilinear_sample Result;
	uint8 *TextPtr = (uint8 *)Texture->Memory + Y*Texture->Pitch + X*sizeof(uint32);

	Result.A = *(uint32 *)(TextPtr);
	Result.B = *(uint32 *)(TextPtr + sizeof(uint32));
	Result.C = *(uint32 *)(TextPtr + Texture->Pitch);
	Result.D = *(uint32 *)(TextPtr + Texture->Pitch + sizeof(uint32));

	return Result;
}

inline vec4
SRGBBilinearBlend(bilinear_sample TexelSample, real32 fX, real32 fY)
{
 	vec4 TexelA = Unpack4x8(TexelSample.A);
	vec4 TexelB = Unpack4x8(TexelSample.B);
	vec4 TexelC = Unpack4x8(TexelSample.C);
	vec4 TexelD = Unpack4x8(TexelSample.D);

	TexelA = SRGB255ToLinear1(TexelA);
	TexelB = SRGB255ToLinear1(TexelB);
	TexelC = SRGB255ToLinear1(TexelC);
	TexelD = SRGB255ToLinear1(TexelD);

	vec4 Texel = Lerp(fY, Lerp(fX, TexelA, TexelB), Lerp(fX, TexelC, TexelD));
	
	return Texel;
}

inline vec4
UnscaleAndBiasNormal(vec4 Normal)
{
	vec4 Result;

	real32 Inv255 = 1.0f / 255.0f;

	Result.x = -1.0f + 2.0f*(Inv255*Normal.x);
	Result.y = -1.0f + 2.0f*(Inv255*Normal.y);
	Result.z = -1.0f + 2.0f*(Inv255*Normal.z);

	Result.w = Inv255*Normal.w;

	return Result;
}

internal void
DrawRectangle(loaded_bitmap *Buffer, vec2 vMin, vec2 vMax, vec4 Color, rectangle2i ClipRect, bool32 Even)
{
	real32 R = Color.r;
	real32 G = Color.g;
	real32 B = Color.b;
	real32 A = Color.a;

	rectangle2i FillRect;
	FillRect.MinX = RoundReal32ToInt32(vMin.x);
    FillRect.MinY = RoundReal32ToInt32(vMin.y);
    FillRect.MaxX = RoundReal32ToInt32(vMax.x);
    FillRect.MaxY = RoundReal32ToInt32(vMax.y);

	FillRect = Intersect(FillRect, ClipRect);
	if (!Even == (FillRect.MinY & 1))
	{
		FillRect.MinY += 1;
	}
	
    uint32 Color32 = ((RoundReal32ToUInt32(A * 255.0f) << 24) |
					  (RoundReal32ToUInt32(R * 255.0f) << 16) |
					  (RoundReal32ToUInt32(G * 255.0f) << 8) |
					  (RoundReal32ToUInt32(B * 255.0f) << 0));			    

    uint8 *Row = ((uint8 *)Buffer->Memory
				  + FillRect.MinX * BITMAP_BYTES_PER_PIXEL
				  + FillRect.MinY * Buffer->Pitch);
    
    for (int Y = FillRect.MinY; Y < FillRect.MaxY; Y += 2)    
    {
		uint32 *Pixel = (uint32 *)Row;
		for (int X = FillRect.MinX; X < FillRect.MaxX; ++X)    
		{
			*Pixel++ = Color32;		
		}
	
		Row += 2*Buffer->Pitch;
    }
}

inline vec3
SampleEnvironmentMap(vec2 ScreenSpaceUV, vec3 SampleDirection, real32 Roughness, environment_map *Map,
					 real32 DistanceFromMapInZ)
{
	uint32 LODIndex = (uint32)(Roughness*(real32)(ArrayCount(Map->LOD) - 1) + 0.5f);
	Assert(LODIndex < ArrayCount(Map->LOD));

	loaded_bitmap *LOD = &Map->LOD[LODIndex];

	real32 UVsPerMeter = 0.1f;
	real32 C = (UVsPerMeter*DistanceFromMapInZ) / SampleDirection.y;
	vec2 Offset = C * Vec2(SampleDirection.x, SampleDirection.z);

	vec2 UV = ScreenSpaceUV + Offset;

	UV.x = Clamp01(UV.x);
	UV.y = Clamp01(UV.y);
	
	real32 tX = UV.x*(real32)(LOD->Width - 2);
	real32 tY = UV.y*(real32)(LOD->Height - 2);

	int32 X = (int32)tX;
	int32 Y = (int32)tY;

	real32 fX = tX - X;
	real32 fY = tY - Y;

	Assert((X >= 0) && (X < LOD->Width));
	Assert((Y >= 0) && (Y < LOD->Height));

#if 0
	uint8 *TexelPtr = (uint8 *)LOD->Memory + Y*LOD->Pitch + X*sizeof(uint32);
	*(uint32 *)TexelPtr = 0xFFFFFFFF;
#endif
	
	bilinear_sample LODSample = BilinearSample(LOD, X, Y);
	vec3 Result = SRGBBilinearBlend(LODSample, fX, fY).xyz;;    

	return Result;
}

internal void
DrawRectangleSlowly(loaded_bitmap *Buffer, vec2 Origin, vec2 XAxis, vec2 YAxis, vec4 Color,
					loaded_bitmap *Texture, loaded_bitmap *NormalMap,
					environment_map *Top,
					environment_map *Middle,
					environment_map *Bottom,
					real32 PixelsToMeters)
{
	BEGIN_TIMED_BLOCK(DrawRectangleSlowly);

	Color.rgb *= Color.a;

	real32 XAxisLength = Length(XAxis);
	real32 YAxisLength = Length(YAxis);

	vec2 NXAxis = (YAxisLength / XAxisLength) * XAxis;
	vec2 NYAxis = (XAxisLength / YAxisLength) * YAxis;

	real32 NZScale = 0.5f*(XAxisLength + YAxisLength);
	
	real32 InvXAxisLengthSq = 1.0f / LengthSq(XAxis);
	real32 InvYAxisLengthSq = 1.0f / LengthSq(YAxis);

	uint32 Color32 = ((RoundReal32ToUInt32(Color.a * 255.0f) << 24) |
					  (RoundReal32ToUInt32(Color.r * 255.0f) << 16) |
					  (RoundReal32ToUInt32(Color.g * 255.0f) << 8) |
					  (RoundReal32ToUInt32(Color.b * 255.0f) << 0));

	int WidthMax = Buffer->Width - 1;
	int HeightMax = Buffer->Height - 1;

	real32 InvWidthMax = 1.0f / (real32)WidthMax;
	real32 InvHeightMax = 1.0f/ (real32)HeightMax;
	
	real32 OriginZ = 0.0f;
	real32 OriginY = (Origin + 0.5f*XAxis + 0.5f*YAxis).y;
	real32 FixedCastY = InvHeightMax*OriginY;
	
	int XMin = WidthMax;
	int XMax = 0;
	int YMin = HeightMax;
	int YMax = 0;

	vec2 P[4] = {Origin, Origin + XAxis, Origin + YAxis, Origin + XAxis + YAxis};
	for (int PIndex = 0; PIndex < ArrayCount(P); ++PIndex)
	{
		vec2 TestP = P[PIndex];
		int FloorX = FloorReal32ToInt32(TestP.x);
		int CeilX = CeilReal32ToInt32(TestP.x);
		int FloorY = FloorReal32ToInt32(TestP.y);
		int CeilY = CeilReal32ToInt32(TestP.y);

		if (FloorX < XMin) {XMin = FloorX;}
		if (FloorY < YMin) {YMin = FloorY;}
		if (CeilX > XMax) {XMax = CeilX;}
		if (CeilY > YMax) {YMax = CeilY;}
	}

	if (XMin < 0) {XMin = 0;}
	if (YMin < 0) {YMin = 0;}
	if (XMax > WidthMax) {XMax = WidthMax;}
	if (YMax > HeightMax) {YMax = HeightMax;}

	uint8 *Row = ((uint8 *)Buffer->Memory
				  + XMin * BITMAP_BYTES_PER_PIXEL
				  + YMin * Buffer->Pitch);
	for (int Y = YMin; Y <= YMax; ++Y)
	{
		uint32 *Pixel = (uint32 *)Row;
		for (int X = XMin; X <= XMax; ++X)
		{
			vec2 PxielP = Vec2i(X, Y);
			vec2 d = PxielP - Origin;
			
			real32 Edge0 = Inner(d, -Perp(XAxis));
			real32 Edge1 = Inner(d - XAxis, -Perp(YAxis));
			real32 Edge2 = Inner(d - XAxis - YAxis, Perp(XAxis));
			real32 Edge3 = Inner(d - YAxis, Perp(YAxis));

			if ((Edge0 < 0) &&
				(Edge1 < 0) &&
				(Edge2 < 0) &&
				(Edge3 < 0))
			{
#if 1
				vec2 ScreenSpaceUV = {InvWidthMax*(real32)X, FixedCastY};
				real32 ZDiff = PixelsToMeters*((real32)Y - OriginY);
#else
				vec2 ScreenSpaceUV = {InvWidthMax*(real32)X, InvHeightMax*(real32)Y};
				real32 ZDiff = 0.0f;
#endif
				
				real32 U = InvXAxisLengthSq * Inner(d, XAxis);
				real32 V = InvYAxisLengthSq * Inner(d, YAxis);

#if 0
				Assert((U >= 0) && (U <= 1.0f));
				Assert((V >= 0) && (V <= 1.0f));
#endif
				
				real32 tX = (U * (real32)(Texture->Width - 2));
				real32 tY = (V * (real32)(Texture->Height - 2));

				int32 X = (int32)tX;
				int32 Y = (int32)tY;

				real32 fX = tX - (real32)X;
				real32 fY = tY - (real32)Y;
				
                Assert((X >= 0) && (X < Texture->Width));
                Assert((Y >= 0) && (Y < Texture->Height));

				bilinear_sample TexelSample = BilinearSample(Texture, X, Y);
				vec4 Texel = SRGBBilinearBlend(TexelSample, fX, fY);
				
#if 0
				if (NormalMap)
				{
					bilinear_sample NormalSample = BilinearSample(NormalMap, X, Y);
					
					vec4 NormalA = Unpack4x8(NormalSample.A);
					vec4 NormalB = Unpack4x8(NormalSample.B);
					vec4 NormalC = Unpack4x8(NormalSample.C);
					vec4 NormalD = Unpack4x8(NormalSample.D);

					vec4 Normal = Lerp(fY, Lerp(fX, NormalA, NormalB), Lerp(fX, NormalC, NormalD));

					Normal = UnscaleAndBiasNormal(Normal);

					Normal.xy = Normal.x*NXAxis + Normal.y*NYAxis;
					Normal.z *= NZScale;					
					Normal.xyz = Normalize(Normal.xyz);
					
					vec3 BounceDirection = 2.0f*Normal.z*Normal.xyz;
					BounceDirection.z += -1.0f;					

					BounceDirection.z = -BounceDirection.z;
					
					environment_map *FarMap = 0;
					real32 Pz = OriginZ + ZDiff;
					real32 MapZ = 2.0f;
					real32 tEnvMap = BounceDirection.y;
					real32 tFarMap = 0.0f;
					if (tEnvMap < -0.5f)
					{
						tFarMap = -1.0f - 2.0f*tEnvMap;
						FarMap = Bottom;
					}
					else if (tEnvMap > 0.5f)
					{
						tFarMap = 2.0f*(tEnvMap - 0.5f);
						FarMap = Top;
					}

					tFarMap *= tFarMap;
					tFarMap *= tFarMap;
					
					vec3 LightColor = {0, 0, 0}; 
					if (FarMap)
					{
						real32 DistanceFromMapInZ = FarMap->Pz - Pz;
						vec3 FarMapColor = SampleEnvironmentMap(ScreenSpaceUV, BounceDirection, Normal.w, FarMap,
																DistanceFromMapInZ);
						LightColor = Lerp(tFarMap, LightColor, FarMapColor);
					}

					Texel.rgb = Texel.rgb + Texel.a*LightColor;
#if 0
					Texel.rgb = Vec3(0.5f, 0.5f, 0.5f) + 0.5f*BounceDirection;
					Texel.rgb *= Texel.a;
#endif
				}
#endif

				Texel = Hadamard(Texel, Color);
				Texel.r = Clamp01(Texel.r);
				Texel.g = Clamp01(Texel.g);
				Texel.b = Clamp01(Texel.b);

				vec4 Dest = {(real32)((*Pixel >> 16) & 0xFF),
					         (real32)((*Pixel >> 8) & 0xFF),
					         (real32)((*Pixel >> 0) & 0xFF),
							 (real32)((*Pixel >> 24) & 0xFF)};

				Dest = SRGB255ToLinear1(Dest);
				
				vec4 Blended = (1.0f-Texel.a)*Dest + Texel;

				vec4 Blended255 = Linear1ToSRGB255(Blended);
				
				*Pixel = (((uint32)(Blended255.a + 0.5f) << 24) |
						  ((uint32)(Blended255.r + 0.5f) << 16) |
						  ((uint32)(Blended255.g + 0.5f) << 8) |
						  ((uint32)(Blended255.b + 0.5f) << 0));

//				*Pixel = Color32;


			}

			++Pixel;
		}

		Row += Buffer->Pitch;
	}

	END_TIMED_BLOCK(DrawRectangleSlowly);
}

internal void
DrawRectangleQuickly(loaded_bitmap *Buffer, vec2 Origin, vec2 XAxis, vec2 YAxis, vec4 Color,
					 loaded_bitmap *Texture, real32 PixelsToMeters,
					 rectangle2i ClipRect, bool32 Even)
{
	BEGIN_TIMED_BLOCK(DrawRectangleQuickly);

	Color.rgb *= Color.a;

	real32 XAxisLength = Length(XAxis);
	real32 YAxisLength = Length(YAxis);

	vec2 NXAxis = (YAxisLength / XAxisLength) * XAxis;
	vec2 NYAxis = (XAxisLength / YAxisLength) * YAxis;

	real32 NZScale = 0.5f*(XAxisLength + YAxisLength);
	
	real32 InvXAxisLengthSq = 1.0f / LengthSq(XAxis);
	real32 InvYAxisLengthSq = 1.0f / LengthSq(YAxis);

	rectangle2i FillRect = InvertedInfinityRectangle();

	vec2 P[4] = {Origin, Origin + XAxis, Origin + YAxis, Origin + XAxis + YAxis};
	for (int PIndex = 0; PIndex < ArrayCount(P); ++PIndex)
	{
		vec2 TestP = P[PIndex];
		int FloorX = FloorReal32ToInt32(TestP.x);
		int CeilX = CeilReal32ToInt32(TestP.x) + 1;
		int FloorY = FloorReal32ToInt32(TestP.y);
		int CeilY = CeilReal32ToInt32(TestP.y) + 1;

		if (FloorX < FillRect.MinX) {FillRect.MinX = FloorX;}
		if (FloorY < FillRect.MinY) {FillRect.MinY = FloorY;}
		if (CeilX > FillRect.MaxX) {FillRect.MaxX = CeilX;}
		if (CeilY > FillRect.MaxY) {FillRect.MaxY = CeilY;}
	}

	FillRect = Intersect(FillRect, ClipRect);
	if (!Even == (FillRect.MinY & 1))
	{
		FillRect.MinY += 1;
	}
	
	if (HasArea(FillRect))
	{
		__m128i StartClipMask = _mm_set1_epi8(-1);
		__m128i EndClipMask = _mm_set1_epi8(-1);

		__m128i StartClipMasks[] = {_mm_slli_si128(StartClipMask, 0*4),
			                        _mm_slli_si128(StartClipMask, 1*4),
			                        _mm_slli_si128(StartClipMask, 2*4),
			                        _mm_slli_si128(StartClipMask, 3*4)};

		__m128i EndClipMasks[] = {_mm_srli_si128(EndClipMask, 0*4),
			                      _mm_srli_si128(EndClipMask, 3*4),
			                      _mm_srli_si128(EndClipMask, 2*4),
			                      _mm_srli_si128(EndClipMask, 1*4)};

		uint32 StartMisalign = FillRect.MinX & 3;
		uint32 EndMisalign = FillRect.MaxX & 3;
		if (StartMisalign)
		{
			StartClipMask = StartClipMasks[StartMisalign];
			FillRect.MinX = FillRect.MinX & ~3;
		}
		if (EndMisalign)
		{
			EndClipMask = EndClipMasks[EndMisalign];
			FillRect.MaxX = (FillRect.MaxX & ~3) + 4;
		}
				
		vec2 nXAxis = InvXAxisLengthSq * XAxis;
		vec2 nYAxis = InvYAxisLengthSq * YAxis;

		real32 Inv255 = 1.0f / 255.0f;
		real32 One255 = 255.0f;

		__m128 One = _mm_set1_ps(1.0f);
		__m128 Half = _mm_set1_ps(0.5f);
		__m128 One255_4x = _mm_set1_ps(255.0f);
		__m128 Zero = _mm_set1_ps(0.0f);
		__m128 Half_4x = _mm_set1_ps(0.5f);
		__m128 Four_4x = _mm_set1_ps(4.0f);
		__m128 Inv255_4x = _mm_set1_ps(Inv255);
		__m128i MaskFF = _mm_set1_epi32(0xFF);
		__m128i MaskFFFF = _mm_set1_epi32(0xFFFF);
		__m128i MaskFF00FF = _mm_set1_epi32(0xFF00FF);
		__m128 Colorr_4x = _mm_set1_ps(Color.r);
		__m128 Colorg_4x = _mm_set1_ps(Color.g);
		__m128 Colorb_4x = _mm_set1_ps(Color.b);
		__m128 Colora_4x = _mm_set1_ps(Color.a);
		__m128 nXAxisx_4x = _mm_set1_ps(nXAxis.x);
		__m128 nXAxisy_4x = _mm_set1_ps(nXAxis.y);
		__m128 nYAxisx_4x = _mm_set1_ps(nYAxis.x);
		__m128 nYAxisy_4x = _mm_set1_ps(nYAxis.y);
		__m128 Originx_4x = _mm_set1_ps(Origin.x);
		__m128 Originy_4x = _mm_set1_ps(Origin.y);
		__m128 MaxColorValue = _mm_set1_ps(255.0f*255.0f);
	
		__m128 WidthM2 = _mm_set1_ps((real32)Texture->Width - 2.0f);
		__m128 HeightM2 = _mm_set1_ps((real32)Texture->Height - 2.0f);
	
		uint32 TexturePitch = Texture->Pitch;
		__m128i TexturePitch_4x = _mm_set1_epi32(TexturePitch);
		uint8 *TextureMemory = (uint8 *)Texture->Memory;
		uint8 *Row = ((uint8 *)Buffer->Memory
					  + FillRect.MinX * BITMAP_BYTES_PER_PIXEL
					  + FillRect.MinY * Buffer->Pitch);		
		int32 RowAdvance = Buffer->Pitch*2;
		
		
		int MinX = FillRect.MinX;
		int MinY = FillRect.MinY;
		int MaxX = FillRect.MaxX;
		int MaxY = FillRect.MaxY;

		BEGIN_TIMED_BLOCK(ProcessPixel);

		for (int Y = MinY; Y < MaxY; Y += 2)
		{
			uint32 *Pixel = (uint32 *)Row;

			__m128 PixelPy = _mm_set1_ps((real32)Y);
			PixelPy = _mm_sub_ps(PixelPy, Originy_4x);
			__m128 PynX = _mm_mul_ps(PixelPy, nXAxisy_4x);
			__m128 PynY = _mm_mul_ps(PixelPy, nYAxisy_4x);
			
			__m128 PixelPx = _mm_set_ps((real32)(MinX + 3),
										(real32)(MinX + 2),
										(real32)(MinX + 1),
										(real32)(MinX + 0));
			PixelPx = _mm_sub_ps(PixelPx, Originx_4x);

			__m128i ClipMask = StartClipMask;
			
			for (int XI = MinX; XI < MaxX; XI += 4)
			{
			
#define mm_square(a) _mm_mul_ps(a, a)
#define M(a, i) ((float *)&(a))[i]
#define Mi(a, i) ((int *)&(a))[i]

				__m128 U = _mm_add_ps(_mm_mul_ps(PixelPx, nXAxisx_4x), PynX);
				__m128 V = _mm_add_ps(_mm_mul_ps(PixelPx, nYAxisx_4x), PynY);
			
				__m128i WriteMask = _mm_castps_si128(_mm_and_ps(_mm_and_ps(_mm_cmpge_ps(U, Zero), _mm_cmple_ps(U, One)),
																_mm_and_ps(_mm_cmpge_ps(V, Zero), _mm_cmple_ps(V, One))));

				WriteMask = _mm_and_si128(WriteMask, ClipMask);

				{
					__m128i OriginalDest = _mm_load_si128((__m128i *)Pixel);

					U = _mm_min_ps(_mm_max_ps(U, Zero), One);
					V = _mm_min_ps(_mm_max_ps(V, Zero), One);
			
					__m128 tX = _mm_add_ps(_mm_mul_ps(U, WidthM2), Half);
					__m128 tY = _mm_add_ps(_mm_mul_ps(V, HeightM2), Half);

					__m128i FetchX_4x = _mm_cvttps_epi32(tX);			
					__m128i FetchY_4x = _mm_cvttps_epi32(tY);
						
					__m128 fX = _mm_sub_ps(tX, _mm_cvtepi32_ps(FetchX_4x));
					__m128 fY = _mm_sub_ps(tY, _mm_cvtepi32_ps(FetchY_4x));

					FetchX_4x = _mm_slli_epi32(FetchX_4x, 2);
					FetchY_4x = _mm_or_si128(_mm_mullo_epi16(FetchY_4x, TexturePitch_4x),
											 _mm_slli_epi32(_mm_mulhi_epi16(FetchY_4x, TexturePitch_4x), 16));

					__m128i Fetch_4x = _mm_add_epi32(FetchX_4x, FetchY_4x);
			
					int Fetch0 = Mi(Fetch_4x, 0);
					int Fetch1 = Mi(Fetch_4x, 1);
					int Fetch2 = Mi(Fetch_4x, 2);
					int Fetch3 = Mi(Fetch_4x, 3);
						
					uint8 *TexelPtr0 = (uint8 *)TextureMemory + Fetch0;
					uint8 *TexelPtr1 = (uint8 *)TextureMemory + Fetch1;
					uint8 *TexelPtr2 = (uint8 *)TextureMemory + Fetch2;
					uint8 *TexelPtr3 = (uint8 *)TextureMemory + Fetch3;

					__m128i SampleA = _mm_setr_epi32(*(uint32 *)(TexelPtr0),
													 *(uint32 *)(TexelPtr1),
													 *(uint32 *)(TexelPtr2),
													 *(uint32 *)(TexelPtr3));
			
					__m128i SampleB = _mm_setr_epi32(*(uint32 *)(TexelPtr0 + sizeof(uint32)),
													 *(uint32 *)(TexelPtr1 + sizeof(uint32)),
													 *(uint32 *)(TexelPtr2 + sizeof(uint32)),
													 *(uint32 *)(TexelPtr3 + sizeof(uint32)));

					__m128i SampleC = _mm_setr_epi32(*(uint32 *)(TexelPtr0 + TexturePitch),
													 *(uint32 *)(TexelPtr1 + TexturePitch),
													 *(uint32 *)(TexelPtr2 + TexturePitch),
													 *(uint32 *)(TexelPtr3 + TexturePitch));
			
					__m128i SampleD = _mm_setr_epi32(*(uint32 *)(TexelPtr0 + TexturePitch + sizeof(uint32)),
													 *(uint32 *)(TexelPtr1 + TexturePitch + sizeof(uint32)),
													 *(uint32 *)(TexelPtr2 + TexturePitch + sizeof(uint32)),
													 *(uint32 *)(TexelPtr3 + TexturePitch + sizeof(uint32)));

					//NOTE; Unpack bilinear samples
					__m128i TexelArb = _mm_and_si128(SampleA, MaskFF00FF);
					__m128i TexelAag = _mm_and_si128(_mm_srli_epi32(SampleA, 8), MaskFF00FF);
					TexelArb = _mm_mullo_epi16(TexelArb, TexelArb);
					__m128 TexelAa = _mm_cvtepi32_ps(_mm_srli_epi32(TexelAag, 16));
					TexelAag = _mm_mullo_epi16(TexelAag, TexelAag);

					__m128i TexelBrb = _mm_and_si128(SampleA, MaskFF00FF);
					__m128i TexelBag = _mm_and_si128(_mm_srli_epi32(SampleA, 8), MaskFF00FF);
					TexelBrb = _mm_mullo_epi16(TexelBrb, TexelBrb);
					__m128 TexelBa = _mm_cvtepi32_ps(_mm_srli_epi32(TexelBag, 16));
					TexelBag = _mm_mullo_epi16(TexelBag, TexelBag);

					__m128i TexelCrb = _mm_and_si128(SampleA, MaskFF00FF);
					__m128i TexelCag = _mm_and_si128(_mm_srli_epi32(SampleA, 8), MaskFF00FF);
					TexelCrb = _mm_mullo_epi16(TexelCrb, TexelCrb);
					__m128 TexelCa = _mm_cvtepi32_ps(_mm_srli_epi32(TexelCag, 16));
					TexelCag = _mm_mullo_epi16(TexelCag, TexelCag);

					__m128i TexelDrb = _mm_and_si128(SampleA, MaskFF00FF);
					__m128i TexelDag = _mm_and_si128(_mm_srli_epi32(SampleA, 8), MaskFF00FF);
					TexelDrb = _mm_mullo_epi16(TexelDrb, TexelDrb);
					__m128 TexelDa = _mm_cvtepi32_ps(_mm_srli_epi32(TexelDag, 16));
					TexelDag = _mm_mullo_epi16(TexelDag, TexelDag);
			
					//NOTE; Load Destination
					__m128 Destr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 16), MaskFF));
					__m128 Destg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 8), MaskFF));
					__m128 Destb = _mm_cvtepi32_ps(_mm_and_si128(OriginalDest, MaskFF));
					__m128 Desta = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 24), MaskFF));

					//NOTE; Convert texture from sRGB to linear brightness space				   
					__m128 TexelAr = _mm_cvtepi32_ps(_mm_srli_epi32(TexelArb, 16));
					__m128 TexelAg = _mm_cvtepi32_ps(_mm_and_si128(TexelAag, MaskFFFF));
					__m128 TexelAb = _mm_cvtepi32_ps(_mm_and_si128(TexelArb, MaskFFFF));

					__m128 TexelBr = _mm_cvtepi32_ps(_mm_srli_epi32(TexelBrb, 16));
					__m128 TexelBg = _mm_cvtepi32_ps(_mm_and_si128(TexelBag, MaskFFFF));
					__m128 TexelBb = _mm_cvtepi32_ps(_mm_and_si128(TexelBrb, MaskFFFF));

					__m128 TexelCr = _mm_cvtepi32_ps(_mm_srli_epi32(TexelCrb, 16));
					__m128 TexelCg = _mm_cvtepi32_ps(_mm_and_si128(TexelCag, MaskFFFF));
					__m128 TexelCb = _mm_cvtepi32_ps(_mm_and_si128(TexelCrb, MaskFFFF));

					__m128 TexelDr = _mm_cvtepi32_ps(_mm_srli_epi32(TexelDrb, 16));
					__m128 TexelDg = _mm_cvtepi32_ps(_mm_and_si128(TexelDag, MaskFFFF));
					__m128 TexelDb = _mm_cvtepi32_ps(_mm_and_si128(TexelDrb, MaskFFFF));

					//NOTE; Bilinear texture blend
					__m128 ifX = _mm_sub_ps(One, fX);
					__m128 ifY = _mm_sub_ps(One, fY);

					__m128 l0 = _mm_mul_ps(ifY, ifX);
					__m128 l1 = _mm_mul_ps(ifY, fX);
					__m128 l2 = _mm_mul_ps(fY, ifX);
					__m128 l3 = _mm_mul_ps(fY, fX);
				
					__m128 Texelr = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAr), _mm_mul_ps(l1, TexelBr)),
											   _mm_add_ps(_mm_mul_ps(l2, TexelCr), _mm_mul_ps(l3, TexelDr)));
					__m128 Texelg = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAg), _mm_mul_ps(l1, TexelBg)),
											   _mm_add_ps(_mm_mul_ps(l2, TexelCg), _mm_mul_ps(l3, TexelDg)));
					__m128 Texelb = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAb), _mm_mul_ps(l1, TexelBb)),
											   _mm_add_ps(_mm_mul_ps(l2, TexelCb), _mm_mul_ps(l3, TexelDb)));
					__m128 Texela = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAa), _mm_mul_ps(l1, TexelBa)),
											   _mm_add_ps(_mm_mul_ps(l2, TexelCa), _mm_mul_ps(l3, TexelDa)));

					//NOTE; Moderate by incoming color
					Texelr = _mm_mul_ps(Texelr, Colorr_4x);
					Texelg = _mm_mul_ps(Texelg, Colorg_4x);
					Texelb = _mm_mul_ps(Texelb, Colorb_4x);
					Texela = _mm_mul_ps(Texela, Colora_4x);

					Texelr = _mm_min_ps(_mm_max_ps(Texelr, Zero), MaxColorValue);
					Texelg = _mm_min_ps(_mm_max_ps(Texelg, Zero), MaxColorValue);
					Texelb = _mm_min_ps(_mm_max_ps(Texelb, Zero), MaxColorValue);

					//NOTE; Convert from sRGB to linear brightness space
					Destr = mm_square(Destr);
					Destg = mm_square(Destg);
					Destb = mm_square(Destb);
					Desta = Desta;

					//NOTE; Destination blend
					__m128 InvTexelA = _mm_sub_ps(One, _mm_mul_ps(Texela, Inv255_4x));
					__m128 Blendedr = _mm_add_ps(_mm_mul_ps(InvTexelA, Destr), Texelr);
					__m128 Blendedg = _mm_add_ps(_mm_mul_ps(InvTexelA, Destg), Texelg);
					__m128 Blendedb = _mm_add_ps(_mm_mul_ps(InvTexelA, Destb), Texelb);
					__m128 Blendeda = _mm_add_ps(_mm_mul_ps(InvTexelA, Desta), Texela);

					//NOTE; Convert from linear to sRGB				
					Blendedr = _mm_mul_ps(Blendedr, _mm_rsqrt_ps(Blendedr));
					Blendedg = _mm_mul_ps(Blendedg, _mm_rsqrt_ps(Blendedg));
					Blendedb = _mm_mul_ps(Blendedb, _mm_rsqrt_ps(Blendedb));
					Blendeda = Blendeda;
			
					__m128i Intr = _mm_cvtps_epi32(Blendedr);
					__m128i Intg = _mm_cvtps_epi32(Blendedg);
					__m128i Intb = _mm_cvtps_epi32(Blendedb);
					__m128i Inta = _mm_cvtps_epi32(Blendeda);

					__m128i Sr = _mm_slli_epi32(Intr, 16);
					__m128i Sg = _mm_slli_epi32(Intg, 8);
					__m128i Sb = Intb;
					__m128i Sa = _mm_slli_epi32(Inta, 24);

					__m128i Out = _mm_or_si128(_mm_or_si128(Sr, Sg), _mm_or_si128(Sb, Sa));

					__m128i MaskedOut = _mm_or_si128(_mm_and_si128(Out, WriteMask),
													 _mm_andnot_si128(WriteMask, OriginalDest));

					_mm_store_si128((__m128i *)Pixel, MaskedOut);
				}
				
				PixelPx = _mm_add_ps(PixelPx, Four_4x);			
				Pixel += 4;

				if ((XI + 8) < MaxX)
				{
					ClipMask = _mm_set1_epi8(-1);
				}
				else
				{
					ClipMask = EndClipMask;
				}
			}

			Row += RowAdvance;
		}
	
		END_TIMED_COUNTED(ProcessPixel, GetClampedRectArea(FillRect));		
	}

	END_TIMED_BLOCK(DrawRectangleQuickly);
}

internal void
DrawMatte(loaded_bitmap *Buffer, loaded_bitmap *Bitmap,
          real32 RealX, real32 RealY, real32 CAlpha = 1.0f)
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
            // TODO(casey): Check this for math errors
//            real32 A = 255.0f*(RSA + RDA - RSA*RDA);
            real32 A = InvRSA*DA;
            real32 R = InvRSA*DR;
            real32 G = InvRSA*DG;
            real32 B = InvRSA*DB;

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
            vec4 Texel = {(real32)((*Source >> 16) & 0xFF),
				          (real32)((*Source >> 8) & 0xFF),
				          (real32)((*Source >> 0) & 0xFF),
				          (real32)((*Source >> 24) & 0xFF)};

			Texel = SRGB255ToLinear1(Texel);

			Texel *= CAlpha;
			
			vec4 D = {(real32)((*Dest >> 16) & 0xFF),
				      (real32)((*Dest >> 8) & 0xFF),
				      (real32)((*Dest >> 0) & 0xFF),
					  (real32)((*Dest >> 24) & 0xFF)};
			
			D = SRGB255ToLinear1(D);

			vec4 Result = (1.0f-Texel.a)*D + Texel;

			Result = Linear1ToSRGB255(Result);
			
            *Dest = (((uint32)(Result.a + 0.5f) << 24) |
                     ((uint32)(Result.r + 0.5f) << 16) |
                     ((uint32)(Result.g + 0.5f) << 8) |
                     ((uint32)(Result.b + 0.5f) << 0));
            
            ++Dest;
            ++Source;
        }

        DestRow += Buffer->Pitch;
        SourceRow += Bitmap->Pitch;
    }
}

internal void
ChangeSaturation(loaded_bitmap *Buffer, real32 Level)
{
	uint8 *DestRow = (uint8 *)Buffer->Memory;
	for (int32 Y = 0; Y < Buffer->Height; ++Y)
	{
		uint32 *Dest = (uint32 *)DestRow;
		for (int32 X = 0; X < Buffer->Width; ++X)
		{
			vec4 D = Unpack4x8(*Dest);

			D = SRGB255ToLinear1(D);

			real32 Avg = (1.0f / 3.0f) * (D.r + D.g + D.b);
			vec3 Delta = Vec3(D.r - Avg, D.g - Avg, D.b - Avg);

			vec4 Result = Vec4(Vec3(Avg, Avg, Avg) + Delta*Level, D.a);

			Result = Linear1ToSRGB255(Result);

			*Dest++ = (((uint8)(Result.a + 0.5f) << 24) |
					   ((uint8)(Result.r + 0.5f) << 16) |
					   ((uint8)(Result.g + 0.5f) << 8) |
					   ((uint8)(Result.b + 0.5f) << 0));
		}

		DestRow += Buffer->Pitch;
	}
}

internal render_group *
AllocateRenderGroup(game_assets *Assets, memory_arena *Arena, uint32 MaxPushBufferSize)
{
	render_group *Result = PushStruct(Arena, render_group);

	if (MaxPushBufferSize == 0)
	{
		MaxPushBufferSize = (uint32)GetArenaSizeRemaining(Arena);
	}
	Result->PushBufferBase = (uint8 *)PushSize(Arena, MaxPushBufferSize);

	Result->MaxPushBufferSize = MaxPushBufferSize;
	Result->PushBufferSize = 0;

	Result->Assets = Assets;
	Result->GlobalAlpha = 1.0f;
	
	Result->Transform.OffsetP = Vec3(0.0f, 0.0f, 0.0f);
	Result->Transform.Scale = 1.0f;

	return Result;
}

inline void
Perspective(render_group *RenderGroup, int32 PixelWidth, int32 PixelHeight, real32 MetersToPixels,
			real32 FocalLength, real32 DistanceAboveTarget)
{
	real32 PixelsToMeters = SafeRatio1(1.0f, MetersToPixels);
	RenderGroup->MonitorHalfDimInMeters = {0.5f*PixelWidth*PixelsToMeters,
		                                   0.5f*PixelHeight*PixelsToMeters};

	RenderGroup->Transform.MetersToPixels = MetersToPixels;
	RenderGroup->Transform.FocalLength = FocalLength;
	RenderGroup->Transform.DistanceAboveTarget = DistanceAboveTarget;
	RenderGroup->Transform.ScreenCenter = Vec2(0.5f*PixelWidth,
											   0.5f*PixelHeight);

	RenderGroup->Transform.Orthographic = false;
}

inline void
Orthographic(render_group *RenderGroup, int32 PixelWidth, int32 PixelHeight, real32 MetersToPixels)
{
	real32 PixelsToMeters = SafeRatio1(1.0f, MetersToPixels);
	RenderGroup->MonitorHalfDimInMeters = {0.5f*PixelWidth*PixelsToMeters,
		                                   0.5f*PixelHeight*PixelsToMeters};

	RenderGroup->Transform.MetersToPixels = MetersToPixels;
	RenderGroup->Transform.FocalLength = 1.0f;
	RenderGroup->Transform.DistanceAboveTarget = 1.0f;
	RenderGroup->Transform.ScreenCenter = Vec2(0.5f*PixelWidth,
											   0.5f*PixelHeight);

	RenderGroup->Transform.Orthographic = true;
}

struct entity_basis_p_result
{
	vec2 P;
	real32 Scale;
	bool32 Valid;
};
inline entity_basis_p_result
GetRenderEntityBasisP(render_transform *Transform, vec3 OriginalP)   
{	
	entity_basis_p_result Result = {};

	vec3 P = Vec3(OriginalP.xy, 0.0f) + Transform->OffsetP;

	if (Transform->Orthographic)
	{
		Result.Scale = Transform->MetersToPixels;
		Result.P = Transform->ScreenCenter + Transform->MetersToPixels*P.xy;
		Result.Valid = true;
	}
	else
	{
		real32 OffsetZ = 0.0f;

		real32 DistanceAboveTarget = Transform->DistanceAboveTarget;

#if 0
		DistanceAboveTarget += 50.0f;
#endif
	
		real32 DistanceToPZ = DistanceAboveTarget - P.z;
		real32 NearClipPlane = 0.2f;
	
		vec3 RawXY = Vec3(P.xy, 1.0f); // NOTE; isnt offset in pixels? why can we just add them together?

		if (DistanceToPZ > NearClipPlane)
		{
			vec3 ProjectedXY = (1.0f / DistanceToPZ) * Transform->FocalLength*RawXY;
			Result.Scale = Transform->MetersToPixels*ProjectedXY.z;
			Result.P = Transform->ScreenCenter + Transform->MetersToPixels*ProjectedXY.xy + Vec2(0.0f, Result.Scale*OffsetZ);
			Result.Valid = true;
		}
	}
	
	return Result;
}

internal void
RenderGroupToOutput(render_group *RenderGroup, loaded_bitmap *OutputBuffer,
					rectangle2i ClipRect, bool32 Even)
{
	BEGIN_TIMED_BLOCK(RenderGroupToOutput);

	real32 NullPixelsToMeters = 1.0f;
   
	for (uint32 BaseAddress = 0; BaseAddress < RenderGroup->PushBufferSize;)
	{
		render_group_entry_header *Header = (render_group_entry_header *)(RenderGroup->PushBufferBase + BaseAddress);
		BaseAddress += sizeof(*Header);

		void *Data = (uint8 *)Header + sizeof(*Header);
		switch (Header->Type)
		{
		    case RenderGroupEntryType_render_entry_clear:
     		{
     			render_entry_clear *Entry = (render_entry_clear *)Data;

				vec4 Color = Entry->Color;
				DrawRectangle(OutputBuffer, Vec2(0, 0), Vec2i(OutputBuffer->Width, OutputBuffer->Height), Color,
							  ClipRect, Even);

     			BaseAddress += sizeof(*Entry);
     		} break;
     			 
		    case RenderGroupEntryType_render_entry_bitmap:
     		{
     			render_entry_bitmap *Entry = (render_entry_bitmap *)Data;
				
#if 0
				DrawBitmap(OutputTarget, Entry->Bitmap, P.x, P.y, Entry->Color.a);
#endif
      			DrawRectangleQuickly(OutputBuffer, Entry->P,
									 Vec2(Entry->Size.x, 0),
									 Vec2(0, Entry->Size.y),
									 Entry->Color,
									 Entry->Bitmap, NullPixelsToMeters,
									 ClipRect, Even);

     			BaseAddress += sizeof(*Entry);
     		} break;
     
     		case RenderGroupEntryType_render_entry_rectangle:
     		{
     			render_entry_rectangle *Entry = (render_entry_rectangle *)Data;
     				
     			DrawRectangle(OutputBuffer, Entry->P, Entry->P + Entry->Dim, Entry->Color,
							  ClipRect, Even);

     			BaseAddress += sizeof(*Entry);			
     		} break;
     
		    case RenderGroupEntryType_render_entry_coordinate_system:
		    {
				render_entry_coordinate_system *Entry = (render_entry_coordinate_system *)Data;

#if 0
				DrawRectangleSlowly(OutputBuffer, Entry->Origin, Entry->XAxis, Entry->YAxis, Entry->Color,
									Entry->Texture, Entry->NormalMap,
									Entry->Top, Entry->Middle, Entry->Bottom,
									PixelsToMeters);

				vec4 Color = {1, 1, 0, 1};
				vec2 Dim = {2, 2};
				vec2 P = Entry->Origin;
				vec2 vMax = Entry->Origin + Entry->XAxis + Entry->YAxis;
				
				DrawRectangle(OutputBuffer, P - Dim, P + Dim, Color);

				P = Entry->Origin + Entry->XAxis;
				DrawRectangle(OutputBuffer, P - Dim, P + Dim, Color);

				P = Entry->Origin + Entry->YAxis;
				DrawRectangle(OutputBuffer, P - Dim, P + Dim, Color);

				DrawRectangle(OutputBuffer, vMax - Dim, vMax + Dim, Color);
				
#if 0
				for (uint32 Index = 0; Index < ArrayCount(Entry->Points); ++Index)
				{
					vec2 Point = Entry->Points[Index];
					P = Entry->Origin + Point.x*Entry->XAxis + Point.y*Entry->YAxis;
					DrawRectangle(OutputBuffer, P - Dim, P + Dim, Color);
				}
#endif
#endif
				BaseAddress += sizeof(*Entry);
    		} break;

			InvalidDefaultCase;
		}		
	}

	END_TIMED_BLOCK(RenderGroupToOutput);
}

struct tile_render_work
{
	render_group *RenderGroup;
	loaded_bitmap *OutputTarget;
	rectangle2i ClipRect;
};

internal PLATFORM_WORK_QUEUE_CALLBACK(DoTileRenderWork)
{
	tile_render_work *Work = (tile_render_work *)Data;

	RenderGroupToOutput(Work->RenderGroup, Work->OutputTarget, Work->ClipRect, true);
	RenderGroupToOutput(Work->RenderGroup, Work->OutputTarget, Work->ClipRect, false);
}

internal void
RenderGroupToOutput(render_group *RenderGroup, loaded_bitmap *OutputTarget)
{	
	Assert(((uintptr)OutputTarget->Memory & 15) == 0);

	rectangle2i ClipRect;
	ClipRect.MinX = 0;
	ClipRect.MaxX = OutputTarget->Width;
	ClipRect.MinY = 0;
	ClipRect.MaxY = OutputTarget->Height;
			
	tile_render_work Work;
	Work.RenderGroup = RenderGroup;
	Work.OutputTarget = OutputTarget;
	Work.ClipRect = ClipRect;

	DoTileRenderWork(0, &Work);
}

internal void
TiledRenderGroupToOutput(platform_work_queue *RenderQueue,
						 render_group *RenderGroup, loaded_bitmap *OutputTarget)
{
	int const TileCountX = 4;
	int const TileCountY = 4;
	tile_render_work WorkArray[TileCountX*TileCountY];
	
	Assert(((uintptr)OutputTarget->Memory & 15) == 0);
	int TileWidth = OutputTarget->Width / TileCountX;
	int TileHeight = OutputTarget->Height / TileCountY;

	TileWidth = ((TileWidth + 3) / 4) * 4;
	
	int WorkCount = 0;
	for (int TileY = 0; TileY < TileCountY; ++TileY)
	{
		for (int TileX = 0; TileX < TileCountX; ++TileX)
		{
			tile_render_work *Work = WorkArray + WorkCount++;

			rectangle2i ClipRect;
			ClipRect.MinX = TileX*TileWidth;
			ClipRect.MaxX = ClipRect.MinX + TileWidth;
			ClipRect.MinY = TileY*TileHeight;
			ClipRect.MaxY = ClipRect.MinY + TileHeight;

			if (TileX == (TileCountX - 1))
			{
				ClipRect.MaxX = OutputTarget->Width;				
			}
			if (TileY == (TileCountY - 1))
			{
				ClipRect.MaxY = OutputTarget->Height;
			}
			
			Work->RenderGroup = RenderGroup;
			Work->OutputTarget = OutputTarget;
			Work->ClipRect = ClipRect;

#if 1
			PlatformAddEntry(RenderQueue, DoTileRenderWork, Work);
#else
			DoTileRenderWork(RenderQueue, Work);
#endif
		}
	}

	PlatformCompleteAllWork(RenderQueue);
}

#define PushRenderElement(Group, type) (type *)_PushRenderElement(Group, sizeof(type), RenderGroupEntryType_##type)
inline void *
_PushRenderElement(render_group *Group, uint32 Size, render_group_entry_type Type)
{
	void *Result = 0;

	Size += sizeof(render_group_entry_header);
	
	if (Group->PushBufferSize + Size < Group->MaxPushBufferSize)
	{
		render_group_entry_header *Header = (render_group_entry_header *)(Group->PushBufferBase + Group->PushBufferSize);
		Header->Type = Type;
		Result = (uint8 *)Header + sizeof(*Header);
		Group->PushBufferSize += Size;		
	}
	else
	{
		InvalidCodePath;
	}

	return Result;
}

inline void
PushBitmap(render_group *RenderGroup, loaded_bitmap *Bitmap, real32 Height, vec3 Offset, vec4 Color=Vec4(1, 1, 1, 1))
{
	vec2 Size = Vec2(Height*Bitmap->WidthOverHeight, Height);
	vec2 Align = Hadamard(Bitmap->AlignPercentage, Size);
	vec3 P = Offset - Vec3(Align, 0);

	entity_basis_p_result Basis = GetRenderEntityBasisP(&RenderGroup->Transform, P);
	if (Basis.Valid)
	{
		render_entry_bitmap *Entry = PushRenderElement(RenderGroup, render_entry_bitmap);
		if (Entry)
		{
			Entry->Bitmap = Bitmap;
			Entry->P = Basis.P;
			Entry->Color = RenderGroup->GlobalAlpha*Color;
			Entry->Size = Basis.Scale*Size;
		}
	}
}

inline void
PushBitmap(render_group *RenderGroup, game_asset_id ID, real32 Height, vec3 Offset, vec4 Color=Vec4(1, 1, 1, 1))
{
	loaded_bitmap *Bitmap = GetBitmap(RenderGroup->Assets, ID);
	if (Bitmap)
	{
		PushBitmap(RenderGroup, Bitmap, Height, Offset, Color);
	}
	else
	{
		LoadAsset(RenderGroup->Assets, ID);
	}
}

inline void
PushRect(render_group *RenderGroup, vec3 Offset, vec2 Dim, vec4 Color)
{
	vec3 P = Offset - Vec3(0.5f*Dim, 0);

	entity_basis_p_result Basis = GetRenderEntityBasisP(&RenderGroup->Transform, P);

	if (Basis.Valid)
	{
		render_entry_rectangle *Entry = PushRenderElement(RenderGroup, render_entry_rectangle);
		if (Entry)
		{
			Entry->P = Basis.P;
			Entry->Color = Color;
			Entry->Dim = Basis.Scale*Dim;
		}
	}
}

inline void
PushRectOutline(render_group *RenderGroup, vec3 Offset, vec2 Dim, vec4 Color)
{
	real32 Thickness = 0.1f;
	
	PushRect(RenderGroup, Offset - Vec3(0, 0.5f*Dim.y, 0), Vec2(Dim.x, Thickness), Color);
	PushRect(RenderGroup, Offset + Vec3(0, 0.5f*Dim.y, 0), Vec2(Dim.x, Thickness), Color);

	PushRect(RenderGroup, Offset - Vec3(0.5f*Dim.x, 0, 0), Vec2(Thickness, Dim.y), Color);
	PushRect(RenderGroup, Offset + Vec3(0.5f*Dim.x, 0, 0), Vec2(Thickness, Dim.y), Color);
}

inline void
Clear(render_group *RenderGroup, vec4 Color)
{
	render_entry_clear *Entry = PushRenderElement(RenderGroup, render_entry_clear);
	if (Entry)
	{
		Entry->Color = Color;
	}
}

inline render_entry_coordinate_system *
CoordinateSystem(render_group *RenderGroup, vec2 Origin, vec2 XAxis, vec2 YAxis, vec4 Color,
				 loaded_bitmap *Texture, loaded_bitmap *NormalMap,
				 environment_map *Top, environment_map *Middle, environment_map *Bottom)
{
	render_entry_coordinate_system *Entry = PushRenderElement(RenderGroup, render_entry_coordinate_system);
	if (Entry)
	{
		Entry->Origin = Origin;
		Entry->XAxis = XAxis;
		Entry->YAxis = YAxis;
		Entry->Color = Color;
		Entry->Texture = Texture;
		Entry->NormalMap = NormalMap;
		Entry->Top = Top;
		Entry->Middle = Middle;
		Entry->Bottom = Bottom;
	}

	return Entry;
}

inline vec2
Unproject(render_group *Group, vec2 ProjectedXY, real32 AtDistanceFromCamera)
{
	vec2 WorldXY = (AtDistanceFromCamera / Group->Transform.FocalLength) * ProjectedXY;

	return WorldXY;
}

inline rectangle2
GetCameraRectangleAtDistance(render_group *Group, real32 AtDistanceFromCamera)
{
	vec2 RawXY = Unproject(Group, Group->MonitorHalfDimInMeters, AtDistanceFromCamera);
	rectangle2 Result = RectCenterHalfDim(Vec2(0, 0), RawXY);
	
	return Result;
}

inline rectangle2
GetCameraRectangleAtTarget(render_group *Group)
{
	rectangle2 Result = GetCameraRectangleAtDistance(Group, Group->Transform.DistanceAboveTarget);

	return Result;
}
