	//======================================================
//	Game.cpp[]
// 
//	蛻ｶ菴懆・ｼ壼燕驥守ｿｼ			譌･莉假ｼ・024//
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
#include "cutin.h"
#include "gimmick.h"

//======================================================
// 
//======================================================
LIGHTOBJECT Light;

//======================================================
//
//======================================================
static int g_BgmID[4] = { -1, -1, -1, -1 };
bool input2 = false;

// コマンドが入力されたときに立つフラグ
static bool s_IsKonamiCodeEntered = false;
static bool g_GameInitialized = false;
static bool g_IsFirstFrame = true;
static bool s_GameStarted = false;
static bool s_IsCountSound = false;
static bool s_IsIntroSound = false;

static ID3D11DeviceContext* g_pContext = NULL;
static	ID3D11ShaderResourceView* g_Texture[6];

static int s_OldCount = -1;
static float s_CountAnimeTimer = 0.0f;

// カウントダウン用いーじんぐ
static inline float EaseCountDown(float t)
{
	const float bounceScale = 0.7f;          // 跳ね返りの強さ
	const float bounce = bounceScale + 1.0f; // 跳ね返りの大きさを調整する
	float timeAdj = t - 1.0f;                // 時間を調整して-1から0の範囲で変化させる

	// f(t) = 1 + C3(bounceScale - 1)^3 + C1(bounce - 1)^2
	return 1.0f + bounceScale * powf(timeAdj, 3.0f) + bounce * powf(timeAdj, 2.0f);
}


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
	Cutin_Initialize();

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

	//g_BgmID = LoadAudio("asset\\Audio\\BGM_Game_Gengengenkidamon.wav");	// 繧ｵ繧ｦ繝ｳ繝峨Ο繝ｼ繝・
	//PlayAudio(g_BgmID, true);		// 蜀咲函髢句ｧ具ｼ医Ν繝ｼ繝励≠繧奇ｼ・
	//PlayAudio(g_BgmID);			// 蜀咲函髢句ｧ具ｼ医Ν繝ｼ繝励↑縺暦ｼ・
	//PlayAudio(g_BgmID, false);	// 蜀咲函髢句ｧ具ｼ医Ν繝ｼ繝励↑縺暦ｼ・

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
	Cutin_Finalize();
	//Building_Finalize();


	UnloadAudio(g_BgmID[0]);
	DamageText_Finalize();
	g_GameInitialized = false;
	s_GameStarted = false;
	Loader::Reset();
	s_IsCountSound = false;
	s_IsIntroSound = false;
}

//======================================================
//	譖ｴ譁ｰ蜃ｦ逅・
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

		g_BgmID[0] = LoadAudio("asset\\Audio\\BGM_Game_Gengengenkidamon.wav");

		g_IsFirstFrame = false;
	}
	// If the intro BGM was played, stop and unload it once we leave the INTRO phase
	if (s_IsIntroSound && GetGamePhase() != PHASE_INTRO)
	{
		if (g_BgmID[2] >= 0)
		{
			UnloadAudio(g_BgmID[2]);
			g_BgmID[2] = -1;
		}
		s_IsIntroSound = false;
	}

	if (GetGamePhase() == PHASE_PLAY && !s_GameStarted)
	{
		PlayAudio(g_BgmID[0], true);

		// 蜈ｨ繝励Ξ繧､繝､繝ｼ縺ｮ蜊ｵ繧貞牡繧・
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			PLAYEROBJECT* p = GetPlayer(i);
			if (p && p->active && p->duringRespawn)
			{
				p->duringRespawn = false;// 蜊ｵ迥ｶ諷九ｒ隗｣髯､
				p->isEggBreaking = true; // 蜑ｲ繧・
			}
		}
		s_GameStarted = true;
	}

	// ------------------------------------
	// 
	// ------------------------------------
	// 繧ｳ繝槭Φ繝峨〒菴ｿ逕ｨ縺吶ｋ蜈ｨ縺ｦ縺ｮ繧ｭ繝ｼ縺ｮ謚ｼ荳九ヨ繝ｪ繧ｬ繝ｼ繧偵メ繧ｧ繝・け縺励∵､懷・髢｢謨ｰ縺ｫ貂｡縺・
	if (Keyboard_IsKeyDownTrigger(KK_P))
	{
		if (!s_IsKonamiCodeEntered)	 s_IsKonamiCodeEntered = true;
		else					s_IsKonamiCodeEntered = false;
	}

	Player_Update();
	Meteor_Update();
	Field_Update();
	Building_UpdateAll();
	Effect_Update();
	MeteorEffectUpdate();
	Cutin_Update();

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
	DamageText_Update();

	//ゲームシーンへ遷移
	if (Keyboard_IsKeyDownTrigger(KK_F1) && (GetFadeState() == FADE_NONE))
	{
		// 繝輔ぉ繝ｼ繝峨い繧ｦ繝医＆縺帙※繧ｷ繝ｼ繝ｳ繧貞・繧頑崛縺医ｋ
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

	// 建物の描画
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

	// 隕石: 範囲表示のみ
	Meteor_DrawRange(s_IsKonamiCodeEntered);
	MeteorEffectDraw();
	if (GetGamePhase() == PHASE_COUNTDOWN || GetGamePhase() == PHASE_PLAY)
	{
		Player_Draw(s_IsKonamiCodeEntered);
	}
	// 隕石: モデルのみ
	Meteor_DrawModel(s_IsKonamiCodeEntered);

	// 2D描画
	Light.SetEnable(FALSE);
	Shader_SetLight(Light.Light);
	SetDepthTest(FALSE);

	DamageText_Draw();
	Cutin_Draw();

	if (GetGamePhase() == PHASE_INTRO)
	{// 繝溘・繝ｫ繧ｷ繝・ぅ蜷肴惆
		float cx = (float)Direct3D_GetBackBufferWidth() / 2.0f;
		float cy = (float)Direct3D_GetBackBufferHeight() / 2.0f;

		XMFLOAT2 pos = { cx - 380.0f * SCREEN_ADJUST_X, cy - 280.0f * SCREEN_ADJUST_Y };
		XMFLOAT2 size = { 540.0f * SCREEN_ADJUST_X, 150.0f * SCREEN_ADJUST_Y };

		g_pContext->PSSetShaderResources(0, 1, &g_Texture[4]);
		SetBlendState(BLENDSTATE_ALPHA);
		Shader_BeginUI();
		DrawSprite(pos, size, color::white);

		//DrawTextEx(L"繝溘・繝ｫ繧ｷ繝・ぅ", cx - 620.0f, cy - 340.0f, 70.0f, L"FZ繧ｴ繝ｳ繧ｿ縺九↑", TextColor::Black);
	}

	// 繝輔ぉ繝ｼ繝牙・逅・
	float fadeAlpha = 0.0f;
	float INTRO_TIME = 3.5f; // INTRO_DURATION・・amera.cpp・峨→蜷後§謨ｰ蛟､縺ｫ縺吶ｋ
	float FADE_TIME = 0.5f;

	if (GetGamePhase() == PHASE_INTRO)
	{
		float timer = GetGamePhaseTimer();
		float fadeStartTime = INTRO_TIME - FADE_TIME;

		if (timer >= fadeStartTime)
		{
			fadeAlpha = (timer - fadeStartTime) / FADE_TIME;
		}

		// フェードイン完了後（FADE_TIME経過後）にBGMを再生開始
		if (!s_IsIntroSound && timer >= 0.25f) 
		{
			s_IsIntroSound = true;
			g_BgmID[2] = LoadAudio("asset\\Audio\\BGM_Game_Gengengenkidamon.wav");
			PlayAudio(g_BgmID[2], false);
		}
	}
	else if (GetGamePhase() == PHASE_COUNTDOWN)
	{
		float timer = GetGamePhaseTimer();

		// 繧ｫ繧ｦ繝ｳ繝医ム繧ｦ繝ｳ縺ｮ蠎冗乢縺ｯ蠕舌・↓譏弱ｋ縺上☆繧具ｼ磯乗・蠎ｦ 1.0 竊・0.0・・
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

		// 逕ｻ髱｢荳ｭ螟ｮ縺ｮ蠎ｧ讓吶ｒ險育ｮ・
		float cx = (float)Direct3D_GetBackBufferWidth() / 2.0f;
		float cy = (float)Direct3D_GetBackBufferHeight() / 2.0f;

		XMFLOAT2 pos = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
		XMFLOAT2 size = { 300.0f, 300.0f };
		XMFLOAT2 sizeGO = { 600.0f, 300.0f };


		if (!s_IsCountSound)
		{
			s_IsCountSound = true;
			g_BgmID[1] = LoadAudio("asset\\Audio\\Countdown.wav");
			PlayAudio(g_BgmID[1], false);
		}	

		if (s_OldCount != count)
		{// カウントが変わったらアニメーションをリセット
			s_OldCount = count;
			s_CountAnimeTimer = 0.0f;
		}

		// タイマーを進める
		s_CountAnimeTimer += (1.0f / 60.0f) * 3.5f; // *でアニメーション速度を調節
		if (s_CountAnimeTimer > 1.0f) s_CountAnimeTimer = 1.0f;

		// アニメーション係数を計算
		float animScale = EaseCountDown(s_CountAnimeTimer);
		XMFLOAT2 animaSize = { size.x * animScale, size.y * animScale };
		XMFLOAT2 animaSizeGO = { sizeGO.x * animScale, sizeGO.y * animScale };

		if (count == 3)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[3]);
			DrawSprite(pos, animaSize, color::white);
		}
		else if (count == 2)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[2]);
			DrawSprite(pos, animaSize, color::white);

		}
		else if (count == 1)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[1]);
			DrawSprite(pos, animaSize, color::white);
		}
		else if (count == 0)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[5]);
			DrawSprite(pos, animaSizeGO, color::white);
		}
	}

	// UI謠冗判・医↓繧・▲3・・
	if (GetGamePhase() == PHASE_PLAY)
	{
		// 繧ｹ繝ｩ繧､繝峨う繝ｳ縺ｮ險育ｮ・
		float playTime = GetGamePhaseTimer();
		float slideDuration = 0.4f;
		float OffsetY = 0.0f;

		if (playTime < slideDuration)
		{
			float t = playTime / slideDuration;
			float easeT = 1.0f - powf(1.0f - t, 3.0f); // 譛蛻昴・騾溘＞縲∵怙蠕後↓貂幃・

			// 逕ｻ髱｢螟悶°繧牙ｮ壻ｽ咲ｽｮ縺ｫ蜷代°縺｣縺ｦ遘ｻ蜍・
			OffsetY = 800.0f * (1.0f - easeT);
		}

		// 繝薙Η繝ｼ繝昴・繝・謠冗判鬆伜沺)繧剃ｸ譎ら噪縺ｫ縺壹ｉ縺・
		ID3D11DeviceContext* pContext = Direct3D_GetDeviceContext();
		UINT numViewports = 1;
		D3D11_VIEWPORT vp;

		// 迴ｾ蝨ｨ縺ｮ險ｭ螳壹ｒ菫晏ｭ倥＠縺ｦ縺翫￥
		pContext->RSGetViewports(&numViewports, &vp);

		// 縺壹ｉ縺咏畑縺ｮ險ｭ螳壹ｒ菴懊ｊ驕ｩ逕ｨ縺吶ｋ
		D3D11_VIEWPORT slideVp = vp;
		slideVp.TopLeftY += OffsetY; // 逕ｻ髱｢蜈ｨ菴薙ｒ繧ｪ繝輔そ繝・ヨ蛻・ｸ九↓縺壹ｉ縺・
		pContext->RSSetViewports(1, &slideVp);
	
		// 縺壹ｌ縺溽判髱｢縺ｫ蟇ｾ縺励※縺・▽繧る壹ｊUI繧呈緒逕ｻ縺吶ｋ
		Shader_SetColor(color::white);
		Effect_Draw();
		Player_DrawHP();
		Player_DrawText();

		// 謠冗判縺檎ｵゅｏ縺｣縺溘ｉ蜈・・繝薙Η繝ｼ繝昴・繝・逕ｻ髱｢菴咲ｽｮ)縺ｫ謌ｻ縺・
		pContext->RSSetViewports(1, &vp);
	}
}

