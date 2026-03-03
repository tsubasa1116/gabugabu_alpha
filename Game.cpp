//======================================================
//	Game.cpp[]
// 
//	åˆ¶ä½œè€E¼šå‰é‡ç¿¼			æ—¥ä»˜ï¼E024//
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
//	æ§‹é€ è¬¡å®£è¨€
//======================================================
LIGHTOBJECT Light;

//======================================================
//
//======================================================
static int g_BgmID = NULL;
bool input2 = false;

// ƒRƒ}ƒ“ƒh‚ª“ü—Í‚³‚ê‚½‚Æ‚«‚É—§‚Âƒtƒ‰ƒO
static bool s_IsKonamiCodeEntered = false;
static bool g_GameInitialized = false;
static bool g_IsFirstFrame = true;
static bool s_GameStarted = false;

static ID3D11DeviceContext* g_pContext = NULL;
static	ID3D11ShaderResourceView* g_Texture[6];

// ‚±‚±‚ÅÀ‘Ì‚ğì‚éi1‰ÓŠ‚¾‚¯Ij
float g_hitStopTimer = 0.0f;

// ŠÖ”‚ÌgŒ³‚à‚±‚±‚É‘‚­
void StartHitStop(float duration) {
	g_hitStopTimer = duration;
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

	//g_BgmID = LoadAudio("asset\\Audio\\BGM_Game_Gengengenkidamon.wav");	// ã‚µã‚¦ãƒ³ãƒ‰ãƒ­ãƒ¼ãƒE
	//PlayAudio(g_BgmID, true);		// å†ç”Ÿé–‹å§‹ï¼ˆãƒ«ãƒ¼ãƒ—ã‚ã‚Šï¼E
	//PlayAudio(g_BgmID);			// å†ç”Ÿé–‹å§‹ï¼ˆãƒ«ãƒ¼ãƒ—ãªã—ï¼E
	//PlayAudio(g_BgmID, false);	// å†ç”Ÿé–‹å§‹ï¼ˆãƒ«ãƒ¼ãƒ—ãªã—ï¼E

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
//	æ›´æ–°å‡¦çE
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

		// å…¨ãƒ—ãƒ¬ã‚¤ãƒ¤ãƒ¼ã®åµã‚’å‰²ã‚E
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			PLAYEROBJECT* p = GetPlayer(i);
			if (p && p->active && p->duringRespawn)
			{
				p->duringRespawn = false;// åµçŠ¶æ…‹ã‚’è§£é™¤
				p->isEggBreaking = true; // å‰²ã‚E
			}
		}
		s_GameStarted = true;
	}

	// ------------------------------------
	// 
	// ------------------------------------
	// ã‚³ãƒãƒ³ãƒ‰ã§ä½¿ç”¨ã™ã‚‹å…¨ã¦ã®ã‚­ãƒ¼ã®æŠ¼ä¸‹ãƒˆãƒªã‚¬ãƒ¼ã‚’ãƒã‚§ãƒE‚¯ã—ã€æ¤œåEé–¢æ•°ã«æ¸¡ãE
	if (Keyboard_IsKeyDownTrigger(KK_P))
	{
		if (!s_IsKonamiCodeEntered)	s_IsKonamiCodeEntered = true;
		else						s_IsKonamiCodeEntered = false;
	}

	float currentDeltaTime = DELTA_TIME;

	// ƒqƒbƒgƒXƒgƒbƒv’†‚È‚çƒ^ƒCƒ}[‚ğŒ¸‚ç‚µ‚ÄADELTA_TIME‚ğ0‚É‚·‚é
	if (g_hitStopTimer > 0.0f) {
		g_hitStopTimer -= DELTA_TIME;
		currentDeltaTime = 0.0f; // ‚ğ~‚ß‚éI
	}

	// ƒvƒŒƒCƒ„[‚âŒš•¨‚ÌXV‚É‚Í currentDeltaTime ‚ğg‚¤‚æ‚¤‚É‚·‚é
	// ------------------------------------
	// 
	// ------------------------------------
	Player_Update(currentDeltaTime);

	Player_Update();
	Meteor_Update();
	Field_Update();
	Building_UpdateAll();
	Effect_Update();
	MeteorEffectUpdate();

	// Œš•¨ƒGƒtƒFƒNƒgXVi1“‚¸‚Âj
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

	//ƒQ[ƒ€ƒV[ƒ“‚Ö‘JˆÚ
	if (Keyboard_IsKeyDownTrigger(KK_F1) && (GetFadeState() == FADE_NONE))
	{
		// ãƒ•ã‚§ãƒ¼ãƒ‰ã‚¢ã‚¦ãƒˆã•ã›ã¦ã‚·ãƒ¼ãƒ³ã‚’åEã‚Šæ›¿ãˆã‚‹
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

	// å»ºç‰©ã‚¨ãƒ•ã‚§ã‚¯ãƒˆãEä¸€æ‹¬æç”»EEDç©ºé–“ï¼E
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

	// è¦Î: ”ÍˆÍ•\¦‚Ì‚İ
	Meteor_DrawRange(s_IsKonamiCodeEntered);
	MeteorEffectDraw();
	if (GetGamePhase() == PHASE_COUNTDOWN || GetGamePhase() == PHASE_PLAY)
	{
		Player_Draw(s_IsKonamiCodeEntered);
	}
	// è¦Î: ƒ‚ƒfƒ‹‚Ì‚İ
	Meteor_DrawModel(s_IsKonamiCodeEntered);

	// 2D•`‰æ
	Light.SetEnable(FALSE);
	Shader_SetLight(Light.Light);
	SetDepthTest(FALSE);

	DamageText_Draw();

	if (GetGamePhase() == PHASE_INTRO)
	{// ãƒŸãEãƒ«ã‚·ãƒE‚£åæœ­
		float cx = (float)Direct3D_GetBackBufferWidth() / 2.0f;
		float cy = (float)Direct3D_GetBackBufferHeight() / 2.0f;

		XMFLOAT2 pos = { cx - 380.0f * SCREEN_ADJUST_X, cy - 280.0f * SCREEN_ADJUST_Y };
		XMFLOAT2 size = { 540.0f * SCREEN_ADJUST_X, 150.0f * SCREEN_ADJUST_Y };

		g_pContext->PSSetShaderResources(0, 1, &g_Texture[4]);
		SetBlendState(BLENDSTATE_ALPHA);
		Shader_BeginUI();
		DrawSprite(pos, size, color::white);

		//DrawTextEx(L"ãƒŸãEãƒ«ã‚·ãƒE‚£", cx - 620.0f, cy - 340.0f, 70.0f, L"FZã‚´ãƒ³ã‚¿ã‹ãª", TextColor::Black);
	}

	// ãƒ•ã‚§ãƒ¼ãƒ‰åEçE
	float fadeAlpha = 0.0f;
	float INTRO_TIME = 3.5f; // INTRO_DURATIONEEamera.cppE‰ã¨åŒã˜æ•°å€¤ã«ã™ã‚‹
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

		// ã‚«ã‚¦ãƒ³ãƒˆãƒ€ã‚¦ãƒ³ã®åºç›¤ã¯å¾ã€E«æ˜ã‚‹ãã™ã‚‹ï¼ˆé€æEåº¦ 1.0 â†E0.0EE
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

		// ç”»é¢ä¸­å¤®ã®åº§æ¨™ã‚’è¨ˆç®E
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
			DrawTextEx(text, cx - 30.0f, cy - 50.0f, 150.0f, L"FZã‚´ãƒ³ã‚¿ã‹ãª", TextColor::P4color);*/
		}
		else if (count == 2)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[2]);
			DrawSprite(pos, size, color::white);

			/*wchar_t text[8];
			swprintf_s(text, L"%d", count);
			DrawTextEx(text, cx - 30.0f, cy - 50.0f, 150.0f, L"FZã‚´ãƒ³ã‚¿ã‹ãª", TextColor::P3color);*/
		}
		else if (count == 1)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[1]);
			DrawSprite(pos, size, color::white);

			/*wchar_t text[8];
			swprintf_s(text, L"%d", count);
			DrawTextEx(text, cx - 20.0f, cy - 50.0f, 150.0f, L"FZã‚´ãƒ³ã‚¿ã‹ãª", TextColor::P2color);*/
		}
		else if (count == 0)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[5]);
			DrawSprite(pos, sizeGO, color::white);

			/*DrawTextEx(L"GO!", cx - 180.0f, cy - 50.0f, 150.0f, L"FZã‚´ãƒ³ã‚¿ã‹ãª", TextColor::P1color);*/
		}
	}

	// UIæç”»Eˆã«ã‚E£3EE
	if (GetGamePhase() == PHASE_PLAY)
	{
		// ã‚¹ãƒ©ã‚¤ãƒ‰ã‚¤ãƒ³ã®è¨ˆç®E
		float playTime = GetGamePhaseTimer();
		float slideDuration = 0.4f;
		float OffsetY = 0.0f;

		if (playTime < slideDuration)
		{
			float t = playTime / slideDuration;
			float easeT = 1.0f - powf(1.0f - t, 3.0f); // æœ€åˆãEé€Ÿã„ã€æœ€å¾Œã«æ¸›é€E

			// ç”»é¢å¤–ã‹ã‚‰å®šä½ç½®ã«å‘ã‹ã£ã¦ç§»å‹E
			OffsetY = 800.0f * (1.0f - easeT);
		}

		// ãƒ“ãƒ¥ãƒ¼ãƒãEãƒEæç”»é ˜åŸŸ)ã‚’ä¸€æ™‚çš„ã«ãšã‚‰ãE
		ID3D11DeviceContext* pContext = Direct3D_GetDeviceContext();
		UINT numViewports = 1;
		D3D11_VIEWPORT vp;

		// ç¾åœ¨ã®è¨­å®šã‚’ä¿å­˜ã—ã¦ãŠã
		pContext->RSGetViewports(&numViewports, &vp);

		// ãšã‚‰ã™ç”¨ã®è¨­å®šã‚’ä½œã‚Šé©ç”¨ã™ã‚‹
		D3D11_VIEWPORT slideVp = vp;
		slideVp.TopLeftY += OffsetY; // ç”»é¢å…¨ä½“ã‚’ã‚ªãƒ•ã‚»ãƒEƒˆåˆE¸‹ã«ãšã‚‰ãE
		pContext->RSSetViewports(1, &slideVp);
	
		// ãšã‚ŒãŸç”»é¢ã«å¯¾ã—ã¦ãE¤ã‚‚é€šã‚ŠUIã‚’æç”»ã™ã‚‹
		Shader_SetColor(color::white);
		Effect_Draw();
		Player_DrawHP();
		Player_DrawText();

		// æç”»ãŒçµ‚ã‚ã£ãŸã‚‰å…EEãƒ“ãƒ¥ãƒ¼ãƒãEãƒEç”»é¢ä½ç½®)ã«æˆ»ãE
		pContext->RSSetViewports(1, &vp);
	}

}

