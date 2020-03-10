#ifndef VECTOR3_H_
#define VECTOR3_H_

#include "structs.h"

struct /*ALIGN*/ Vector3 {
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

	Vector3() : x(0), y(0), z(0) {}

	Vector3(const float x, const float y, const float z) : x(x), y(y), z(z) {}

	Vector3(const float* v);

	float L2Norm() const;

	float SqrL2Norm() const;

	void Normalize();

	Vector3 CrossProduct(const Vector3& v) const;

	Vector3 Abs() const;

	Vector3 Max(const float a = 0) const;
	float DotProduct(const Vector3& v) const;
	char LargestComponent(const bool absolute_value = false);

	void Print();

	void SetupFromThis(float& vx, float& vy, float& vz);

	Color3f AsColor();

	Vector3& Normalized();

	float LargestValue();

	inline Vector3 Orthogonal(const Vector3& v) const { return (abs(v.x) > abs(v.z)) ? Vector3(-v.y, v.x, 0.0f) : Vector3(0.0f, -v.z, v.y); }
	inline Vector3 Orthogonal() const { return (abs(x) > abs(z)) ? Vector3(-y, x, 0.0f) : Vector3(0.0f, -z, y); }

	inline Vector3 ReflectVector(const Vector3& in) const {
		float dotNormalIn = this->DotProduct(in);
		return (2.f * dotNormalIn) * *this - in;
	}

	float& operator [] (const int idx) { return data[idx % 3]; }


	friend Vector3 operator-(const Vector3& v);

	friend Vector3 operator+(const Vector3& u, const Vector3& v);
	friend Vector3 operator-(const Vector3& u, const Vector3& v);

	friend Vector3 operator*(const Vector3& v, const float a);
	friend Vector3 operator*(const float a, const Vector3& v);
	friend Vector3 operator*(const Vector3& u, const Vector3& v);

	friend Vector3 operator/(const Vector3& v, const float a);

	friend void operator+=(Vector3& u, const Vector3& v);
	friend void operator-=(Vector3& u, const Vector3& v);
	friend void operator*=(Vector3& v, const float a);
	friend void operator/=(Vector3& v, const float a);
};

#endif
