
//Manager.h
#pragma once

enum SCENE
{
	SCENE_NONE = 0,		//何もなし
	SCENE_TITLE,		//タイトルシーン
	SCENE_START,		//ゲームスタートシーン
	SCENE_SETTING,		//設定シーン
	SCENE_SOUND,		//サウンドテストシーン
	SCENE_GAME,			//ゲームシーン
	SCENE_RESULT,		//リザルトルシーン

};

void	Manager_Initialize();
void	Manager_Finalize();
void	Manager_Update();
void	Manager_Draw();

void	SetScene(SCENE scene);
