

internal void
DiffuseBRDF(ray* In, u32* RandomState)
{
    v3f Direction = Unit(In->Ng) + V3f(RandBilateral(RandomState),
                                       RandBilateral(RandomState),
                                       RandBilateral(RandomState));
    
    *In = Ray(In->O + In->TFar*In->D, Direction, 1e-5, INF);
}

internal void
GlossyBRDF(ray* In, u32* RandomState)
{
    v3f Offset = V3f(RandBilateral(RandomState),
                     RandBilateral(RandomState),
                     RandBilateral(RandomState))*0.2;
    *In = Ray(In->O + In->TFar*In->D,In->D - Unit(In->Ng)* 2.0*Inner(In->D, Unit(In->Ng)) + Offset, 1e-5, INF);
}
