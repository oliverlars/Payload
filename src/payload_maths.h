internal v3f
operator+(v3f A, v3f B)
{
    v3f Result;
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    Result.Z = A.Z + B.Z;
    return(Result);
}

internal v3f
operator+(v3f A, f32 B)
{
    v3f Result;
    Result.X = A.X + B;
    Result.Y = A.Y + B;
    Result.Z = A.Z + B;
    return(Result);
}

internal v3f
operator-(v3f A, v3f B)
{
    v3f Result;
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    Result.Z = A.Z - B.Z;
    return(Result);
}

internal v3f
operator-(v3f A, f32 B)
{
    v3f Result;
    Result.X = A.X - B;
    Result.Y = A.Y - B;
    Result.Z = A.Z - B;
    return(Result);
}

internal v3f
operator*(v3f A, f32 B)
{
    v3f Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    Result.Z = A.Z * B;
    return(Result);
}

internal v3f
operator*(f32 B,v3f A)
{
    v3f Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    Result.Z = A.Z * B;
    return(Result);
}

internal v3f
operator*(mat3 B, v3f A)
{
    v3f Result;
    Result.X = A.X*B.M00 + A.Y*B.M01 + A.Z*B.M02;
    Result.Y = A.X*B.M10 + A.Y*B.M11 + A.Z*B.M12;
    Result.Z = A.X*B.M20 + A.Y*B.M21 + A.Z*B.M22;
    return(Result);
}

internal v3f
operator/(v3f A, f32 B)
{
    v3f Result;
    Result.X = A.X / B;
    Result.Y = A.Y / B;
    Result.Z = A.Z / B;
    return(Result);
}

internal inline f32
Inner(v3f A, v3f B)
{
    f32 Result;
    Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z;
    return(Result);
}

internal inline v3f
Cross(v3f A, v3f B)
{
    v3f Result;
    Result.X = A.Y*B.Z - B.Y*A.Z;
    Result.Y = A.Z*B.X - B.Z*A.X;
    Result.Z = A.X*B.Y - B.X*A.Y;
    return(Result);
}

internal inline f32
LengthSqrd(v3f A)
{
    f32 Result;
    Result = Inner(A,A);
    return(Result);
}

internal inline f32
Length(v3f A)
{
    f32 Result;
    Result = sqrt(LengthSqrd(A));
    return(Result);
}

internal inline v3f
Unit(v3f A)
{
    v3f Result;
    Result = A/Length(A);
    return(Result);
}

internal inline f32
Clamp(f32 A, f32 Min, f32 Max)
{
    f32 Result = A;
    if(Result > Max)
    {
        Result = Max;
    }
    else if(Result < Min)
    {
        Result = Min;
    }
    return(Result);
}
internal inline v3f
ClampV3(v3f A, f32 Min, f32 Max)
{
    v3f Result;
    Result.X = Clamp(A.X, Min, Max);
    Result.Y = Clamp(A.Y, Min, Max);
    Result.Z = Clamp(A.Z, Min, Max);
    return(Result);
}


internal v4
V3ToV4(v3f A)
{
    v4 Result = {A.X, A.Y, A.Z, 1};
    return(Result);
}

internal v3f
V4ToV3(v4 A)
{
    v3f Result = {A.X, A.Y, A.Z};
    return(Result);
}

internal v4
operator+(v4 A, v4 B)
{
    v4 Result;
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    Result.Z = A.Z + B.Z;
    Result.W = A.W * B.W;
    return(Result);
}

internal v4
operator+(v4 A, f32 B)
{
    v4 Result;
    Result.X = A.X + B;
    Result.Y = A.Y + B;
    Result.Z = A.Z + B;
    Result.W = A.W * B;
    return(Result);
}

internal v4
operator-(v4 A, v4 B)
{
    v4 Result;
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    Result.Z = A.Z - B.Z;
    Result.W = A.W - B.W;
    return(Result);
}

internal v4
operator-(v4 A, f32 B)
{
    v4 Result;
    Result.X = A.X - B;
    Result.Y = A.Y - B;
    Result.Z = A.Z - B;
    Result.W = A.W - B;
    return(Result);
}

internal v4
operator*(v4 A, f32 B)
{
    v4 Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    Result.Z = A.Z * B;
    Result.W = A.W * B;
    return(Result);
}

internal v4
operator*(f32 B,v4 A)
{
    v4 Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    Result.Z = A.Z * B;
    Result.W = A.W * B;
    return(Result);
}

internal v4
operator*(mat4 B, v4 A)
{
    v4 Result;
    Result.X = A.X*B.M00 + A.Y*B.M01 + A.Z*B.M02 + A.W*B.M03;
    Result.Y = A.X*B.M10 + A.Y*B.M11 + A.Z*B.M12 + A.W*B.M13;
    Result.Z = A.X*B.M20 + A.Y*B.M21 + A.Z*B.M22 + A.W*B.M23;
    Result.W = A.X*B.M30 + A.Y*B.M31 + A.Z*B.M32 + A.W*B.M33;
    return(Result);
}

internal v4
operator/(v4 A, f32 B)
{
    v4 Result;
    Result.X = A.X / B;
    Result.Y = A.Y / B;
    Result.Z = A.Z / B;
    Result.W = A.W / B;
    return(Result);
}

internal inline f32
Inner(v4 A, v4 B)
{
    f32 Result;
    Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z + A.W*B.W;
    return(Result);
}

internal inline f32
LengthSqrd(v4 A)
{
    f32 Result;
    Result = Inner(A,A);
    return(Result);
}

internal inline f32
Length(v4 A)
{
    f32 Result;
    Result = sqrt(LengthSqrd(A));
    return(Result);
}

internal inline v4
Unit(v4 A)
{
    v4 Result;
    Result = A/Length(A);
    return(Result);
}


