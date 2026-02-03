// Camera.h

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
using namespace DirectX;

class CAMERA
{
public:
	XMFLOAT3 position;		// 座標
	XMFLOAT3 atPosition;	// 注視点
	XMFLOAT3 upVector;		// 上方ベクトル

	XMMATRIX view;			// ビュー行列
	XMMATRIX projection;	// プロジェクション行列

	float fov;		// 視野角（画角）
	float aspect;	// 画面のアスペクト比
	float nearClip;	// 近面クリップ距離
	float farClip;	// 遠面クリップ距離
};

void Camera_Initialize();
void Camera_Finalize();
void Camera_Update();
void Camera_Draw();

void SetCameraFov(float);
void SetCameraAspect(float);
void SetCameraClip(float, float);

void SetCameraPosition(XMFLOAT3);
void SetCameraAtPosition(XMFLOAT3);
void SetCameraUpVector(XMFLOAT3);

XMMATRIX GetViewMatrix();
XMMATRIX GetProjectionMatrix();

