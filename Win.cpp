//======================================================
//	Win.cpp[]
// 
//	制作者：田中佑奈			日付：2026//
//======================================================

#include "Manager.h"
#include "sprite.h"
#include "keyboard.h"
#include "Win.h"
#include "fade.h"
#include "swipe.h"
#include "shader.h"
#include "player.h"
#include "model.h"

static ID3D11ShaderResourceView* g_Texture = NULL;		// 背景
static ID3D11ShaderResourceView* g_Texture2 = NULL;		// ストライプ
static ID3D11ShaderResourceView* g_Texture3 = NULL;		// PLAYER WIN
static ID3D11ShaderResourceView* g_Texture4 = NULL;		// 次へ
static ID3D11ShaderResourceView* g_WinTex = nullptr;    // 既存モデル用テクスチャ
static ID3D11ShaderResourceView* g_Texture5 = NULL;		// 王冠
static ID3D11ShaderResourceView* g_Texture6 = NULL;		// アニメーしぃん

static TexMetadata g_WinTexMeta{};
static TexMetadata g_TexMeta2{}; // ストライプ
static TexMetadata g_TexMeta3{}; // PLAYER WIN
static TexMetadata g_TexMeta4{}; // 次へ
static TexMetadata g_TexMeta5{}; // 王冠
static TexMetadata g_TexMeta6{}; // アニメーしぃん

static MODEL* g_WinModel = nullptr;						// 既存モデル

static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

static float g_SlideOffsetTop = 0.0f;
static float g_SlideOffsetBottom = 0.0f;
const float SLIDE_SPEED = 2.0f;	

const int ANIM_START = 8;
const int ANIM_END = 15;
const float ANIM_SPEED = 0.12f; // 1フレームあたりの秒数（お好みで調整）
static int g_AnimFrame = ANIM_START;
static float g_AnimTimer = 0.0f;

//======================================================
//	初期化関数
//======================================================
void Win_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 背景テクスチャ読み込み
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(L"asset\\texture\\uiWinRord_v1.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
		assert(g_Texture);//読み込み失敗時にダイアログを表示
	}

	// ストライプテクスチャ読み込み
	{
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICFile(L"asset\\texture\\uiWinBand1P_v1.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture2);
		g_TexMeta2 = metadata;
		assert(g_Texture2);
	}

	// PLAYER WINテクスチャ読み込み
	{
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICFile(L"asset\\texture\\uiWinText1P_v1.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture3);
		g_TexMeta3 = metadata;
		assert(g_Texture3);
	}

	// 次へテクスチャ読み込み
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(L"asset\\texture\\next.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture4);
		g_TexMeta4 = metadata;
		assert(g_Texture4);//読み込み失敗時にダイアログを表示
	}

	// 王冠テクスチャ読み込み
	{
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICFile(L"asset\\texture\\oukan.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture5);
		g_TexMeta5 = metadata;
		assert(g_Texture5);
	}

	// アニメ―しぃん
	{
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICFile(L"asset\\texture\\characterWin01_v2.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture6);
		g_TexMeta6 = metadata;
		assert(g_Texture6);
	}

	// フェードインのセット
	XMFLOAT4	color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	SetFade(60.0f, color, FADE_IN, SCENE_GAME);
}

//======================================================
//	終了処理関数
//======================================================
void Win_Finalize()
{
	// テクスチャの解放など
	SAFE_RELEASE(g_Texture);
	SAFE_RELEASE(g_Texture2);
	SAFE_RELEASE(g_Texture3);
	SAFE_RELEASE(g_Texture4);
	SAFE_RELEASE(g_Texture5);
	SAFE_RELEASE(g_Texture6);
	SAFE_RELEASE(g_WinTex);

	// モデルの解放（モデル解放関数があればそちらを使用）
	if (g_WinModel)
	{
		// プロジェクトのモデル解放関数に置き換えてください
		// 例: UnloadModel(g_WinModel);
		g_WinModel = nullptr;
	}

	// ポインタリセット（借りているだけなのでReleaseはしない）
	g_pDevice = nullptr;
	g_pContext = nullptr;

	// アニメーション状態リセット
	g_AnimFrame = ANIM_START;
	g_AnimTimer = 0.0f;
	g_SlideOffsetTop = 0.0f;
	g_SlideOffsetBottom = 0.0f;
}

//======================================================
//	更新処理
//======================================================
void Win_Update()
{
	// キー入力チェック
	if (Keyboard_IsKeyDownTrigger(KK_ENTER) && (GetFadeState() == FADE_NONE))
	{
		XMFLOAT4 color(0.0f, 0.0f, 0.0f, 1.0f);
		SetFade(40.0f, color, FADE_OUT, SCENE_RESULT);
	}

	// スライドオフセット更新（正規化しない。描画側で各自fmodfする）
	g_SlideOffsetTop += SLIDE_SPEED;
	g_SlideOffsetBottom -= SLIDE_SPEED;

	// オーバーフロー防止のみ（十分大きい値でリセット）
	const float WRAP_LIMIT = 100000.0f;
	if (g_SlideOffsetTop > WRAP_LIMIT) g_SlideOffsetTop -= WRAP_LIMIT;
	if (g_SlideOffsetBottom < -WRAP_LIMIT) g_SlideOffsetBottom += WRAP_LIMIT;

	g_AnimTimer += DELTA_TIME;
	if (g_AnimTimer >= ANIM_SPEED)
	{
		g_AnimTimer = 0.0f;
		if (g_AnimFrame < ANIM_END)	g_AnimFrame++;
	}
}

//======================================================
//	描画関数
//======================================================
void Win_Draw()
{
	// シェーダーを描画パイプラインに設定
	Shader_Begin();

	// 画面サイズ取得
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

	// 頂点シェーダーに変換行列を設定（UI用：直交投影）
	Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
		0.0f,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		0.0f,
		0.0f,
		1.0f));

	// 背景描画
	if (g_Texture)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture);//g_Textureを使うように設定する
		SetBlendState(BLENDSTATE_NONE);//ブレンド無し
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };	//スプライトの色
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
		XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };
		DrawSprite(pos, size, col);//1枚絵を表示
	}

	// ストライプ描画
	if (g_Texture2)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture2);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT }; // 位置はお好みで
		XMFLOAT2 size = { (float)g_TexMeta2.width, (float)g_TexMeta2.height };
		DrawSprite(pos, size, col);
	}

	// バナー縮小率
	const float BANNER_SCALE = 0.7f;

	// バナーの高さ・位置
	float bannerYTop = (float)g_TexMeta2.height * BANNER_SCALE / 2.0f;
	float bannerYBottom = SCREEN_HEIGHT - (float)g_TexMeta2.height * BANNER_SCALE / 2.0f;

	// 上バナー（右スライド）
	if (g_Texture2)
		DrawSlidingBanner(g_Texture2, bannerYTop, g_SlideOffsetTop, (float)g_TexMeta2.width * BANNER_SCALE, (float)g_TexMeta2.height * BANNER_SCALE);
	if (g_Texture3 && g_Texture5)
		DrawPlayerWinCrownBanner(g_Texture3, g_TexMeta3, g_Texture5, g_TexMeta5, bannerYTop, g_SlideOffsetTop, BANNER_SCALE);

	// 下バナー（左スライド）
	if (g_Texture2)
		DrawSlidingBanner(g_Texture2, bannerYBottom, g_SlideOffsetBottom, (float)g_TexMeta2.width * BANNER_SCALE, (float)g_TexMeta2.height * BANNER_SCALE);
	if (g_Texture3 && g_Texture5)
		DrawPlayerWinCrownBanner(g_Texture3, g_TexMeta3, g_Texture5, g_TexMeta5, bannerYBottom, g_SlideOffsetBottom, BANNER_SCALE);

	// 次へ描画
	if (g_Texture4)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture4);//g_Textureを使うように設定する
		SetBlendState(BLENDSTATE_ALPHA);//ブレンド無し
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };	//スプライトの色
		XMFLOAT2 pos = { SCREEN_WIDTH - 80, SCREEN_HEIGHT  - 180 };
		XMFLOAT2 size = { (float)g_TexMeta4.width, (float)g_TexMeta4.height };
		DrawSprite(pos, size, col);//1枚絵を表示
	}

	if (g_Texture6)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture6); 
		
		int frame = g_AnimFrame; // 8～15
		int framesPerRow = 8;
		int frameX = frame % framesPerRow; // 0～7
		int frameY = frame / framesPerRow; // 1

		float frameWidth = (float)g_TexMeta6.width / 8.0f;
		float frameHeight = (float)g_TexMeta6.height / 8.0f;

		float u0 = frameX * frameWidth / g_TexMeta6.width;
		float v0 = frameY * frameHeight / g_TexMeta6.height;
		float u1 = (frameX + 1) * frameWidth / g_TexMeta6.width;
		float v1 = (frameY + 1) * frameHeight / g_TexMeta6.height;

		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 250 };
		XMFLOAT2 size = { frameWidth * 1.3f, frameHeight * 1.3f }; // 拡大例
		XMFLOAT4 col = { 1, 1, 1, 1 };

		DrawSpriteUV(pos, size, col, XMFLOAT2(u0, v0), XMFLOAT2(u1, v1));
	}

	// スプライトの描画は第1形態ブレンド無しかアルファにする（既存スタイルに合わせる）
	SetBlendState(BLENDSTATE_NONE);
}

void DrawSlidingBanner(ID3D11ShaderResourceView* tex, float y, float offset, float width, float height)
{
	XMFLOAT2 size = { width, height };
	XMFLOAT4 col = { 1, 1, 1, 1 };
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// 画面幅取得
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();

	// この関数の繰り返し単位(width)で正規化
	float normOffset = fmodf(offset, width);
	// 開始位置を画面左端より手前に配置
	float startX = normOffset - width;

	// 画面全体をカバーするまで繰り返し描画
	for (float x = startX; x < SCREEN_WIDTH + width; x += width)
	{
		XMFLOAT2 pos = { x + width / 2, y };
		DrawSprite(pos, size, col);
	}
}

void DrawPlayerWinCrownBanner(
	ID3D11ShaderResourceView* texPlayerWin, TexMetadata& metaPlayerWin,
	ID3D11ShaderResourceView* texCrown, TexMetadata& metaCrown,
	float y, float offset, float scale)
{
	float widthPlayerWin = (float)metaPlayerWin.width * scale;
	float heightPlayerWin = (float)metaPlayerWin.height * scale;
	float widthCrown = (float)metaCrown.width * scale / 10;
	float heightCrown = (float)metaCrown.height * scale / 10;

	// 間隔
	float spacing = 50.0f;

	// 1セットの幅（PLAYER WIN + spacing + 王冠 + spacing）
	float setWidth = widthPlayerWin + spacing + widthCrown + spacing;

	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();

	// この関数の繰り返し単位(setWidth)で正規化
	float normOffset = fmodf(offset, setWidth);
	// 開始位置を画面左端より手前に配置
	float startX = normOffset - setWidth;

	for (float x = startX; x < SCREEN_WIDTH + setWidth; x += setWidth)
	{
		// PLAYER WIN
		g_pContext->PSSetShaderResources(0, 1, &texPlayerWin);
		XMFLOAT2 posPW = { x + widthPlayerWin / 2, y };
		XMFLOAT2 sizePW = { widthPlayerWin, heightPlayerWin };
		XMFLOAT4 col = { 1, 1, 1, 1 };
		DrawSprite(posPW, sizePW, col);

		// 王冠
		g_pContext->PSSetShaderResources(0, 1, &texCrown);
		XMFLOAT2 posCrown = { x + widthPlayerWin + spacing + widthCrown / 2, y };
		XMFLOAT2 sizeCrown = { widthCrown, heightCrown };
		DrawSprite(posCrown, sizeCrown, col);
	}
}