#if !defined(HANDMADE_MATH_H)
#define HANDMADE_MATH_H

union vec2
{
	struct
	{
		real32 X, Y;
	};
	real32 E2[2];
};

inline vec2
Vec2(real32 X, real32 Y)
{
	vec2 Result;
	Result.X = X;
	Result.Y = Y;

	return Result;
}

inline vec2
operator+(vec2 A, vec2 B)
{
	vec2 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;

	return Result;
}

inline vec2 &
operator+=(vec2 &A, vec2 B)
{
	A = A+B;

	return A;
}

inline vec2
operator-(vec2 A)
{
	vec2 Result;
	Result.X = -A.X;
	Result.Y = -A.Y;

	return Result;
}

inline vec2
operator-(vec2 A, vec2 B)
{
	vec2 Result;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;

	return Result;
}

inline vec2
operator*(real32 A, vec2 B)
{
    vec2 Result;
    Result.X = A*B.X;
    Result.Y = A*B.Y;

    return Result;
}

inline vec2
operator*(vec2 B, real32 A)
{
    vec2 Result;
	Result = A*B;
	
    return Result;
}

inline vec2 &
operator*=(vec2 &B, real32 A)
{
	B = A*B;

	return B;
}

inline real32
Square(real32 A)
{
	real32 Result = A*A;

	return Result;
}

inline real32
Inner(vec2 A, vec2 B)
{
	real32 Result = A.X*B.X + A.Y*B.Y;

	return Result;
}

#endif /* HANDMADE_MATH_H */
