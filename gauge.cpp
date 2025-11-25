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

static ID3D11ShaderResourceView* g_Texture = NULL;

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

float fireValue = 1;
float waterValue = 1;
float windValue = 1;
float earthValue = 1;
float OuterGaugeValue = 1;

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
	ImGui::Begin("Gauge Debug");
	if (ImGui::Button("1 +1"))
	{
		fireValue += 0.1f;
	}
	else if (ImGui::Button("2 +1"))
	{
		waterValue += 0.1f;
	}
	else if (ImGui::Button("3 +1"))
	{
		windValue += 0.1f;
	}
	else if (ImGui::Button("4 +1"))
	{
		earthValue += 0.1f;
	}

	if (ImGui::Button("out +1"))
	{
		OuterGaugeValue += 0.1f;
	}
	else if (ImGui::Button("out -1"))
	{
		OuterGaugeValue -= 0.1f;
	}
	ImGui::End();
}


//====================================================================================
// 描画
//====================================================================================
void Gauge_Draw(void)
{
	Shader_BeginUI();

	Shader_BeginOutGauge();
	Shader_SetOutGauge(OuterGaugeValue, color::blue);
	DrawSprite({ 100,100 }, { 90,90 }, color::white);

	Shader_BeginGauge();
	Shader_SetGaugeMulti(fireValue, waterValue, windValue, earthValue,
		color::red, color::blue, color::green, color::yellow);

	DrawSprite({ 100,100 }, { 80,80 }, color::white);
}

