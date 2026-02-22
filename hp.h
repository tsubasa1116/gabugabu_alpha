#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "polygon.h"
#include "sprite.h"
#include "shader.h"

#define HPBER_MAX (4)		// HPバー最大数
#define HPBAR_SPEED (3.0f)	// HPバーのスムーズ減少速度

struct hp {
	XMFLOAT2 pos;           // 位置
	XMFLOAT2 size;          // サイズ
	float current;          // 現在のHPバー幅（即座に減る）
	float target;           // 目標HPバー幅
	float damageCurrent;    // ダメージ表示用（遅れて減る赤バー）
	float damageDelay;      // 赤バーが減り始めるまでの遅延フレーム
	float damageTimer;		// ダメージ用タイマー
	bool use;               // 使用中かどうか
	XMFLOAT4 backColor;     // 背景色
	XMFLOAT4 fillColor;     // 残量色
	XMFLOAT4 damageColor;   // ダメージ色（赤）
	
	XMFLOAT2 shakeOffset;   // 描画時に加えるオフセット
	float shakeTimer;       // 残りフレーム数
	float shakeDuration;    // 設定したフレーム長さ
	float shakeAmplitude;   // 振幅
	float shakeSpeed;       // 振動速度
	int gaugeIndex;
	int shakeTexNum;        // シェイク中に使うテクスチャ番号（-1で無効）
	int deathTexNum;
};

//=============================================
// HPバーを設定したいcppで呼び出す
//=============================================
void InitializeHP(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, hp* bar, XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 backColor, XMFLOAT4 fillColor);
void UpdateHP(hp* bar);
void DrawHP(const hp* bar, int texNum, bool isDead);
void SetHPValue(hp* bar, int currentHP, int maxHP);
void FinalizeHP(hp* bar);

// シェイク　フレーム単位（duration）とピクセル（amplitude）
// int shakeTexNum = -1 を渡すと、シェイク中にそのテクスチャを使用する
void SetHPShake(hp* bar, float amplitude = 8.0f, float duration = 15.0f, float speed = 1.5f, int shakeTexNum = -1);
void SetDeathHP(hp* bar, int deathTexNum = -1);
hp* GetHPBar(int HPIndex);

