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
