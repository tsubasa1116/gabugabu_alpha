// DamageText.cpp
// ダメージ表記管理用

#include "DamageText.h"
#include "makeText.h"
#include "Camera.h"
#include "direct3d.h"

#include <vector>
#include <string>
#include <sstream>
#include <DirectXMath.h>

using namespace DirectX;

struct DamageText
{
	std::wstring text; // 表示する文字列
	XMFLOAT3 worldPos; // ワールド座標
	float screenX;   // スクリーン座標X
	float screenY;   // スクリーン座標Y
	float vy;        // Y方向の速度
	float flame;     // フレーム数
	float size;      // 文字サイズ
	TextColor color; // 文字色
	bool alive;	     // 使用フラグ
};

// ダメージ表記リスト
static std::vector<DamageText> g_DamageList;

bool DamageText_Initialize()
{
	g_DamageList.clear();
	return true;
}

void DamageText_Finalize()
{
	g_DamageList.clear();
}

void SetDamageText(const XMFLOAT3& worldPos, int damage, TextColor color)
{
	DamageText e;
	e.text = std::to_wstring(damage);
	e.worldPos = worldPos;
	e.vy = 0.05f; // 上にゆっくり移動
	e.flame = 100.0f; // フレーム数で消す
	e.size = 36.0f;
	e.color = color;
	e.alive = true;
	e.screenX = 0.0f;
	e.screenY = 0.0f;
	g_DamageList.emplace_back(std::move(e));
}

static bool WorldToScreen(const XMFLOAT3& wp, float& outX, float& outY)
{
	// View×Projection で変換
	XMMATRIX view = GetViewMatrix();
	XMMATRIX proj = GetProjectionMatrix();
	XMMATRIX vp = XMMatrixMultiply(view, proj);

	XMVECTOR v = XMVectorSet(wp.x, wp.y, wp.z, 1.0f);
	XMVECTOR t = XMVector4Transform(v, vp);

	float tx = XMVectorGetX(t);
	float ty = XMVectorGetY(t);
	float tw = XMVectorGetW(t);

	if (tw == 0.0f) return false;

	// NDCに正規化（/w）
	float ndcX = tx / tw;
	float ndcY = ty / tw;

	// スクリーンへ変換
	float w = (float)Direct3D_GetBackBufferWidth();
	float h = (float)Direct3D_GetBackBufferHeight();

	outX = (ndcX * 0.5f + 0.5f) * w;
	// 反転
	outY = (1.0f - (ndcY * 0.5f + 0.5f)) * h;

	// 画面外判定（Z が負、または NDC が ±1 を大きく外れる場合は非表示）
	if (XMVectorGetZ(t) / tw < 0.0f) return false;
	if (ndcX < -1.1f || ndcX > 1.1f || ndcY < -1.1f || ndcY > 1.1f) return false;

	return true;
}

void DamageText_Update()
{
	// フレーム毎更新（移動・寿命）
	for (size_t i = 0; i < g_DamageList.size(); )
	{
		auto& e = g_DamageList[i];
		// ワールド上で上へ移動
		e.worldPos.y += e.vy;

		// スクリーン位置を計算（表示判定含む）
		float sx, sy;
		if (WorldToScreen(e.worldPos, sx, sy))
		{
			e.screenX = sx;
			e.screenY = sy;
		}
		else
		{
			// 見えないなら画面外へ
			e.screenX = -10000.0f;
			e.screenY = -10000.0f;
		}

		e.flame -= 1.0f;
		if (e.flame <= 0.0f)
		{
			// 削除
			g_DamageList.erase(g_DamageList.begin() + i);
		}
		else
		{
			++i;
		}
	}
}

void DamageText_Draw()
{
	// DrawTextExを使って全件描画
	for (auto& e : g_DamageList)
	{
		// フェードアウト効果（lifeに応じて文字サイズを少し縮小する、または色を変えるなどはここで調整可能）
		float size = e.size * (0.8f + 0.2f * (e.flame / 60.0f)); // life に応じてわずかに変化

		// 画面外のものはスキップ
		if (e.screenX < -1000.0f || e.screenY < -1000.0f) continue;

		DrawTextEx(
			e.text.c_str(),
			e.screenX,
			e.screenY,
			size,
			L"Impact",
			e.color
		);
	}
}