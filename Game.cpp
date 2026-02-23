//======================================================
//	Game.cpp[]
// 
//	
//======================================================

#include "Manager.h"
#include "sprite.h"
#include "Game.h"
#include "keyboard.h"
#include "makeText.h"
#include "p.h"
#include "field.h"
#include "building.h"
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
//======================================================
//	
//======================================================
LIGHTOBJECT Light;

//======================================================
//
//======================================================
static int g_BgmID = NULL;
bool input2 = false;

const int KONAMI_CODE[] = {
	KK_UP, KK_UP, KK_DOWN, KK_DOWN,
	KK_LEFT, KK_RIGHT, KK_LEFT, KK_RIGHT,
	KK_B, KK_A
};


const int KONAMI_CODE_LENGTH = sizeof(KONAMI_CODE) / sizeof(KONAMI_CODE[0]);


static int s_KonamiCodeIndex = 0;


static bool s_IsKonamiCodeEntered = false;

void CheckKonamiCode(int currentKeyCode)
{

	if (currentKeyCode == KONAMI_CODE[s_KonamiCodeIndex])
	{

		s_KonamiCodeIndex++;


		if (s_KonamiCodeIndex >= KONAMI_CODE_LENGTH)
		{

			s_IsKonamiCodeEntered = !s_IsKonamiCodeEntered;


			s_KonamiCodeIndex = 0;
		}
	}
	else
	{

		s_KonamiCodeIndex = 0;


		if (currentKeyCode == KONAMI_CODE[0])
		{
			s_KonamiCodeIndex = 1;
		}
	}
}

//======================================================
//	
//======================================================
void Game_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
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

	g_BgmID = LoadAudio("asset\\Audio\\BGM_01.wav");
	//PlayAudio(g_BgmID, true);
	//PlayAudio(g_BgmID);		
	//PlayAudio(g_BgmID, false);

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

	//UnloadAudio(g_BgmID);
	DamageText_Finalize();
}

//======================================================
//
//======================================================
void Game_Update()
{
	// ------------------------------------
	// 
	// ------------------------------------
	if (Keyboard_IsKeyDownTrigger(KK_UP))			CheckKonamiCode(KK_UP);
	else if (Keyboard_IsKeyDownTrigger(KK_DOWN))	CheckKonamiCode(KK_DOWN);
	else if (Keyboard_IsKeyDownTrigger(KK_LEFT))	CheckKonamiCode(KK_LEFT);
	else if (Keyboard_IsKeyDownTrigger(KK_RIGHT))	CheckKonamiCode(KK_RIGHT);
	else if (Keyboard_IsKeyDownTrigger(KK_B))		CheckKonamiCode(KK_B);
	else if (Keyboard_IsKeyDownTrigger(KK_A))		CheckKonamiCode(KK_A);
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


	if (Keyboard_IsKeyDownTrigger(KK_F1) && (GetFadeState() == FADE_NONE))
	{

		XMFLOAT4 color(0.0f, 0.0f, 0.0f, 1.0f);
		SetFade(40.0f, color, FADE_OUT, SCENE_RESULT);
	}
}

//======================================================
//
//======================================================
void Game_Draw()
{
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

	Light.SetEnable(FALSE);
	Shader_SetLight(Light.Light);
	SetDepthTest(FALSE);

	Effect_Draw();
	Player_DrawHP();

	Player_DrawText();
	DamageText_Draw();
}

