#pragma once

//Polygon3D.h
#include	"d3d11.h"
#include "collider.h" // AABB を使うためにインクルード

enum Form
{
	Normal = 0,			// 通常
	FirstEvolution,		// 1進化
	SecondEvolution		// 2進化
};

// プレイヤーオブジェクト専用の構造体
struct PlayerObject
{
	XMFLOAT3 position;	// 座標
	XMFLOAT3 rotation;	// 回転角度
	XMFLOAT3 scaling;	// 拡大率
	AABB boundingBox;	// 当たり判定

	Form form;		// 変身形態

	XMFLOAT3 knockback_velocity = { 0.0f, 0.0f, 0.0f }; // 吹き飛ばし用の速度ベクトル
	bool is_knocked_back = false;                       // 吹き飛ばし中かどうか
	float knockback_duration = 0.0f;                    // 吹き飛ばしの残り時間（フレーム数

	float power;	// パワー
	float speed;
};

void	Polygon3D_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void	Polygon3D_Finalize();
void	Polygon3D_Update();
void	Polygon3D_Draw();

PlayerObject* GetPlayer();


