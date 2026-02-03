#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "polygon.h"
#include "sprite.h"
#include "shader.h"

#define HPBER_MAX (4)         // HPバー最大数
#define HPBAR_SPEED (3.0f)    // HPバーのスムーズ減少速度

struct HP {
	XMFLOAT2 pos;      // 位置
	XMFLOAT2 size;     // サイズ
	float current;     // 現在のHPバー幅
	float target;      // 目標HPバー幅
	bool use;          // 使用中かどうか
	XMFLOAT4 backColor;  // 背景色
	XMFLOAT4 fillColor;  // 残量色

	XMFLOAT2 shakeOffset;   // 描画時に加えるオフセット
	float shakeTimer;       // 残りフレーム数
	float shakeDuration;    // 設定したフレーム長さ
	float shakeAmplitude;   // 振幅
	float shakeSpeed;       // 振動速度
	int gaugeIndex;
};

//=============================================
// HPバーを設定したいcppで呼び出す(参考:p.cpp)
//=============================================
void InitializeHP(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HP* bar, XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 backColor, XMFLOAT4 fillColor);
void UpdateHP(HP* bar);
void DrawHP(const HP* bar, int texNum);
void SetHPValue(HP* bar, int currentHP, int maxHP);
void FinalizeHP(HP* bar);

// シェイク　フレーム単位（duration）とピクセル（amplitude）
void SetHPShake(HP* bar, float amplitude = 8.0f, float duration = 15.0f, float speed = 1.5f);
HP* GetHPBar(int HPIndex);

