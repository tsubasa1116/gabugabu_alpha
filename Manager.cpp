//======================================================
//	manager.cpp[]
// 
//	制作者：田中佑奈			日付：2024//
//======================================================
//Manager.cpp

#include "direct3d.h"
#include "Manager.h"
#include "keyboard.h"

#include "Game.h"
#include "Title.h"
#include "Start.h"
#include "Setting.h"
#include "Sound.h"
#include "Result.h"
#include "fade.h"

#include "shader.h"


//グローバル変数
static	SCENE	g_Scene = SCENE_NONE;	//現在のシーン番号
static bool g_InitSettingOnce = false;	//最初だけ初期化したか
static bool g_InitSoundOnce = false;	//最初だけ初期化したか

void	Manager_Initialize()
{
	Fade_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());

	////本来はtitleの初期化でフェードインをセットする
	//XMFLOAT4 color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	//SetFade(60.0f, color, FADE_STATE::FADE_IN, SCENE_GAME);
	//SetScene(SCENE_GAME);	//最初に動かすシーンに切り替える


	//本来の形
	Fade_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
#ifdef _DEBUG
	SetScene(SCENE_GAME);	//debug用に最初からゲームシーンへ
#else
	SetScene(SCENE_TITLE);	//最初に動かすシーンに切り替える
#endif


}

void	Manager_Finalize()
{
	Fade_Finalize();
	SetScene(SCENE_NONE);
}

void	Manager_Update()
{
	switch (g_Scene)	//現在シーンのアップデート関数を呼び出す
	{
	case SCENE_NONE:
		break;
	case SCENE_TITLE:
		Title_Update();
		break;
	case SCENE_START:
		Start_Update();
		break;
	case SCENE_SETTING:
		Setting_Update();
		break;
	case SCENE_SOUND:
		Sound_Update();
		break;
	case SCENE_GAME:
		Game_Update();
		break;
	case SCENE_RESULT:
		Result_Update();
		break;
	default:
		break;
	}

	Fade_Update();

}

void	Manager_Draw()
{
	switch (g_Scene)	//現在シーンの描画関数を呼び出す
	{
	case SCENE_NONE:
		break;
	case SCENE_TITLE:
		Title_Draw();
		break;
	case SCENE_START:
		Start_Draw();
		break;
	case SCENE_SETTING:
		Setting_Draw();
		break;
	case SCENE_SOUND:
		Sound_Draw();
		break;
	case SCENE_GAME:
		Game_Draw();
		break;
	case SCENE_RESULT:
		Result_Draw();
		break;
	default:
		break;
	}

	Fade_Draw();

}

void	SetScene(SCENE scene) //シーンを切り替える
{
	//実行中のシーンを終了させる
	switch (g_Scene)	//現在シーンの終了関数を呼び出す
	{
	case SCENE_NONE:
		break;
	case SCENE_TITLE:
		Title_Finalize();
		break;
	case SCENE_START:
		Start_Finalize();
		break;
	case SCENE_SETTING:
		//Setting_Finalize();
		break;
	case SCENE_SOUND:
		//Sound_Finalize();
		break;
	case SCENE_GAME:
		Game_Finalize();
		break;
	case SCENE_RESULT:
		Result_Finalize();
		break;
	default:
		break;
	}

	g_Scene = scene;	//指定のシーンへ切り替える

	//次のシーンを初期化する
	switch (g_Scene)	//現在シーンの初期化関数を呼び出す
	{
	case SCENE_NONE:
		break;
	case SCENE_TITLE:
		Title_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
		break;
	case SCENE_START:
		Start_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
		break;
	case SCENE_SETTING:
		if (!g_InitSettingOnce)
		{
			Setting_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
			g_InitSettingOnce = true;   //初回だけ true にする
		}
		break;
	case SCENE_SOUND:
		if (!g_InitSettingOnce)
		{
			Sound_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
			g_InitSoundOnce = true;   //初回だけ true にする
		}
		break;
	case SCENE_GAME:
		Game_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
		break;
	case SCENE_RESULT:
		Result_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
		break;
	default:
		break;
	}

}
