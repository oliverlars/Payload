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
