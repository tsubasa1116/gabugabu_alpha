// Manager.h
#pragma once

enum SCENE
{
	SCENE_NONE = 0,		//菴輔ｂ縺ｪ縺
	SCENE_TITLE,		//繧ｿ繧､繝医Ν繧ｷ繝ｼ繝ｳ
	SCENE_START,		//繧ｲ繝ｼ繝繧ｹ繧ｿ繝ｼ繝医す繝ｼ繝ｳ
	SCENE_SETTING,		//險ｭ螳壹す繝ｼ繝ｳ
	SCENE_SOUND,		//繧ｵ繧ｦ繝ｳ繝峨ユ繧ｹ繝医す繝ｼ繝ｳ
	SCENE_GAME,			//繧ｲ繝ｼ繝繧ｷ繝ｼ繝ｳ
	SCENE_WIN,			//蜍晏茜繧ｷ繝ｼ繝ｳ
	SCENE_RESULT,		//繝ｪ繧ｶ繝ｫ繝医Ν繧ｷ繝ｼ繝ｳ

};

void Manager_Initialize();
void Manager_Finalize();
void Manager_Update();
void Manager_Draw();

void SetScene(SCENE scene);
