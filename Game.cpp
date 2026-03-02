//======================================================
//	Game.cpp[]
// 
//	制作者：前野翼			日付：2024//
//======================================================

#include "Manager.h"
#include "sprite.h"
#include "Game.h"
#include "keyboard.h"
#include "makeText.h"
#include "p.h"
#include "field.h"
#include "Building.h"
#include "Effect.h"
#include "score.h"
#include "Audio.h"
#include "gauge.h"
#include "Polygon.h"
#include "player.h"
#include "Camera.h"
#include "Ball.h"
#include "attack.h"
#include "skill.h"
#include "special.h"
#include "fade.h"
#include "DamageText.h"
#include "direct3d.h"
#include "SkyBall.h"
#include "loadThread.h"
#include "shader.h"
#include "color.h"

//======================================================
//	構造謡宣言
//======================================================
LIGHTOBJECT Light;

//======================================================
//
//======================================================
static int g_BgmID = NULL;
bool input2 = false;

// コマンドが入力されたときに立つフラグ
static bool s_IsKonamiCodeEntered = false;
static bool g_GameInitialized = false;
static bool g_IsFirstFrame = true;
static bool s_GameStarted = false;

static ID3D11DeviceContext* g_pContext = NULL;
static	ID3D11ShaderResourceView* g_Texture[6];

//======================================================
//	
//======================================================
void Game_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	if (g_GameInitialized) return;

	Initialize_MakeText();
	CreateRenderTarget_MakeText();

	Player_Initialize(pDevice, pContext);
	Field_Initialize(pDevice, pContext);
	Effect_Initialize(pDevice, pContext);
	Attack_Initialize(pDevice, pContext);
	Skill_Initialize(pDevice, pContext);
	Special_Initialize(pDevice, pContext);
	Camera_Initialize();
	DamageText_Initialize();
	SkyBall_Initialize(pDevice, pContext);

	g_pContext = pContext;

	Loader::AddTask([pDevice]()
	{
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICFile(L"asset\\texture\\fade.bmp", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[0]);

		LoadFromWICFile(L"asset\\texture\\uiCountdown1_v1.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[1]);

		LoadFromWICFile(L"asset\\texture\\uiCountdown2_v1.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[2]);

		LoadFromWICFile(L"asset\\texture\\uiCountdown3_v1.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[3]);

		LoadFromWICFile(L"asset\\texture\\cityName.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[4]);

		LoadFromWICFile(L"asset\\texture\\uiCountdownGo_v1.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[5]);
	});

	//BallInitialize(pDevice, pContext);
	//P_Initialize(pDevice, pContext);		
	//Score_Initialize(pDevice, pContext);

	//g_BgmID = LoadAudio("asset\\Audio\\BGM_Game_Gengengenkidamon.wav");	// サウンドロード
	//PlayAudio(g_BgmID, true);		// 再生開始（ループあり）
	//PlayAudio(g_BgmID);			// 再生開始（ループなし）
	//PlayAudio(g_BgmID, false);	// 再生開始（ループなし）

	XMFLOAT4 para;

	para = XMFLOAT4(0.7f, 0.7f, 0.9999f, 1.0f);
	Light.SetAmbient(para);
	para = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	Light.SetDiffuse(para);
	para = XMFLOAT4(0.5f, -1.0f, 0.0f, 1.0f);

	float len = sqrtf(para.x * para.x + para.y * para.y + para.z * para.z);
	para.x /= len;
	para.y /= len;
	para.z /= len;
	Light.SetDirection(para);

	Loader::StartTaskLoad();
	g_GameInitialized = true;
	g_IsFirstFrame = true;
}

//======================================================
//
//======================================================
void Game_Finalize()
{
	Field_Finalize();
	Effect_Finalize();
	Player_Finalize();
	Camera_Finalize();
	Attack_Finalize();
	Skill_Finalize();
	Special_Finalize();
	SkyBall_Finalize();
	//Building_Finalize();

	//BallFinalize();
	//P_Finalize();
	//Score_Finalize();

	UnloadAudio(g_BgmID);
	DamageText_Finalize();
	g_GameInitialized = false;
	s_GameStarted = false;
	Loader::Reset();
}

//======================================================
//	更新処理
//======================================================
void Game_Update()
{
	if (g_IsFirstFrame)
	{
		Player_Warmup();
		Effect_Warmup();

		g_BgmID = LoadAudio("asset\\Audio\\BGM_Game_Gengengenkidamon.wav");

		g_IsFirstFrame = false;
	}

	if (GetGamePhase() == PHASE_PLAY && !s_GameStarted)
	{
		PlayAudio(g_BgmID, true);

		// 全プレイヤーの卵を割る
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			PLAYEROBJECT* p = GetPlayer(i);
			if (p && p->active && p->duringRespawn)
			{
				p->duringRespawn = false;// 卵状態を解除
				p->isEggBreaking = true; // 割る
			}
		}
		s_GameStarted = true;
	}

	// ------------------------------------
	// 
	// ------------------------------------
	// コマンドで使用する全てのキーの押下トリガーをチェックし、検出関数に渡す
	if (Keyboard_IsKeyDownTrigger(KK_P))
	{
		if(!s_IsKonamiCodeEntered)	s_IsKonamiCodeEntered = true;
		else						s_IsKonamiCodeEntered = false;
	}
	// ------------------------------------
	// 
	// ------------------------------------
	Player_Update();
	Field_Update();
	Building_UpdateAll();
	Effect_Update();

	// 建物エフェクト更新（1棟ずつ）
	int buildingCount = GetBuildingCount();
	for (int i = 0; i < buildingCount; i++)
	{
		Effect_UpdateForBuilding(i);
		//Effect_UpdateAllBuildings();
	}

	Gauge_Update();
	Gauge_Update();
	Camera_Update();
	SkyBall_Update();
	//BallUpdate();
	//P_Update();
	//Score_Update();
	DamageText_Update();

	//ゲームシーンへ遷移
	if (Keyboard_IsKeyDownTrigger(KK_F1) && (GetFadeState() == FADE_NONE))
	{
		// フェードアウトさせてシーンを切り替える
		XMFLOAT4 color(0.0f, 0.0f, 0.0f, 1.0f);
		SetFade(40.0f, color, FADE_OUT, SCENE_WIN);
	}
}

//======================================================
//
//======================================================
void Game_Draw()
{
	Fade_Draw();

	if (!Loader::IsFinished()) return;

	Light.SetEnable(FALSE);
	Shader_SetLight(Light.Light);
	SkyBall_Draw();
	SetDepthTest(FALSE);
	Camera_Draw();

	Light.SetEnable(TRUE);
	Shader_SetLight(Light.Light);
	SetDepthTest(TRUE);

	Field_Draw(s_IsKonamiCodeEntered);

	// 建物エフェクトの一括描画（3D空間）
	{
		Shader_Begin();
		SetBlendState(BLENDSTATE_ALPHA);
		SetDepthReadOnly();
		//Effect_DrawAllBuildings();

		int buildingCount = GetBuildingCount();
		for (int i = 0; i < buildingCount; i++)
		{
			Effect_DrawForBuilding(i);
		}

		SetDepthTest(TRUE);
	}
	if (GetGamePhase() == PHASE_COUNTDOWN || GetGamePhase() == PHASE_PLAY)
	{
		Player_Draw(s_IsKonamiCodeEntered);
	}

	//2D描画
	Light.SetEnable(FALSE);			// ライティングOFF
	Shader_SetLight(Light.Light);	// ライト構造体をシェーダーへセット
	SetDepthTest(FALSE);

	DamageText_Draw();

	if (GetGamePhase() == PHASE_INTRO)
	{// ミールシティ名札
		float cx = (float)Direct3D_GetBackBufferWidth() / 2.0f;
		float cy = (float)Direct3D_GetBackBufferHeight() / 2.0f;

		XMFLOAT2 pos = { cx - 380.0f, cy - 280.0f };
		XMFLOAT2 size = { 540.0f, 150.0f };

		g_pContext->PSSetShaderResources(0, 1, &g_Texture[4]);
		SetBlendState(BLENDSTATE_ALPHA);
		Shader_BeginUI();
		DrawSprite(pos, size, color::white);

		//DrawTextEx(L"ミールシティ", cx - 620.0f, cy - 340.0f, 70.0f, L"FZゴンタかな", TextColor::Black);
	}

	// フェード処理
	float fadeAlpha = 0.0f;
	float INTRO_TIME = 3.5f; // INTRO_DURATION（Camera.cpp）と同じ数値にする
	float FADE_TIME = 0.5f;

	if (GetGamePhase() == PHASE_INTRO)
	{
		float timer = GetGamePhaseTimer();
		float fadeStartTime = INTRO_TIME - FADE_TIME;

		if (timer >= fadeStartTime)
		{
			fadeAlpha = (timer - fadeStartTime) / FADE_TIME;
		}
	}
	else if (GetGamePhase() == PHASE_COUNTDOWN)
	{
		float timer = GetGamePhaseTimer();

		// カウントダウンの序盤は徐々に明るくする（透明度 1.0 → 0.0）
		if (timer <= FADE_TIME)
		{
			fadeAlpha = 1.0f - (timer / FADE_TIME);
		}
	}
	if (fadeAlpha > 0.0f && g_Texture[0])
	{
		float w = (float)Direct3D_GetBackBufferWidth();
		float h = (float)Direct3D_GetBackBufferHeight();

		g_pContext->PSSetShaderResources(0, 1, &g_Texture[0]);
		SetBlendState(BLENDSTATE_ALPHA);

		XMFLOAT2 pos = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
		XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };
		Shader_BeginUI();
		DrawSprite(pos, size, XMFLOAT4(1.0f, 1.0f, 1.0f, fadeAlpha));
	}

	if (GetGamePhase() == PHASE_COUNTDOWN)
	{
		float timer = GetGamePhaseTimer();
		int count = 3 - (int)timer; // 3, 2, 1, 0

		Shader_Begin();
		Shader_BeginUI();
		SetBlendState(BLENDSTATE_ALPHA);
		Shader_SetColor(color::white);

		// 画面中央の座標を計算
		float cx = (float)Direct3D_GetBackBufferWidth() / 2.0f;
		float cy = (float)Direct3D_GetBackBufferHeight() / 2.0f;

		XMFLOAT2 pos = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
		XMFLOAT2 size = { 300.0f, 300.0f };
		XMFLOAT2 sizeGO = { 600.0f, 300.0f };

		if (count == 3)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[3]);
			DrawSprite(pos, size, color::white);

			/*wchar_t text[8];
			swprintf_s(text, L"%d", count);
			DrawTextEx(text, cx - 30.0f, cy - 50.0f, 150.0f, L"FZゴンタかな", TextColor::P4color);*/
		}
		else if (count == 2)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[2]);
			DrawSprite(pos, size, color::white);

			/*wchar_t text[8];
			swprintf_s(text, L"%d", count);
			DrawTextEx(text, cx - 30.0f, cy - 50.0f, 150.0f, L"FZゴンタかな", TextColor::P3color);*/
		}
		else if (count == 1)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[1]);
			DrawSprite(pos, size, color::white);

			/*wchar_t text[8];
			swprintf_s(text, L"%d", count);
			DrawTextEx(text, cx - 20.0f, cy - 50.0f, 150.0f, L"FZゴンタかな", TextColor::P2color);*/
		}
		else if (count == 0)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[5]);
			DrawSprite(pos, sizeGO, color::white);

			/*DrawTextEx(L"GO!", cx - 180.0f, cy - 50.0f, 150.0f, L"FZゴンタかな", TextColor::P1color);*/
		}
	}

	// UI描画（にゅっ3）
	if (GetGamePhase() == PHASE_PLAY)
	{
		// スライドインの計算
		float playTime = GetGamePhaseTimer();
		float slideDuration = 0.4f;
		float OffsetY = 0.0f;

		if (playTime < slideDuration)
		{
			float t = playTime / slideDuration;
			float easeT = 1.0f - powf(1.0f - t, 3.0f); // 最初は速い、最後に減速

			// 画面外から定位置に向かって移動
			OffsetY = 800.0f * (1.0f - easeT);
		}

		// ビューポート(描画領域)を一時的にずらす
		ID3D11DeviceContext* pContext = Direct3D_GetDeviceContext();
		UINT numViewports = 1;
		D3D11_VIEWPORT vp;

		// 現在の設定を保存しておく
		pContext->RSGetViewports(&numViewports, &vp);

		// ずらす用の設定を作り適用する
		D3D11_VIEWPORT slideVp = vp;
		slideVp.TopLeftY += OffsetY; // 画面全体をオフセット分下にずらす
		pContext->RSSetViewports(1, &slideVp);
	
		// ずれた画面に対していつも通りUIを描画する
		Effect_Draw();
		Player_DrawHP();
		Player_DrawText();

		// 描画が終わったら元のビューポート(画面位置)に戻す
		pContext->RSSetViewports(1, &vp);
	}

}

