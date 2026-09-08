#if !defined(HANDMADE_MATH_H)
#define HANDMADE_MATH_H

union vec2
{
	struct
	{
		real32 X, Y;
	};
	real32 E[2];
};

union vec3
{
	struct
	{
		real32 X, Y, Z;
	};
	struct
	{
		real32 R, G, B;
	};
	struct
	{
		vec2 XY;
		real32 Ignored0_;
	};
	struct
	{
		real32 Ignored1_;
		vec2 YZ;
	};
	real32 E[3];	
};

union vec4
{
	struct
	{
		real32 X, Y, Z, W;
	};
	struct
	{
		real32 R, G, B, A;
	};
	real32 E[4];
};

struct rectangle2
{
	vec2 Min, Max;
};

struct rectangle3
{
	vec3 Min, Max;
};

inline vec2
Vec2(real32 X, real32 Y)
{
	vec2 Result;
	Result.X = X;
	Result.Y = Y;

	return Result;
}

inline vec3
Vec3(real32 X, real32 Y, real32 Z)
{
	vec3 Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;

	return Result;
}

inline vec3
Vec3(vec2 XY, real32 Z)
{
	vec3 Result;
	Result.X = XY.X;
	Result.Y = XY.Y;
	Result.Z = Z;

	return Result;
}

inline vec4
Vec4(real32 X, real32 Y, real32 Z, real32 W)
{
	vec4 Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	Result.W = W;

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

inline vec2
Hadamard(vec2 A, vec2 B)
{
	vec2 Result = {A.X*B.X, A.Y*B.Y};

	return Result;
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

inline real32
LengthSq(vec2 A)
{
	real32 Result = Inner(A, A);
	
	return Result;
}

inline real32
Length(vec2 A)
{
	real32 Result = SquareRoot(LengthSq(A));

	return Result;
}

// NOTE vec3 operation

inline vec3
operator*(real32 A, vec3 B)
{
	vec3 Result;

	Result.X = A*B.X;
	Result.Y = A*B.Y;
	Result.Z = A*B.Z;

	return Result;
}

inline vec3
operator*(vec3 B, real32 A)
{
	vec3 Result = A * B;
	
	return Result;
}

inline vec3 &
operator*=(vec3 &B, real32 A)
{
	B = A * B;

	return B;
}

inline vec3
operator-(vec3 A)
{
	vec3 Result;
	Result.X = -A.X;
	Result.Y = -A.Y;
	Result.Z = -A.Z;

	return Result;
}
	
inline vec3
operator+(vec3 A, vec3 B)
{
	vec3 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	Result.Z = A.Z + B.Z;

	return Result;
}

inline vec3 &
operator+=(vec3 &A, vec3 B)
{
	A = A + B;

	return A;
}

inline vec3
operator-(vec3 A, vec3 B)
{
	vec3 Result;

	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	Result.Z = A.Z - B.Z;

	return Result;	
}

inline vec3
Hadamard(vec3 A, vec3 B)
{
	vec3 Result;
	Result.X = A.X*B.X;
	Result.Y = A.Y*B.Y;
	Result.Z = A.Z*B.Z;

	return Result;
}

inline real32
Inner(vec3 A, vec3 B)
{
	real32 Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z;

	return Result;	
}

inline real32
LengthSq(vec3 A)
{
	real32 Result = Inner(A, A);

	return Result;
}

inline real32
Length(vec3 A)
{
	real32 Result = SquareRoot(Inner(A, A));

	return Result;
}

// NOTE rectangle2

inline vec2
GetMinCorner(rectangle2 Rect)
{
	vec2 Result = Rect.Min;
	return Result;
}

inline vec2
GetMaxCorner(rectangle2 Rect)
{
	vec2 Result = Rect.Max;
	return Result;
}

inline vec2
GetCenter(rectangle2 Rect)
{
	vec2 Result = 0.5f*(Rect.Min + Rect.Max);
	return Result;
}

inline rectangle2
RectMinMax(vec2 Min, vec2 Max)
{
	rectangle2 Result;
	Result.Min = Min;
	Result.Max = Max;

	return Result;
}

inline rectangle2
RectMinDim(vec2 Min, vec2 Dim)
{
	rectangle2 Result;
	Result.Min = Min;
	Result.Max = Min + Dim;

	return Result;
}

inline rectangle2
RectHalfCenterDim(vec2 Center, vec2 HalfDim)
{
	rectangle2 Result;
	Result.Min = Center - HalfDim;
	Result.Max = Center + HalfDim;

	return Result;
}

inline rectangle2
AddRadiusTo(rectangle2 A, real32 RadiusW, real32 RadiusH)
{
	rectangle2 Result = A;
	Result.Min = A.Min - Vec2(RadiusW, RadiusH);
	Result.Max = A.Max + Vec2(RadiusW, RadiusH);

	return Result;
}

inline rectangle2
RectCenterDim(vec2 Center, vec2 Dim)
{
	rectangle2 Result = RectHalfCenterDim(Center, 0.5*Dim);

	return Result;
}

inline bool32
IsInRectangle(rectangle2 Rectangle, vec2 Test)
{
	bool32 Result = ((Test.X >= Rectangle.Min.X) &&
					 (Test.X < Rectangle.Max.X) &&
					 (Test.Y >= Rectangle.Min.Y) &&
					 (Test.Y < Rectangle.Max.Y));

	return Result;
}

// NOTE rectangle3

inline vec3
GetMinCorner(rectangle3 Rect)
{
	vec3 Result = Rect.Min;
	return Result;
}

inline vec3
GetMaxCorner(rectangle3 Rect)
{
	vec3 Result = Rect.Max;
	return Result;
}

inline vec3
GetCenter(rectangle3 Rect)
{
	vec3 Result = 0.5f*(Rect.Min + Rect.Max);
	return Result;
}

inline rectangle3
RectMinMax(vec3 Min, vec3 Max)
{
	rectangle3 Result;
	Result.Min = Min;
	Result.Max = Max;

	return Result;
}

inline rectangle3
RectMinDim(vec3 Min, vec3 Dim)
{
	rectangle3 Result;
	Result.Min = Min;
	Result.Max = Min + Dim;

	return Result;
}

inline rectangle3
RectHalfCenterDim(vec3 Center, vec3 HalfDim)
{
	rectangle3 Result;
	Result.Min = Center - HalfDim;
	Result.Max = Center + HalfDim;

	return Result;
}

inline rectangle3
AddRadiusTo(rectangle3 A, vec3 Radius)
{
	rectangle3 Result;
	Result.Min = A.Min - Radius;
	Result.Max = A.Max + Radius;

	return Result;
}

inline rectangle3
RectCenterDim(vec3 Center, vec3 Dim)
{
	rectangle3 Result = RectHalfCenterDim(Center, 0.5*Dim);

	return Result;
}

inline bool32
IsInRectangle(rectangle3 Rectangle, vec3 Test)
{
	bool32 Result = ((Test.X >= Rectangle.Min.X) &&
					 (Test.X <= Rectangle.Max.X) &&
					 (Test.Y >= Rectangle.Min.Y) &&
					 (Test.Y <= Rectangle.Max.Y));

	return Result;
}


#endif /* HANDMADE_MATH_H */
