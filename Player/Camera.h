#pragma once 

#include  "../Utils/Math.hpp"


class Camera {
public:
	Camera(const Vector3& pos); 
	~Camera() = default;

	Camera(const Camera&) = default;
	Camera& operator=(const Camera&) = default;

	Camera(Camera&&) = default;
	Camera& operator=(Camera&&) = default;

public:
	void SetPosition(const Vector3& pos); 
	const Vector3 GetPosition() const;

	Matrix3x3 GetViewMatrix() const;
};
