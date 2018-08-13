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

union v2
{
    struct
    {
        f32 X, Y;
    };
    struct
    {
        f32 U, V;
    };
    f32 E[2];
};

union v3f
{
    struct
    {
        f32 X,Y,Z;
    };
    f32 E[3];
};

union v4
{
    struct
    {
        f32 X,Y,Z,W;
    };
    f32 E[4];
};

union mat3
{
    struct
    {
        f32 M00, M01, M02;
        f32 M10, M11, M12;
        f32 M20, M21, M22;
    };
    f32 E[9];
};

union mat4
{
    struct
    {
        f32 M00, M01, M02, M03;
        f32 M10, M11, M12, M13;
        f32 M20, M21, M22, M23;
        f32 M30, M31, M32, M33;
    };
    f32 E[16];
};

struct vertex
{
    f32 X,Y,Z,R;
};

struct texture
{
    u8* Data;
    s32 Width;
    s32 Height;
    s32 N;
};

struct diffuse
{
    f32 Roughness;
};

enum mat_type
{
    DIFFUSE,
    GLOSSY,
    GLASS
};

struct glossy
{
    f32 Roughness;
};

struct glass
{
    f32 IOR;
    f32 Roughness;
};

struct material;

struct mix
{
    material* A;
    material* B;
    f32 Factor;
};

struct material
{
    mat_type Type;
    v3f Emit;
    v3f Colour;
    texture Texture;
    union
    {
        glossy Glossy;
        diffuse Diffuse;
        glass Glass;
        mix Mix;
    };
};


struct face
{
    u32 V0;
    u32 V1;
    u32 V2;
};

struct vertex_attribs
{
    u32 T0;
    u32 T1;
    u32 T2;
    
    u32 N0;
    u32 N1;
    u32 N2;
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
    v3f Rotation;
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

struct image
{
    v3f* Pixels;
    u32 Width, Height;
};

struct thread_info
{
    u32 XMin, XMax;
    u32 YMin, YMax;
    
    image Image;
    RTCScene* Scene;
    material* Materials;
    v2* TexCoords;
    vertex_attribs* Attribs;
    u32* MatIndices;
    face* Faces;
    camera Camera;
    u32 RandomState;
};

struct thread_queue
{
    u32 WorkCount;
    thread_info* ThreadInfos;
    u32 Samples;
    volatile u64 NextThreadIndex;
    volatile u64 UsedTiles;
    u32 TotalSamples;
};


internal u32
XorShift(u32 *State)
{
    u32 X = *State;
    X ^= X << 13;
    X ^= X >> 17;
    X ^= X << 5;
    *State = X;
    return(X);
}

internal inline v3f
V3f(f32 X, f32 Y, f32 Z)
{
    v3f Result = {X, Y, Z};
    return(Result);
}

internal inline ray
Ray(v3f O, v3f D, f32 Near, f32 Far)
{
    ray Result;
    Result.O = O;
    Result.D = D;
    Result.TNear = Near;
    Result.Time = 0.0;
    Result.TFar = Far;
    Result.Mask = -1;
    Result.ID = 0;
    Result.Ng = {0.0, 0.0, 0.0};
    Result.Flags = 0;
    Result.GeomID = RTC_INVALID_GEOMETRY_ID;
    Result.PrimID = RTC_INVALID_GEOMETRY_ID;
    Result.InstID = RTC_INVALID_GEOMETRY_ID;
    return(Result);
}

internal inline f32
RandUnilateral(u32 *RandomState)
{
    return(f32(XorShift(RandomState))/f32(U32Max));
}

internal inline f32
RandBilateral(u32* RandomState)
{
    return 2.0f*RandUnilateral(RandomState) - 1.0f;
}


internal f32
GammaCorrect(f32 L)
{
    if(L < 0.0f)
    {
        L = 0.0f;
    }
    
    if(L > 1.0f)
    {
        L = 1.0f;
    }
    
    f32 S = L*12.92f;
    if(L > 0.0031308f)
    {
        S = 1.055f*pow(L, 1.0f/2.4f) - 0.055f;
    }
    
    return(S);
}
internal v3f
GammaCorrectV3(v3f L)
{
    v3f Result;
    Result.X = GammaCorrect(L.X);
    Result.Y = GammaCorrect(L.Y);
    Result.Z = GammaCorrect(L.Z);
    return(Result);
}
