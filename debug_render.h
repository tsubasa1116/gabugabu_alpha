#pragma once
//======================================================
//	debug_render.h[]
// 
//	制作者：前野翼			日付：2025//
//======================================================
#pragma once
#include "d3d11.h"
#include "DirectXMath.h"
#include "collider.h" // AABBとHexColliderの定義が必要

// 描画で使用する頂点構造体
struct DebugVertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
	XMFLOAT2 uv; // シェーダーがUVを要求する場合はダミーを入れる
};

// 初期化
void Debug_Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

// 終了処理
void Debug_Finalize();

// AABB（四角い箱）のワイヤーフレーム描画
void Debug_DrawAABB(const AABB& aabb, DirectX::XMFLOAT4 color);

// 六角柱のワイヤーフレーム描画
void Debug_DrawHex(const HexCollider& hex, DirectX::XMFLOAT4 color);