#include "Cutin.h"
#include "sprite.h"
#include "shader.h"
#include "direct3d.h"
#include <cmath>
#include "color.h"

// ----------------------------------------------------
// イージング関数（動きの演出）
// ----------------------------------------------------
// スライドイン用（少しオーバーして止まる）
static inline float EaseOutBack(float t)
 {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;
	float t_m1 = t - 1.0f;
	return 1.0f + c3 * powf(t_m1, 3.0f) + c1 * powf(t_m1, 2.0f);
}
// スライドアウト用（少し後ろに引いてから加速して消える）
static inline float EaseInBack(float t) 
{
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;
	return c3 * t * t * t - c1 * t * t;
}

// ----------------------------------------------------
// 状態管理
// ----------------------------------------------------
enum class CutinState 
{
	HIDDEN, // 非表示
	CUTIN,     // 画面右から登場
	PLAY,   // アニメーション再生
	CUTOUT     // 画面右へ退場
};

struct CutinData {
	CutinState state = CutinState::HIDDEN;
	int playerIndex = 0;
	int typeIndex = 0;
	float timer = 0.0f;
	int animFrame = 0;
	float xPos = 0.0f;
};

static CutinData g_Cutin;
static ID3D11ShaderResourceView* g_CutinTex = nullptr; // カットイン用テクスチャ
static ID3D11ShaderResourceView* g_CharTex[4] = { nullptr };

// ----------------------------------------------------
// 関数実装
// ----------------------------------------------------
void Cutin_Initialize()
{
	g_Cutin.state = CutinState::HIDDEN;
	
	TexMetadata metadata;
	ScratchImage image;
	LoadFromWICFile(L"asset\\texture\\uiCut_v2.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(Direct3D_GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &g_CutinTex);

	LoadFromWICFile(L"asset\\texture\\uiCutCharacter_v1.png", WIC_FLAGS_NONE, &metadata, image); // 電気
	CreateShaderResourceView(Direct3D_GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &g_CharTex[0]);

	LoadFromWICFile(L"asset\\texture\\uiCutCharacter1_v1.png", WIC_FLAGS_NONE, &metadata, image); // ガラス
	CreateShaderResourceView(Direct3D_GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &g_CharTex[1]);

	LoadFromWICFile(L"asset\\texture\\uiCutCharacter2_v1.png", WIC_FLAGS_NONE, &metadata, image); // 植物
	CreateShaderResourceView(Direct3D_GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &g_CharTex[2]);

	LoadFromWICFile(L"asset\\texture\\uiCutCharacter3_v1.png", WIC_FLAGS_NONE, &metadata, image); // コンクリ
	CreateShaderResourceView(Direct3D_GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &g_CharTex[3]);
}

void Cutin_Finalize()
{
	if (g_CutinTex) 
	{
		g_CutinTex->Release();
		g_CutinTex = nullptr;
	}
	for (int i = 0; i < 4; ++i) 
	{
		if (g_CharTex[i]) { g_CharTex[i]->Release(); g_CharTex[i] = nullptr; }
	}
}

void Set_Cutin(int playerIndex, int typeIndex)
{
	g_Cutin.state = CutinState::CUTIN;
	g_Cutin.playerIndex = playerIndex;
	g_Cutin.typeIndex = typeIndex;
	g_Cutin.timer = 0.0f;
	g_Cutin.animFrame = 0;

	// 初期位置（画面の右外）
	g_Cutin.xPos = Direct3D_GetBackBufferWidth() + 300.0f;
}

void Cutin_Update()
{
	if (g_Cutin.state == CutinState::HIDDEN) return;

	// タイマー進行（60FPS想定）
	float DELTA_TIME = 1.0f / 60.0f;
	g_Cutin.timer += DELTA_TIME;

	float startX = Direct3D_GetBackBufferWidth() + 100.0f; // 画面外
	float targetX = Direct3D_GetBackBufferWidth()- 100.0f; // 画面右端の定位置

	switch (g_Cutin.state) 
	{
	case CutinState::CUTIN:
		// 0.4秒かけて登場
	{
		float t = g_Cutin.timer / 0.4f;
		if (t >= 1.0f) 
		{
			t = 1.0f;
			g_Cutin.state = CutinState::PLAY;
			g_Cutin.timer = 0.0f;
		}
		g_Cutin.xPos = startX + (targetX - startX) * EaseOutBack(t);
	}
	break;

	case CutinState::PLAY:
		// 16コマのアニメーションを進める
		g_Cutin.animFrame = (int)(g_Cutin.timer / 0.05f);
		if (g_Cutin.animFrame > 15) g_Cutin.animFrame = 15;

		// 0.8秒経ったら退場へ
		if (g_Cutin.timer >= 0.8f)
		{
			g_Cutin.state = CutinState::CUTOUT;
			g_Cutin.timer = 0.0f;
		}
		break;

	case CutinState::CUTOUT:
		// 0.4秒かけて退場
	{
		float t = g_Cutin.timer / 0.4f;
		if (t >= 1.0f) 
		{
			t = 1.0f;
			g_Cutin.state = CutinState::HIDDEN;
		}
		g_Cutin.xPos = targetX + (startX - targetX) * EaseInBack(t);
	}
	break;
	}
}

void Cutin_Draw()
{
	if (g_Cutin.state == CutinState::HIDDEN) return;
	if (!g_CutinTex) return;

	float cy = Direct3D_GetBackBufferHeight() / 7.0f;
	ID3D11DeviceContext* pContext = Direct3D_GetDeviceContext();
	SetBlendState(BLENDSTATE_ALPHA);

	if(g_CutinTex)
	{
		int bno = (g_Cutin.playerIndex * 16) + g_Cutin.animFrame;

		XMFLOAT2 pos = { g_Cutin.xPos, cy };
		XMFLOAT2 size = { 350.0f, 300.0f };

		// UV座標を手動で計算する
		int cols = 8;
		int rows = 8;
		int col = bno % cols;
		int row = bno / cols;

		float u0 = (float)col / (float)cols;
		float v0 = (float)row / (float)rows;
		float u1 = u0 + 1.0f / (float)cols;
		float v1 = v0 + 1.0f / (float)rows;

		// ★ここで確実に内側を切り抜く！（0.005f など少し大きめの値にする）
		float margin = 0.005f;
		XMFLOAT2 uvMin = { u0 + margin, v0 + margin };
		XMFLOAT2 uvMax = { u1 - margin, v1 - margin };

		ID3D11DeviceContext* pContext = Direct3D_GetDeviceContext();
		pContext->PSSetShaderResources(0, 1, &g_CutinTex);

		// 自分で作った DrawSpriteUV を使う！
		DrawSpriteUV(pos, size, color::white, uvMin, uvMax);
	}

	if (g_Cutin.typeIndex >= 0 && g_Cutin.typeIndex < 4) 
	{
		ID3D11ShaderResourceView* charTex = g_CharTex[g_Cutin.typeIndex];
		if (charTex)
		{
			pContext->PSSetShaderResources(0, 1, &charTex);

			XMFLOAT2 charPos = { g_Cutin.xPos, cy - 13.0f };
			XMFLOAT2 charSize = { 380.0f, 380.0f }; // キャラ画像のサイズに合わせて調整

			DrawSprite(charPos, charSize, color::white); // 普通の1枚絵として描画
		}
	}
}