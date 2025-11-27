#include "color.h"
#include "hp.h"

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

//プレイヤー関連変数
static	ID3D11ShaderResourceView* g_Texture;

// HPバーのスムーズ減少速度
#define HPBAR_SPEED 1.0f


// -------------------------------------------------------------
// 初期化
// -------------------------------------------------------------
void InitializeHP(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HP* bar, XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 backColor, XMFLOAT4 fillColor)
{
	bar->pos = pos;
	bar->size = size;
	bar->current = size.x;
	bar->target = size.x;
	bar->use = true;
	bar->backColor = backColor;
	bar->fillColor = fillColor;

	g_pDevice = pDevice;
	g_pContext = pContext;

	TexMetadata		metadata;
	ScratchImage	image;

	LoadFromWICFile(L"asset\\texture\\poly.jpg", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
	assert(g_Texture);//読み込み失敗時にダイアログを表示
}


// -------------------------------------------------------------
// 更新
// -------------------------------------------------------------
void UpdateHP(HP* bar)
{
	if (!bar->use) return;

	// currentをtargetに近づける
	if (bar->current > bar->target)
	{
		bar->current -= HPBAR_SPEED;

		if (bar->current < bar->target)
		{
			bar->current = bar->target;
			bar->pos.x -= HPBAR_SPEED;
		}
	}
	else if (bar->current < bar->target)
	{
		bar->current += HPBAR_SPEED;

		if (bar->current > bar->target)
		{
			bar->current = bar->target;
		}
	}
}


// -------------------------------------------------------------
// 描画
// -------------------------------------------------------------
void DrawHP(const HP* bar)
{
	if (!bar->use) return;

	Shader_BeginUI();

	Shader_SetColor(color::white);

	g_pContext->PSSetShaderResources(0, 1, &g_Texture);
    
	// 残量（current分だけ横幅を縮める）
	XMFLOAT2 fillPos = {
	 bar->pos.x - (bar->size.x / 2.0f) + (bar->current / 2.0f),
	 bar->pos.y
	};
    DrawSprite(bar->pos, bar->size, bar->backColor);
	
	// 背景（固定幅）
	XMFLOAT2 fillSize = { bar->current, bar->size.y };
	DrawSprite(fillPos, fillSize, bar->fillColor);

	
	

}


// -------------------------------------------------------------
// HP設定
// -------------------------------------------------------------
void SetHPValue(HP* bar, int currentHP, int maxHP)
{
	float ratio = (float)currentHP / (float)maxHP;
	if (ratio < 0.0f) ratio = 0.0f;
	if (ratio > 1.0f) ratio = 1.0f;

	bar->target = bar->size.x * ratio;
}



// -------------------------------------------------------------
// 終了
// -------------------------------------------------------------
void FinalizeHP(HP* bar)
{
	bar->use = false;
}


