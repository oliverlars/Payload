#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <immintrin.h>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <embree3/include/rtcore.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <windows.h>
#include <float.h>
#include <string.h>
#include "payload.h"
#include "payload_maths.h"


b32x PayloadRunning;

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

internal inline vertex
Vertex(float X, float Y, float Z)
{
    vertex Result;
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
}

global u32 RandomState;

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

internal inline RTCRayHit* 
RayToRTCRayHit(ray *Ray)
{
    return reinterpret_cast<RTCRayHit*>(Ray);
}

internal inline RTCRay*
RayToRTCRay(ray *Ray)
{
    return reinterpret_cast<RTCRay*>(Ray);
}

internal ray
CameraRay(camera* Camera, s32 Width, s32 Height, f32 S, f32 T)
{
    f32 Theta = Camera->FOV * Pi32 / 180.0f;
    f32 HalfHeight = tanf(Theta/2.0f);
    f32 Aspect = (float)Width/(float)Height;
    f32 HalfWidth = HalfHeight*Aspect;
    v3f Eye = Unit(Camera->Origin - Camera->LookAt);
    v3f U = Unit(Cross(Camera->Up, Eye));
    v3f V = Cross(Eye, U);
    v3f BottomLeftCorner = Camera->Origin - HalfWidth*U - V*HalfHeight - Eye;
    v3f Horizontal = 2*U*HalfWidth;
    v3f Vertical = 2*V*HalfHeight;
    
    ray Result;
    Result = Ray(Camera->Origin, BottomLeftCorner + Horizontal*S + Vertical*T - Camera->Origin,
                 0.0f, INF);
    return(Result);
}

internal inline u32
F32ToU32(f32 A)
{
    u32 Result = (u32)(A + 0.5f);
    return(Result);
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

internal inline u32
PackV3(v3f F)
{
    f32 R = 255.f*GammaCorrect(F.X);
    f32 G = 255.f*GammaCorrect(F.Y);
    f32 B = 255.f*GammaCorrect(F.Z);
    f32 A = 255.0f;
    
    u32 Result = ((F32ToU32(A) << 24) |
                  (F32ToU32(R) << 16) |
                  (F32ToU32(G) << 8 ) |
                  (F32ToU32(B) << 0 ));
    return(Result);
}

internal inline v3f
UnpackV3(u32 A)
{
    colour* Col = reinterpret_cast<colour*>(&A);
    v3f Result;
    Result.X = f32(Col->R)/255.99f;
    Result.Y = f32(Col->G)/255.99f;
    Result.Z = f32(Col->B)/255.99f;
    return(Result);
}

internal inline v3f
ColourToV3(colour A)
{
    v3f Result;
    Result.X = f32(A.R)/255.99;
    Result.Y = f32(A.G)/255.99;
    Result.Z = f32(A.B)/255.99;
    return(Result);
}

internal inline f32
RandUnilateral()
{
    return(f32(XorShift(&RandomState))/f32(U32Max));
}

internal inline f32
RandBilateral()
{
    return 2.0f*RandUnilateral() - 1.0f;
}

internal inline v3f
Hadamard(v3f A, v3f B)
{
    v3f Result = {A.X*B.X, A.Y*B.Y, A.Z*B.Z};
    return(Result);
}

void LoadOBJ(char* Filename, 
             vertex** Vertices,
             face** Faces,
             RTCGeometry *Mesh,
             material** Materials)
{
    FILE* File = fopen(Filename, "r");
    char Line[256];
    char Type[3];
    s64 NumberOfLines = 0;
    s64 NumberOfVertices = 0;
    s64 NumberOfFaces = 0;
    if(File)
    {
        while(fgets(Line, sizeof(Line), File))
        {
            NumberOfLines++;
            if(strstr(Line, "v ") != NULL)
            {
                NumberOfVertices++;
                
            }
            if(strstr(Line, "f ") != NULL)
            {
                NumberOfFaces++;
            }
        }
    }
    rewind(File);
    *Vertices =(vertex*)
        rtcSetNewGeometryBuffer(*Mesh,RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,sizeof(vertex),NumberOfVertices);
    
    vertex* VerticePtr = *Vertices;
    *Faces = (face*)
        rtcSetNewGeometryBuffer(*Mesh,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT3,sizeof(face),NumberOfFaces);
    
    *Materials = (material*)malloc(NumberOfFaces*sizeof(material));
    material* MatPtr = *Materials;
    for(u32 I = 0; I < NumberOfFaces; I++)
    {
#if 0
        MatPtr->R = rand() % 255;
        MatPtr->G = rand() % 255;
        MatPtr->A = 255;
#endif
        
        if(I > NumberOfFaces-3)
        {
            MatPtr->Emit  = {1.f, 1.f, 1.f};
            MatPtr->Colour = {0.,0.,0.};
        }
        else
        {
            MatPtr->Emit = {0.0, 0.0, 0.0};
            MatPtr->Colour = {0.8f, 0.8f, 0.8f};
        }
        *MatPtr++;
    }
    face* FacePtr = *Faces;
    
    while(fgets(Line, sizeof(Line), File))
    {
        if(strstr(Line, "v ") != NULL)
        {
            vertex Vert = {};
            f32 X = 0; f32 Y = 0; f32 Z = 0;
            sscanf(Line, "%s %f %f %f", Type,&(X), &(Y), &(Z));
            Vert.X = X;
            Vert.Y = Y;
            Vert.Z = Z;
            *VerticePtr++ = Vert;
        }
        
        if(strstr(Line, "f ") != NULL)
        {
            face Tri = {};
            u32 V0 = 0; u32 V1 = 0; u32 V2 = 0;
            u32 A,B,C,D,E,F;
            sscanf(Line, 
                   "%s %u/%u/%u %u/%u/%u %u/%u/%u",
                   Type, &(V0), &A, &B, &(V1), &C, &D, &(V2), &E, &F);
            Tri.V0 = V0-1;
            Tri.V1 = V1-1;
            Tri.V2 = V2-1;
            *FacePtr++ = Tri;
        }
    }
}

internal void
Pathtrace(camera* Camera, 
          u32 W, u32 H,
          v3f* Image,
          RTCScene* Scene,
          material* Colours)
{
    
    RTCIntersectContext Context;
    rtcInitIntersectContext(&Context);
    
    for(s32 Y = 0; Y < H; Y++)
    {
        for(s32 X = 0; X < W; X++)
        {
            ray Persp = CameraRay(Camera, W, H, 
                                  ((float)X + RandBilateral())/float(W), ((float)Y + RandBilateral())/float(H));
            u32 BounceCount;
            
            v3f Result = {0.f,0.f,0.f};
            v3f Attenuation = V3f(1.,1.,1.);
            v3f Sky = V3f(0.5, 0.5, 0.5);
            for(BounceCount = 1; BounceCount < 10; BounceCount++)
            {
                rtcIntersect1(*Scene, &Context, RayToRTCRayHit(&Persp));
                
                if(Persp.GeomID != RTC_INVALID_GEOMETRY_ID)
                {
                    v3f Albedo = Colours[Persp.PrimID].Colour;
                    v3f Emit =  Colours[Persp.PrimID].Emit;
                    Result = Result + Hadamard(Emit, Attenuation);
                    if(LengthSqrd(Emit) > 0.0f)
                    {
                        Image[X + Y*W] = Result/f32(BounceCount);
                        break;
                        
                    }
                    Attenuation = Hadamard(Attenuation, Albedo);
                    Persp = Ray(Persp.O + Persp.TFar*Persp.D,
                                Unit(Persp.Ng) + V3f(RandBilateral(), RandBilateral(), RandBilateral()),
                                0.001f, INF);
                }
                else
                {
                    Result = Result + Hadamard(Attenuation,Sky);
                    Image[X + Y*W] = Result/f32(BounceCount);
                    break;
                }
                
            }
        }
    }
    
}

internal u64
LockedAdd(u64 volatile* A, u64 B)
{
    u64 Result = InterlockedExchangeAdd64((volatile LONG64*)A, B);
    return(Result);
}

internal b32x
RenderTile(thread_queue* Queue)
{
    
    RTCIntersectContext Context;
    rtcInitIntersectContext(&Context);
    u64 NextIndex = LockedAdd(&Queue->NextThreadIndex, 1);
    
    if(NextIndex >= Queue->WorkCount)
    {
        return(false);
    }
    
    thread_info* Info = Queue->ThreadInfos + NextIndex;
    
    image Image = Info->Image;
    u32 XMin = Info->XMin;
    u32 YMin = Info->YMin;
    u32 XMax = Info->XMax;
    u32 YMax = Info->YMax;
    
    camera Camera = Info->Camera;
    RTCScene* Scene = Info->Scene;
    material* Colours = Info->Colours;
    
    for(s32 Y = YMin; Y < YMax; Y++)
    {
        for(s32 X = XMin; X < XMax; X++)
        {
            f32 Contrib = 1.0f/f32(Queue->Samples);
            v3f Col = {};
            for(s32 S = 1; S < Queue->Samples; S++)
            {
                ray Persp = CameraRay(&Camera, Image.Width, Image.Height, 
                                      ((float)X + RandBilateral())/float(Image.Width), ((float)Y + RandBilateral())/float(Image.Height));
                
                u32 BounceCount;
                v3f Result = {0.f,0.f,0.f};
                v3f Attenuation = V3f(1.,1.,1.);
                v3f Sky = {};
                
                for(BounceCount = 1; BounceCount < 10; BounceCount++)
                {
                    rtcIntersect1(*Scene, &Context, RayToRTCRayHit(&Persp));
                    
                    if(Persp.GeomID != RTC_INVALID_GEOMETRY_ID)
                    {
                        v3f Albedo = Colours[Persp.PrimID].Colour;
                        v3f Emit =  Colours[Persp.PrimID].Emit;
                        Result = Result + Hadamard(Emit, Attenuation);
                        if(LengthSqrd(Emit) > 0.0f)
                        {
                            Col = Col + Result/f32(BounceCount);
                            break;
                            
                        }
                        Attenuation = Hadamard(Attenuation, Albedo);
                        Persp = Ray(Persp.O + Persp.TFar*Persp.D,
                                    Unit(Persp.Ng) + V3f(RandBilateral(), RandBilateral(), RandBilateral()),
                                    0.001f, INF);
                    }
                    else
                    {
                        Result = Result + Hadamard(Attenuation,Sky);
                        Col = Col + Result/f32(BounceCount);
                        break;
                    }
                    
                }
                Image.Pixels[X + Y*Image.Width] = Contrib*Col;
            }
        }
    }
    LockedAdd(&Queue->UsedTiles, 1);
    return(true);
}

internal DWORD WINAPI
TileThread(void* lpParameter)
{
    thread_queue *Queue = (thread_queue*)lpParameter;
    while(RenderTile(Queue)){}
    return(0);
}

int main(int ArgCount, char** Args)
{
    char *rtconfig = "verbose=0,threads=12";
    
    RTCDevice Device = rtcNewDevice(rtconfig);
    RandomState = 25;
    RTCScene Scene = rtcNewScene(Device);
    rtcSetSceneBuildQuality(Scene, RTC_BUILD_QUALITY_HIGH);
    RTCGeometry Mesh = rtcNewGeometry (Device, RTC_GEOMETRY_TYPE_TRIANGLE);
    
    vertex* V = nullptr;
    face* F = nullptr;
    material* Materials = nullptr;
    printf("Loading OBJ...\n");
    LoadOBJ("Teapotlight.obj", 
            &V, &F, &Mesh, &Materials);
    printf("Finished loading OBJ\n");
    rtcCommitGeometry(Mesh);
    u32 geomID = rtcAttachGeometry(Scene,Mesh);
    rtcReleaseGeometry(Mesh);
    
    rtcCommitScene(Scene);
    s32 W = 1920;
    s32 H = 1080;
    v3f* Pixels = (v3f*)malloc(H*W*sizeof(v3f));
    v3f* PixelPtr = Pixels;
    image Image = {};
    Image.Width = W;
    Image.Height = H;
    Image.Pixels = Pixels;
    
    camera Camera = {};
    Camera.Up = V3f(0,1,0);
    Camera.LookAt = V3f(0, 0, -3);
    Camera.Origin = V3f(0,3,4);
    Camera.FOV = 50.0f;
    u32 Samples = 1;
    f32 Contrib = 1.0f/float(Samples);
    u32 CountSamples = 1;
    
    
    v3f Col = {0.0f, 0.0f, 0.0f};
    //Pathtrace(&Camera,W, H,Pixels,&Scene, Materials);
    
    u32 TileSize = 64;
    u32 TileCountX = (W + TileSize -1)/TileSize;
    u32 TileCountY = (H + TileSize -1)/TileSize;
    
    thread_queue Queue = {};
    Queue.Samples = 100;
    Queue.ThreadInfos = (thread_info*)malloc(TileCountY*TileCountX*sizeof(thread_info));
    
    clock_t Start = clock();
    for(u32 TileY = 0; TileY < TileCountY; TileY++)
    {
        u32 YMin = TileY*TileSize;
        u32 YMax = YMin + TileSize;
        YMax = YMax > H ? H : YMax;
        
        for(u32 TileX = 0; TileX < TileCountX; TileX++)
        {
            u32 XMin = TileX*TileSize;
            u32 XMax = XMin + TileSize;
            XMax = XMax > W ? W : XMax;
            
            thread_info* ThreadInfo = Queue.ThreadInfos + Queue.WorkCount++;
            assert(Queue.WorkCount <= TileCountX*TileCountY);
            ThreadInfo->Scene = &Scene;
            ThreadInfo->Camera = Camera;
            ThreadInfo->Colours = Materials;
            ThreadInfo->Image = Image;
            ThreadInfo->XMin = XMin;
            ThreadInfo->YMin = YMin;
            ThreadInfo->XMax = XMax;
            ThreadInfo->YMax = YMax;
            
        }
    }
    
    
    for(u32 Cores = 1;
        Cores < 12;
        Cores++)
    {
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0,0,TileThread, &Queue, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }
    
    while(Queue.UsedTiles < TileCountX*TileCountY)
    {
        if(RenderTile(&Queue))
        {
            fprintf(stderr, "\rRendering %d%%... ",
                    100*(u32)Queue.UsedTiles/(TileCountY*TileCountX));
            fflush(stdout);
        }
    }
    
    clock_t End = clock();
    clock_t Total = (End - Start);
    
    printf("Rendering Took: %dms\n", Total);
    
    bitmap_header Header = {};
    Header.FileType = 0x4D42;
    Header.FileSize = sizeof(Header) + W*H*sizeof(u32);
    Header.BitmapOffset = sizeof(Header);
    Header.Size = sizeof(Header) - 14;
    Header.Width = W;
    Header.Height = H;
    Header.Planes = 1;
    Header.BitsPerPixel = 32;
    Header.Compression = 0;
    Header.SizeOfBitmap = W*H*sizeof(u32);
    Header.HorzResolution = 0;
    Header.VertResolution = 0;
    Header.ColorsUsed = 0;
    Header.ColorsImportant = 0;
    
    FILE *Output = fopen("out.bmp", "wb");
    if(Output)
    {
        PixelPtr = Pixels;
        fwrite(&Header, sizeof(Header), 1, Output);
        for(int Y = 0; Y < H; Y++)
        {
            for(int X = 0; X < W; X++)
            {
                u32 P = PackV3(*PixelPtr++);
                fwrite(&P, 1, sizeof(P), Output);
            }
        }
        
        fclose(Output);
    }
    else
    {
        fclose(Output);
    }
    
    return(0);
}
