//======================================================
//	Manager.cpp
//======================================================
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
#include "swipe.h"
#include "shader.h"
#include "LoadingScreen.h"  // 追加

//グローバル変数
static SCENE g_Scene = SCENE_NONE;
static bool g_InitSettingOnce = false;
static bool g_InitSoundOnce = false;

void Manager_Initialize()
{
	Fade_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
	Swipe_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
	LoadingScreen_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());  // 追加

#ifdef _DEBUG
	//DEBUG:
	//SetScene(SCENE_TITLE);
	SetScene(SCENE_GAME);
#else
	SetScene(SCENE_TITLE);
#endif
}

void Manager_Finalize()
{
	LoadingScreen_Finalize();  // 追加
	Fade_Finalize();
	Swipe_Finalize();
	SetScene(SCENE_NONE);
}

void Manager_Update()
{
	// ロード中は通常のシーン更新をスキップ
	if (IsLoading())
	{
		LoadingScreen_Update();
		return;
	}

	switch (g_Scene)
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
	Swipe_Update();
}

void Manager_Draw()
{
	// ロード中はロード画面のみ描画
	if (IsLoading())
	{
		LoadingScreen_Draw();
		return;
	}

	switch (g_Scene)
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
	Swipe_Draw();
}

void SetScene(SCENE scene)
{
	// 実行中のシーンを終了する
	switch (g_Scene)
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
		break;
	case SCENE_SOUND:
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

	g_Scene = scene;

	// 次のシーンを初期化する
	switch (g_Scene)
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
			g_InitSettingOnce = true;
		}
		break;
	case SCENE_SOUND:
		if (!g_InitSoundOnce)
		{
			Sound_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
			g_InitSoundOnce = true;
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
