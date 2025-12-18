#pragma once

#include "d3d11.h"
#include "collider.h"

// マクロ定義
#define SKILL_GLASS_TIME	(10.0f)
#define SKILL_CONCRETE_TIME	(10.0f)
#define SKILL_PLANT_TIME	(10.0f)
#define SKILL_ELECTRIC_TIME	(10.0f)

struct SKILL_OBJECT
{
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scaling;
	//bool Use;

	AABB boundingBox;
};

// Glass専用のスキル管理構造体（5つの箱の情報を格納する）
struct GlassSkill
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
void Skill_Draw();

void Skill_Glass_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Skill_Concrete_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Skill_Plant_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Skill_Electric_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

void Skill_Glass_Update(int playerIndex);
void Skill_Concrete_Update(int playerIndex);
void Skill_Plant_Update(int playerIndex);
void Skill_Electric_Update(int playerIndex);

void Skill_Glass_Draw(int playerIndex);
void Skill_Concrete_Draw(int playerIndex);
void Skill_Plant_Draw(int playerIndex);
void Skill_Electric_Draw(int playerIndex);

SKILL_OBJECT* GetSkill(int playerIndex);
