#include "Camera.h"
#include "CameraGlobals.hpp"

Camera::Camera(const Vector3& pos){
	CameraGlobals::CAMERA_POSITION = pos;
}

void Camera::SetPosition(const Vector3& pos)
{
	CameraGlobals::CAMERA_POSITION = pos;
}

const Vector3 Camera::GetPosition() const
{
    return CameraGlobals::CAMERA_POSITION; 
}

Matrix3x3 Camera::GetViewMatrix() const
{
	auto rotationMatrix = Matrix3x3::Rotation(0.0f); // -theta
	auto translationMatrix = Matrix3x3::Translation(-CameraGlobals::CAMERA_POSITION.x, -CameraGlobals::CAMERA_POSITION.y);

	return rotationMatrix * translationMatrix;
}
