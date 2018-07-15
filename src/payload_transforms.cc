
internal 


internal void
RotateX(v3f* A, f32 Angle)
{
    mat3 RotationMatrix =
    {
        1, 0, 0,
        0, cos(Angle), -sin(Angle),
        0, sin(Angle), cos(Angle),
    };
    
    *A = RotationMatrix* (*A);
}

internal void
RotateY(v3f* A, f32 Angle)
{
    mat3 RotationMatrix =
    {
        cos(Angle), 0, sin(Angle),
        0, 1, 0,
        -sin(Angle), 0, cos(Angle),
    };
    
    *A = RotationMatrix* (*A);
}

internal void
RotateZ(v3f* A, f32 Angle)
{
    mat3 RotationMatrix =
    {
        cos(Angle), -sin(Angle), 0,
        sin(Angle), cos(Angle), 0,
        0,0,1,
    };
    
    *A = RotationMatrix* (*A);
}
