#pragma once

#include <d3d11.h>
#include "collider.h"
#include <DirectXMath.h>
#include <vector>

// マクロ定義
//#define SPECIAL_GLASS_TIME ミサイル全てのactiveがfalseになったら終了なので不要
#define SPECIAL_CONCRETE_TIME		(1.5f)
#define SPECIAL_PLANT_TIME			(5.0f)
#define SPECIAL_ELECTRICITY_TIME	(5.0f)

#define SPECIAL_GLASS_DAMAGE		(100.0f)// ミサイル 1個あたりのダメージ量
#define SPECIAL_CONCRETE_DAMAGE		(75.0f)	// 判定1回のみ
#define SPECIAL_PLANT_DAMAGE		(0.15f)	// スリップダメージ
#define SPECIAL_ELECTRICITY_DAMAGE	(20.0f)	// 雷 1個あたりのダメージ量

#define SPECIAL_GLASSBOX_QUANTITY		(3)	// ガラス 1プレイヤーに飛ばす箱の数
#define SPECIAL_ELECTRICITY_QUANTITY	(6)	// 電気 落雷の数
#define SPECIAL_PLANT_RADIUS		(4.0f)	// 植物 範囲 半径

#define SPECIAL_SE_COUNT				(3)	// スペシャル SEの数 ガラスを除く

// ガラススペシャル ミサイルオブジェクト
struct GLASS_BOX
{
	XMFLOAT3 position;			// 位置
	XMFLOAT3 rotation;			// 回転
	XMFLOAT3 scaling;			// スケール
	XMFLOAT3 dir;				// 移動方向
	XMFLOAT3 targetPosition;	// 目標位置
	bool active;				// 有効状態
	bool spawned;				// 出現済みフラグ
	float spawnDelay;			// 出現までの遅延時間（秒）
	int phase;					// 移動フェーズ 0:上昇 1:横移動 2:降下
};

struct SPECIAL_OBJECT
{
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scaling;

	AABB boundingBox;
};

struct PLANT_CIRCLE
{
	XMFLOAT3 position;
	float radius;
};

void Special_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Special_Finalize();
void Special_Update(int playerIndex);
void Special_Draw(int playerIndex);

// 描画パス分割用（範囲表示 = プレイヤーより先、エフェクト本体 = プレイヤーより後）
void Special_DrawRange(int playerIndex);   // 範囲表示のみ（地面に描画）
void Special_DrawEffect(int playerIndex);  // エフェクト本体のみ（手前に描画）

void Special_Glass_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Special_Concrete_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Special_Plant_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Special_Electricity_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

void Special_Glass_Update(int playerIndex);
void Special_Concrete_Update(int playerIndex);
void Special_Plant_Update(int playerIndex);
void Special_Electricity_Update(int playerIndex);
void Special_Electricity_Update2(int playerIndex);

void Special_Glass_Draw(int playerIndex);
void Special_Concrete_Draw(int playerIndex);
void Special_Plant_Draw(int playerIndex);
void Special_Electricity_Draw(int playerIndex);
void Special_Electricity_Draw2(int playerIndex);

SPECIAL_OBJECT* GetSpecial(int playerIndex);
