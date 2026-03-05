//======================================================
//	collider.h
// -----------------------------------------------------
//	制作者：前野翼			日付：2024//
//======================================================
#pragma once

#include <DirectXMath.h>
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
//	扇型の当たり判定用構造体
//======================================================
struct Sector {
	XMFLOAT3 center;		// 扇の起点（プレイヤー足元）
	XMFLOAT3 forward;		// プレイヤーの向いている方向（正規ベクトル）

	float radius = 1.50f;			// 扇の半径（攻撃の届く距離）
	float angleDegree = 90.0f;		// 扇の「全角」（例：90度なら左右に45度ずつ）
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


// ==============================================================================
// 六角柱コライダー (Hexagon Collider)
// ==============================================================================
struct HexCollider
{
	XMFLOAT3 center;	// 中心座標
	float radius;		// 半径（中心から角までの距離）
	float height;		// 高さ（厚み）
};

// 点と六角柱の当たり判定（中に入っているか？）
bool CheckPointHexCollision(const XMFLOAT3& point, const HexCollider& hex);

bool CheckAABBHexCollision(const AABB& box, const HexCollider& hex);

// ==============================================================================
// 円コライダー (Circle Collider)
// ==============================================================================
struct Circle
{
	XMFLOAT3 center;	// 中心座標
	float radius;		// 半径
};

/**
 * @brief 円とAABBの衝突判定
 * @param circle 円の中心座標と半径
 * @param box AABBデータ
 * @return 衝突していれば true
 */
bool CheckCircleAABBCollision(const Circle& circle, const AABB& box);

// ==============================================================================
// 扇型コライダー (Sector Collider)
// ==============================================================================
bool CheckSectorCollision(const XMFLOAT3& targetPos, const Sector& sector);
bool CheckAABBSectorCollision(const AABB& targetBox, const Sector& sector);