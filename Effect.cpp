// Effect.cpp

#include "Effect.h"
#include "sprite.h"
#include "shader.h"
#include "color.h"

#define EFFECT_SPRITE_X		(8)
#define EFFECT_SPRITE_Y		(8)
#define EFFECT_FRAME_MAX	(64)
#define EFFECT_SPEED		(2.5f)
#define EFFECT_TEX_MAX		(16)
#define EFFECT_MAX			(16)

// グローバル変数
// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

static ID3D11ShaderResourceView* g_Texture[EFFECT_TEX_MAX] = {};
static bool g_ReleaseOwned[EFFECT_TEX_MAX] = {};
static int g_CurrentTexNo = 0;

static EFFECT effect[EFFECT_MAX];

static int g_EffectFrame = 0;
static int g_EffectTimer = 0;

static bool g_EffectLoopFlag = false;

//===============================================
//　テクスチャセット用関数
//===============================================
static void Effect_LoadTexture(int i, const wchar_t* num)
{
	assert(i >= 0 && i < EFFECT_TEX_MAX);
	assert(g_pDevice);

	TexMetadata metadata{};
	ScratchImage image{};
	HRESULT hr = LoadFromWICFile(num, WIC_FLAGS_NONE, &metadata, image);
	assert(SUCCEEDED(hr));

	hr = CreateShaderResourceView(g_pDevice,image.GetImages(), image.GetImageCount(),metadata, &g_Texture[i]);
	assert(SUCCEEDED(hr));
	assert(g_Texture[i]);
}

//===============================================
//　初期化
//===============================================
void Effect_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		effect[i].enable = false;
		effect[i].pos = XMFLOAT3(0, 0, 0);
		effect[i].size = XMFLOAT2(0, 0);
		effect[i].frameCnt = 0;
		effect[i].texNo = 0;
	}

	Effect_LoadTexture( 0, L"Asset\\Texture\\uiLightBigGlass_v1.png");			// 第2形態 エフェクト ガラス
	Effect_LoadTexture( 1, L"Asset\\Texture\\uiLightBigConcrete_v1.png");		// 第2形態 エフェクト コンクリート
	Effect_LoadTexture( 2, L"Asset\\Texture\\uiLightBigTree_v1.png");			// 第2形態 エフェクト 植物
	Effect_LoadTexture( 3, L"Asset\\Texture\\uiLightBigElectricity_v1.png");	// 第2形態 エフェクト 電気
	Effect_LoadTexture( 4, L"Asset\\Texture\\uiLightBigGlass_v1.png");			// 第3形態 エフェクト ガラス
	Effect_LoadTexture( 5, L"Asset\\Texture\\uiLightBigConcrete_v1.png");		// 第3形態 エフェクト コンクリート
	Effect_LoadTexture( 6, L"Asset\\Texture\\uiLightBigTree_v1.png");			// 第3形態 エフェクト 植物
	Effect_LoadTexture( 7, L"Asset\\Texture\\uiLightBigElectricity_v1.png");	// 第3形態 エフェクト 電気
	Effect_LoadTexture( 8, L"Asset\\Texture\\effectSkillGlassConcrete_v2.png");	// スキル エフェクト ガラス・コンクリート
	Effect_LoadTexture( 9, L"Asset\\Texture\\effectSkillTree_v2.png");			// スキル エフェクト 植物
	Effect_LoadTexture(10, L"Asset\\Texture\\effectSkillElectricity_v2.png");	// スキル エフェクト 電気
	Effect_LoadTexture(11, L"Asset\\Texture\\effectHit01_v2.png");				// ヒット エフェクト コンクリートの建物・プレイヤーを攻撃した時
	Effect_LoadTexture(12, L"Asset\\Texture\\effectHit02_v2.png");				// ヒット エフェクト 電気・ガラス・植物の建物を攻撃した時
	Effect_LoadTexture(13, L"Asset\\Texture\\effectSmoke_20per.png");			// 建物 煙エフェクト 20%破壊
	Effect_LoadTexture(14, L"Asset\\Texture\\effectSmoke_50per.png");			// 建物 煙エフェクト 50%破壊
	Effect_LoadTexture(15, L"Asset\\Texture\\effectWin_v1.png");				// 撃墜 エフェクト
}

//===============================================
//　終了
//===============================================
void Effect_Finalize()
{
	for (int i = 0; i < EFFECT_MAX; i++)
	{
		if (g_Texture[i] && g_ReleaseOwned[i])
		{
			g_Texture[i]->Release();
		}
		g_Texture[i] = nullptr;
		g_ReleaseOwned[i] = false;
	}
}

//===============================================
//　更新
//===============================================
void Effect_Update()
{
	g_EffectTimer++;

	if (g_EffectTimer >= EFFECT_SPEED)
	{
		g_EffectTimer = 0;
		g_EffectFrame++;

		if (!g_EffectLoopFlag)
		{
			g_EffectTimer++;
			if (g_EffectFrame > 29)
			{
				g_EffectLoopFlag = true;
				g_EffectFrame = 32;
			}
			if (g_EffectFrame >= EFFECT_FRAME_MAX)
			{
				g_EffectFrame = 0;
			}
		}
		else
		{
			// ループ
			g_EffectFrame++;
			if (g_EffectFrame >= 61)
			{
				g_EffectFrame = 32;
			}
		}
	}
	//g_EffectTimer++;

	//if (g_EffectTimer >= EFFECT_SPEED)
	//{
	//	g_EffectTimer = 0;
	//	g_EffectFrame++;

	//	if (!g_EffectLoopFlag)
	//	{
	//		g_EffectTimer++;
	//		if (g_EffectFrame > 29)
	//		{
	//			g_EffectLoopFlag = true;
	//			g_EffectFrame = 32;
	//		}
	//		if (g_EffectFrame >= EFFECT_FRAME_MAX)
	//		{
	//			g_EffectFrame = 0;
	//		}
	//	}
	//	else
	//	{
	//		// ループ
	//		g_EffectFrame++;
	//		if (g_EffectFrame >= 61)
	//		{
	//			g_EffectFrame = 32;
	//		}
	//	}
	//}
}

//===============================================
//　描画
//===============================================
void Effect_Draw()
{
	if (g_CurrentTexNo < 0 || g_CurrentTexNo >= EFFECT_TEX_MAX) return;
	ID3D11ShaderResourceView* tex = g_Texture[g_CurrentTexNo];
	if (!tex) return;

	int fx = g_EffectFrame % EFFECT_SPRITE_X;
	int fy = g_EffectFrame / EFFECT_SPRITE_X;

	float u = 1.0f / EFFECT_SPRITE_X;
	float v = 1.0f / EFFECT_SPRITE_Y;
	
	XMFLOAT2 uvMin = { fx * u, fy * v };
	XMFLOAT2 uvMax = { uvMin.x + u, uvMin.y + v };

	// シェーダーを描画パイプラインに設定
	Shader_Begin();
	Shader_BeginUI();

	SetBlendState(BLENDSTATE_ALPHA);

	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		if (!effect[i].enable) continue;

		int texNo = effect[i].texNo;
		if (texNo < 0 || texNo >= EFFECT_TEX_MAX) continue;
		ID3D11ShaderResourceView* tex = g_Texture[texNo];
		if (!tex) continue;

		XMFLOAT2 pos = { effect[i].pos.x, effect[i].pos.y };
		XMFLOAT2 size = effect[i].size;

		g_pContext->PSSetShaderResources(0, 1, &tex);
		DrawSpriteUV(pos, size, color::white, uvMin, uvMax);
	}
}

//===============================================
//　セット関数
//===============================================
void Effect_Set(int texNo, XMFLOAT2 pos, XMFLOAT2 size)
{
	if (texNo < 0 || texNo >= EFFECT_TEX_MAX) return;
	if (!g_Texture[texNo]) return;
	
	// 空きを探す
	int slot = -1;
	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		if (!effect[i].enable)
		{
			slot = i;
			break;
		}
	}

	if (slot < 0) return;

	effect[slot].enable = true;
	effect[slot].pos = XMFLOAT3(pos.x, pos.y, 0.0f);
	effect[slot].size = size;
	effect[slot].frameCnt = 0;
	effect[slot].texNo = texNo;
}

//===============================================
//　エフェクト消去
//===============================================
void Effect_Clear(int pIndex)
{
	// プレイヤーごとのエフェクト位置
	const XMFLOAT2 playerEffectPos[4] =
	{
		{ 175.0f, 620.0f }, // プレイヤー1
		{ 490.0f, 620.0f },  // プレイヤー2
		{ 805.0f, 620.0f }, // プレイヤー3
		{ 1120.0f, 620.0f }  // プレイヤー4
	};

	if (pIndex < 0 || pIndex >= 4) return;

	XMFLOAT2 targetPos = playerEffectPos[pIndex];

	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		if (!effect[i].enable) continue;

		// 位置が一致するエフェクトを無効化
		if (fabsf(effect[i].pos.x - targetPos.x) < 1.0f &&fabsf(effect[i].pos.y - targetPos.y) < 1.0f)
		{
			effect[i].enable = false;
		}
	}
}