//======================================================
//	title.cpp
// 
//	制作者：田中佑奈			日付：2026/02/26
//======================================================
#include "Manager.h"
#include "sprite.h"
#include "keyboard.h"
#include "Title.h"
#include "fade.h"
#include "swipe.h"
#include "transition.h"
#include "shader.h"
#include <chrono>
#include <cmath>
#include "VideoTexture.h"
#include "color.h"
#include "input.h"

static VideoTexture g_VideoTex;

static	ID3D11ShaderResourceView* g_Texture = NULL;		// 背景
static	ID3D11ShaderResourceView* g_Texture2 = NULL;	// ゲームロゴ
static	ID3D11ShaderResourceView* g_Texture3 = NULL;	// はじめるボタン
static	ID3D11ShaderResourceView* g_Texture4 = NULL;	// はじめるボタン
static	ID3D11ShaderResourceView* g_Texture5 = NULL;	// はじめるボタン
static	ID3D11ShaderResourceView* g_Texture6 = NULL;	// はじめるボタン
static	ID3D11ShaderResourceView* g_Texture7 = NULL;	// はじめるボタン
static	DirectX::TexMetadata g_Metadata3{};
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// アニメーション用タイマー
static std::chrono::steady_clock::time_point g_TitleLastTime;
static float g_TitleElapsed = 0.0f;

// ポップイン設定（調整可）
static constexpr float LOGO_POP_DURATION = 0.8f;      // ロゴのポップ所要時間（秒）
static constexpr float BUTTON_POP_DURATION = 0.8f;  // ボタンのポップ所要時間（秒）
static constexpr float BUTTON_DELAY_AFTER_LOGO = 0.3f; // ロゴ完了からボタン開始までの遅延（秒）
static constexpr float ICON_POP_DURATION = 0.6f;       // アイコンのポップイン所要時間（秒）
static constexpr float ICON_DELAY_AFTER_LOGO = 0.2f;   // ロゴ完了からアイコン開始までの遅延（秒）

// イージング（サインのイーズアウト）
static inline float EaseOutSine(float t) {
	if (t <= 0.0f) return 0.0f;
	if (t >= 1.0f) return 1.0f;
	return sinf(t * 3.14159265f * 0.5f);
}

void Title_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 時間初期化
	g_TitleLastTime = std::chrono::steady_clock::now();
	g_TitleElapsed = 0.0f;

	//背景
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(L"asset\\texture\\uiStart_v3.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
		assert(g_Texture);//読み込み失敗時にダイアログを表示
	}

	// ロゴ
	{
		TexMetadata		metadata2;
		ScratchImage	image2;
		LoadFromWICFile(L"asset\\texture\\TitleLogoBlur.png", WIC_FLAGS_NONE, &metadata2, image2);
		CreateShaderResourceView(pDevice, image2.GetImages(), image2.GetImageCount(), metadata2, &g_Texture2);
		assert(g_Texture2);//読み込み失敗時にダイアログを表示
	}

	// はじめるボタン
	{
		TexMetadata		metadata3;
		ScratchImage	image3;
		LoadFromWICFile(L"asset\\texture\\startON.png", WIC_FLAGS_NONE, &metadata3, image3);
		CreateShaderResourceView(pDevice, image3.GetImages(), image3.GetImageCount(), metadata3, &g_Texture3);
		assert(g_Texture3);//読み込み失敗時にダイアログを表示

		// 実ピクセルサイズを保持（描画時にアスペクト比を保つため）
		g_Metadata3 = metadata3;
	}

	// 電気ボタン
	{
		TexMetadata		metadata4;
		ScratchImage	image4;
		LoadFromWICFile(L"asset\\texture\\light.png", WIC_FLAGS_NONE, &metadata4, image4);
		CreateShaderResourceView(pDevice, image4.GetImages(), image4.GetImageCount(), metadata4, &g_Texture4);
		assert(g_Texture4);//読み込み失敗時にダイアログを表示
	}

	// 植物ボタン
	{
		TexMetadata		metadata5;
		ScratchImage	image5;
		LoadFromWICFile(L"asset\\texture\\tree.png", WIC_FLAGS_NONE, &metadata5, image5);
		CreateShaderResourceView(pDevice, image5.GetImages(), image5.GetImageCount(), metadata5, &g_Texture5);
		assert(g_Texture5);//読み込み失敗時にダイアログを表示
	}

	// 植物ボタン
	{
		TexMetadata		metadata6;
		ScratchImage	image6;
		LoadFromWICFile(L"asset\\texture\\concreat.png", WIC_FLAGS_NONE, &metadata6, image6);
		CreateShaderResourceView(pDevice, image6.GetImages(), image6.GetImageCount(), metadata6, &g_Texture6);
		assert(g_Texture6);//読み込み失敗時にダイアログを表示
	}

	// 植物ボタン
	{
		TexMetadata		metadata7;
		ScratchImage	image7;
		LoadFromWICFile(L"asset\\texture\\ice.png", WIC_FLAGS_NONE, &metadata7, image7);
		CreateShaderResourceView(pDevice, image7.GetImages(), image7.GetImageCount(), metadata7, &g_Texture7);
		assert(g_Texture7);//読み込み失敗時にダイアログを表示
	}

	//フェードインのセット（初期入力はここで無視しても良い）
	if (Keyboard_IsKeyDown(KK_SPACE))
	{
		XMFLOAT4	color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		SetSwipe(60.0f, color, SWIPE_IN, SCENE_SOUND);
	}
	if (Keyboard_IsKeyDown(KK_ENTER))
	{
		XMFLOAT4	color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		SetFade(60.0f, color, FADE_IN, SCENE_START);
	}

}

void Title_Finalize()
{
	//テクスチャの解放など
	SAFE_RELEASE(g_Texture);
	SAFE_RELEASE(g_Texture2);
	SAFE_RELEASE(g_Texture3);
}
void Title_Update()
{

	// 時間差分更新
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsed = now - g_TitleLastTime;
	float dt = elapsed.count();
	g_TitleLastTime = now;
	g_TitleElapsed += dt;

	// キー入力チェック（フェード中は受け付けない）
	if ((Keyboard_IsKeyDownTrigger(KK_SPACE) || g_Input->Plus) && (GetSwipeState() == SWIPE_NONE))
	{
		XMFLOAT4	color(0.0f, 0.0f, 0.0f, 1.0f);
		SetSwipe(40.0f, color, SWIPE_OUT, SCENE_SOUND);
	}
	if ((Keyboard_IsKeyDownTrigger(KK_ENTER) || g_Input->A) && (GetFadeState() == FADE_NONE))
	{
		XMFLOAT4	color(0.0f, 0.0f, 0.0f, 1.0f);
		SetTransition(40.0f, color, TRANSITION_OUT, SCENE_START);
	}
}

void Title_Draw()
{
	// シェーダーを描画パイプラインに設定
	Shader_Begin();
	Shader_SetColor(color::white);

	// 頂点シェーダーに変換行列を設定（UI用：直交投影）
	Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
		0.0f,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		0.0f,
		0.0f,
		1.0f));

	// 背景描画
	if (g_Texture)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture);
		SetBlendState(BLENDSTATE_NONE);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
		XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };
		DrawSprite(pos, size, col);
	}

	// 属性アイコン（ロゴの後に左右からスライドイン）
	{
		// テクスチャ・スライド方向の定義
		// g_Texture4(電気), g_Texture6(コンクリ) → 左から
		// g_Texture5(植物), g_Texture7(ガラス)   → 右から
		ID3D11ShaderResourceView* iconTextures[4] = { g_Texture4, g_Texture5, g_Texture6, g_Texture7 };
		bool fromLeft[4] = { true, false, true, false };

		float iconSize = SCREEN_HEIGHT * 0.15f; // アイコンサイズ

		// 各アイコンの最終位置（左右の画面端ぴったり）
		float halfIcon = iconSize * 10.0f / 2.0f; // 描画サイズの半分
		XMFLOAT2 iconEndPos[4] = {
			{ halfIcon,                  SCREEN_HEIGHT * 0.55f },  // 電気：左端
			{ SCREEN_WIDTH - halfIcon,   SCREEN_HEIGHT * 0.55f },  // 植物：右端
			{ halfIcon,                  SCREEN_HEIGHT * 0.45f },  // コンクリ：左端
			{ SCREEN_WIDTH - halfIcon,   SCREEN_HEIGHT * 0.45f },  // ガラス：右端
		};

		float iconStart = LOGO_POP_DURATION + ICON_DELAY_AFTER_LOGO;

		for (int i = 0; i < 4; i++)
		{
			if (!iconTextures[i]) continue;

			float tIcon = (g_TitleElapsed - iconStart) / ICON_POP_DURATION;
			if (tIcon < 0.0f) tIcon = 0.0f;
			if (tIcon > 1.0f) tIcon = 1.0f;
			float eIcon = EaseOutSine(tIcon);

			// スライド開始位置（左からなら画面左外、右からなら画面右外）
			float startX = fromLeft[i] ? -iconSize : SCREEN_WIDTH + iconSize;
			float endX = iconEndPos[i].x;
			float posX = startX + (endX - startX) * eIcon;
			float posY = iconEndPos[i].y;
			float iconAlpha = eIcon;

			g_pContext->PSSetShaderResources(0, 1, &iconTextures[i]);
			SetBlendState(BLENDSTATE_ALPHA);
			XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, iconAlpha };
			XMFLOAT2 pos = { posX, posY };
			XMFLOAT2 size = { iconSize * 10.0f, iconSize * 10.0f };
			DrawSprite(pos, size, col);
		}

		SetBlendState(BLENDSTATE_NONE);
	}

	// ロゴ（ポップイン）
	if (g_Texture2)
	{
		// ロゴアニメ進行
		float tLogo = g_TitleElapsed / LOGO_POP_DURATION;
		if (tLogo > 1.0f) tLogo = 1.0f;
		float eLogo = EaseOutSine(tLogo); // 0..1
		// 少し縮小スタートからポップ
		float logoScale = 0.6f + 0.4f * eLogo;
		float logoAlpha = eLogo;

		g_pContext->PSSetShaderResources(0, 1, &g_Texture2);
		SetBlendState(BLENDSTATE_ALPHA);

		XMFLOAT2 baseLogoSize = { SCREEN_WIDTH * 0.55f, SCREEN_HEIGHT * 0.55f };
		XMFLOAT2 logoSize = { baseLogoSize.x * logoScale * 1.0f, baseLogoSize.y * logoScale * 1.0f };
		XMFLOAT2 logoPos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT * 0.4f };

		// DrawSprite は中心位置基準を想定しているのでそのまま渡す
		XMFLOAT4 logoCol = { 1.0f, 1.0f, 1.0f, logoAlpha };
		DrawSprite(logoPos, logoSize, logoCol);

		SetBlendState(BLENDSTATE_NONE);
	}

	// はじめるボタン（ロゴの後にポップイン）	if (g_Texture3)
	{
		// ボタンアニメ進行（開始遅延を考慮）
		float buttonStart = LOGO_POP_DURATION + BUTTON_DELAY_AFTER_LOGO;
		float tButton = (g_TitleElapsed - buttonStart) / BUTTON_POP_DURATION;
		if (tButton < 0.0f) tButton = 0.0f;
		if (tButton > 1.0f) tButton = 1.0f;
		float eButton = EaseOutSine(tButton);
		float buttonScale = 0.5f + 0.5f * eButton; // 0.5->1.0
		float buttonAlpha = eButton;

		// 明度設定（1.0f = 元の色、1.5f = 明るめ、0.5f = 暗め）
		float brightness = 1.3f;
		Shader_SetColor(XMFLOAT4(brightness, brightness, brightness, 1.0f));

		// 描画サイズ（アスペクト比を維持）
		float texW = (g_Metadata3.width > 0) ? (float)g_Metadata3.width : 100.0f;
		float texH = (g_Metadata3.height > 0) ? (float)g_Metadata3.height : 50.0f;
		float desiredWidth = SCREEN_WIDTH * 0.40f;
		float scale = desiredWidth / texW;
		XMFLOAT2 baseButtonSize = { texW * scale, texH * scale };
		XMFLOAT2 buttonSize = { baseButtonSize.x * buttonScale * 0.8f, baseButtonSize.y * buttonScale * 0.8f };

		XMFLOAT2 buttonPos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT * 0.8f };
		XMFLOAT4 buttonCol = { 1.0f, 1.0f, 1.0f, buttonAlpha };

		g_pContext->PSSetShaderResources(0, 1, &g_Texture3);
		SetBlendState(BLENDSTATE_ALPHA);
		DrawSprite(buttonPos, buttonSize, buttonCol);
		SetBlendState(BLENDSTATE_NONE);

		// 他の描画に影響しないよう元に戻す
		Shader_SetColor(color::white);
	}

	//// 動画をテクスチャとして描画
	//ID3D11ShaderResourceView* pVideoSRV = g_VideoTex.GetShaderResourceView();
	//if (pVideoSRV)
	//{
	//	g_pContext->PSSetShaderResources(0, 1, &pVideoSRV);
	//	SetBlendState(BLENDSTATE_NONE);
	//	XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	//	XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };
	//	XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
	//	g_VideoTex.SetPlaybackSpeed(1.0f);
	//	DrawSprite(pos, size, col);
	//}
}