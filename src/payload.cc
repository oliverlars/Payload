#include <C:\Python35\Include\Python.h>
#include <stdio.h>
#include <assert.h>
#include <immintrin.h>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <embree3/include/rtcore.h>
#include <stdlib.h>
//_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
//_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#include <inttypes.h>
#include <math.h>
#include <float.h>
#include <windows.h>
#include <string.h>
#include "payload.h"
#include "payload_maths.h"

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
    //return (float(rand())/float(RAND_MAX));
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
    
    *Materials = (material*)Alloc(NumberOfFaces*sizeof(material));
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
            MatPtr->Colour = {0.8, 0.8, 0.8};
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

internal v3f
Pathtrace(camera *Camera, 
          f32 X, f32 Y,
          f32 W, f32 H, 
          RTCScene *Scene,
          material* Colours)
{
    
    RTCIntersectContext Context;
    rtcInitIntersectContext(&Context);
    
    ray Persp = CameraRay(Camera, W, H, X/W, Y/H);
    
    v3f Result = {0.f,0.f,0.f};
    v3f Attenuation = V3f(1.,1.,1.);
    v3f Sky = V3f(0.0, 0.0, 0.0);
    u32 BounceCount;
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
                return(Result/f32(BounceCount));
                
            }
            Attenuation = Hadamard(Attenuation, Albedo);
            Persp = Ray(Persp.O + Persp.TFar*Persp.D,
                        Unit(Persp.Ng) + V3f(RandBilateral(), RandBilateral(), RandBilateral()),
                        0.001f, INF);
        }
        else
        {
            Result = Result + Hadamard(Attenuation,Sky);
            return(Result/f32(BounceCount));
        }
    }
    return(Result/f32(BounceCount));
}

int main(int ArgCount, char** Args)
{
    char *rtconfig = "verbose=3,threads=12";
    
    RTCDevice Device = rtcNewDevice(rtconfig);
    RandomState = 24;
    RTCScene Scene = rtcNewScene(Device);
    rtcSetSceneBuildQuality(Scene, RTC_BUILD_QUALITY_HIGH);
    RTCGeometry Mesh = rtcNewGeometry (Device, RTC_GEOMETRY_TYPE_TRIANGLE);
    
    vertex* V = nullptr;
    face* F = nullptr;
    material* Materials = nullptr;
    printf("Loading OBJ...\n");
    LoadOBJ("Teapotlight.obj", &V, &F, &Mesh, &Materials);
    
    rtcCommitGeometry(Mesh);
    u32 geomID = rtcAttachGeometry(Scene,Mesh);
    rtcReleaseGeometry(Mesh);
    
    rtcCommitScene(Scene);
    s32 W = 1280;
    s32 H = 720;
    u32* Pixels = (u32*)Alloc(H*W*sizeof(u32));
    u32* PixelPtr = Pixels;
    
    camera Camera = {};
    Camera.Up = V3f(0,1,0);
    Camera.LookAt = V3f(0, 0, -3);
    Camera.Origin = V3f(0,3,4);
    Camera.FOV = 50.0f;
    u32 Samples = 100;
    f32 Contrib = 1.0f/float(Samples);
    
    for(int Y = 0; Y < H; Y++)
    {
        printf("\rRendering... %.2f%%", 100.0*(float(Y)/float(H)));
        for(int X = 0; X < W; X++)
        {
            v3f Col = {0.0f, 0.0f, 0.0f};
            for(int S = 0; S < Samples; S++)
            {
                Col = Col+ Contrib*Pathtrace(&Camera, 
                                             float(X) + RandBilateral(),
                                             float(Y) + RandBilateral(), W, H, &Scene, Materials);
            }
            PixelPtr[X + Y*W] = PackV3(Col);
        }
    }
    
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
        fwrite(&Header, sizeof(Header), 1, Output);
        fwrite(Pixels, W*H*sizeof(u32), 1, Output);
        fclose(Output);
    }
    
    free(V);
    free(F);
    rtcReleaseDevice(Device);
    return(0);
}