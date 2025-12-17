#include "color.h"
#include "hp.h"

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

//プレイヤー関連変数
static	ID3D11ShaderResourceView* g_Texture[6];

// HPバーのスムーズ減少速度
#define HPBAR_SPEED 3.0f


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
	
	LoadFromWICFile(L"asset\\texture\\uiHpGauge_v2.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[0]);
	assert(g_Texture[0]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiEvolveEffect_v1.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[1]);
	assert(g_Texture[1]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiBaseBlue_v2.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[2]);
	assert(g_Texture[2]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiBaseGreen_v2.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[3]);
	assert(g_Texture[3]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiBaseRed_v2.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[4]);
	assert(g_Texture[4]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiBaseYellow_v2.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[5]);
	assert(g_Texture[5]);//読み込み失敗時にダイアログを表示
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
			//bar->pos.x -= HPBAR_SPEED;
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
// 旧バージョnnン：ポリゴン版HPバー
//void DrawHP(const HP* bar)
//{
//	if (!bar->use) return;
//
//	Shader_BeginUI();
//
//	Shader_SetColor(color::white);
//
//	g_pContext->PSSetShaderResources(0, 1, &g_Texture);
//
//	const float border = 3.0f; // 外枠の太さ
//
//	XMFLOAT2 borderPos = bar->pos;
//	XMFLOAT2 borderSize = { bar->size.x + border * 2, bar->size.y + border * 2 };
//
//	DrawSprite(borderPos, borderSize, color::black);
//    
//	// 残量（current分だけ横幅を縮める）
//	XMFLOAT2 fillPos = {
//	 bar->pos.x - (bar->size.x / 2.0f) + (bar->current / 2.0f),
//	 bar->pos.y
//	};
//    DrawSprite(bar->pos, bar->size, bar->backColor);
//	
//	// 背景（固定幅）
//	XMFLOAT2 fillSize = { bar->current, bar->size.y };
//	DrawSprite(fillPos, fillSize, bar->fillColor);
//
//}

void DrawHP(const HP* bar, int texNum)
{
	if (!bar->use) return;

	Shader_BeginUI();
	Shader_SetColor(color::white);

	// HP割合
	float ratio = bar->current / bar->size.x;
	ratio = max(0.0f, min(1.0f, ratio));

	

	// 画像バー本体（横に削る）
	XMFLOAT2 uvMin = { 0.0f, 0.0f };
	XMFLOAT2 uvMax = { ratio, 1.0f };
    
	XMFLOAT2 backSize = { bar->size.x, bar->size.y };
	XMFLOAT2 backPos = { bar->pos.x - (bar->size.x / 2.0f) + backSize.x / 2.0f, bar->pos.y
	};
	
	XMFLOAT2 fillSize = { bar->size.x * ratio, bar->size.y };
	XMFLOAT2 fillPos = { bar->pos.x - (bar->size.x / 2.0f) + (fillSize.x / 2.0f), bar->pos.y
	};

	// サイズ・位置調整（ごり押し）
	XMFLOAT2 fillSizeOK = { fillSize .x / 1.88f, fillSize .y / 2.0f};
	XMFLOAT2 fillPosOK = {bar->pos.x - (bar->size.x / 2.0f) + fillSizeOK.x / 2.0f + 97.0f, bar->pos.y + 33.0f};
	
	g_pContext->PSSetShaderResources(0, 1, &g_Texture[texNum]);
	DrawSprite(backPos, backSize, bar->backColor);
	g_pContext->PSSetShaderResources(0, 1, &g_Texture[0]);
	Shader_BeginHpber();
	Shader_SetHpber(color::green, color::black, { 10.0f, 15.0f }, 0.3f, 20.0f);
	DrawSpriteUV(fillPosOK, fillSizeOK, bar->fillColor, uvMin, uvMax);
	Shader_Begin();

	/*g_pContext->PSSetShaderResources(0, 1, &g_Texture[1]);
	DrawSprite({ 74, 647 }, {110, 110}, XMFLOAT4(1, 1, 1, 0.6));*/
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


