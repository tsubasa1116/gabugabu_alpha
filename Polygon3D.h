// Polygon3D.h

#pragma once

#include "d3d11.h"
#include "collider.h" // AABB を使うためにインクルード

enum Form
{
	Normal = 0,			// 通常
	FirstEvolution,		// 1進化
	SecondEvolution		// 2進化
};

// プレイヤーオブジェクト専用の構造体
struct PLAYEROBJECT
{
	XMFLOAT3 position;	// 座標
	XMFLOAT3 rotation;	// 回転角度
	XMFLOAT3 scaling;	// 拡大率
	AABB boundingBox;	// 当たり判定
	float hp;			// 体力
	float maxHp;		// 最大体力
	float power;		// パワー
	float speed;		// スピード
	int residue;		// 残機
	bool active;        // 生存フラグ
	bool isAttaking;	// 攻撃中かどうか
	float attackTimer;	// 攻撃タイマー
	float moveAngle = 0.0f;	// プレイヤー固有の回転補間用角度
	XMFLOAT3 moveDir = { 0.0f, 0.0f, 0.0f };	// 移動ベクトル

	int breakCount_Glass;		// 破壊した数 ガラス
	int breakCount_Plant;		// 破壊した数 植物
	int breakCount_Concrete;	// 破壊した数 コンクリート
	int breakCount_Electricity;	// 破壊した数 電気


	float gl, pl, co, el;
	float gaugeOuter;

	Form form;	// 変身形態

	XMFLOAT3 knockback_velocity = { 0.0f, 0.0f, 0.0f };	// 吹き飛ばし用の速度ベクトル
	bool is_knocked_back = false;						// 吹き飛ばし中かどうか
	float knockback_duration = 0.0f;					// 吹き飛ばしの残り時間（フレーム数
};

void Polygon3D_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Polygon3D_Finalize();
void Polygon3D_Update();
void Polygon3D_Draw(bool s_IsKonamiCodeEntered);

void Polygon3D_Attack();
void Polygon3D_Respawn();

void Polygon3D_DrawResidue(int i);

PLAYEROBJECT* GetPlayer1();
PLAYEROBJECT* GetPlayer2();


