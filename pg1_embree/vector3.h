#ifndef VECTOR3_H_
#define VECTOR3_H_

#include "structs.h"

struct /*ALIGN*/ vec3f {
public:
	union
	{
		struct {
			float x;
			float y;
			float z;
		};

		float data[3];
	};

	vec3f() : x(0), y(0), z(0) {}

	vec3f(const float x, const float y, const float z) : x(x), y(y), z(z) {}

	vec3f(const float* v);

	float L2Norm() const;

	float SqrL2Norm() const;

	vec3f& normalize();
	vec3f normalized();

	vec3f cross(const vec3f& v) const;

	vec3f Abs() const;

	vec3f Max(const float a = 0) const;
	float DotProduct(const vec3f& v) const;
	char LargestComponent(const bool absolute_value = false);

	void Print();

	void SetupFromThis(float& vx, float& vy, float& vz);

	clr3f AsColor();


	float LargestValue();

	inline vec3f Orthogonal(const vec3f& v) const { return (abs(v.x) > abs(v.z)) ? vec3f(-v.y, v.x, 0.0f) : vec3f(0.0f, -v.z, v.y); }
	inline vec3f Orthogonal() const { return (abs(x) > abs(z)) ? vec3f(-y, x, 0.0f) : vec3f(0.0f, -z, y); }

	inline vec3f ReflectVector(const vec3f& in) const {
		float dotNormalIn = this->DotProduct(in);
		return (2.f * dotNormalIn) * *this - in;
	}

	inline vec3f reflect(const vec3f& normal) const { return (2 * DotProduct(normal)) * normal - (*this); }

	float& operator [] (const int idx) { return data[idx % 3]; }


	friend vec3f operator-(const vec3f& v);

	friend vec3f operator+(const vec3f& u, const vec3f& v);
	friend vec3f operator-(const vec3f& u, const vec3f& v);

	friend vec3f operator*(const vec3f& v, const float a);
	friend vec3f operator*(const float a, const vec3f& v);
	friend vec3f operator*(const vec3f& u, const vec3f& v);

	friend vec3f operator/(const vec3f& v, const float a);

	friend void operator+=(vec3f& u, const vec3f& v);
	friend void operator-=(vec3f& u, const vec3f& v);
	friend void operator*=(vec3f& v, const float a);
	friend void operator/=(vec3f& v, const float a);
};

#endif
