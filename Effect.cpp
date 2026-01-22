
//Effect.cpp

#include "Effect.h"
#include "sprite.h"
#include "shader.h"
#include "color.h"

//グローバル変数
// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

static ID3D11ShaderResourceView* g_Texture;

#define EFFECT_SPLIT_X (8)
#define EFFECT_SPLIT_Y (4)
#define EFFECT_FRAME_MAX (32)
#define EFFECT_SPEED (2)

static int g_EffectFrame = 0;
static int g_EffectTimer = 0;

static XMFLOAT2 g_EffectBasePos   = { 0.0f, 0.0f };
static XMFLOAT2 g_EffectSize  = { 0.0f, 0.0f };
//static int g_EffectCount = 1;
//static float g_EffectSpacingX = 0.0f;

//メイン処理関数
void Effect_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// テクスチャ読み込み
	/*TexMetadata metadata;
	ScratchImage image;
	LoadFromWICFile(L"Asset\\Texture\\uiLightLoopBigConcrete_v1.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
	assert(g_Texture);*/
}

void Effect_Finalize()
{
	if (g_Texture != NULL)
	{
		g_Texture->Release();
	}
}

void Effect_Update()
{
	g_EffectTimer++;
	if (g_EffectTimer >= EFFECT_SPEED)
	{
		g_EffectTimer = 0;
		g_EffectFrame++;

		if (g_EffectFrame >= 30)
		{
			g_EffectFrame = 0;
		}
		if (g_EffectFrame >= EFFECT_FRAME_MAX)
		{
			g_EffectFrame = 0;
		}
	}
}

void Effect_Draw()
{
	// シェーダーを描画パイプラインに設定
	Shader_Begin();
	Shader_BeginUI();

	int fx = g_EffectFrame % EFFECT_SPLIT_X;
	int fy = g_EffectFrame / EFFECT_SPLIT_X;

	float u = 1.0f / EFFECT_SPLIT_X;
	float v = 1.0f / EFFECT_SPLIT_Y;
	
	XMFLOAT2 uvMin = { fx * u, fy * v };
	XMFLOAT2 uvMax = { uvMin.x + u, uvMin.y + v };

	g_pContext->PSSetShaderResources(0, 1, &g_Texture);
	SetBlendState(BLENDSTATE_ALPHA);
	DrawSpriteUV(g_EffectBasePos, g_EffectSize, color::white, uvMin, uvMax);
	//DrawSpriteUV(g_EffectBasePos, g_EffectSize, color::white, uvMin, uvMax);

	//// 複数配置に対応：Xだけずらして描画
	//for (int i = 0; i < g_EffectCount; ++i)
	//{
	//	XMFLOAT2 pos = { g_EffectBasePos.x + i * g_EffectSpacingX, g_EffectBasePos.y };
	//	DrawSpriteUV(pos, g_EffectSize, color::white, uvMin, uvMax);
	//}

}

void Effect_Set(ID3D11ShaderResourceView* tex, XMFLOAT2 pos, XMFLOAT2 size)
{
	g_Texture = tex;
	g_EffectBasePos = pos;
	g_EffectSize = size;

	// 互換性のため単体表示（count=1, spacing=0）を設定
	//g_EffectCount = 1;
	//g_EffectSpacingX = 0.0f;

}

//void Effect_SetMultiple(ID3D11ShaderResourceView* tex, XMFLOAT2 basePos, XMFLOAT2 size, int count, float spacingX)
//{
//	if (count <= 0) count = 1;
//	g_Texture = tex;
//	g_EffectBasePos = basePos;
//	g_EffectSize = size;
//	g_EffectCount = count;
//	g_EffectSpacingX = spacingX;
//}
