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
#include <tchar.h>
#include <gl/gl.h>
#include <float.h>
#include <string.h>
#include "payload.h"
#include "payload_maths.h"
#include "payload_transforms.cc"
#include "payload_brdf.cc"

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
#if 0
internal ray
CameraRay(camera* Camera, s32 Width, s32 Height, f32 S, f32 T)
{
    f32 Theta = Camera->FOV * Pi32 / 180.0f;
    f32 HalfHeight = tanf(Theta/2.0f);
    f32 Aspect = (float)Width/(float)Height;
    f32 HalfWidth = HalfHeight*Aspect;
    v3f Eye = V3f(0,0,1);
    
    RotateX(&Eye, 0);
    RotateY(&Eye, 0);
    RotateZ(&Eye, 0);
    
    
    v3f U = Unit(Cross(V3f(0,1,0), Eye));
    v3f V = Cross(Eye, U);
    v3f BottomLeftCorner = Camera->Origin - HalfWidth*U - V*HalfHeight - Eye;
    v3f Horizontal = 2*U*HalfWidth;
    v3f Vertical = 2*V*HalfHeight;
    v3f Direction =BottomLeftCorner + Horizontal*S + Vertical*T - Camera->Origin;
    ray Result;
    
    Result = Ray(Camera->Origin, Direction,0.0f, INF);
    
    return(Result);
    
    
    
}
#endif

internal ray
CameraRay(camera* Camera, s32 Width, s32 Height, f32 S, f32 T)
{
    f32 Theta = Camera->FOV * Pi32 / 180.0f;
    f32 Aspect = (f32)Width/(f32)Height;
    v3f ZDir = {0,1,0};
    v3f XDir = Unit(Cross(ZDir,V3f(0,0,1)));
    v3f YDir = Unit(Cross(ZDir, XDir));
    v3f Centre =  {};
    f32 HalfH = tanf(Theta/2.0f);
    f32 HalfW = HalfH*Aspect;
    v3f ScreenPos = Centre + S*HalfW*XDir + T*HalfH*YDir;
    
    v3f Dir = ScreenPos - V3f(0,1,0);
    RotateX(&Dir, 1.57);
    //RotateY(&Dir, Camera->Rotation.X);
    //RotateZ(&Dir, 0);
    ray Result;
    Result = Ray(Camera->Origin,Dir, 0.0f, INF);
    
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

internal inline v3f
Hadamard(v3f A, v3f B)
{
    v3f Result = {A.X*B.X, A.Y*B.Y, A.Z*B.Z};
    return(Result);
}

internal b32x
GetLine(char* Line, u32 Size,  char** Buffer, u32 NumElements, u32* CurrentElement)
{
    if(*CurrentElement == NumElements)
    {
        return(false);
    }
    char* CurrentChar = Line;
    char* TempBuffer = *Buffer;
    memset(Line, 0, Size);
    do
    {
        (*CurrentElement)++;
        *CurrentChar++ = *TempBuffer;
    } while(*TempBuffer++ != '\n');
    
    *Buffer = TempBuffer;
    return true;
}

internal void
LoadOBJ(char* Filename, 
        vertex** Vertices,
        face** Faces,
        RTCGeometry *Mesh,
        u32** Materials)
{
    HANDLE File = CreateFileA(Filename, GENERIC_READ,0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(File == INVALID_HANDLE_VALUE)
    {
        goto FileError;
    }
    u32 FileSize = GetFileSize(File, NULL);
    char* Buffer = reinterpret_cast<char*>(malloc(FileSize*sizeof(u8)));
    char* BufferPtr = Buffer;
    ReadFile(File, Buffer, FileSize, 0, 0);
    if(Buffer == NULL)
    {
        goto BufferError;
    }
    char Line[256];
    char Type[3];
    s64 NumberOfLines = 0;
    s64 NumberOfVertices = 0;
    s64 NumberOfFaces = 0;
    u32 CurrentSize = 0;
    while(GetLine(Line, sizeof(Line), &BufferPtr, FileSize, &CurrentSize))
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
    *Vertices =(vertex*)rtcSetNewGeometryBuffer(*Mesh,RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,sizeof(vertex),NumberOfVertices);
    
    vertex* VerticePtr = *Vertices;
    *Faces = (face*)rtcSetNewGeometryBuffer(*Mesh,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT3,sizeof(face),NumberOfFaces);
    
    *Materials = (u32*)malloc(NumberOfFaces*sizeof(u32));
    
    u32* MatPtr = *Materials;
    for(u32 I = 0; I < NumberOfFaces; I++)
    {
#if 1
        if(I > NumberOfFaces-3)
        {
            *MatPtr = 2;
        }
        else
        {
            *MatPtr = 0;
        }
#endif
        MatPtr++;
    }
    face* FacePtr = *Faces;
    BufferPtr = Buffer;
    CurrentSize = 0;
    while(GetLine(Line, sizeof(Line), &BufferPtr, FileSize, &CurrentSize))
    {
        if(strstr(Line, "v ") != NULL)
        {
            vertex Vert = {};
            f32 X = 0; f32 Y = 0; f32 Z = 0;
            sscanf(Line, "%s %f %f %f", Type, &X, &Y, &Z);
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
                   Type, &V0, &A, &B, &V1, &C, &D, &V2, &E, &F);
            Tri.V0 = V0-1;
            Tri.V1 = V1-1;
            Tri.V2 = V2-1;
            *FacePtr++ = Tri;
        }
    }
    BufferError:    
    free(Buffer);
    FileError:
    CloseHandle(File);
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
    material* Materials = Info->Materials;
    u32* MatIndices = Info->MatIndices;
    
    for(s32 Y = YMin; Y < YMax; Y++)
    {
        
        for(s32 X = XMin; X < XMax; X++)
        {
            
            f32 Contrib = 1.0f/f32(Queue->Samples);
            v3f Col = {};
            for(s32 S = 0; S < Queue->Samples; S++)
            {
                f32 ScreenX =-1.0f+2.0f*((f32)X + RandBilateral(&Info->RandomState))/f32(Image.Width);
                f32 ScreenY =-1.0 +2.0f*(((f32)Y + RandBilateral(&Info->RandomState))/(f32)Image.Height);
                ray Persp = CameraRay(&Camera, Image.Width, 
                                      Image.Height, ScreenX, ScreenY);
                
                u32 BounceCount;
                v3f Result = {0.f,0.f,0.f};
                v3f Attenuation = V3f(1.,1.,1.);
                v3f Sky = {};
                
                for(BounceCount = 1; BounceCount < 10; BounceCount++)
                {
                    rtcIntersect1(*Scene, &Context, RayToRTCRayHit(&Persp));
                    
                    if(Persp.GeomID != RTC_INVALID_GEOMETRY_ID)
                    {
                        u32 MatIndex = MatIndices[Persp.PrimID];
                        
                        material Material = Materials[MatIndex];
                        v3f Albedo = Material.Colour;
                        v3f Emit =  Material.Emit;
                        Result = Result + Hadamard(Emit, Attenuation);
                        if(LengthSqrd(Emit) > 0.0f)
                        {
                            Col = Col + Result/f32(BounceCount);
                            break;
                            
                        }
                        Attenuation = Hadamard(Attenuation, Albedo);
                        
                        
                        switch(Material.Type)
                        {
                            case DIFFUSE:
                            DiffuseBRDF(&Persp, &Info->RandomState);
                            break;
                            case GLOSSY:
                            GlossyBRDF(&Persp, &Info->RandomState);
                            break;
                        }
                        
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

u32* GLBuffer;

#if 1
int Render()
{
    char *rtconfig = "verbose=0,threads=12";
    
    RTCDevice Device = rtcNewDevice(rtconfig);
    RTCScene Scene = rtcNewScene(Device);
    rtcSetSceneBuildQuality(Scene, RTC_BUILD_QUALITY_HIGH);
    RTCGeometry Mesh = rtcNewGeometry (Device, RTC_GEOMETRY_TYPE_TRIANGLE);
    
    vertex* V = nullptr;
    face* F = nullptr;
    material Materials[3] = {};
    u32* MatIndices;
    
    Materials[0].Type = mat_type::DIFFUSE;
    Materials[0].Colour = {1.0, 1.0, 1.0};
    
    Materials[1].Type = mat_type::GLOSSY;
    Materials[1].Colour = {0.0, 1.0, 0.0};
    
    Materials[2].Type = mat_type::DIFFUSE;
    Materials[2].Emit = {2.0,2.0,2.0};
    
    printf("Loading OBJ...\n");
    LoadOBJ("buddhalight.obj", 
            &V, &F, &Mesh, &MatIndices);
    printf("Finished loading OBJ\n");
    rtcCommitGeometry(Mesh);
    u32 geomID = rtcAttachGeometry(Scene,Mesh);
    rtcReleaseGeometry(Mesh);
    
    rtcCommitScene(Scene);
    s32 W = 1280;
    s32 H = 720;
    v3f* Pixels = (v3f*)malloc(H*W*sizeof(v3f));
    v3f* PixelPtr = Pixels;
    image Image = {};
    Image.Width = W;
    Image.Height = H;
    Image.Pixels = Pixels;
    
    camera Camera = {};
    Camera.Origin = V3f(0,4.5,12);
    Camera.Rotation = V3f(0,0,0);
    Camera.FOV = 50.0f;
    u32 Samples = 10;
    f32 Contrib = 1.0f/float(Samples);
    u32 CountSamples = 1;
    
    
    v3f Col = {0.0f, 0.0f, 0.0f};
    
    u32 TileSize = 64;
    u32 TileCountX = (W + TileSize -1)/TileSize;
    u32 TileCountY = (H + TileSize -1)/TileSize;
    
    thread_queue Queue = {};
    Queue.Samples = 10;
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
            ThreadInfo->Materials = Materials;
            ThreadInfo->MatIndices = MatIndices;
            ThreadInfo->Image = Image;
            ThreadInfo->XMin = XMin;
            ThreadInfo->YMin = YMin;
            ThreadInfo->XMax = XMax;
            ThreadInfo->YMax = YMax;
            ThreadInfo->RandomState = 25;
        }
    }
    
    for(u32 Cores = 1; Cores < 12; Cores++)
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
    
    u32* PackedPixels = (u32*)malloc(W*H*sizeof(u32));
    FILE *Output = fopen("out.bmp", "wb");
    if(Output)
    {
        PixelPtr = Pixels;
        fwrite(&Header, sizeof(Header), 1, Output);
        for(int Y = 0; Y < H; Y++)
        {
            for(int X = 0; X < W; X++)
            {
                PackedPixels[X + Y*W] = PackV3(*PixelPtr++);
                u32 P = PackedPixels[X + Y*W];
                fwrite(&P, 1, sizeof(P), Output);
            }
        }
        
        fclose(Output);
    }
    else
    {
        fclose(Output);
    }
    GLBuffer = PackedPixels;
    
    return(0);
}
#endif 

internal b32x
InitOpenGL(HWND Window)
{
    HDC WindowDC = GetDC(Window);
    
    PIXELFORMATDESCRIPTOR PixelFormat = {};
    PixelFormat.nSize = sizeof(PixelFormat);
    PixelFormat.nVersion = 1;
    PixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    PixelFormat.cColorBits = 32;
    PixelFormat.cAlphaBits = 8;
    PixelFormat.iLayerType = PFD_MAIN_PLANE;
    
    s32 FormatIndex = ChoosePixelFormat(WindowDC, &PixelFormat);
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(WindowDC, FormatIndex, sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(WindowDC, FormatIndex, &SuggestedPixelFormat);
    
    HGLRC OpenGLRC = wglCreateContext(WindowDC);
    
    if(wglMakeCurrent(WindowDC, OpenGLRC))
    {
        return true;
    }
    else
    {
        MessageBox(0, _T("Failed to initialise OpenGL"), _T("Payload IPR"), 0);
        return false;
    }
    ReleaseDC(Window, WindowDC);
}

internal void
Display()
{
    f32 P = 1.f;
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0,0);
    glVertex2f(-P, -P);
    
    glTexCoord2f(1,0);
    glVertex2f(P, -P);
    
    glTexCoord2f(1,1);
    glVertex2f(P, P);
    
    
    glTexCoord2f(0,0);
    glVertex2f(-P, -P);
    
    glTexCoord2f(0,1);
    glVertex2f(-P, P);
    
    glTexCoord2f(1,1);
    glVertex2f(P, P);
    glEnd();
}

LRESULT CALLBACK WinProc(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam)
{
    PAINTSTRUCT Ps;  
    HDC WindowDC = GetDC(Hwnd);  
    GLuint TextureHandle;
    switch (Msg)  
    {  
        case WM_WINDOWPOSCHANGING:
        break;
        case WM_PAINT:  
        glViewport(0,0,1280, 720);
        
        glGenTextures(1, &TextureHandle);
        glBindTexture(GL_TEXTURE_2D, TextureHandle);
        glTexImage2D(GL_TEXTURE_2D,0, GL_RGBA8, 1280, 720,0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, GLBuffer);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        
        glEnable(GL_TEXTURE_2D);
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        Display();
        
        SwapBuffers(WindowDC);
        glDeleteTextures(1, &TextureHandle);
        break;  
        case WM_DESTROY:  
        PostQuitMessage(0);  
        break;  
        case WM_ERASEBKGND:
        return 1;
        default:  
        return DefWindowProc(Hwnd, Msg, WParam, LParam);  
        break;  
    }  
    
    return 0;  
}

int CALLBACK
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int CmdShow)
{
    Render();
    WNDCLASSEX WinClass = {};
    WinClass.cbSize = sizeof(WNDCLASSEX);
    WinClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WinClass.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    WinClass.lpfnWndProc = WinProc;
    WinClass.hInstance = Instance;
    WinClass.lpszClassName = _T("Payload");
    
    if(!RegisterClassEx(&WinClass))
    {
        MessageBox(0, _T("Failed to register class"), _T("Payload IPR"), 0);
        return 1;
    }
    HWND HWnd = CreateWindow("Payload", "Payload IPR", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                             1280, 720, 0, 0, Instance, 0);
    
    if(!HWnd)
    {
        MessageBox(0, _T("Failed to create Window"), _T("Payload IPR"), 0);
        return 1;
    }
    if(!InitOpenGL(HWnd))
    {
        return 1;
    }
    ShowWindow(HWnd,CmdShow);  
    UpdateWindow(HWnd);  
    MSG Msg;  
    while (GetMessage(&Msg, NULL, 0, 0))  
    {  
        TranslateMessage(&Msg);  
        DispatchMessage(&Msg);  
    }  
    
    return 0;
}
