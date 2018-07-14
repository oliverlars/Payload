

internal ray
DiffuseBRDF(v3f HitLocation, u32* RandomState)
{
    ray Result;
    v3f Direction = V3f(RandBilateral(RandomState),
                        RandBilateral(RandomState),
                        RandBilateral(RandomState));
    
    Result = Ray(HitLocation, Direction, 1e-5, INF);
    return Result;
}

internal ray
GlossyBRDF(v3f HitLocation, u32* RandomState)
{
    
}

internal ray
GlassBRDF(v3f HitLocation)
{
    
}

internal ray
MixBRDF(v3f HitLocation)
{
    
}