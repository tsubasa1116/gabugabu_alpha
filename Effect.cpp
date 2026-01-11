
//Effect.cpp

#include "Effect.h"
#include "sprite.h"
#include "shader.h"
#include "color.h"

#define EFFECT_SPLIT_X 8
#define EFFECT_SPLIT_Y 8
#define EFFECT_FRAME_MAX 64
#define EFFECT_SPEED 2
#define EFFECT_TEX_MAX 4

//グローバル変数
// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

static ID3D11ShaderResourceView* g_Texture[EFFECT_TEX_MAX] = {};
static bool g_ReleaseOwned[EFFECT_TEX_MAX] = {};
static int g_CurrentTexNo = 0;

static int g_EffectFrame = 0;
static int g_EffectTimer = 0;

static XMFLOAT2 g_EffectPos   = { 0.0f, 0.0f };
static XMFLOAT2 g_EffectSize  = { 0.0f, 0.0f };

static void Effect_LoadTexture(int i, const wchar_t* path)
{
	assert(i >= 0 && i < EFFECT_TEX_MAX);
	assert(g_pDevice);

	TexMetadata metadata{};
	ScratchImage image{};
	HRESULT hr = LoadFromWICFile(path, WIC_FLAGS_NONE, &metadata, image);
	assert(SUCCEEDED(hr));

	hr = CreateShaderResourceView(g_pDevice,
		image.GetImages(), image.GetImageCount(),
		metadata, &g_Texture[i]);
	assert(SUCCEEDED(hr));
	assert(g_Texture[i]);
}

//メイン処理関数
void Effect_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	Effect_LoadTexture(0, L"Asset\\Texture\\uiLightBigGlass_v1.png");
	Effect_LoadTexture(1, L"Asset\\Texture\\uiLightBigConcrete_v1.png");
	Effect_LoadTexture(2, L"Asset\\Texture\\uiLightBigTree_v1.png");
	Effect_LoadTexture(3, L"Asset\\Texture\\uiLightBigElectricity_v1.png");
}

void Effect_Finalize()
{
	for (int i = 0; i < EFFECT_TEX_MAX; i++)
	{
		if (g_Texture[i] && g_ReleaseOwned[i])
		{
			g_Texture[i]->Release();
		}
		g_Texture[i] = nullptr;
		g_ReleaseOwned[i] = false;
	}
}

void Effect_Update()
{
	g_EffectTimer++;
	if (g_EffectTimer >= EFFECT_SPEED)
	{
		g_EffectTimer = 0;
		g_EffectFrame++;

		if (g_EffectFrame >= 58)
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
	if (g_CurrentTexNo < 0 || g_CurrentTexNo >= EFFECT_TEX_MAX) return;
	ID3D11ShaderResourceView* tex = g_Texture[g_CurrentTexNo];
	if (!tex) return;

	// シェーダーを描画パイプラインに設定
	Shader_Begin();
	Shader_BeginUI();

	int fx = g_EffectFrame % EFFECT_SPLIT_X;
	int fy = g_EffectFrame / EFFECT_SPLIT_X;

	float u = 1.0f / EFFECT_SPLIT_X;
	float v = 1.0f / EFFECT_SPLIT_Y;
	
	XMFLOAT2 uvMin = { fx * u, fy * v };
	XMFLOAT2 uvMax = { uvMin.x + u, uvMin.y + v };

	g_pContext->PSSetShaderResources(0, 1, &tex);
	SetBlendState(BLENDSTATE_ALPHA);
	DrawSpriteUV(g_EffectPos, g_EffectSize, color::white, uvMin, uvMax);
}

void Effect_Set(int texNo, XMFLOAT2 pos, XMFLOAT2 size)
{
	if (texNo < 0 || texNo >= EFFECT_TEX_MAX) return;
	if (!g_Texture[texNo]) return;

	g_CurrentTexNo = texNo;
	g_EffectPos = pos;
	g_EffectSize = size;
}
