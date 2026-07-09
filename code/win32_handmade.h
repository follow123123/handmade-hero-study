#if !defined(WIN32_HANDMADE_H)

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct win32_window_dimension
{
    int Width;
    int Height;
};

struct win32_sound_output
{
    int SamplesPerSecond;
    uint32 RunningSampleIndex;
    int WavePeriod;
    int BytesPerSample;
    int SecondaryBufferSize;
    DWORD SafetyBytes;
    real32 tSine;
    int LatencySampleCount;
};

struct win32_debug_time_marker
{
    DWORD OutputPlayCursor;
    DWORD OutputWriteCursor;
    DWORD OutputLocation;
    DWORD OutputByteCount;
    DWORD ExpectedFlipPlayCursor;
    
    DWORD FlipPlayCursor;
    DWORD FlipWriteCursor;
};

struct win32_state
{
    uint64 TotalSize;
    void *GameMemoryBlock;
  
    HANDLE RecordingHandle;
    int InputRecordingIndex;

    HANDLE PlayBackHandle;
    int InputPlayingIndex;
};

#define WIN32_HANDMADE_H
#endif 
