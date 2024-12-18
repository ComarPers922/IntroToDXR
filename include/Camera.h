#pragma once
#include <DirectXMath.h>

using DirectX::XMFLOAT3;
using DirectX::XMMATRIX;
using DirectX::XM_PI;

class Camera
{
public:
	Camera(
		const float& width,
		const float& height,
		const float& verticalFOV = 65.f * (XM_PI / 180.f),
		const float& nearPlane = 0.01f,
		const float& farPlane = 1000.f,
		const XMFLOAT3& pos = XMFLOAT3(0, 0, 0),
		const XMFLOAT3& dir = XMFLOAT3(0, 0, 1));
	XMFLOAT3 pos, forwardDir;
	float verticalFOV, width, height, nearPlane, farPlane;
};

