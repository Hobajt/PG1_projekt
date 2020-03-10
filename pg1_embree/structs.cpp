#include "stdafx.h"
#include "structs.h"

clr3f clr3f::operator * (float f) {
	return { r * f, g * f, b * f };
}

clr3f clr3f::operator * (const clr3f& rhs) {
	return { r * rhs.r, g * rhs.g, b * rhs.b };
}

clr3f& clr3f::operator*=(float f) {
	r *= f;
	g *= f;
	b *= f;
	return *this;
}

clr3f& clr3f::operator *= (const clr3f& rhs) {
	r *= rhs.r;
	g *= rhs.g;
	b *= rhs.b;
	return *this;
}

clr3f clr3f::operator + (float f) {
	return { r + f, g + f, b + f };
}

clr3f clr3f::operator + (const clr3f& rhs) {
	return { r + rhs.r, g + rhs.g, b + rhs.b };
}

clr3f& clr3f::operator += (const clr3f& rhs) {
	r += rhs.r;
	g += rhs.g;
	b += rhs.b;
	return *this;
}

clr3f& clr3f::operator-=(const clr3f& rhs) {
	r -= rhs.r;
	g -= rhs.g;
	b -= rhs.b;
	return *this;
}

clr3f& clr3f::AsLinear(float gamma) {
	r = __toLinear(r, gamma);
	g = __toLinear(g, gamma);
	b = __toLinear(b, gamma);
	return *this;
}

clr3f& clr3f::AsSRGB(float gamma) {
	r = __toSRGB(r, gamma);
	g = __toSRGB(g, gamma);
	b = __toSRGB(b, gamma);
	return *this;
}

bool clr3f::IsZero() {
	return b == 0.f && g == 0.f && r == 0.f;
}

float clr3f::__toLinear(float color, float gamma) {
	if (color <= 0.0f) return 0.0f;
	else if (color >= 1.0f) return 1.0f;
	assert((color >= 0.0f) && (color <= 1.0f));
	if (color <= 0.04045f) {
		return color / 12.92f;
	}
	else {
		const float a = 0.055f;
		return powf((color + a) / (1.0f + a), gamma);
	}
}

float clr3f::__toSRGB(float color, float gamma) {
	if (color <= 0.0f) return 0.0f;
	else if (color >= 1.0f) return 1.0f;
	assert((color >= 0.0f) && (color <= 1.0f));
	if (color <= 0.0031308f) {
		return 12.92f * color;
	}
	else {
		const float a = 0.055f;
		return (1.0f + a) * powf(color, 1.0f / gamma) - a;
	}
}
