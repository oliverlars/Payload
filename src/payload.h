#define internal static
#define global static
#define Assert assert

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef s32 b32;
typedef s32 b32x;

typedef float f32;
typedef double f64;

#define U32Max ((u32)-1)
#define F32Max FLT_MAX
#define F32Min -FLT_MAX
#define Pi32 3.14159265359f
#define Tau32 6.28318530717958647692f
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define INF FLT_MAX

union v3f
{
    struct
    {
        f32 X,Y,Z;
    };
    f32 E[3];
};
struct array
{
    array(u64 Size)
    {
        
    }
    
    u64 Size;
    
};

struct vertex
{
    f32 X,Y,Z,R;
};

struct material
{
    v3f Colour;
    v3f Emit;
};

struct face
{
    u32 V0;
    u32 V1;
    u32 V2;
};

struct RTC_ALIGN(16) ray
{
    v3f O;
    f32 TNear;
    v3f D;
    f32 Time;
    f32 TFar;
    u32 Mask;
    u32 ID;
    u32 Flags;
    v3f Ng;
    f32 U;
    f32 V;
    u32 PrimID;
    u32 GeomID;
    u32 InstID;
};

struct camera
{
    v3f Up;
    v3f LookAt;
    v3f Origin;
    f32 FOV;
};

#pragma pack(push, 1)
struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    s32 Width;
    s32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    s32 HorzResolution;
    s32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
};
#pragma pack(pop)

union colour
{
    struct
    {
        u8 B;
        u8 G;
        u8 R;
        u8 A;
    };
    u32 Pack;
};

