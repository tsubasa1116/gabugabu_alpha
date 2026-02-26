//======================================================
//	fade.cpp
//======================================================

#include "fade.h"
#include "shader.h"
#include "LoadingScreen.h"

FadeObject g_Fade;	// フェード処理構造体

static ID3D11ShaderResourceView* g_Texture = NULL;	// テクスチャ１枚を表すオブジェクト
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

void Fade_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// テクスチャ読み込み
	TexMetadata		metadata;
	ScratchImage	image;
	LoadFromWICFile(L"asset\\texture\\fade.bmp", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
	assert(g_Texture);	// 読み込み失敗時にダイアログを表示

	g_Fade.fadecolor.x = 0.0f;
	g_Fade.fadecolor.y = 0.0f;
	g_Fade.fadecolor.z = 0.0f;
	g_Fade.fadecolor.w = 1.0f;
	g_Fade.frame = 60.0f;	// 60フレームでフェード完了
	g_Fade.state = FADE_STATE::FADE_NONE;
	g_Fade.useLoading = false;
	g_Fade.loadingVideo = nullptr;
}

void Fade_Finalize()
{
	if (g_Texture != NULL)
	{
		g_Texture->Release();
		g_Texture = NULL;
	}
}

void Fade_Update()
{
}

void Fade_Draw()
{
	// 現在の状態
	switch (g_Fade.state)
	{
	case FADE_STATE::FADE_NONE:
		return;

	case FADE_STATE::FADE_IN:
		if (g_Fade.fadecolor.w < 0.0)
		{
			// フェードイン終了
			g_Fade.fadecolor.w = 0.0f;
			g_Fade.state = FADE_STATE::FADE_NONE;
		}
		break;

	case FADE_STATE::FADE_OUT:
		if (g_Fade.fadecolor.w > 1.0f)
		{
			// フェードアウト終了
			g_Fade.fadecolor.w = 1.0;

			// ロード画面を使用するかどうか
			if (g_Fade.useLoading)
			{
				// ロード画面付きでシーン遷移(フェードアウト)
				g_Fade.state = FADE_STATE::FADE_NONE;
				SetSceneWithLoading(g_Fade.scene, g_Fade.loadingVideo);
				
				// useLoadingフラグをリセット
				g_Fade.useLoading = false;
				g_Fade.loadingVideo = nullptr;
			}
			else
			{
				// 通常版シーン遷移
				SetFade((int)g_Fade.frame, g_Fade.fadecolor, FADE_STATE::FADE_IN, g_Fade.scene);
				SetScene(g_Fade.scene);
			}
		}
		break;
	}

	// スプライト表示
	Shader_Begin();

	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

	Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
		0.0f,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		0.0f,
		0.0f,
		1.0f));

	// テクスチャをセット
	g_pContext->PSSetShaderResources(0, 1, &g_Texture);

	// スプライト描画
	SetBlendState(BLENDSTATE_ALPHA);//αブレンド
	XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };
	DrawSprite(pos, size, g_Fade.fadecolor);

	// フェード処理
	switch (g_Fade.state)
	{
		case FADE_STATE::FADE_IN:
			g_Fade.fadecolor.w -= (1.0f / g_Fade.frame);	// 透明にしていく
			break;
		case FADE_STATE::FADE_OUT:
			g_Fade.fadecolor.w += (1.0f / g_Fade.frame);	// 不透明にしていく
			break;
	}


}

void SetFade(int fadeframe, XMFLOAT4 color, FADE_STATE state, SCENE scene)
{
	g_Fade.frame = fadeframe;
	g_Fade.fadecolor = color;
	g_Fade.state = state;
	g_Fade.scene = scene;

	if (g_Fade.state == FADE_IN)
	{
		g_Fade.fadecolor.w = 1.0f;	// 不透明にする
	}
	else
	{
		g_Fade.fadecolor.w = 0.0f;	// 透明にする
	}
}

// ロード画面付きフェード
void SetFadeWithLoading(int fadeframe, XMFLOAT4 color, FADE_STATE state, SCENE scene, const wchar_t* videoPath)
{
	g_Fade.frame = (float)fadeframe;
	g_Fade.fadecolor = color;
	g_Fade.state = state;
	g_Fade.scene = scene;
	g_Fade.useLoading = true;        // ロード画面あり
	g_Fade.loadingVideo = videoPath; // 動画パス

	if (g_Fade.state == FADE_IN)
	{
		g_Fade.fadecolor.w = 1.0f;
	}
	else
	{
		g_Fade.fadecolor.w = 0.0f;
	}
}

FADE_STATE GetFadeState()
{
	return	g_Fade.state;	// 現在の状態
}


