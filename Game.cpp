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

// コマンドが入力されたときに立つフラグ
static bool s_IsKonamiCodeEntered = false;

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

	UnloadAudio(g_BgmID);
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
	Effect_Update();
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

	Camera_Draw();	// Drawの最初で呼ぶ！
	Field_Draw	(s_IsKonamiCodeEntered);
	Player_Draw	(s_IsKonamiCodeEntered);

	Light.SetEnable(FALSE);			
	Shader_SetLight(Light.Light);	
	SetDepthTest(FALSE);
    
	Effect_Draw();
	Player_DrawHP();
	
	Player_DrawText();
	DamageText_Draw();
}

