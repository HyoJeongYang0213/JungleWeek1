#pragma once 
#include <math.h>
#include <algorithm>
#include <cmath>

struct Vector3
{
	float x, y, z;
	constexpr Vector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

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

	Vector3 operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }

	Vector3 Normalize() const
	{
		float len = Length();
		if (len > 0)
			return Vector3(x / len, y / len, z / len);
		return Vector3(0, 0, 0);
	}

};


struct Matrix3x3
{
public:
	Matrix3x3()
	{
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				m[i][j] = (i == j) ? 1.0f : 0.0f;
			}
		}
		
	}
	
	~Matrix3x3() = default;

	Matrix3x3(const Matrix3x3&) = default;
	Matrix3x3& operator=(const Matrix3x3&) = default;

	Matrix3x3(Matrix3x3&&)= default;
	Matrix3x3& operator=(Matrix3x3&&) = default;

	Matrix3x3 operator*(const Matrix3x3& other) const
	{
		Matrix3x3 result;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				result.m[i][j] = 0.0f;
				for (int k = 0; k < 4; ++k)
				{
					result.m[i][j] += m[i][k] * other.m[k][j];
				}
			}
		}
		return result;
	}

public:

	static Matrix3x3 Orthographic(float left, float right, float bottom, float top)
	{
		Matrix3x3 result;
		result.m[0][0] = 2.0f / (right - left);
		result.m[1][1] = 2.0f / (top - bottom);
		result.m[0][2] = -(right + left) / (right - left);
		result.m[1][2] = -(top + bottom) / (top - bottom);
		result.m[2][2] = 1.0f;

		return result;
	}

	static Matrix3x3 Translation(float tx, float ty)
	{
		Matrix3x3 result;
		result.m[0][2] = tx;
		result.m[1][2] = ty;
		return result;
	}

	static Matrix3x3 Rotation(float angle)
	{
		Matrix3x3 result;
		float cosA = std::cos(angle);
		float sinA = std::sin(angle);
		result.m[0][0] = cosA;
		result.m[0][1] = -sinA;
		result.m[1][0] = sinA;
		result.m[1][1] = cosA;
		return result;
	}


public:

	void Transpose()
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = i + 1; j < 4; ++j)
			{
				std::swap(m[i][j], m[j][i]);
			}
		}
	}

public:
	float m[4][4];
};