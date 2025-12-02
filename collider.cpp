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
// 一番シンプル
// マイクラの当たり判定
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

void FieldCalculateAABB(AABB& boundingBox, const XMFLOAT3& position, const XMFLOAT3& scaling) // 新しい関数
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



// ==============================================================================
// 点と六角柱の衝突判定
// ------------------------------------------------------------------------------
// 入力: point (判定したい点の座標), hex (六角柱データ)
// 出力: true = 中に入っている, false = 入っていない
// ==============================================================================
bool CheckPointHexCollision(const XMFLOAT3& point, const HexCollider& hex)
{
	// 1. 高さ(Y軸)の判定
	// 六角柱の底面から上面の間にあるか？
	// 中心座標を基準に、上下に height/2 の範囲でチェックします
	float halfHeight = hex.height / 2.0f;
	if (point.y < hex.center.y - halfHeight || point.y > hex.center.y + halfHeight)
	{
		return false; // 高さが合わないなら衝突していない
	}

	// 2. 平面(XZ平面)での六角形判定
	// ポイント：六角形は対称なので、絶対値を使って第1象限だけで計算できます。

	// 中心からの距離の絶対値をとる
	float dx = fabsf(point.x - hex.center.x);
	float dz = fabsf(point.z - hex.center.z);
	float r = hex.radius;

	// A. 外接矩形（バウンディングボックス）による簡易チェック
	// 横幅は半径、縦幅は 半径 * sin(60°) = r * √3 / 2 (約0.866)
	// ※六角形の向きによって dx と dz の条件は入れ替わりますが、
	//   field.cppの配置を見ると、横に平らなタイプか頂点が上なタイプかで微調整が必要です。
	//   ここでは一般的な「頂点が上下にある」タイプ（Pointy Topped）を想定します。
	//   もし「頂点が左右にある」タイプなら、dxとdzの計算を入れ替えてください。

	// Pointy Topped (頂点がZ軸方向) の場合:
	// 幅: r * sqrt(3)/2, 高さ: r
	// Flat Topped (頂点がX軸方向) の場合:
	// 幅: r, 高さ: r * sqrt(3)/2

	// field.cpp では X間隔が広く、Z間隔が sin60 なので、
	// ここでは「Flat Topped（頂点がX軸方向）」として計算します。

	float flat_height = r * 0.866025f; // r * sin(60)

	// まず、明らかに範囲外なら除外
	if (dx > r || dz > flat_height) return false;

	// B. 角のカット判定（斜めの辺の内側か？）
	// 六角形の斜めの辺の方程式を利用します。
	// Flat Toppedの場合の条件式: dz <= flat_height - dx * tan(30°)
	// tan(30°) = 0.57735...

	// もっと単純な内積を使った判定式：
	// 「長方形部分」と「三角形部分」の組み合わせで判定
	// 式： dx * 0.5 + dz * 0.866 <= r * 0.866

	// Flat Topped (横長) 用の判定式:
	// dx * 0.5f + dz * 0.866025f <= r * 0.866025f
	// でもこれだと分かりにくいので、もっと直感的な方法：

	// 「X軸方向の距離」 + 「Z軸方向の距離の拡大版」が半径を超えていないか
	if (dx / 2.0f + dz * 0.866025f <= flat_height)
	{
		return true; // 内側！
	}

	// 長方形部分のチェック (中心付近)
	if (dx <= r * 0.5f) // 半分の幅以内なら、高さチェックだけでOK（上で通過済み）
	{
		return true;
	}

	return false;
}