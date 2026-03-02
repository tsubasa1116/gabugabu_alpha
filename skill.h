#pragma once

#include <d3d11.h>
#include "collider.h"

// マクロ定義
#define SKILL_GLASS_TIME		(10.0f)
#define SKILL_CONCRETE_TIME		(10.0f)
#define SKILL_PLANT_TIME		(10.0f)
#define SKILL_ELECTRICITY_TIME	(10.0f)

#define SKILL_GLASS_COOLTIME		(5.0f)
#define SKILL_CONCRETE_COOLTIME		(5.0f)
#define SKILL_PLANT_COOLTIME		(5.0f)
#define SKILL_ELECTRICITY_COOLTIME	(5.0f)

#define SKILL_GLASS_DAMAGE			(0.25f)

#define SKILL_SE_COUNT				(4)	// スキル SEの数

struct SKILL_OBJECT
{
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scaling;

	AABB boundingBox;
};

// Glass専用のスキル管理構造体（5つの箱の情報を格納する）
struct SKILL_GLASS
{
	// Glassスキルが生成する5つの箱
	SKILL_OBJECT boxes[5];

	// スキルの現在の状態
	bool isActive = false;
	float duration = 0.0f;

	// スキルの全体的な親座標が必要な場合
	XMFLOAT3 parentPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
};

void Skill_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Skill_Finalize();
void Skill_Update(int playerIndex);
void Skill_Draw(int playerIndex);

void Skill_Glass_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Skill_Concrete_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Skill_Plant_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Skill_Electricity_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

void Skill_Glass_Update(int playerIndex);
void Skill_Concrete_Update(int playerIndex);
void Skill_Plant_Update(int playerIndex);
void Skill_Electricity_Update(int playerIndex);

void Skill_Glass_Draw(int playerIndex);
void Skill_Concrete_Draw(int playerIndex);
void Skill_Plant_Draw(int playerIndex);
void Skill_Electricity_Draw(int playerIndex);

SKILL_OBJECT* GetSkill(int playerIndex);
