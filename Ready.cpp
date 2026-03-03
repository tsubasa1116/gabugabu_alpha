//======================================================
//	ready.cpp[]
// 
//	制作者：田中佑奈			日付：2024//
//======================================================

//Ready.cpp
#include	"Manager.h"
#include	"sprite.h"
#include	"keyboard.h"

#include	"Ready.h"

#include "fade.h"
#include "swipe.h"
#include "shader.h"
#include "input.h"

#include <chrono>
#include <cmath>
#include "LoadingScreen.h"
#include "loadThread.h"

static	ID3D11ShaderResourceView* g_Texture = NULL;	//テクスチャ１枚を表すオブジェクト
static	ID3D11ShaderResourceView* g_Texture2 = NULL;
static	ID3D11ShaderResourceView* g_Texture3 = NULL;
static	ID3D11ShaderResourceView* g_Texture4 = NULL;
static	ID3D11ShaderResourceView* g_Texture5 = NULL;
static	ID3D11ShaderResourceView* g_Texture6 = NULL;
static	ID3D11ShaderResourceView* g_Texture7 = NULL;
static	ID3D11ShaderResourceView* g_Texture8 = NULL;
static	ID3D11ShaderResourceView* g_Texture9 = NULL;
static	ID3D11ShaderResourceView* g_Texture10 = NULL;
static	ID3D11ShaderResourceView* g_Texture11 = NULL;
static	ID3D11ShaderResourceView* g_Texture12 = NULL;
static	ID3D11ShaderResourceView* g_Texture13 = NULL;
static	ID3D11ShaderResourceView* g_Texture14 = NULL;
static	ID3D11ShaderResourceView* g_Texture15 = NULL;
static	ID3D11ShaderResourceView* g_Texture16 = NULL;
static	ID3D11ShaderResourceView* g_Texture17 = NULL;
static	ID3D11ShaderResourceView* g_Texture18 = NULL;
static	ID3D11ShaderResourceView* g_Texture19 = NULL;

static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

static TexMetadata g_TexMeta2{};
static TexMetadata g_TexMeta3{};
static TexMetadata g_TexMeta4{};
static TexMetadata g_TexMeta5{};
static TexMetadata g_TexMeta6{};
static TexMetadata g_TexMeta7{};
static TexMetadata g_TexMeta8{};
static TexMetadata g_TexMeta9{};
static TexMetadata g_TexMeta10{};
static TexMetadata g_TexMeta11{};
static TexMetadata g_TexMeta12{};
static TexMetadata g_TexMeta13{};
static TexMetadata g_TexMeta14{};
static TexMetadata g_TexMeta15{};
static TexMetadata g_TexMeta16{};
static TexMetadata g_TexMeta17{};
static TexMetadata g_TexMeta18{};
static TexMetadata g_TexMeta19{};

// プレイヤー参加フラグ（一度押したらtrue）
static bool g_PlayerJoined[4] = { false, false, false, false };

// OKポップインアニメーション用
static constexpr float OK_POP_DURATION = 0.4f; // ポップイン所要時間（秒）
static float g_OKPopElapsed[4] = { -1.0f, -1.0f, -1.0f, -1.0f }; // -1 = 非表示

// 準備完了スライドイン用
static constexpr float READY_SLIDE_DURATION = 0.3f; // スライド所要時間（秒）
static float g_ReadySlideElapsed = -1.0f; // -1 = 非表示
static bool g_AllJoinedTriggered = false;  // 全員参加検知済みフラグ

// 準備テキストポップアウト用
static constexpr float TEXT_POPOUT_DURATION = 0.3f; // ポップアウト所要時間（秒）
static float g_TextPopOutElapsed = -1.0f; // -1 = 未開始

// 時間管理
static std::chrono::steady_clock::time_point g_ReadyLastTime;

static bool g_ReadyInitialized = false;
static bool g_IsWarmedUp = false;

// イージング（サインのイーズアウト）
static inline float EaseOutSine(float t) {
	if (t <= 0.0f) return 0.0f;
	if (t >= 1.0f) return 1.0f;
	return sinf(t * 3.14159265f * 0.5f);
}

void Ready_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	if (g_ReadyInitialized) return;

	g_pDevice = pDevice;
	g_pContext = pContext;

	// 参加フラグ初期化
	for (int i = 0; i < 4; i++) g_PlayerJoined[i] = false;

	// OKポップイン初期化
	for (int i = 0; i < 4; i++) g_OKPopElapsed[i] = -1.0f;

	// 準備完了スライド初期化
	g_ReadySlideElapsed = -1.0f;
	g_AllJoinedTriggered = false;
	g_TextPopOutElapsed = -1.0f;
	g_IsWarmedUp = false;

	// 時間初期化
	g_ReadyLastTime = std::chrono::steady_clock::now();

	Loader::AddTask([pDevice]()
		{
			//白テクスチャ読み込み
			{
				TexMetadata		metadata;
				ScratchImage	image;
				LoadFromWICFile(L"asset\\texture\\white.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
				assert(g_Texture);//読み込み失敗時にダイアログを表示
			}

			// 背景読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\ready_color.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture2);
				g_TexMeta2 = metadata;
				assert(g_Texture2);
			}

			// 黒枠読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\blackLine.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture3);
				g_TexMeta3 = metadata;
				assert(g_Texture3);
			}

			// 1Ｐシルエット読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character1OFF.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture4);
				g_TexMeta4 = metadata;
				assert(g_Texture4);
			}

			// ２Ｐシルエット読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character2OFF.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture5);
				g_TexMeta5 = metadata;
				assert(g_Texture5);
			}

			// ３Ｐシルエット読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character3OFF.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture6);
				g_TexMeta6 = metadata;
				assert(g_Texture6);
			}

			// ４Ｐシルエット読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character4OFF.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture7);
				g_TexMeta7 = metadata;
				assert(g_Texture7);
			}

			// プレイヤーナンバー読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\playerNumber.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture8);
				g_TexMeta8 = metadata;
				assert(g_Texture8);
			}

			// 吹き出し読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\hukidashi.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture9);
				g_TexMeta9 = metadata;
				assert(g_Texture9);
			}

			// 吹き出し読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\ready1.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture10);
				g_TexMeta10 = metadata;
				assert(g_Texture10);
			}

			// １Ｐキャラクター読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character1ON.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture11);
				g_TexMeta11 = metadata;
				assert(g_Texture11);
			}

			// ２Ｐキャラクター読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character2ON.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture12);
				g_TexMeta12 = metadata;
				assert(g_Texture12);
			}

			// ３Ｐキャラクター読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character3ON.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture13);
				g_TexMeta13 = metadata;
				assert(g_Texture13);
			}

			// ４Ｐキャラクター読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character4ON.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture14);
				g_TexMeta14 = metadata;
				assert(g_Texture14);
			}

			// １ＰＯＫ読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\OK.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture15);
				g_TexMeta15 = metadata;
				assert(g_Texture15);
			}

			// ２ＰＯＫ読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\OK.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture16);
				g_TexMeta16 = metadata;
				assert(g_Texture16);
			}

			// ３ＰＯＫ読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\OK.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture17);
				g_TexMeta17 = metadata;
				assert(g_Texture17);
			}

			// ４ＰＯＫ読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\OK.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture18);
				g_TexMeta18 = metadata;
				assert(g_Texture18);
			}

			// 準備完了読み込み
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\everyoneOK.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture19);
				g_TexMeta19 = metadata;
				assert(g_Texture19);
			}
		});

		Loader::StartTaskLoad();
		g_ReadyInitialized = true;

}


void Ready_Warmup()
{
	if (!g_pContext) return;

	// 全テクスチャを配列にまとめる
	ID3D11ShaderResourceView* textures[] = {
		g_Texture,  g_Texture2,  g_Texture3,  g_Texture4,  g_Texture5,
		g_Texture6,  g_Texture7,  g_Texture8,  g_Texture9,  g_Texture10,
		g_Texture11, g_Texture12, g_Texture13, g_Texture14, g_Texture15,
		g_Texture16, g_Texture17, g_Texture18, g_Texture19
	};

	for (auto tex : textures)
	{
		if (tex)
		{
			// スロット0にセットして、0ポリゴン描画（バインドを強制する）
			g_pContext->PSSetShaderResources(0, 1, &tex);
			g_pContext->Draw(0, 0);
		}
	}


	// 終わったらスロットを空にしておく
	ID3D11ShaderResourceView* nullSRV = nullptr;
	g_pContext->PSSetShaderResources(0, 1, &nullSRV);
}

void Ready_Finalize()
{
	//テクスチャの解放など
	SAFE_RELEASE(g_Texture);
	SAFE_RELEASE(g_Texture2);
	SAFE_RELEASE(g_Texture3);
	SAFE_RELEASE(g_Texture4);
	SAFE_RELEASE(g_Texture5);
	SAFE_RELEASE(g_Texture6);
	SAFE_RELEASE(g_Texture7);
	SAFE_RELEASE(g_Texture8);
	SAFE_RELEASE(g_Texture9);
	SAFE_RELEASE(g_Texture10);
	SAFE_RELEASE(g_Texture11);
	SAFE_RELEASE(g_Texture12);
	SAFE_RELEASE(g_Texture13);
	SAFE_RELEASE(g_Texture14);
	SAFE_RELEASE(g_Texture15);
	SAFE_RELEASE(g_Texture16);
	SAFE_RELEASE(g_Texture17);
	SAFE_RELEASE(g_Texture18);
	SAFE_RELEASE(g_Texture19);

	g_ReadyInitialized = false;
}

void Ready_Update()
{
	// 時間差分更新
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsed = now - g_ReadyLastTime;
	float dt = elapsed.count();
	g_ReadyLastTime = now;

	// プレイヤー参加判定（押すたびにポップインリスタート）
	if (Keyboard_IsKeyDownTrigger(KK_D1) || (g_Input[0].A)) { g_PlayerJoined[0] = true; g_OKPopElapsed[0] = 0.0f; }
	if (Keyboard_IsKeyDownTrigger(KK_D2) || (g_Input[1].A)) { g_PlayerJoined[1] = true; g_OKPopElapsed[1] = 0.0f; }
	if (Keyboard_IsKeyDownTrigger(KK_D3) || (g_Input[2].A)) { g_PlayerJoined[2] = true; g_OKPopElapsed[2] = 0.0f; }
	if (Keyboard_IsKeyDownTrigger(KK_D4) || (g_Input[3].A)) { g_PlayerJoined[3] = true; g_OKPopElapsed[3] = 0.0f; }

	// OKポップインタイマー進行
	for (int i = 0; i < 4; i++)
	{
		if (g_OKPopElapsed[i] >= 0.0f)
			g_OKPopElapsed[i] += dt;
	}

	// 全員参加したら準備完了スライド開始 & テキストポップアウト開始
	if (!g_AllJoinedTriggered &&
		g_PlayerJoined[0] && g_PlayerJoined[1] &&
		g_PlayerJoined[2] && g_PlayerJoined[3])
	{
		g_AllJoinedTriggered = true;
		g_ReadySlideElapsed = 0.0f;
		g_TextPopOutElapsed = 0.0f;
	}

	// 準備完了スライドタイマー進行
	if (g_ReadySlideElapsed >= 0.0f)
		g_ReadySlideElapsed += dt;

	// テキストポップアウトタイマー進行
	if (g_TextPopOutElapsed >= 0.0f)
		g_TextPopOutElapsed += dt;

	//キー入力チェック
	//スタートボタンが押されたらシーンを切り替え
	//フェード処理中はキーを受け付けない
	if ((Keyboard_IsKeyDownTrigger(KK_ENTER) || (g_Input->X) )&& (GetFadeState() == FADE_NONE) && !IsLoading())
	{
		if(g_AllJoinedTriggered)
		{
			XMFLOAT4 color(0.0f, 0.0f, 0.0f, 1.0f);
			SetFadeWithLoading(40, color, FADE_OUT, SCENE_GAME, L"asset\\movie\\gameLoad.mp4");
		}
	}
	
}

void Ready_Draw()
{
	if (!Loader::IsFinished) return;

	if (!g_IsWarmedUp)
	{
		Ready_Warmup();
		g_IsWarmedUp = true;
	}

	// シェーダーを描画パイプラインに設定
	Shader_Begin();

	// 画面サイズ取得
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

	// 頂点シェーダーに変換行列を設定
	Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
		0.0f,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		0.0f,
		0.0f,
		1.0f));

	// 白描画
	if (g_Texture)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture);//g_Textureを使うように設定する
		SetBlendState(BLENDSTATE_NONE);//ブレンド無し
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };	//スプライトの色
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
		XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };
		DrawSprite(pos, size, col);//1枚絵を表示
	}

	// 背景描画
	if (g_Texture2)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture2);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta2.width * 0.08f, (float)g_TexMeta2.height * 0.08f };
		DrawSprite(pos, size, col);
	}

	// プレイヤーナンバー描画
	if (g_Texture8)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture8);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 110 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta8.width * 0.08f, (float)g_TexMeta8.height * 0.08f };
		DrawSprite(pos, size, col);
	}

	// １Ｐシルエット描画
	if (g_Texture4)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture4);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2 + 50, SCREEN_HEIGHT / 2 + 30 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta4.width * 0.08f, (float)g_TexMeta4.height * 0.08f };
		DrawSprite(pos, size, col);
	}

	// ２Ｐシルエット描画
	if (g_Texture5)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture5);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2 , SCREEN_HEIGHT / 2 + 33 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta5.width * 0.08f, (float)g_TexMeta5.height * 0.08f };
		DrawSprite(pos, size, col);
	}

	// ３Ｐシルエット描画
	if (g_Texture6)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture6);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 33 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta6.width * 0.08f, (float)g_TexMeta6.height * 0.08f };
		DrawSprite(pos, size, col);
	}

	// ４Ｐシルエット描画
	if (g_Texture7)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture7);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 33 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta7.width * 0.08f, (float)g_TexMeta7.height * 0.08f };
		DrawSprite(pos, size, col);
	}

	// １Ｐ描画（参加済みなら常に表示）
	if (g_PlayerJoined[0] && g_Texture11)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture11);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 30 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta11.width * 0.08f, (float)g_TexMeta11.height * 0.08f };
		DrawSprite(pos, size, col);
	}

	// ２Ｐ描画（参加済みなら常に表示）
	if (g_PlayerJoined[1] && g_Texture12)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture12);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2 , SCREEN_HEIGHT / 2 + 33 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta12.width * 0.08f, (float)g_TexMeta12.height * 0.08f };
		DrawSprite(pos, size, col);
	}

	// ３Ｐ描画（参加済みなら常に表示）
	if (g_PlayerJoined[2] && g_Texture13)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture13);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 25 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta13.width * 0.08f, (float)g_TexMeta13.height * 0.08f };
		DrawSprite(pos, size, col);
	}

	// ４Ｐ描画（参加済みなら常に表示）
	if (g_PlayerJoined[3] && g_Texture14)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture14);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 28 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta14.width * 0.08f, (float)g_TexMeta14.height * 0.08f };
		DrawSprite(pos, size, col);
	}

	// 黒枠描画
	if (g_Texture3)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture3);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta3.width * 0.08f, (float)g_TexMeta3.height * 0.08f };
		DrawSprite(pos, size, col);
	}

	// 準備ができたらボタンを押してね描画（全員参加で右へポップアウト）
	if (g_Texture10)
	{
		if (g_TextPopOutElapsed >= 0.0f)
		{
			float t = g_TextPopOutElapsed / TEXT_POPOUT_DURATION;
			if (t > 1.0f) t = 1.0f;
			float e = EaseOutSine(t);

			// 中央から右画面外へ
			float startX = SCREEN_WIDTH / 2;
			float endX = SCREEN_WIDTH * 1.5f;
			float posX = startX + (endX - startX) * e;
			float alpha = 1.0f - e;

			// 完全に消えたら描画しない
			if (t < 1.0f)
			{
				g_pContext->PSSetShaderResources(0, 1, &g_Texture10);
				SetBlendState(BLENDSTATE_ALPHA);
				XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, alpha };
				XMFLOAT2 pos = { posX, SCREEN_HEIGHT / 2 + 4 };
				XMFLOAT2 size = { (float)g_TexMeta9.width * 0.08f, (float)g_TexMeta9.height * 0.012f };
				DrawSprite(pos, size, col);
			}
		}
		else
		{
			// 通常表示
			g_pContext->PSSetShaderResources(0, 1, &g_Texture10);
			SetBlendState(BLENDSTATE_ALPHA);
			XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
			XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 4 };
			XMFLOAT2 size = { (float)g_TexMeta9.width * 0.08f, (float)g_TexMeta9.height * 0.012f };
			DrawSprite(pos, size, col);
		}
	}

	// 吹き出し描画
	if (g_Texture9)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture9);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 60 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta9.width * 0.078f, (float)g_TexMeta7.height * 0.078f };
		DrawSprite(pos, size, col);
	}

	// 描画
	if (g_Texture9)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture9);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 60 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta9.width * 0.078f, (float)g_TexMeta7.height * 0.078f };
		DrawSprite(pos, size, col);
	}

	// 吹き出し描画
	if (g_Texture9)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture9);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 60 }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta9.width * 0.078f, (float)g_TexMeta7.height * 0.078f };
		DrawSprite(pos, size, col);
	}

	// OKテクスチャ・位置の定義（1P?4P）
	ID3D11ShaderResourceView* okTextures[4] = { g_Texture15, g_Texture16, g_Texture17, g_Texture18 };
	TexMetadata* okMetas[4] = { &g_TexMeta15, &g_TexMeta16, &g_TexMeta17, &g_TexMeta18 };
	XMFLOAT2 okPositions[4] = {
		{ SCREEN_WIDTH / 2 - 505, SCREEN_HEIGHT / 2 - 250 },
		{ SCREEN_WIDTH / 2 + 505, SCREEN_HEIGHT / 2 - 250 },
		{ SCREEN_WIDTH / 2 - 515, SCREEN_HEIGHT / 2 + 130 },
		{ SCREEN_WIDTH / 2 + 510, SCREEN_HEIGHT / 2 + 130 }
	};

	// 準備完了描画（全員参加で左からスライドイン）
	if (g_ReadySlideElapsed >= 0.0f && g_Texture19)
	{
		float t = g_ReadySlideElapsed / READY_SLIDE_DURATION;
		if (t > 1.0f) t = 1.0f;
		float e = EaseOutSine(t);

		// 左画面外からスライドイン
		float startX = -SCREEN_WIDTH * 0.3f;
		float endX = SCREEN_WIDTH / 2;
		float posX = startX + (endX - startX) * e;

		g_pContext->PSSetShaderResources(0, 1, &g_Texture19);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, e };
		XMFLOAT2 pos = { posX, SCREEN_HEIGHT / 2 + 4 };
		XMFLOAT2 size = { (float)g_TexMeta9.width * 0.08f, (float)g_TexMeta9.height * 0.012f };
		DrawSprite(pos, size, col);
	}

	// １Ｐ～４ＰＯＫ描画（ポップイン付き）
	for (int i = 0; i < 4; i++)
	{
		if (g_OKPopElapsed[i] < 0.0f || !okTextures[i]) continue;

		float t = g_OKPopElapsed[i] / OK_POP_DURATION;
		if (t > 1.0f) t = 1.0f;
		float e = EaseOutSine(t);

		float okScale = 0.5f + 0.5f * e;  // 0.5 → 1.0
		float okAlpha = e;                 // 0 → 1

		float baseScaleX = (i == 3) ? 0.078f : 0.08f; // 4Pだけ元のスケールが違う
		float baseScaleY = (i == 3) ? 0.078f : 0.08f;

		XMFLOAT2 baseSize = {
			(float)okMetas[i]->width * baseScaleX,
			(float)okMetas[i]->height * baseScaleY
		};
		XMFLOAT2 size = { baseSize.x * okScale, baseSize.y * okScale };

		g_pContext->PSSetShaderResources(0, 1, &okTextures[i]);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, okAlpha };
		DrawSprite(okPositions[i], size, col);
	}
}