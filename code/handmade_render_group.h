#if !defined(HANDMADE_RENDER_GROUP_H)
#define HANDMADE_RENDER_GROUP_H

struct loaded_bitmap
{
	vec2 AlignPercentage;
	real32 WidthOverHeight;
	
	int32 Width;
	int32 Height;
	int32 Pitch;
	void *Memory;
};

struct environment_map
{
	loaded_bitmap LOD[4];
	real32 Pz;
};

enum render_group_entry_type
{
	RenderGroupEntryType_render_entry_clear,
	RenderGroupEntryType_render_entry_bitmap,
	RenderGroupEntryType_render_entry_rectangle,
	RenderGroupEntryType_render_entry_coordinate_system,
};

struct render_group_entry_header
{
	render_group_entry_type Type;
};

struct render_entry_clear
{
	vec4 Color;
};

struct render_entry_saturation
{
	real32 Level;
};

struct render_entry_bitmap
{	
	loaded_bitmap *Bitmap;

	vec4 Color;
	vec2 P;
	vec2 Size;
};

struct render_entry_rectangle
{
	vec2 P;
	vec4 Color;
	vec2 Dim;
};

struct render_entry_coordinate_system
{
	vec2 Origin;
	vec2 XAxis;
	vec2 YAxis;
	vec4 Color;
	loaded_bitmap *Texture;
	loaded_bitmap *NormalMap;

	environment_map *Top;
	environment_map *Middle;
	environment_map *Bottom;	
};


struct render_transform
{
	real32 MetersToPixels;
	vec2 ScreenCenter;
	
	real32 FocalLength;
	real32 DistanceAboveTarget;

	vec3 OffsetP;
	real32 Scale;
};

struct render_group
{
	real32 GlobalAlpha;

	vec2 MonitorHalfDimInMeters;
	
	render_transform Transform;

	uint32 MaxPushBufferSize;
	uint32 PushBufferSize;	
	uint8 *PushBufferBase;
};

#endif /* HANDMADE_RENDER_GROUP_H */
