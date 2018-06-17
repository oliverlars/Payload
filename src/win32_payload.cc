#include <windows.h>
#include <stdint.h>
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

#define RENDER(name) int name()
typedef RENDER(render);

internal inline void*
Alloc(u64 Size)
{
    void* Result = VirtualAlloc(NULL, Size, MEM_COMMIT, PAGE_READWRITE);
    return(Result);
}

struct win32_payload
{
    HMODULE PayloadCode;
    render* Render;
};

internal win32_payload
InitPayload(char* DLLName)
{
    win32_payload Result = {};
    Result.PayloadCode = LoadLibraryA(DLLName);
    if(Result.PayloadCode)
    {
        Result.Render = (render*)GetProcAddress(Result.PayloadCode, "Render");
    }
    return(Result);
}

internal void
QuitPayload(win32_payload* Payload)
{
    FreeLibrary(Payload->PayloadCode);
}

int main()
{
    win32_payload Payload = InitPayload("Payload.dll");
    Payload.Render();
    QuitPayload(&Payload);
    return(0);
}