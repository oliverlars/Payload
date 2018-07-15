

internal void
DiffuseBRDF(ray* RayIn, u32* RandomState)
{
    v3f Direction = Unit(RayIn->Ng) + V3f(RandBilateral(RandomState),
                                          RandBilateral(RandomState),
                                          RandBilateral(RandomState));
    
    *RayIn = Ray(RayIn->O + RayIn->TFar*RayIn->D, Direction, 1e-5, INF);
}

internal void
GlossyBRDF(ray* RayIn, u32* RandomState)
{
    v3f Offset = V3f(RandBilateral(RandomState),
                     RandBilateral(RandomState),
                     RandBilateral(RandomState))*0.2;
    v3f Origin = RayIn->O + RayIn->TFar*RayIn->D;
    v3f Reflection = RayIn->D - Unit(RayIn->Ng)* 2.0*Inner(RayIn->D, Unit(RayIn->Ng));
    
    *RayIn = Ray(Origin, Reflection + Offset, 1e-5, INF);
}
