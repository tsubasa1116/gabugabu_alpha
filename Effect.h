// Effect.h

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"

class EFFECT
{
public:
	bool enable;
	XMFLOAT3 pos;
	XMFLOAT2 size;
	int frameCnt;	// アニメーションカウンター
	int texNo;
};

// メイン処理関数
void Effect_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Effect_Finalize();
void Effect_Update();
void Effect_Draw();
void Effect_Set(int texNo, XMFLOAT2 pos, XMFLOAT2 size);
void Effect_Clear(int pIndex);
