#pragma once

#include <d3d11.h>
#include "collider.h"
#include <DirectXMath.h>
#include <vector>

// マクロ定義
//#define SPECIAL_GLASS_TIME ミサイル全てのactiveがfalseになったら終了なので不要
#define SPECIAL_CONCRETE_TIME		(1.5f)
#define SPECIAL_PLANT_TIME			(10.0f)
#define SPECIAL_ELECTRIC_TIME		(5.0f)

#define SPECIAL_GLASS_DAMAGE		(15.0f)	// ミサイル1個あたりのダメージ量
#define SPECIAL_CONCRETE_DAMAGE		(30.0f)	// 判定1回のみ
#define SPECIAL_PLANT_DAMAGE		(0.05f)	// スリップダメージ
#define SPECIAL_ELECTRIC_DAMAGE		(0.0f)	// ダメージなし

#define SPECIAL_GLASSBOX_QUANTITY	(3)		// ガラス 1プレイヤーに飛ばす箱の数
#define SPECIAL_ELECTRIC_QUANTITY	(4)		// 電気 落雷の数

// electricCircles を外部から参照可能にする
extern Circle electricCircles[SPECIAL_ELECTRIC_QUANTITY];

// ガラススペシャル ミサイルオブジェクト
struct GLASS_BOX
{
	XMFLOAT3 position;			// 位置
	XMFLOAT3 rotation;			// 回転
	XMFLOAT3 scaling;			// スケール
	XMFLOAT3 dir;				// 移動方向
	XMFLOAT3 targetPosition;	// 目標位置
	bool active;				// 有効状態
};

// ガラススペシャル ミサイルリストを外部から参照可能にする
extern std::vector<GLASS_BOX> glassBoxes;

struct SPECIAL_OBJECT
{
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scaling;

	AABB boundingBox;
};

void Special_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Special_Finalize();
void Special_Update(int playerIndex);
void Special_Draw();

void Special_Glass_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Special_Concrete_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Special_Plant_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Special_Electric_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

void Special_Glass_Update(int playerIndex);
void Special_Concrete_Update(int playerIndex);
void Special_Plant_Update(int playerIndex);
void Special_Electric_Update(int playerIndex);

void Special_Glass_Draw(int playerIndex);
void Special_Concrete_Draw(int playerIndex);
void Special_Plant_Draw(int playerIndex);
void Special_Electric_Draw(int playerIndex);

SPECIAL_OBJECT* GetSpecial(int playerIndex);