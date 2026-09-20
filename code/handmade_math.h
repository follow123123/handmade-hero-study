#if !defined(HANDMADE_MATH_H)
#define HANDMADE_MATH_H

union vec2
{
	struct
	{
		real32 x, y;
	};
	real32 E[2];
};

union vec3
{
	struct
	{
		real32 x, y, z;
	};
	struct
	{
		real32 r, g, b;
	};
	struct
	{
		vec2 xy;
		real32 Ignored0_;
	};
	struct
	{
		real32 Ignored1_;
		vec2 yz;
	};
	real32 E[3];	
};

union vec4
{
	struct
	{
		real32 x, y, z, W;
	};
	struct
	{
		real32 r, g, b, A;
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
	Result.x = X;
	Result.y = Y;

	return Result;
}

inline vec3
Vec3(real32 X, real32 Y, real32 Z)
{
	vec3 Result;
	Result.x = X;
	Result.y = Y;
	Result.z = Z;

	return Result;
}

inline vec3
Vec3(vec2 XY, real32 Z)
{
	vec3 Result;
	Result.x = XY.x;
	Result.y = XY.y;
	Result.z = Z;

	return Result;
}

inline vec4
Vec4(real32 X, real32 Y, real32 Z, real32 W)
{
	vec4 Result;
	Result.x = X;
	Result.y = Y;
	Result.z = Z;
	Result.W = W;

	return Result;
}

// real32

inline real32
Square(real32 A)
{
	real32 Result = A*A;

	return Result;
}

inline real32
Lerp(real32 t, real32 A, real32 B)
{
	real32 Result = (1.0f - t)*A + t*B;

	return Result;
}

inline real32
Clamp(real32 Value, real32 Min, real32 Max)
{
	real32 Result = Value;
	if (Result < Min)
	{
		Result = Min;
	}
	if (Result > Max)
	{
		Result = Max;
	}

	return Result;
}

inline real32
Clamp01(real32 Value)
{
	real32 Result = Clamp(Value, 0, 1);

	return Result;
}

inline real32
SafeRatioN(real32 Numerator, real32 Divisor, real32 N)
{
	real32 Result = N;
	if (Divisor != 0)
	{
		Result = Numerator / Divisor;
	}

	return Result;
}

inline real32
SafeRatio0(real32 Numerator, real32 Divisor)
{
	real32 Result = SafeRatioN(Numerator, Divisor, 0);

	return Result;
}

inline real32
SafeRatio1(real32 Numerator, real32 Divisor)
{
	real32 Result = SafeRatioN(Numerator, Divisor, 1);

	return Result;
}

// vec2

inline vec2
operator+(vec2 A, vec2 B)
{
	vec2 Result;
	Result.x = A.x + B.x;
	Result.y = A.y + B.y;

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
	Result.x = -A.x;
	Result.y = -A.y;

	return Result;
}

inline vec2
operator-(vec2 A, vec2 B)
{
	vec2 Result;
	Result.x = A.x - B.x;
	Result.y = A.y - B.y;

	return Result;
}

inline vec2
operator*(real32 A, vec2 B)
{
    vec2 Result;
    Result.x = A*B.x;
    Result.y = A*B.y;

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
Vec2i(int32 X, int32 Y)
{
	vec2 Result = {(real32)X, (real32)Y};

	return Result;
}

inline vec2
Vec2i(uint32 X, uint32 Y)
{
	vec2 Result = {(real32)X, (real32)Y};

	return Result;
}

inline vec2
Hadamard(vec2 A, vec2 B)
{
	vec2 Result = {A.x*B.x, A.y*B.y};

	return Result;
}

inline real32
Inner(vec2 A, vec2 B)
{
	real32 Result = A.x*B.x + A.y*B.y;

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

inline vec2
Clamp01(vec2 Value)
{
	vec2 Result;
	Result.x = Clamp01(Value.x);
	Result.y = Clamp01(Value.y);

	return Result;
}

// NOTE vec3 operation

inline vec3
operator*(real32 A, vec3 B)
{
	vec3 Result;

	Result.x = A*B.x;
	Result.y = A*B.y;
	Result.z = A*B.z;

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
	Result.x = -A.x;
	Result.y = -A.y;
	Result.z = -A.z;

	return Result;
}
	
inline vec3
operator+(vec3 A, vec3 B)
{
	vec3 Result;
	Result.x = A.x + B.x;
	Result.y = A.y + B.y;
	Result.z = A.z + B.z;

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

	Result.x = A.x - B.x;
	Result.y = A.y - B.y;
	Result.z = A.z - B.z;

	return Result;	
}

inline vec3
Clamp01(vec3 Value)
{
	vec3 Result;
	Result.x = Clamp01(Value.x);
	Result.y = Clamp01(Value.y);
	Result.z = Clamp01(Value.z);

	return Result;
}

inline vec3
Hadamard(vec3 A, vec3 B)
{
	vec3 Result;
	Result.x = A.x*B.x;
	Result.y = A.y*B.y;
	Result.z = A.z*B.z;

	return Result;
}

inline real32
Inner(vec3 A, vec3 B)
{
	real32 Result = A.x*B.x + A.y*B.y + A.z*B.z;

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
	bool32 Result = ((Test.x >= Rectangle.Min.x) &&
					 (Test.x < Rectangle.Max.x) &&
					 (Test.y >= Rectangle.Min.y) &&
					 (Test.y < Rectangle.Max.y));

	return Result;
}

inline vec2
GetBarycentric(rectangle2 Rect, vec2 P)
{
	vec2 Result;
	Result.x = SafeRatio0(P.x - Rect.Min.x, Rect.Max.x - Rect.Min.x);
	Result.y = SafeRatio0(P.y - Rect.Min.y, Rect.Max.y - Rect.Min.y);

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
	bool32 Result = ((Test.x >= Rectangle.Min.x) &&
					 (Test.y >= Rectangle.Min.y) &&
					 (Test.z >= Rectangle.Min.z) &&
					 (Test.x < Rectangle.Max.x) &&
					 (Test.y < Rectangle.Max.y) &&
					 (Test.z < Rectangle.Max.z));

	return Result;
}

inline bool32
RectanglesIntersect(rectangle3 A, rectangle3 B)
{
	bool32 Result = !((B.Max.x <= A.Min.x) ||
					  (B.Min.x >= A.Max.x) ||
					  (B.Max.y <= A.Min.y) ||
					  (B.Min.y >= A.Max.y) ||
					  (B.Max.z <= A.Min.z) ||
					  (B.Min.z >= A.Max.z));

	return Result;
}

inline vec3
GetBarycentric(rectangle3 Rect, vec3 P)
{
	vec3 Result;
	Result.x = SafeRatio0(P.x - Rect.Min.x, Rect.Max.x - Rect.Min.x);
	Result.y = SafeRatio0(P.y - Rect.Min.y, Rect.Max.y - Rect.Min.y);
	Result.z = SafeRatio0(P.z - Rect.Min.z, Rect.Max.z - Rect.Min.z);

	return Result;
}

inline rectangle2
ToRectangleXY(rectangle3 Rect)
{
	rectangle2 Result;
	Result.Min = Rect.Min.xy;
	Result.Max = Rect.Max.xy;

	return Result;
}

#endif /* HANDMADE_MATH_H */
