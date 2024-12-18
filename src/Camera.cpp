#include "Camera.h"

Camera::Camera(const float& width,
	const float& height,
	const float& verticalFOV,
	const float& near,
	const float& far, 
	const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir):
	width(width), height(height),
	verticalFOV(verticalFOV),
	near(near), far(far),
	pos(pos), dir(dir)
{

}
