#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "collider.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

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
	//XMFLOAT3 s = scaling;	// オブジェクトのスケール
	//XMFLOAT3 p = position;	// オブジェクトの中心座標
	XMFLOAT3 half;	// 半分サイズ

	// スケールの半分を計算
	half.x = 0.5f * scaling.x;
	half.y = 0.5f * scaling.y;
	half.z = 0.5f * scaling.z;

	// AABBの最小点 (Min) は 中心座標 - 半分のサイズ
	// pObject->boundingBox.Min.x = p.x - half.x; // 古い処理
	boundingBox.Min.x = position.x - half.x; // 新しい処理 (引数で受け取ったAABBを更新)
	boundingBox.Min.y = position.y - half.y;
	boundingBox.Min.z = position.z - half.z;

	// AABBの最大点 (Max) は 中心座標 + 半分のサイズ
	// pObject->boundingBox.Max.x = p.x + half.x; // 古い処理
	boundingBox.Max.x = position.x + half.x; // 新しい処理
	boundingBox.Max.y = position.y + half.y;
	boundingBox.Max.z = position.z + half.z;
}

void FieldCalculateAABB(AABB& boundingBox, const XMFLOAT3& position, const XMFLOAT3& scaling) // 古いフィールドコライダー関数
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
	// --- 各軸での重なり範囲を計算 ---
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
	// --- 衝突判定 ---
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
	// --- 最小重なり軸の特定 (MTVの計算) ---
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
	// 六角柱の底面から上面の間にあるか
	float halfH = hex.height / 2.0f;

	// 六角柱の上面と底面のy座標
	float max_y = hex.center.y + halfH;
	float min_y = hex.center.y - halfH;

	if (
		point.y < min_y ||	// プレイヤーが柱より下
		point.y > max_y)	// プレイヤーが柱より上
	{
		return false; // 高さが合わないなら衝突していない
	}

	// 平面(XZ平面)での六角形判定
	// 六角形は対称なので、絶対値を使って第1象限だけで計算できる

	// 点と六角柱の距離の絶対値をとる
	float dx = fabsf(point.x - hex.center.x);
	float dz = fabsf(point.z - hex.center.z);

	float sin60 = sinf(XMConvertToRadians(60.0f));

	float max_x = hex.radius;		// 六角の最大x座標
	float max_z = max_x * sin60;	// 六角の最大z座標

	// 六角形が内接している長方形の範囲外なら除外
	if (dx > max_x || dz > max_z) return false;

	// 斜めの辺に対して垂直なベクトル：
	//	右斜めの辺とx軸のなす角 120度 - 90度 = 30度
	float nx = cosf(XMConvertToRadians(30.0f));
	//float ny = sinf(XMConvertToRadians(30.0f));
	float ny = 0.5f;	// sin30 は 0.5 になる

	// 法線 n と調べる点 d との内積
	// （法線 n は単位ベクトルだから内積は dcosφ （φ は n と d のなす角）になる）
	//	つまりベクトル d を法線 n に射影した時のベクトルが出てくることになる
	float dn = dx * nx + dz * ny;

	// 六角形の中心から斜めの辺までの距離
	// （アポセム：（多角形の中心）から（多角形の各辺の中点）までの距離）
	//	法線 n の方向にベクトルを伸ばして、斜めの辺に当たるまでの長さのベクトルになる
	// 
	// アポセム：（半径）*（cosθ）より、
	//		float apothem = hex.radius * cosf(XMConvertToRadians(30.0f));
	// 
	// cos30 は sin60 と等しい（cos(90 - 30) = sin60）になる
	// ついでに hex.radius（半径）は max_x と置ける
	// 
	// つまりアポセムは max_x * sin60 = max_z の計算と一致するため、
	//		float apothm = max_z; // と、置ける

	if (dn <= max_z)	// 法線に射影された調べたい点 と apothm の比較
	{
		return true; // 内側！
	}

	// 安全のための最後のチェック
	// 長方形部分のチェック (中心付近)
	//if (dx <= max_x * 0.5f) // 半分の幅以内なら、高さチェックだけでOK（上で通過済み）
	//{
	//	return true;
	//}

	return false;
}

// collider.cpp の末尾に追加

// ==============================================================================
// AABBと六角柱の衝突判定（足場判定用）
// ------------------------------------------------------------------------------
// AABBの底面の四隅のいずれかが六角形に入っていれば true を返す
// これにより「ギリギリ端っこに乗っている」状態を判定できます。
// ==============================================================================
bool CheckAABBHexCollision(const AABB& box, const HexCollider& hex)
{
	//// 1. Y軸（高さ）の判定
	//// 六角形のY範囲を取得
	//float hexMinY = hex.center.y - hex.height / 2.0f;
	//float hexMaxY = hex.center.y + hex.height / 2.0f;

	//// AABBの足元(Min.y)から頭(Max.y)が、六角形の高さ範囲とかすっているか確認
	//// ※少し余裕(+0.5f)を持たせないと、着地した瞬間に判定が外れることがあります
	//if (box.Min.y > hexMaxY + 0.5f) return false;	// プレイヤーが高すぎる
	//if (box.Max.y < hexMinY) return false;			// プレイヤーが低すぎる

	// 2. XZ平面（水平方向）の判定
	// AABBの底面の4つの角の座標を作成
	XMFLOAT3 corners[5];

	corners[0] = XMFLOAT3(box.Min.x, box.Min.y, box.Max.z); // 左奥
	corners[1] = XMFLOAT3(box.Max.x, box.Min.y, box.Max.z); // 右奥
	corners[2] = XMFLOAT3(box.Min.x, box.Min.y, box.Min.z); // 左手前
	corners[3] = XMFLOAT3(box.Max.x, box.Min.y, box.Min.z); // 右手前

	// 中心も念のためチェック（プレイヤーが六角形より巨大な場合用）
	corners[4] = XMFLOAT3(
		(box.Min.x + box.Max.x) * 0.5f, 
		//(box.Min.y + box.Max.y) * 0.5f,	// AABBのy座標の中心
		(box.Min.y),						// AABBの底面のy座標
		(box.Min.z + box.Max.z) * 0.5f);

	// いずれかの点が六角形の中にあれば「乗っている」とみなす
	for (int i = 0; i < 5; i++)
	{
		if (CheckPointHexCollision(corners[i], hex))
		{
			return true;
		}
	}

	return false;
}

// ==============================================================================
// 円とAABBの衝突判定
// ------------------------------------------------------------------------------
// 入力: circle (円の中心座標と半径), box (AABBデータ)
// 出力: true = 衝突している, false = 衝突していない
// ==============================================================================
bool CheckCircleAABBCollision(const Circle& circle, const AABB& box)
{
	// 円の中心とAABBの最も近い点を計算
	float closestX = max(box.Min.x, min(circle.center.x, box.Max.x));
	float closestY = max(box.Min.y, min(circle.center.y, box.Max.y));
	float closestZ = max(box.Min.z, min(circle.center.z, box.Max.z));
	
	// 円の中心と最も近い点との距離を計算
	float dx = circle.center.x - closestX;
	float dy = circle.center.y - closestY;
	float dz = circle.center.z - closestZ;
	
	// 衝突判定: 距離が円の半径以下であれば衝突
	float distanceSquared = dx * dx + dy * dy + dz * dz;
	return distanceSquared <= (circle.radius * circle.radius);
}



// ==============================================================================

//======================================================
//	扇型の当たり判定
//======================================================
bool CheckSectorCollision(const XMFLOAT3& targetPos, const Sector& sector)
{
	// 1. 距離の判定
	XMVECTOR vCenter = XMLoadFloat3(&sector.center);
	XMVECTOR vTarget = XMLoadFloat3(&targetPos);

	XMVECTOR diff = vTarget - vCenter;
	// Y軸（高さ）を無視して平面で判定する場合、ここでdiff.y = 0にすると正確

	float distance;
	XMStoreFloat(&distance, XMVector3Length(diff));

	if (distance > sector.radius) return false;	// 遠すぎるならアウト
	if (distance < 0.0001f) return true;		// ほぼ同じ位置ならセーフ

	// 2. 角度の判定
	// ベクトルを正規化（長さを1にする）
	XMVECTOR vForward = XMLoadFloat3(&sector.forward);
	XMVECTOR vToTarget = XMVector3Normalize(diff);

	// 内積（Dot Product）を使って、2つのベクトルのなす角の「cos」を出す
	float dot;
	XMStoreFloat(&dot, XMVector3Dot(vForward, vToTarget));

	// 扇の端っこの角度（半角）をcosに変換
	// 例：全角90度なら、中心から左右に45度
	float halfAngleCos = cosf(XMConvertToRadians(sector.angleDegree * 0.5f));

	// 内積の結果（cosθ）が、判定基準のcosより大きければ範囲内
	// (cosは角度が小さいほど値が大きくなる)
	if (dot >= halfAngleCos)
	{
		return true;
	}

	return false;
}

bool CheckAABBSectorCollision(const AABB& targetBox, const Sector& sector)
{
	// --- 1. 高さ(Y軸)の判定 ---
	// 扇の高さ範囲を仮に「上下1.0f」とする（攻撃の種類に合わせて調整してね）
	float attackTop = sector.center.y + 2.0f;    // 扇の判定の上端
	float attackBottom = sector.center.y - 1.0f; // 扇の判定の下端（足元）

	if (targetBox.Min.y > attackTop || targetBox.Max.y < attackBottom)
	{
		return false; // 高さが合わないなら当たらない
	}

	// --- 2. XZ平面での5点チェック (4隅 + 中心) ---
	XMFLOAT3 testPoints[5];
	// AABBの底面の4隅
	testPoints[0] = { targetBox.Min.x, sector.center.y, targetBox.Min.z };
	testPoints[1] = { targetBox.Max.x, sector.center.y, targetBox.Min.z };
	testPoints[2] = { targetBox.Min.x, sector.center.y, targetBox.Max.z };
	testPoints[3] = { targetBox.Max.x, sector.center.y, targetBox.Max.z };
	// 中心点
	testPoints[4] = {
		(targetBox.Min.x + targetBox.Max.x) * 0.5f,
		sector.center.y,
		(targetBox.Min.z + targetBox.Max.z) * 0.5f
	};

	// いずれかの点が扇型に入っていれば「当たり！」
	for (int i = 0; i < 5; i++)
	{
		if (CheckSectorCollision(testPoints[i], sector))
		{
			return true;
		}
	}

	return false;
}