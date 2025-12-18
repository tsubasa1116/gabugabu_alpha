/*==============================================================================

   ポリゴン描画 [gauge.cpp]
--------------------------------------------------------------------------------

==============================================================================*/
#include "gauge.h"
#include "keyboard.h"
#include "imgui.h"
#include "sprite.h"
#include "color.h"
#include "shader.h"

static GaugeData g_Gauge[GAUGE_PLAYER_MAX];

static ID3D11ShaderResourceView* g_Texture = NULL;

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;


//====================================================================================
// 初期化
//====================================================================================
void Gauge_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// デバイスとデバイスコンテキストのチェック
	if (!pDevice || !pContext) {
		hal::dout << "Gauge_Initialize() : 与えられたデバイスかコンテキストが不正です" << std::endl;
		return;
	}

	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	for (int i = 0; i < GAUGE_PLAYER_MAX; i++)
	{
		g_Gauge[i].fire  = 1;
		g_Gauge[i].water = 1;
		g_Gauge[i].wind  = 1;
		g_Gauge[i].earth = 1;
		g_Gauge[i].outer = 1;
		g_Gauge[i].pos   = { 0,0 };
	}

	TexMetadata		metadata;
	ScratchImage	image;
	LoadFromWICFile(L"asset\\texture\\uiEvolveEffect_v1.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
	assert(g_Texture);//読み込み失敗時にダイアログを表示

}


//====================================================================================
// 終了
//====================================================================================
void Gauge_Finalize(void)
{

}


//====================================================================================
// 更新
//====================================================================================
void Gauge_Update(void)
{

}


//====================================================================================
// 他のファイルでゲージをセットする関数
//====================================================================================
void Gauge_Set(int i, float Glass, float Plant, float Concrete, float Electricity,
	           float outer, const XMFLOAT2& pos)
{
	if (i < 0 || i >= GAUGE_PLAYER_MAX) return;

	g_Gauge[i].fire  = Glass;
	g_Gauge[i].water = Plant;
	g_Gauge[i].wind  = Concrete;
	g_Gauge[i].earth = Electricity;
	g_Gauge[i].outer = outer;
	g_Gauge[i].pos   = pos;
}


//====================================================================================
// 描画
//====================================================================================
void Gauge_Draw(int i)
{
	const GaugeData& g = g_Gauge[i];

	Shader_BeginUI();

	Shader_BeginOutGauge();
	Shader_SetOutGauge(g.outer, color::red);
	DrawSprite(g.pos, { 85,85 }, color::white);

	Shader_BeginGauge();
	Shader_SetGaugeMulti(g.fire, g.water, g.wind, g.earth,
		                 color::sky, color::green, color::gray, color::yellow);

	DrawSprite(g.pos, { 75,75 }, color::white);

	/*SetBlendState(BLENDSTATE_ALFA);
	g_pContext->PSSetShaderResources(0, 1, &g_Texture);
	DrawSprite({ 10, 10 }, { 10, 10 }, color::white);*/
}

