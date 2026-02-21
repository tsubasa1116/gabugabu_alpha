#include "color.h"
#include "hp.h"
#include <cmath> 
#include "gauge.h"

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

//プレイヤー関連変数
static	ID3D11ShaderResourceView* g_Texture[10];

hp HPBar[HPBER_MAX];

// HPバーのスムーズ減少速度
#define DAMAGE_BAR_SPEED (1.5f)		// 赤バーの減少速度
#define DAMAGE_BAR_DELAY (30.0f)	// 赤バーが減り始めるまでの遅延フレーム
#define SIZE_ADJUST	((1.88f *  (SCREEN_WIDTH / 1280.0f)))
#define POS_ADJUST	((86.4f *  (SCREEN_WIDTH / 1280.0f)))

// -------------------------------------------------------------
// 初期化
// -------------------------------------------------------------
void InitializeHP(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, hp* bar, XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 backColor, XMFLOAT4 fillColor)
{
	bar->pos = pos;
	bar->size = size;
	bar->current = size.x;
	bar->target = size.x;
	bar->damageCurrent = size.x; 
	bar->damageDelay = 0.0f;      
	bar->use = true;
	bar->backColor = backColor;
	bar->fillColor = fillColor;
	bar->damageColor = color::red;
	bar->damageTimer = 0.0f;

	// シェイク初期化
	bar->shakeOffset = { 0.0f, 0.0f };
	bar->shakeTimer = 0.0f;
	bar->shakeDuration = 0.0f;
	bar->shakeAmplitude = 0.0f;
	bar->shakeSpeed = 0.0f;
	bar->gaugeIndex = -1;
	bar->shakeTexNum = -1; // シェイク中の代替テクスチャなし

	g_pDevice = pDevice;
	g_pContext = pContext;

	TexMetadata		metadata;
	ScratchImage	image;
	
	LoadFromWICFile(L"asset\\texture\\uiHpGauge_v3.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[0]);
	assert(g_Texture[0]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiEvolveEffect_v1.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[1]);
	assert(g_Texture[1]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiBaseRed_v5.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[2]);
	assert(g_Texture[2]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiBaseBlue_v5.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[3]);
	assert(g_Texture[3]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiBaseYellow_v5.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[4]);
	assert(g_Texture[4]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiBaseGreen_v5.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[5]);
	assert(g_Texture[5]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiBaseCryRed_v5.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[6]);
	assert(g_Texture[6]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiBaseCryBlue_v5.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[7]);
	assert(g_Texture[7]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiBaseCryYellow_v5.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[8]);
	assert(g_Texture[8]);//読み込み失敗時にダイアログを表示

	LoadFromWICFile(L"asset\\texture\\uiBaseCryGreen_v5.png", WIC_FLAGS_NONE, &metadata, image);//テクスチャは変更可
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[9]);
	assert(g_Texture[9]);//読み込み失敗時にダイアログを表示

}

// -------------------------------------------------------------
// 更新
// -------------------------------------------------------------
void UpdateHP(hp* bar)
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

	// ダメージバー更新（少しディレイをかけてから進む）
	if (bar->damageTimer > 0.0f)
	{
		bar->damageTimer -= 1.0f;
		if (bar->damageTimer < 0.0f)
		{
			bar->damageTimer = 0.0f;
		}
	}
	else
	{
		// 遅延が終わったら通常の処理
		if (bar->damageCurrent > bar->current)
		{
			bar->damageCurrent -= DAMAGE_BAR_SPEED;
			if (bar->damageCurrent < bar->current)
			{
				bar->damageCurrent = bar->current;
			}
		}
	}

	// シェイク更新（フレーム単位で減らす）
	if (bar->shakeTimer > 0.0f && bar->shakeDuration > 0.0f)
	{
		// このHPバーに対応するゲージのみシェイクさせる
		if (bar->gaugeIndex >= 0)
		{
			Gauge_SetShakeOffset(bar->gaugeIndex, bar->shakeOffset);
		}

		// 残りフレームを1減らす
		bar->shakeTimer -= 1.0f;
		if (bar->shakeTimer < 0.0f) bar->shakeTimer = 0.0f;

		// 正規化された残り
		float t = bar->shakeTimer / bar->shakeDuration; // 減衰用
		// 経過フレーム数に速度を掛ける
		float elapsed = bar->shakeDuration - bar->shakeTimer;
		float phase = elapsed * bar->shakeSpeed * 0.5f; // 調整
		// X方向をsin、Y方向をcosで振動させる
		float x = sinf(phase) * bar->shakeAmplitude * t;
		float y = cosf(phase * 1.3f) * (bar->shakeAmplitude * 0.5f) * t; // Yは少し小さめ

		bar->shakeOffset = { x, y };
	}
	else
	{
		bar->shakeOffset = { 0.0f, 0.0f };
	}
}


// -------------------------------------------------------------
// 描画
// -------------------------------------------------------------
void DrawHP(const hp* bar, int texNum)
{
	if (!bar->use) return;

	Shader_BeginUI();
	Shader_SetColor(color::white);

	// HP割合
	float ratio = bar->current / bar->size.x;
	ratio = max(0.0f, min(1.0f, ratio));

	// ダメージバー割合
	float damageRatio = bar->damageCurrent / bar->size.x;
	damageRatio = max(0.0f, min(1.0f, damageRatio));

	XMFLOAT2 backSize = { bar->size.x, bar->size.y };

	// 描画位置にシェイクオフセットを加える
	XMFLOAT2 drawPos = { bar->pos.x + bar->shakeOffset.x, bar->pos.y + bar->shakeOffset.y };

	XMFLOAT2 backPos = { drawPos.x - (bar->size.x / 2.0f) + backSize.x / 2.0f, drawPos.y };

	// シェイク中なら代替テクスチャを使う
	int backTexIndex = texNum;
	if (bar->shakeTimer > 0.0f && bar->shakeTexNum >= 0) {
		backTexIndex = bar->shakeTexNum;
	}

	// 背景を描画
	g_pContext->PSSetShaderResources(0, 1, &g_Texture[backTexIndex]);
	DrawSprite(backPos, backSize, bar->backColor);

	float AdjScreenX = SCREEN_ADJUST_X;

	// 赤バー（ダメージ表示）を先に描画
	if (damageRatio > ratio)
	{
		XMFLOAT2 damageUvMin = { 0.0f, 0.0f };
		XMFLOAT2 damageUvMax = { damageRatio, 1.0f };

		XMFLOAT2 damageFillSize   = { bar->size.x * damageRatio * AdjScreenX, bar->size.y };
		XMFLOAT2 damageFillSizeOK = { damageFillSize.x / SIZE_ADJUST, damageFillSize.y };
		XMFLOAT2 damageFillPosOK  = { drawPos.x - (bar->size.x / 2.0f) + damageFillSizeOK.x / 2.0f + POS_ADJUST, drawPos.y };

		g_pContext->PSSetShaderResources(0, 1, &g_Texture[0]);
		Shader_BeginHpber();
		Shader_SetHpber(bar->damageColor, bar->damageColor, 1.0f, 1.0f);  // 赤一色
		DrawSpriteUV(damageFillPosOK, damageFillSizeOK, bar->damageColor, damageUvMin, damageUvMax);
	}

	// 緑バー（現在HP）を上に描画 
	XMFLOAT2 uvMin = { 0.0f, 0.0f };
	XMFLOAT2 uvMax = { ratio, 1.0f };

	XMFLOAT2 fillSize =   { bar->size.x * ratio * AdjScreenX, bar->size.y };
	XMFLOAT2 fillSizeOK = { fillSize.x / SIZE_ADJUST, fillSize.y };
	XMFLOAT2 fillPosOK =  { drawPos.x - (bar->size.x / 2.0f) + fillSizeOK.x / 2.0f + POS_ADJUST, drawPos.y };

	g_pContext->PSSetShaderResources(0, 1, &g_Texture[0]);
	Shader_BeginHpber();
	Shader_SetHpber(color::white, color::yellow, 0.3f, 0.5f);
	DrawSpriteUV(fillPosOK, fillSizeOK, bar->fillColor, uvMin, uvMax);

	Shader_Begin();
}


// -------------------------------------------------------------
// HP設定
// -------------------------------------------------------------
void SetHPValue(hp* bar, int currentHP, int maxHP)
{
    float ratio = (float)currentHP / (float)maxHP;
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    float newTarget = bar->size.x * ratio;
    
    // ダメージを受けた時にタイマーをセット
    if (newTarget < bar->target)
    {
        bar->damageTimer = DAMAGE_BAR_DELAY;  // damageDelay → damageTimer
    }
    
    bar->target = newTarget;
}

// シェイク
void SetHPShake(hp* bar, float amplitude, float duration, float speed, int shakeTexNum)
{
	if (!bar) return;
	if (duration <= 0.0f)
	{
		// 無効ならすぐクリア
		bar->shakeTimer = 0.0f;
		bar->shakeDuration = 0.0f;
		bar->shakeAmplitude = 0.0f;
		bar->shakeSpeed = 0.0f;
		bar->shakeOffset = { 0.0f, 0.0f };
		bar->shakeTexNum = -1;
		return;
	}

	bar->shakeAmplitude = amplitude;
	bar->shakeDuration = duration;
	bar->shakeSpeed = speed;
	bar->shakeTimer = duration; // 残りフレームをdurationでセット
	bar->shakeTexNum = shakeTexNum; // シェイク中に使うテクスチャ（-1で無効）
}

// -------------------------------------------------------------
// 終了
// -------------------------------------------------------------
void FinalizeHP(hp* bar)
{
	bar->use = false;
}

hp* GetHPBar(int HPIndex)
{
	if (HPIndex < 0 || HPIndex >= HPBER_MAX) return nullptr;
	return &HPBar[HPIndex];
}