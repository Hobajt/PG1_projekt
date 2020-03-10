#include "stdafx.h"
#include "vector3.h"
#include "mymath.h"

vec3f::vec3f(const float* v) {
	assert(v != NULL);

	x = v[0];
	y = v[1];
	z = v[2];
}

float vec3f::L2Norm() const {
	return sqrt(sqr(x) + sqr(y) + sqr(z));
}

float vec3f::SqrL2Norm() const {
	return sqr(x) + sqr(y) + sqr(z);
}

vec3f& vec3f::normalize() {
	const float norm = SqrL2Norm();

	if (norm != 0) {
		const float rn = 1 / sqrt(norm);

		x *= rn;
		y *= rn;
		z *= rn;
	}
	return *this;
}

vec3f vec3f::normalized() {
	const float norm = SqrL2Norm();

	if (norm != 0) {
		const float rn = 1 / sqrt(norm);

		return vec3f{ x * rn, y * rn, z * rn };
	}
	return vec3f{ 0.f, 0.f, 0.f };
}

vec3f vec3f::cross(const vec3f& v) const {
	return vec3f(
		y * v.z - z * v.y,
		z * v.x - x * v.z,
		x * v.y - y * v.x);
}

vec3f vec3f::Abs() const {
	return vec3f(abs(x), abs(y), abs(z));
}

vec3f vec3f::Max(const float a) const {
	return vec3f(max(x, a), max(y, a), max(z, a));
}

float vec3f::DotProduct(const vec3f& v) const {
	return x * v.x + y * v.y + z * v.z;
}

char vec3f::LargestComponent(const bool absolute_value) {
	const vec3f d = (absolute_value) ? vec3f(abs(x), abs(y), abs(z)) : *this;

	if (d.x > d.y) {
		if (d.x > d.z) {
			return 0;
		}
		else {
			return 2;
		}
	}
	else {
		if (d.y > d.z) {
			return 1;
		}
		else {
			return 2;
		}
	}

	return -1;
}

void vec3f::Print() {
	printf("(%0.3f, %0.3f, %0.3f)\n", x, y, z);
	//printf( "_point %0.3f,%0.3f,%0.3f\n", x, y, z );
}

void vec3f::SetupFromThis(float& vx, float& vy, float& vz) {
	vx = x;
	vy = y;
	vz = z;
}

clr3f vec3f::AsColor() {
	return { x, y, z };
}

float vec3f::LargestValue() {
	return max(x, max(y, z));
}

vec3f operator-(const vec3f& v) {
	return vec3f(-v.x, -v.y, -v.z);
}

vec3f operator+(const vec3f& u, const vec3f& v) {
	return vec3f(u.x + v.x, u.y + v.y, u.z + v.z);
}

vec3f operator-(const vec3f& u, const vec3f& v) {
	return vec3f(u.x - v.x, u.y - v.y, u.z - v.z);
}

vec3f operator*(const vec3f& v, const float a) {
	return vec3f(a * v.x, a * v.y, a * v.z);
}

vec3f operator*(const float a, const vec3f& v) {
	return vec3f(a * v.x, a * v.y, a * v.z);
}

vec3f operator*(const vec3f& u, const vec3f& v) {
	return vec3f(u.x * v.x, u.y * v.y, u.z * v.z);
}

vec3f operator/(const vec3f& v, const float a) {
	return v * (1 / a);
}

void operator+=(vec3f& u, const vec3f& v) {
	u.x += v.x;
	u.y += v.y;
	u.z += v.z;
}

void operator-=(vec3f& u, const vec3f& v) {
	u.x -= v.x;
	u.y -= v.y;
	u.z -= v.z;
}

void operator*=(vec3f& v, const float a) {
	v.x *= a;
	v.y *= a;
	v.z *= a;
}

void operator/=(vec3f& v, const float a) {
	const float r = 1 / a;

	v.x *= r;
	v.y *= r;
	v.z *= r;
}
