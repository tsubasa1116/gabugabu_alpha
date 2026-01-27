#pragma once

#include <d3d11.h>
#include "collider.h"
#include <DirectXMath.h>
#include <vector>

// マクロ定義
#define SPECIAL_GLASS_TIME			(10.0f)	// 
#define SPECIAL_GLASS_LOCKON_TIME	(3.0f)	// スペシャル ガラス ロックオン時間
#define SPECIAL_CONCRETE_TIME		(10.0f)	// 
#define SPECIAL_PLANT_TIME			(10.0f)	// 
#define SPECIAL_ELECTRIC_TIME		(10.0f)	// 

struct SPECIAL_OBJECT
{
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scaling;
	//bool Use;

	AABB boundingBox;
};

// ミサイル（Glass）構造体
struct GLASS_MISSILE
{
	bool	active = false;
	XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 vel = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float	speed = 0.0f;
};

// Glass専用のスキル管理構造体（5つの箱の情報を格納する）
struct SPECIAL_GLASS
{
	// Glassスキルが生成する5つの箱
	SPECIAL_OBJECT boxes[5];

	// スキルの現在の状態
	bool isActive = false;
	float duration = 0.0f;

	// スキルの全体的な親座標が必要な場合
	XMFLOAT3 parentPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// 他プレイヤーの位置を格納する配列（ロックオン時に一度だけ保存）
	std::vector<XMFLOAT3> lockedTargets;

	// 発射済みフラグ（ロックオン後に一度だけ発射）
	bool hasSpawned = false;

	// ロックオン済みフラグ（ロックオンデータを保存したか）
	bool locked = false;

	// ミサイル配列
	GLASS_MISSILE missiles[5];
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
