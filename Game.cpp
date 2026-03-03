//======================================================
//	Game.cpp[]
// 
//	制作老E��前野翼			日付！E024//
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
#include "gimmick.h"

//======================================================
//	構造謡宣言
//======================================================
LIGHTOBJECT Light;

//======================================================
//
//======================================================
static int g_BgmID = NULL;
bool input2 = false;

// �R�}���h�����͂��ꂽ�Ƃ��ɗ��t���O
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

	Meteor_Initialize(pDevice, pContext);
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

	//g_BgmID = LoadAudio("asset\\Audio\\BGM_Game_Gengengenkidamon.wav");	// サウンドローチE
	//PlayAudio(g_BgmID, true);		// 再生開始（ループあり！E
	//PlayAudio(g_BgmID);			// 再生開始（ループなし！E
	//PlayAudio(g_BgmID, false);	// 再生開始（ループなし！E

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
	Meteor_Finalize();
	SkyBall_Finalize();
	//Building_Finalize();

	UnloadAudio(g_BgmID);
	DamageText_Finalize();
	g_GameInitialized = false;
	s_GameStarted = false;
	Loader::Reset();
}

//======================================================
//	更新処琁E
// 
// 
// 
// 
// 
// 
// 
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

		// 全プレイヤーの卵を割めE
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			PLAYEROBJECT* p = GetPlayer(i);
			if (p && p->active && p->duringRespawn)
			{
				p->duringRespawn = false;// 卵状態を解除
				p->isEggBreaking = true; // 割めE
			}
		}
		s_GameStarted = true;
	}

	// ------------------------------------
	// 
	// ------------------------------------
	// コマンドで使用する全てのキーの押下トリガーをチェチE��し、検�E関数に渡ぁE
	if (Keyboard_IsKeyDownTrigger(KK_P))
	{
		if (!s_IsKonamiCodeEntered)	s_IsKonamiCodeEntered = true;
		else						s_IsKonamiCodeEntered = false;
	}

	Player_Update();
	Meteor_Update();
	Field_Update();
	Building_UpdateAll();
	Effect_Update();
	MeteorEffectUpdate();

	// �����G�t�F�N�g�X�V�i1�����j
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
	DamageText_Update();

	//�Q�[���V�[���֑J��
	if (Keyboard_IsKeyDownTrigger(KK_F1) && (GetFadeState() == FADE_NONE))
	{
		// フェードアウトさせてシーンを�Eり替える
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

	// 建物エフェクト�E一括描画�E�ED空間！E
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

	// 覐�: �͈͕\���̂�
	Meteor_DrawRange(s_IsKonamiCodeEntered);
	MeteorEffectDraw();
	if (GetGamePhase() == PHASE_COUNTDOWN || GetGamePhase() == PHASE_PLAY)
	{
		Player_Draw(s_IsKonamiCodeEntered);
	}
	// 覐�: ���f���̂�
	Meteor_DrawModel(s_IsKonamiCodeEntered);

	// 2D�`��
	Light.SetEnable(FALSE);
	Shader_SetLight(Light.Light);
	SetDepthTest(FALSE);

	DamageText_Draw();

	if (GetGamePhase() == PHASE_INTRO)
	{// ミ�EルシチE��名札
		float cx = (float)Direct3D_GetBackBufferWidth() / 2.0f;
		float cy = (float)Direct3D_GetBackBufferHeight() / 2.0f;

		XMFLOAT2 pos = { cx - 380.0f * SCREEN_ADJUST_X, cy - 280.0f * SCREEN_ADJUST_Y };
		XMFLOAT2 size = { 540.0f * SCREEN_ADJUST_X, 150.0f * SCREEN_ADJUST_Y };

		g_pContext->PSSetShaderResources(0, 1, &g_Texture[4]);
		SetBlendState(BLENDSTATE_ALPHA);
		Shader_BeginUI();
		DrawSprite(pos, size, color::white);

		//DrawTextEx(L"ミ�EルシチE��", cx - 620.0f, cy - 340.0f, 70.0f, L"FZゴンタかな", TextColor::Black);
	}

	// フェード�E琁E
	float fadeAlpha = 0.0f;
	float INTRO_TIME = 3.5f; // INTRO_DURATION�E�Eamera.cpp�E�と同じ数値にする
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

		// カウントダウンの序盤は徐、E��明るくする（透�E度 1.0 ↁE0.0�E�E
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

		// 画面中央の座標を計箁E
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

	// UI描画�E�にめE��3�E�E
	if (GetGamePhase() == PHASE_PLAY)
	{
		// スライドインの計箁E
		float playTime = GetGamePhaseTimer();
		float slideDuration = 0.4f;
		float OffsetY = 0.0f;

		if (playTime < slideDuration)
		{
			float t = playTime / slideDuration;
			float easeT = 1.0f - powf(1.0f - t, 3.0f); // 最初�E速い、最後に減送E

			// 画面外から定位置に向かって移勁E
			OffsetY = 800.0f * (1.0f - easeT);
		}

		// ビューポ�EチE描画領域)を一時的にずらぁE
		ID3D11DeviceContext* pContext = Direct3D_GetDeviceContext();
		UINT numViewports = 1;
		D3D11_VIEWPORT vp;

		// 現在の設定を保存しておく
		pContext->RSGetViewports(&numViewports, &vp);

		// ずらす用の設定を作り適用する
		D3D11_VIEWPORT slideVp = vp;
		slideVp.TopLeftY += OffsetY; // 画面全体をオフセチE��刁E��にずらぁE
		pContext->RSSetViewports(1, &slideVp);
	
		// ずれた画面に対してぁE��も通りUIを描画する
		Shader_SetColor(color::white);
		Effect_Draw();
		Player_DrawHP();
		Player_DrawText();

		// 描画が終わったら允E�Eビューポ�EチE画面位置)に戻ぁE
		pContext->RSSetViewports(1, &vp);
	}
}

