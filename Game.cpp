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
#include "building.h"
#include "Effect.h"
#include "score.h"
#include "Audio.h"
#include "gauge.h"
#include "Polygon.h"
#include "Player.h"
#include "Camera.h"
#include "Ball.h"
#include "attack.h"
#include "skill.h"
#include "special.h"
#include "fade.h"
#include "DamageText.h"
#include "direct3d.h"

//======================================================
//	構造謡宣言
//======================================================
LIGHTOBJECT Light;

//======================================================
//	グローバル変数
//======================================================
static int g_BgmID = NULL;	// サウンド管理ID
bool input2 = false;

// コマンドが入力されたときに立つフラグ
static bool s_IsKonamiCodeEntered = false;

//======================================================
//	初期化関数
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

	//BallInitialize(pDevice, pContext);
	//P_Initialize(pDevice, pContext);		// プレイヤーの初期化
	//Score_Initialize(pDevice, pContext);

	g_BgmID = LoadAudio("asset\\Audio\\BGM_01.wav");	// サウンドロード
	//PlayAudio(g_BgmID, true);		// 再生開始(ループあり)
	//PlayAudio(g_BgmID);			// 再生開始（ループなし）
	//PlayAudio(g_BgmID, false);	// 再生開始（ループなし）

	//ライト初期化
	XMFLOAT4 para;

	para = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);	// 環境光の色
	Light.SetAmbient(para);
	para = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);	// 光の色
	Light.SetDiffuse(para);
	para = XMFLOAT4(0.5f, -1.0f, 0.0f, 1.0f);	// 光方向

	float len = sqrtf(para.x * para.x + para.y * para.y + para.z * para.z);
	para.x /= len;
	para.y /= len;
	para.z /= len;
	Light.SetDirection(para);	// 光の方向（正規化済）
}

//======================================================
//	終了処理関数
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
	
	//BallFinalize();
	//P_Finalize();
	//Score_Finalize();

	UnloadAudio(g_BgmID);	// サウンドの解放
	DamageText_Finalize();
}

//======================================================
//	更新処理
//======================================================
void Game_Update()
{
	// ------------------------------------
	//  コナミコマンド検出
	// ------------------------------------
	// コマンドで使用する全てのキーの押下トリガーをチェックし、検出関数に渡す
	if (Keyboard_IsKeyDownTrigger(KK_P))
	{
		if(!s_IsKonamiCodeEntered)	s_IsKonamiCodeEntered = true;
		else						s_IsKonamiCodeEntered = false;
	}
	// ------------------------------------
	// 更新処理
	// ------------------------------------
	Player_Update();
	Field_Update();
	Effect_Update();
	Gauge_Update();
	Camera_Update();	// プレイヤーの更新の後に呼ぶ

	//BallUpdate();
	//P_Update();
	//Score_Update();
	DamageText_Update();

	//ゲームシーンへ遷移
	if (Keyboard_IsKeyDownTrigger(KK_F1) && (GetFadeState() == FADE_NONE))
	{
		// フェードアウトさせてシーンを切り替える
		XMFLOAT4 color(0.0f, 0.0f, 0.0f, 1.0f);
		SetFade(40.0f, color, FADE_OUT, SCENE_RESULT);
	}
}

//======================================================
//	描画関数
//======================================================
void Game_Draw()
{ 
	Light.SetEnable(TRUE);			// ライティングON
	Shader_SetLight(Light.Light);	// ライト構造体をシェーダーへセット
	SetDepthTest(TRUE);

	Camera_Draw();	// Drawの最初で呼ぶ！
	Field_Draw	(s_IsKonamiCodeEntered);
	Player_Draw	(s_IsKonamiCodeEntered);

	//2D描画
	Light.SetEnable(FALSE);			// ライティングOFF
	Shader_SetLight(Light.Light);	// ライト構造体をシェーダーへセット
	SetDepthTest(FALSE);
    
	Effect_Draw();
	Player_DrawHP();
	
	
	Player_DrawText();
	DamageText_Draw();
	//DrawTextEx(
	//	L"こんにちは世界",			// 表示する文字
	//	600, 400,					// 位置
	//	60.0f,						// サイズ
	//	L"玉ねぎ楷書激無料版v7改",	// フォント
	//	TextColor::Yellow			// 色
	//);
	
	//P_Draw();
}

