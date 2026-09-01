#include "Camera.h"

Camera::Camera(const Vector3& pos) : mPosition(pos)
{
}

void Camera::SetPosition(const Vector3& pos)
{
	mPosition = pos;
}

const Vector3 Camera::GetPosition() const
{
    return mPosition; 
}

Matrix3x3 Camera::GetViewMatrix() const
{
	auto rotationMatrix = Matrix3x3::Rotation(0.0f); // -theta
	auto translationMatrix = Matrix3x3::Translation(-mPosition.x, -mPosition.y);

	return rotationMatrix * translationMatrix;
}
