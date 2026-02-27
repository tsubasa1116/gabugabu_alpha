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

	//BallInitialize(pDevice, pContext);
	//P_Initialize(pDevice, pContext);		
	//Score_Initialize(pDevice, pContext);

	g_BgmID = LoadAudio("asset\\Audio\\BGM_01.wav");	// サウンドロード
	//PlayAudio(g_BgmID, true);		// 再生開始(ループあり)
	//PlayAudio(g_BgmID);			// 再生開始（ループなし）
	//PlayAudio(g_BgmID, false);	// 再生開始（ループなし）

	XMFLOAT4 para;

	para = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	Light.SetAmbient(para);
	para = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
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

	//BallFinalize();
	//P_Finalize();
	//Score_Finalize();

	UnloadAudio(g_BgmID);	// サウンドの解放
	DamageText_Finalize();
	g_GameInitialized = false;
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
		g_IsFirstFrame = false;
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

	// ★ 建物エフェクトの一括描画（3D空間）
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
	Player_Draw(s_IsKonamiCodeEntered);

	//2D描画
	Light.SetEnable(FALSE);			// ライティングOFF
	Shader_SetLight(Light.Light);	// ライト構造体をシェーダーへセット
	SetDepthTest(FALSE);

	Effect_Draw();
	Player_DrawHP();

	Player_DrawText();
	DamageText_Draw();
}

