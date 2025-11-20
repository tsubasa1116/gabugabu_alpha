#pragma once
//======================================================
//	collider.h
// -----------------------------------------------------
//	制作者：前野翼			日付：2024//
//======================================================
#include <DirectXMath.h>   // ← これが必須！ XMFLOAT3 の定義
using namespace DirectX;
//======================================================
//	コライダー
//======================================================
struct AABB
{
	XMFLOAT3 Min;	// 最小点
	XMFLOAT3 Max;	// 最大点
};


//======================================================
// 最小重なり量 (MTV) を返す構造体
//======================================================
struct MTV
{
	XMFLOAT3 translation;	// 押し戻すための移動ベクトル
	float overlap;			// 最小の重なり量
	bool isColliding;		// 衝突したかどうか
};

//======================================================
//	当たり判定 計算関数
//======================================================

/**
 * @brief オブジェクトの座標と拡縮からAABB（軸並行境界ボックス）を計算する
 * @param boundingBox [out] 計算結果が格納されるAABB
 * @param position オブジェクトの中心座標
 * @param scaling オブジェクトの拡縮（サイズ）
 */
void CalculateAABB(AABB& boundingBox, const XMFLOAT3& position, const XMFLOAT3& scaling);

/**
 * @brief 2つのAABBが衝突しているかチェックする
 * @return 衝突していれば true
 */
bool CheckAABBCollision(const AABB& a, const AABB& b);

/**
 * @brief 2つのAABBの衝突を検出し、押し戻しベクトル(MTV)を計算する
 * @param pMovingObject 動くオブジェクトのAABB
 * @param pStaticObject 静的なオブジェクトのAABB
 * @return 衝突情報と押し戻しベクトル(MTV)
 */
MTV CalculateAABBMTV(const AABB& pMovingObject, const AABB& pStaticObject);