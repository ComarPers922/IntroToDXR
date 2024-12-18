#include "Camera.h"

Camera::Camera(const float& width,
	const float& height,
	const float& verticalFOV,
	const float& nearPlane,
	const float& farPlane,
	const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& forwardDir):
	width(width), height(height),
	verticalFOV(verticalFOV),
	nearPlane(nearPlane), farPlane(farPlane),
	pos(pos), forwardDir(forwardDir)
{

}
