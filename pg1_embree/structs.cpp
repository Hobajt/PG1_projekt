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

bool clr3f::IsZero() {
	return b == 0.f && g == 0.f && r == 0.f;
}

clr3f clr3f::AsLinear(const clr3f& c, float gamma) {
	return clr3f{
		__toLinear(c.r, gamma),
		__toLinear(c.g, gamma),
		__toLinear(c.b, gamma)
	};
}

clr3f clr3f::AsSRGB(const clr3f& c, float _1_gamma) {
	return clr3f{
		__toSRGB(c.r, _1_gamma),
		__toSRGB(c.g, _1_gamma),
		__toSRGB(c.b, _1_gamma)
	};
}

clr3f clr3f::ToLinear(float gamma) {
	return clr3f{ 
		__toLinear(r, gamma), 
		__toLinear(g, gamma), 
		__toLinear(b, gamma) 
	};
}

clr3f clr3f::ToSRGB(float _1_gamma) {
	return clr3f{
		__toSRGB(r, _1_gamma),
		__toSRGB(g, _1_gamma),
		__toSRGB(b, _1_gamma)
	};
}

clr3f& clr3f::AsLinear(float gamma) {
	r = __toLinear(r, gamma);
	g = __toLinear(g, gamma);
	b = __toLinear(b, gamma);
	return *this;
}

clr3f& clr3f::AsSRGB(float _1_gamma) {
	r = __toSRGB(r, _1_gamma);
	g = __toSRGB(g, _1_gamma);
	b = __toSRGB(b, _1_gamma);
	return *this;
}

clr4f clr4f::operator*(float f) const {
	return clr4f{ r * f,g * f,b * f,a };
}

clr4f clr4f::operator*(const clr4f& rhs) const {
	return clr4f{ r * rhs.r,g * rhs.g,b * rhs.b, a };
}

clr4f& clr4f::operator*=(float f) {
	r *= f;
	g *= f;
	b *= f;
	return *this;
}

clr4f& clr4f::operator*=(const clr4f& rhs) {
	r *= rhs.r;
	g *= rhs.g;
	b *= rhs.b;
	return *this;
}

clr4f clr4f::operator+(const clr4f& rhs) const {
	return clr4f{ r + rhs.r, g + rhs.g, b + rhs.b, a };
}

clr4f clr4f::operator+(float rhs) const {
	return clr4f{ r + rhs, g + rhs, b + rhs, a };
}

clr4f& clr4f::operator+=(const clr4f& rhs) {
	r += rhs.r;
	g += rhs.g;
	b += rhs.b;
	return *this;
}

clr4f clr4f::operator/(const clr4f& rhs) const {
	return clr4f{ r / rhs.r, g / rhs.g, b / rhs.b, a };
}

clr4f& clr4f::operator/=(const clr4f& rhs) {
	r /= rhs.r;
	g /= rhs.g;
	b /= rhs.b;
	return *this;
}

clr4f clr4f::ToLinear(float gamma) {
	return clr4f{
		__toLinear(r, gamma),
		__toLinear(g, gamma),
		__toLinear(b, gamma),
		a
	};
}

clr4f clr4f::ToSRGB(float _1_gamma) {
	return clr4f{
		__toSRGB(r, _1_gamma),
		__toSRGB(g, _1_gamma),
		__toSRGB(b, _1_gamma),
		a
	};
}

clr4f operator+(float lhs, const clr4f& rhs) {
	return rhs + lhs;
}

float __toSRGB(float c_linear, float _1_gamma) {
	if (c_linear <= 0.0f) return 0.0f;
	else if (c_linear >= 1.0f) return 1.0f;
	assert((c_linear >= 0.0f) && (c_linear <= 1.0f));
	if (c_linear <= 0.0031308f) {
		return 12.92f * c_linear;
	}
	else {
		const float a = 0.055f;
		return (1.0f + a) * powf(c_linear, _1_gamma) - a;
	}

}

float __toLinear(float c_srgb, float gamma) {
	if (c_srgb <= 0.0f) return 0.0f;
	else if (c_srgb >= 1.0f) return 1.0f;
	assert((c_srgb >= 0.0f) && (c_srgb <= 1.0f));
	if (c_srgb <= 0.04045f) {
		return c_srgb / 12.92f;
	}
	else {
		const float a = 0.055f;
		return powf((c_srgb + a) / (1.0f + a), gamma);
	}
}
