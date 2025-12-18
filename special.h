#pragma once

#include "d3d11.h"
#include "collider.h"

// マクロ定義
#define SPECIAL_GLASS_TIME	    (10.0f)
#define SPECIAL_CONCRETE_TIME	(10.0f)
#define SPECIAL_PLANT_TIME	    (10.0f)
#define SPECIAL_ELECTRIC_TIME	(10.0f)

struct SPECIAL_OBJECT
{
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scaling;
	//bool Use;

	AABB boundingBox;
};

// Glass専用のスキル管理構造体（5つの箱の情報を格納する）
struct GlassSpecial
{
    // Glassスキルが生成する5つの箱
    SPECIAL_OBJECT boxes[5];

    // スキルの現在の状態
    bool isActive = false;
    float duration = 0.0f;

    // スキルの全体的な親座標が必要な場合
    XMFLOAT3 parentPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
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
