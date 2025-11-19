
//field.h
#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
#include "sprite.h"
#include "shader.h"
#include "collider.h"

using namespace DirectX;

//MAP構成ブロックの種類
enum FIELD
{
	FIELD_BOX = 0,
	FIELD_BUILDING,
	FIELD_MAX
};
//MAPデータ構造体
class MAPDATA
{
public:
	XMFLOAT3	pos;	//ブロックの座標
	AABB boundingBox;	// 当たり判定
	FIELD		no;		//ブロックの種類
	//その他必要な物は追加する

	bool isActive = true;
};
void Field_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Field_Finalize(void);
void Field_Draw(void);
void Field_Update(void);

// ゲッターの戻り値の型を MapBlock* に変更
MAPDATA* GetFieldObjects();
int GetFieldObjectCount();

//BOX作成関数
void	CreateBox();