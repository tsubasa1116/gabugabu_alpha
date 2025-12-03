//field.h
#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
#include "sprite.h"
#include "shader.h"
#include "collider.h"
#include "Building.h"

using namespace DirectX;

//=========================================
// MAP構成ブロックの種類
//=========================================
enum FIELD
{
    FIELD_BOX = 0,        // 箱
    FIELD_BUILDING,       // 建物（Building）
    FIELD_MAX
};


//=========================================
// MAPデータ構造体（1マス分）
//=========================================
class MAPDATA
{
public:
    XMFLOAT3 pos;         // 座標
    AABB boundingBox;     // 当たり判定
    FIELD no;             // 種類（FIELD_BOX / FIELD_BUILDING）

    bool isActive = true;

    // --- ここを追加！ ---
    BuildingType type = BuildingType::None;
    
};


//=========================================
// Field 関数
//=========================================
void Field_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Field_Finalize(void);
void Field_Draw(void);
void Field_Update(void);

MAPDATA* GetFieldObjects();
int GetFieldObjectCount();

//BOX作成関数
void	CreateBox();