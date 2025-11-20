#include "d3d11.h"
#include "DirectXMath.h"

using namespace DirectX;

#include "collider.h"

// ==============================================================================
// AABB
// ------------------------------------------------------------------------------
// オブジェクトの回転に追従しないコライダー
// つまりコライダーは回転しない
// 
// 一番シンプルだしプロトタイプだからいいよね
// マイクラの当たり判定もこれだし
// ==============================================================================
void CalculateAABB(AABB& boundingBox, const XMFLOAT3& position, const XMFLOAT3& scaling) // 新しい関数
{
	XMFLOAT3 s = scaling;	// オブジェクトのスケール
	XMFLOAT3 p = position;	// オブジェクトの中心座標
	XMFLOAT3 half;	// 半分サイズ

	// ワールド空間でのボックスの「半分のサイズ」を計算
	half.x = 0.5f * s.x;
	half.y = 0.5f * s.y;
	half.z = 0.5f * s.z;

	// AABBの最小点 (Min) は 中心座標 - 半分のサイズ
	// pObject->boundingBox.Min.x = p.x - half.x; // 古い処理
	boundingBox.Min.x = p.x - half.x; // 新しい処理 (引数で受け取ったAABBを更新)
	boundingBox.Min.y = p.y - half.y;
	boundingBox.Min.z = p.z - half.z;

	// AABBの最大点 (Max) は 中心座標 + 半分のサイズ
	// pObject->boundingBox.Max.x = p.x + half.x; // 古い処理
	boundingBox.Max.x = p.x + half.x; // 新しい処理
	boundingBox.Max.y = p.y + half.y;
	boundingBox.Max.z = p.z + half.z;
}

// ==============================================================================
// AABB同士の衝突判定関数
// ------------------------------------------------------------------------------
// 衝突していれば true を返す
// ==============================================================================
bool CheckAABBCollision(const AABB& a, const AABB& b)
{
	// 重なっていなければ false を返す
	if (a.Max.x < b.Min.x || a.Min.x > b.Max.x) return false;	// X軸での重なりをチェック
	if (a.Max.y < b.Min.y || a.Min.y > b.Max.y) return false;	// Y軸での重なりをチェック
	if (a.Max.z < b.Min.z || a.Min.z > b.Max.z) return false;	// Z軸での重なりをチェック

	// すべての軸で重なっていれば衝突！
	return true;
}

// ==============================================================================
// AABBとAABBの衝突検出と最小移動ベクトルの計算
//	pMovingObject: 動くオブジェクトのAABB (object.boundingBox)
//	pStaticObject: 動かないオブジェクトのAABB
// ==============================================================================
MTV CalculateAABBMTV(const AABB& pMovingObject, const AABB& pStaticObject)
{
	MTV result = { {0.0f, 0.0f, 0.0f}, 0.0f, false };

	// --------------------------------------------------------------------------
	// --- 1. 各軸での重なり範囲を計算 ---
	// --------------------------------------------------------------------------
	// MinとMaxの差分から、両AABBの中心間距離を引くことで重なりを計算する

	// X軸
	float centerDiffX = pStaticObject.Min.x + (pStaticObject.Max.x - pStaticObject.Min.x) / 2.0f
		- (pMovingObject.Min.x + (pMovingObject.Max.x - pMovingObject.Min.x) / 2.0f);

	float halfExtentX = (pStaticObject.Max.x - pStaticObject.Min.x) / 2.0f
		+ (pMovingObject.Max.x - pMovingObject.Min.x) / 2.0f;

	// X軸の重なり (正の値なら重なっている)
	float overlapX = halfExtentX - fabsf(centerDiffX);


	// Y軸
	float centerDiffY = pStaticObject.Min.y + (pStaticObject.Max.y - pStaticObject.Min.y) / 2.0f
		- (pMovingObject.Min.y + (pMovingObject.Max.y - pMovingObject.Min.y) / 2.0f);

	float halfExtentY = (pStaticObject.Max.y - pStaticObject.Min.y) / 2.0f
		+ (pMovingObject.Max.y - pMovingObject.Min.y) / 2.0f;

	float overlapY = halfExtentY - fabsf(centerDiffY);


	// Z軸
	float centerDiffZ = pStaticObject.Min.z + (pStaticObject.Max.z - pStaticObject.Min.z) / 2.0f
		- (pMovingObject.Min.z + (pMovingObject.Max.z - pMovingObject.Min.z) / 2.0f);

	float halfExtentZ = (pStaticObject.Max.z - pStaticObject.Min.z) / 2.0f
		+ (pMovingObject.Max.z - pMovingObject.Min.z) / 2.0f;

	float overlapZ = halfExtentZ - fabsf(centerDiffZ);

	// --------------------------------------------------------------------------
	// --- 2. 衝突判定 ---
	// --------------------------------------------------------------------------
	// 一つでも重なりがなければ衝突していない
	if (overlapX <= 0.0f || overlapY <= 0.0f || overlapZ <= 0.0f)
	{
		result.isColliding = false;
		return result;
	}

	// 衝突あり
	result.isColliding = true;

	// --------------------------------------------------------------------------
	// --- 3. 最小重なり軸の特定 (MTVの計算) ---
	// --------------------------------------------------------------------------
	if (overlapX < overlapY && overlapX < overlapZ)
	{
		// X軸が最小の重なり
		result.overlap = overlapX;

		// 押し戻し方向の決定 (中心点の差分で、+X方向か-X方向かを決める)
		if (centerDiffX > 0)
		{
			// 静的オブジェクトが動的オブジェクトより+X方向にいる -> 動的オブジェクトを-Xに押す
			result.translation = { -overlapX, 0.0f, 0.0f };
		}
		else
		{
			// 静的オブジェクトが動的オブジェクトより-X方向にいる -> 動的オブジェクトを+Xに押す
			result.translation = { overlapX, 0.0f, 0.0f };
		}
	}
	else if (overlapY < overlapZ)
	{
		// Y軸が最小の重なり
		result.overlap = overlapY;

		if (centerDiffY > 0)
		{
			result.translation = { 0.0f, -overlapY, 0.0f };
		}
		else
		{
			result.translation = { 0.0f, overlapY, 0.0f };
		}
	}
	else
	{
		// Z軸が最小の重なり
		result.overlap = overlapZ;

		if (centerDiffZ > 0)
		{
			result.translation = { 0.0f, 0.0f, -overlapZ };
		}
		else
		{
			result.translation = { 0.0f, 0.0f, overlapZ };
		}
	}

	return result;
}