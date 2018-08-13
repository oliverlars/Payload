
inline static v3f
SampleTexture(texture Texture, v2 Coords)
{
    u32 X = Texture.Width*Coords.U;
    u32 Y = Texture.Height*Coords.V;
    u8 N = Texture.N;
    u32 Index = (X + Y*Texture.Width)*N;
    v3f Result = 
    {
        Texture.Data[Index],
        Texture.Data[Index+1],
        Texture.Data[Index+2]
    };
    
    return Result/255.0f;
}