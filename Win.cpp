//======================================================
//	Win.cpp[]
// 
//	制作者：田中佑奈			日付：2026/03/03
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
#include "input.h"
#include "color.h"

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

// アニメーション設定（16×16シート：16列×16行、ただし12行使用）
const int ANIM_COLS = 16;       // 1行あたりのコマ数
const int ANIM_ROWS = 16;       // シート全体の行数（実際使うのは12行）
const int ANIM_PLAY_START = 0;  // 再生開始コマ
const int ANIM_PLAY_END = 15;   // 再生終了コマ
const float ANIM_SPEED = 0.12f; // 1フレームあたりの秒数
static int g_AnimFrame = ANIM_PLAY_START;
static float g_AnimTimer = 0.0f;
static int g_AnimRow = 0;       // 再生する行番号

// 行ごとのループ開始コマ
// 行0～7（第2・第3形態）: コマ6～15をループ
// 行8～11（第1形態）:     コマ5～15をループ
static inline int GetLoopStart(int row)
{
	return (row >= 8) ? 5 : 6;
}

// 勝者情報
static int g_WinnerIndex = -1;
static PlayerType g_WinnerType = PlayerType::None;
static Form g_WinnerForm = Form::First;

//======================================================
//	勝者検索関数
//======================================================
static int FindWinner()
{
	for (int i = 0; i < 4; i++)
	{
		PLAYEROBJECT* p = GetPlayer(i);
		if (p == nullptr) continue;

		if (p->rank == 1)
		{
			return i;
		}
	}
	return -1;
}

//======================================================
//	勝者に応じたテクスチャパスを取得
//======================================================
// プレイヤー番号に応じた背景パス
static const wchar_t* GetWinBgPath(int winnerIndex)
{
	// 勝者のプレイヤー番号に応じた背景テクスチャ
	switch (winnerIndex)
	{
	case 0:  return L"asset\\texture\\uiWinRord_v1.png";  // 1P用
	case 1:  return L"asset\\texture\\uiWinRordBlue_v1.png";  // 2P用
	case 2:  return L"asset\\texture\\uiWinRordYellow_v1.png";  // 3P用
	case 3:  return L"asset\\texture\\uiWinRordGreen_v1.png";  // 4P用
	default: return L"asset\\texture\\uiWinRord_v1.png";
	}
}

// プレイヤー番号に応じたストライプパス
static const wchar_t* GetWinBandPath(int winnerIndex)
{
	switch (winnerIndex)
	{
	case 0:  return L"asset\\texture\\uiWinBand1P_v1.png";
	case 1:  return L"asset\\texture\\uiWinBand2P_v1.png";
	case 2:  return L"asset\\texture\\uiWinBand3P_v1.png";
	case 3:  return L"asset\\texture\\uiWinBand4P_v1.png";
	default: return L"asset\\texture\\uiWinBand1P_v1.png";
	}
}

// プレイヤー番号に応じたPLAYER WINテキストパス
static const wchar_t* GetWinTextPath(int winnerIndex)
{
	switch (winnerIndex)
	{
	case 0:  return L"asset\\texture\\uiWinText1P_v1.png";
	case 1:  return L"asset\\texture\\uiWinText2P_v4.png";
	case 2:  return L"asset\\texture\\uiWinText3P_v3.png";
	case 3:  return L"asset\\texture\\uiWinText4P_v3.png";
	default: return L"asset\\texture\\uiWinText1P_v1.png";
	}
}

//======================================================
//	形態・属性・プレイヤー番号からスプライトシートの行番号を取得
//
//	【characterWinNew_v1.png（64×64シート）】
//	行0:  コンクリ 第2形態
//	行1:  電気     第2形態
//	行2:  ガラス   第2形態
//	行3:  植物     第2形態
//	行4:  コンクリ 第3形態
//	行5:  電気     第3形態
//	行6:  ガラス   第3形態
//	行7:  植物     第3形態
//	行8:  1P 第1形態
//	行9:  2P 第1形態
//	行10: 3P 第1形態
//	行11: 4P 第1形態
//======================================================
static int GetAnimRow(Form form, PlayerType type, int winnerIndex)
{
	if (form == Form::First)
	{
		// 第1形態: 行8=1P, 行9=2P, 行10=3P, 行11=4P
		switch (winnerIndex)
		{
		case 0: return 8;  // 1P
		case 1: return 9;  // 2P
		case 2: return 10; // 3P
		case 3: return 11; // 4P
		default: return 8;
		}
	}

	// 属性のベース行
	int baseRow = 0;
	switch (type)
	{
	case PlayerType::Concrete:    baseRow = 0; break;
	case PlayerType::Electricity: baseRow = 1; break;
	case PlayerType::Glass:       baseRow = 2; break;
	case PlayerType::Plant:       baseRow = 3; break;
	default:                      baseRow = 0; break;
	}

	// 第2形態 → +0、第3形態 → +4
	if (form == Form::Third)
		baseRow += 4;

	return baseRow;
}

//======================================================
//	初期化関数
//======================================================
void Win_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 勝者を特定
	g_WinnerIndex = FindWinner();
	if (g_WinnerIndex >= 0)
	{
		PLAYEROBJECT* winner = GetPlayer(g_WinnerIndex);
		if (winner)
		{
			g_WinnerType = winner->type;
			g_WinnerForm = winner->form;
		}
	}

	// 形態・属性・プレイヤー番号に応じたアニメーション行を決定
	g_AnimRow = GetAnimRow(g_WinnerForm, g_WinnerType, g_WinnerIndex);

	// 勝者に応じたテクスチャパスを決定
	const wchar_t* bgPath = GetWinBgPath(g_WinnerIndex);
	const wchar_t* bandPath = GetWinBandPath(g_WinnerIndex);
	const wchar_t* textPath = GetWinTextPath(g_WinnerIndex);

	// 背景テクスチャ読み込み
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(bgPath, WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
		assert(g_Texture);//読み込み失敗時にダイアログを表示
	}

	// ストライプテクスチャ読み込み
	{
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICFile(bandPath, WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture2);
		g_TexMeta2 = metadata;
		assert(g_Texture2);
	}

	// PLAYER WINテクスチャ読み込み
	{
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICFile(textPath, WIC_FLAGS_NONE, &metadata, image);
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

	// アニメーション（1枚に統合されたシート）
	{
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICFile(L"asset\\texture\\characterWinNew_v1.png", WIC_FLAGS_NONE, &metadata, image);
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
	g_AnimFrame = ANIM_PLAY_START;
	g_AnimTimer = 0.0f;
	g_AnimRow = 0;
	g_SlideOffsetTop = 0.0f;
	g_SlideOffsetBottom = 0.0f;

	// 勝者情報リセット
	g_WinnerIndex = -1;
	g_WinnerType = PlayerType::None;
	g_WinnerForm = Form::First;
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
		g_AnimFrame++;

		// コマ15を超えたらループ開始コマに戻る
		if (g_AnimFrame > ANIM_PLAY_END)
		{
			g_AnimFrame = GetLoopStart(g_AnimRow);
		}
	}
}

//======================================================
//	描画関数
//======================================================
void Win_Draw()
{
	// シェーダーを描画パイプラインに設定
	Shader_Begin();

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
	const float BANNER_SCALE = 0.85f;

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
		XMFLOAT2 pos = { SCREEN_WIDTH - 80, SCREEN_HEIGHT - 230 };
		XMFLOAT2 size = { (float)g_TexMeta4.width, (float)g_TexMeta4.height };
		DrawSprite(pos, size, col);//1枚絵を表示
	}

	// キャラアニメーション描画
	if (g_Texture6)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture6);

		int frameX = g_AnimFrame; // 列 (0～15)
		int frameY = g_AnimRow;   // 行 (形態・属性・プレイヤー番号で決定済み)

		float frameWidth = (float)g_TexMeta6.width / (float)ANIM_COLS;
		float frameHeight = (float)g_TexMeta6.height / (float)ANIM_ROWS;

		float u0 = (float)frameX / (float)ANIM_COLS;
		float v0 = (float)frameY / (float)ANIM_ROWS;
		float u1 = (float)(frameX + 1) / (float)ANIM_COLS;
		float v1 = (float)(frameY + 1) / (float)ANIM_ROWS;

		// 形態ごとのサイズ・位置を設定
		XMFLOAT2 pos;
		XMFLOAT2 size;

		switch (g_WinnerForm)
		{
		case Form::First:
			pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 28.0f * SCREEN_ADJUST_Y };
			size = { frameWidth * 1.5f, frameHeight * 1.5f };
			break;

		case Form::Second:
			pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 140.0f * SCREEN_ADJUST_Y };
			size = { frameWidth * 1.9f, frameHeight * 1.9f };
			break;

		case Form::Third:
			pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 10.0f * SCREEN_ADJUST_Y };
			size = { frameWidth * 1.4f, frameHeight * 1.4f };
			break;

		default:
			pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 30.0f * SCREEN_ADJUST_Y };
			size = { frameWidth * 1.2f, frameHeight * 1.2f };
			break;
		}

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