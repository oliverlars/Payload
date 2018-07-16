


internal void
RotateX(v4* A, f32 Angle)
{
    mat4 RotationMatrix =
    {
        1, 0, 0,0,
        0, cos(Angle), -sin(Angle),0,
        0, sin(Angle), cos(Angle),0, 
        0,0,0,1,
    };
    
    *A = RotationMatrix* (*A);
}

internal void
RotateY(v4* A, f32 Angle)
{
    mat4 RotationMatrix =
    {
        cos(Angle), 0, sin(Angle),
        0, 1, 0,0,
        -sin(Angle), 0, cos(Angle),1,
        0,0,0,1,
    };
    
    *A = RotationMatrix* (*A);
}

internal void
RotateZ(v4* A, f32 Angle)
{
    mat4 RotationMatrix =
    {
        cos(Angle), -sin(Angle), 0,
        sin(Angle), cos(Angle), 0,
        0,0,1,0,
        0,0,0,1
    };
    
    *A = RotationMatrix* (*A);
}


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
        cos(Angle), -sin(Angle),
        sin(Angle), cos(Angle),
        0,0,1,
    };
    
    *A = RotationMatrix* (*A);
}


internal void
Rotate(v4* A, f32 XAngle, f32 YAngle, f32 ZAngle)
{
    RotateX(A, XAngle);
    RotateY(A, YAngle);
    RotateZ(A, ZAngle);
}

internal void 
Translate(v4* A, f32 X, f32 Y, f32 Z)
{
    mat4 TranslationMatrix =
    {
        1,0,0,X,
        0,1,0,Y,
        0,0,1,Z,
        0,0,0,1,
    };
    
    *A = TranslationMatrix * (*A);
}


internal void 
Translate(v4* A, v3f B)
{
    mat4 TranslationMatrix =
    {
        1,0,0,B.X,
        0,1,0,B.Y,
        0,0,1,B.Z,
        0,0,0,1,
    };
    
    *A = TranslationMatrix * (*A);
}

