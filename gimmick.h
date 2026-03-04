//======================================================
//	gimmick.h
// 
//	隕石ギミック
//======================================================
#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "collider.h"

using namespace DirectX;

// マクロ定義
#define METEOR_COOLTIME			(5.0f)	// クールタイム（秒）
#define METEOR_RANGE_RADIUS		(1.5f)	// 技範囲（半径）
#define METEOR_FALL_SPEED		(8.0f)	// 隕石の落下速度
#define METEOR_START_HEIGHT		(15.0f)	// 隕石の出現高さ
#define METEOR_DAMAGE			(40.0f)	// 隕石のダメージ

// 範囲アニメーション定数
#define METEOR_RANGE_SHEET_COLS	(8)		// スプライトシート列数
#define METEOR_RANGE_SHEET_ROWS	(8)		// スプライトシート行数
#define METEOR_RANGE_FRAME_MAX	(30)	// アニメーション総フレーム数
#define METEOR_RANGE_ANIM_TIME	(0.15f)	// 1フレームあたりの秒数

// 隕石モデルのスケール
#define METEOR_MODEL_SCALE		(1.0f)	// FBXモデルの表示スケール

// 隕石オブジェクト
struct METEOR_OBJECT
{
	XMFLOAT3 position;		// 座標
	XMFLOAT3 rotation;		// 回転角度
	XMFLOAT3 scaling;		// 拡大率
	XMFLOAT3 targetPos;		// 着弾目標位置
	Circle   collider;		// 当たり判定（円）
	bool active;			// 有効フラグ
	bool landed;			// 着弾フラグ
};

// ギミック管理構造体（プレイヤーごと）
struct GIMMICK_STATE
{
	bool enabled;			// ギミック有効フラグ
	float coolTimer;		// クールタイムタイマー
	bool canFire;			// 発射可能フラグ
	METEOR_OBJECT meteor;	// 隕石オブジェクト

	// 範囲アニメーション用
	int   rangeAnimFrame;	// 現在のアニメーションフレーム
	float rangeAnimTimer;	// アニメーションタイマー

	bool hitPlayer;			// true = プレイヤーに命中した着弾
};

void Meteor_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Meteor_Finalize();
void Meteor_Update();
void Meteor_Draw(bool debugDraw = false);

// 描画パス分割用
void Meteor_DrawRange(bool debugDraw = false);	// 範囲表示のみ
void Meteor_DrawModel(bool debugDraw = false);	// 隕石モデルのみ

GIMMICK_STATE* GetGimmick(int playerIndex);