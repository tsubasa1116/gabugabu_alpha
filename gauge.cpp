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

#define GAUGE_IN_SIZE   (XMFLOAT2(55.0f * SCREEN_ADJUST_X, 55.0f * SCREEN_ADJUST_Y))
#define GAUGE_OUT_SIZE  (XMFLOAT2(62.0f * SCREEN_ADJUST_X, 62.0f * SCREEN_ADJUST_Y))
#define SKILL_COOL_SIZE (XMFLOAT2(45.0f * SCREEN_ADJUST_X, 45.0f * SCREEN_ADJUST_Y))
#define SKILL_SIZE      (XMFLOAT2(75.0f * SCREEN_ADJUST_X, 75.0f * SCREEN_ADJUST_Y))
#define SKILL_TEXT_SIZE (XMFLOAT2(50.0f * SCREEN_ADJUST_X, 15.0f * SCREEN_ADJUST_Y))

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
		g_Gauge[i].skill = 1;
		g_Gauge[i].pos   = { 0,0 };
		g_Gauge[i].shakeOffset = { 0.0f, 0.0f };  // シェイクオフセット初期化
		g_Gauge[i].type = PlayerType::None;
	}


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
void Gauge_Set(int i, float Glass, float Plant, float Concrete, float Electricity, float outer, float skill, const XMFLOAT2& pos, PlayerType type)
{
	if (i < 0 || i >= GAUGE_PLAYER_MAX) return;

	g_Gauge[i].fire  = Glass;
	g_Gauge[i].water = Plant;
	g_Gauge[i].wind  = Concrete;
	g_Gauge[i].earth = Electricity;
	g_Gauge[i].outer = outer;
	g_Gauge[i].skill = skill;
	g_Gauge[i].pos   = pos;
	g_Gauge[i].type = type;
}

//====================================================================================
// 通常ゲージ描画（内ゲージ＋外ゲージ）
//====================================================================================
void Gauge_DrawBasic(int i)
{
	const GaugeData& g = g_Gauge[i];

	// シェイクオフセットを適用した描画位置
	XMFLOAT2 drawPos = { g.pos.x + g.shakeOffset.x, g.pos.y + g.shakeOffset.y };

	// UI用シェーダー設定
	Shader_BeginUI();

	// 内ゲージ描画
	Shader_BeginGauge();
	Shader_SetGaugeMulti(g.fire, g.water, g.wind, g.earth);
	Shader_SetGaugeTextures();
	SetBlendState(BLENDSTATE_ALPHA);

	DrawSprite(drawPos, GAUGE_IN_SIZE, color::white);

	// 外ゲージ描画
	Shader_BeginOutGauge();
	Shader_SetOutGauge(g.outer, color::white);
	Shader_SetOutGaugeTextures();
	SetBlendState(BLENDSTATE_ALPHA);

	DrawSprite(drawPos, GAUGE_OUT_SIZE, color::white);
}

//====================================================================================
// スキルゲージ描画（下面／テキスト／上面）
//====================================================================================
void Gauge_DrawSkill(int i)
{
	const GaugeData& g = g_Gauge[i];

	// スキルゲージ描画
	int typeIndex = 0;
	if (g.type != PlayerType::None)
	{
		typeIndex = static_cast<int>(g.type) - 1;  // Noneが0なので-1
	}

	// スキルゲージ(下面)描画
	Shader_Begin();
	Shader_BeginUI();
	Shader_SetSkillCoolGaugeTextures(typeIndex);
	SetBlendState(BLENDSTATE_ALPHA);

	DrawSprite({ g.pos.x + (170 * SCREEN_ADJUST_X), g.pos.y - (45 * SCREEN_ADJUST_Y) }, SKILL_COOL_SIZE, color::white);

	// スキルゲージ(上面)描画
	Shader_BeginSkillGauge();
	Shader_SetSingleGauge(g.skill);
	Shader_SetSkillGaugeTextures(typeIndex);
	SetBlendState(BLENDSTATE_ALPHA);

	DrawSprite({ g.pos.x + (170 * SCREEN_ADJUST_X), g.pos.y - (45 * SCREEN_ADJUST_Y) }, SKILL_SIZE, color::white);

	// スキルテキスト描画
	Shader_Begin();
	Shader_BeginUI();
	Shader_SetSkillTextTextures(typeIndex);

	DrawSprite({ g.pos.x + (170 * SCREEN_ADJUST_X), g.pos.y - (10 * SCREEN_ADJUST_Y) }, SKILL_TEXT_SIZE, color::white);


}

//====================================================================================
// 両方描画
//====================================================================================
void Gauge_Draw(int i)
{
	// 通常ゲージ描画
	Gauge_DrawBasic(i);
	// スキルゲージ描画
	Gauge_DrawSkill(i);
}

//====================================================================================
// シェイクオフセット設定
//====================================================================================
void Gauge_SetShakeOffset(int i, const XMFLOAT2& offset)
{
	if (i < 0 || i >= GAUGE_PLAYER_MAX) return;
	g_Gauge[i].shakeOffset = offset;
}
