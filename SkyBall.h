#pragma once

// SkyBall.h

#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
using namespace DirectX;
#include "model.h"

// 天球オブジェクト
class SkyBallObject
{
public:
	XMFLOAT3	Position;
	XMFLOAT3	Rotation;
	XMFLOAT3	Scaling;

	MODEL* Model;

	float		Speed;	// 回転量
};

void SkyBall_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void SkyBall_Finalize();
void SkyBall_Update();
void SkyBall_Draw();

SkyBallObject* GetSkyBall();
