#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "polygon.h"
#include "sprite.h"
#include "shader.h"


struct HP {
	XMFLOAT2 pos;      // 位置
	XMFLOAT2 size;     // サイズ
	float current;     // 現在のHPバー幅
	float target;      // 目標HPバー幅
	bool use;          // 使用中かどうか
	XMFLOAT4 backColor;  // 背景色
	XMFLOAT4 fillColor;  // 残量色
};

//=============================================
// HPバーを設定したいcppで呼び出す(参考:p.cpp)
//=============================================
void InitializeHP(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HP* bar, XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 backColor, XMFLOAT4 fillColor);
void UpdateHP(HP* bar);
void DrawHP(const HP* bar);
void SetHPValue(HP* bar, int currentHP, int maxHP);
void FinalizeHP(HP* bar);

