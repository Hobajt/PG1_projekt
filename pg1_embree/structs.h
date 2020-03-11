#pragma once

struct Vertex3f { float x, y, z; }; // a single vertex position structure matching certain format

using Normal3f = Vertex3f; // a single vertex normal structure matching certain format

struct Coord2f { float u, v; }; // texture coord structure

struct Triangle3ui { unsigned int v0, v1, v2; }; // indicies of a single triangle, the struct must match certain format, e.g. RTC_FORMAT_UINT3

struct RTC_ALIGN(16) clr4f {
	struct { float r, g, b, a; }; // a = 1 means that the pixel is opaque

	clr4f operator * (float f) const;
	clr4f operator * (const clr4f& rhs) const;
	clr4f& operator *= (float f);
	clr4f& operator *= (const clr4f& rhs);

	clr4f operator + (const clr4f& rhs) const;
	clr4f operator + (float rhs) const;
	clr4f& operator += (const clr4f& rhs);


	clr4f operator / (const clr4f& rhs) const;
	clr4f& operator /= (const clr4f& rhs);
};

clr4f operator + (float lhs, const clr4f& rhs);

struct clr3f {
	float r, g, b;

	operator clr4f() const { return { r, g, b, 1.f }; }

	clr3f operator * (float f);
	clr3f operator * (const clr3f& rhs);
	clr3f& operator *= (float f);
	clr3f& operator *= (const clr3f& rhs);

	clr3f operator + (float f);
	clr3f operator + (const clr3f& rhs);
	clr3f& operator += (const clr3f& rhs);
	clr3f& operator -= (const clr3f& rhs);

	clr3f& AsLinear(float gamma = 2.4f);
	clr3f& AsSRGB(float gamma = 2.8f);

	bool IsZero();

	float LargestValue() const { return max(r, max(g, b)); }

private:
	float __toLinear(float color, float gamma);
	float __toSRGB(float color, float gamma);
};