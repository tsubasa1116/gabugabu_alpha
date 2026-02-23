// fade.h

#pragma once

#include "direct3d.h"
#include "sprite.h"
#include "Manager.h"

enum FADE_STATE
{
	FADE_NONE = 0,
	FADE_IN,
	FADE_OUT,
};

struct FadeObject
{
	FADE_STATE	state;			// フェード処理状態
	float		count;			// カウンター
	float		frame;			// フェード所要時間
	XMFLOAT4	fadecolor;		// フェード色
	SCENE		scene;			// 次に切り替えるシーン
	bool		useLoading;		// ロード画面を使用するか（追加）
	const wchar_t* loadingVideo;// ロード画面用動画パス（追加）
};

void Fade_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Fade_Finalize();
void Fade_Update();
void Fade_Draw();
void SetFade(int fadeframe, XMFLOAT4 color, FADE_STATE state, SCENE scene);

// ロード画面付きフェード
void SetFadeWithLoading(int fadeframe, XMFLOAT4 color, FADE_STATE state, SCENE scene, 
						const wchar_t* videoPath = L"asset\\movie\\road.mp4");

FADE_STATE GetFadeState();



