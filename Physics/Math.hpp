#pragma once 
#include <math.h>

struct Vector3
{
	float x, y, z;
	Vector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

	float SquaredLength() const { return x * x + y * y + z * z; }

	float Length() const { return sqrtf(SquaredLength()); }

	float Dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }
	float Cross(const Vector3& other) const { return x * other.y - y * other.x; }

	Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
	Vector3& operator+=(const Vector3& other) { x += other.x; y += other.y; z += other.z; return *this; }

	Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
	Vector3& operator-=(const Vector3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }

	Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
	Vector3 operator/(float scalar) const { return Vector3(x / scalar, y / scalar, z / scalar); }

	Vector3 Normalize() const
	{
		float len = Length();
		if (len > 0)
			return Vector3(x / len, y / len, z / len);
		return Vector3(0, 0, 0);
	}

};