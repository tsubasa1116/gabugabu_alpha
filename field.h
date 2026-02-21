//======================================================
// field.h
//======================================================
#pragma once


#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
#include "sprite.h"
#include "shader.h"
#include "collider.h"

using namespace DirectX;

//======================================================
// フィールドの種類（カテゴリのみ）
//======================================================
enum FIELD
{
	FIELD_BOX = 0,		// 何も置かないマス

	FIELD_Glass,		// ガラス建物
	FIELD_Concrete,		// コンクリ建物
	FIELD_Plant,		// 植物建物
	FIELD_Electricity,	// 電気建物

	FIELD_None,
	FIELD_MAX			// 終端マーカー
};

//======================================================
// MAPデータ（1マス分）
//======================================================
struct MAPDATA
{
	XMFLOAT3 pos{};			// 座標
	AABB boundingBox{};		// 当たり判定
	FIELD no;				// フィールド種別

	//==================================================
	// ★超重要★
	// 同じ FIELD 種別の中で、どのモデルを使うか
	// 0 = 1つ目のモデル
	// 1 = 2つ目のモデル …
	//==================================================
	int variant = 0;

	bool isActive = true;

	// 六角形当たり判定用
	float radius = 1.0f;
	float height = 2.25f;

	float respawnTimer;
	float respawnTimeMax;
};

//======================================================
// Field 関数
//======================================================
void Field_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Field_Finalize(void);
void Field_Draw(bool s_IsKonamiCodeEntered);
void Field_Update(void);

MAPDATA* GetFieldObjects();
int GetFieldObjectCount();
