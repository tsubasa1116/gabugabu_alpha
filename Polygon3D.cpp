// =====================================================
//	polygon3D.cpp[]
// 
//	制作者：平岡颯馬			日付：2025/12/16
//======================================================

#include <d3d11.h>
#include <iostream>
#include "DirectXMath.h"
using namespace DirectX;

#include "direct3d.h"
#include "shader.h"
#include "keyboard.h"
#include "sprite.h"
#include "color.h"
#include "hp.h"
#include "gauge.h"
#include "Effect.h"
#include "polygon3D.h"
#include "Camera.h"
#include "input.h"
#include "skill.h"
#include "special.h"
#include "field.h"
#include "collider.h"
#include "debug_render.h"
#include "debug_ostream.h"
#include "attack.h" 

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

//======================================================
//	マクロ定義
//======================================================
#define	NUM_VERTEX		(6)
#define HPBER_SIZE_X	(270.0f)	// HPバーのサイズ
#define HPBER_SIZE_Y	(270.0f)	// 〃
#define GAUGE_POS_X		(77.0f)		// HPバーを基準としたゲージの位置調整
#define GAUGE_POS_Y		(36.0f)		// 〃

//======================================================
//	構造体宣言
//======================================================
// オブジェクト
PLAYEROBJECT object[PLAYER_MAX];

//======================================================
//	グローバル変数
//======================================================
static ID3D11Device* g_pDevice = NULL;
static ID3D11DeviceContext* g_pContext = NULL;
static HP HPBar[PLAYER_MAX];

// 頂点バッファ
static ID3D11Buffer* g_VertexBuffer = NULL;

// インデックスバッファ
static ID3D11Buffer* g_IndexBuffer = NULL;

// テクスチャ変数
static ID3D11ShaderResourceView* g_Texture[25];

// エフェクト デバッグ用タイマー
static float g_effectElapsed = 0.0f; // 秒単位での経過時間を保持

// プレイヤー アニメーション用変数
static int   g_animFrame[PLAYER_MAX];
static float g_animTimer[PLAYER_MAX];
static const float ANIM_FRAME_TIME = 0.15f; // 1フレームあたりの秒数
static const int   SHEET_COLS = 16;
static const int   SHEET_ROWS = 16;

static int g_victoryState[PLAYER_MAX] = { 0 };			// 0=なし, 1=初回(208～220) 再生中, 2=ループ(216～220)
static float g_downHoldTimer[PLAYER_MAX] = { 0.0f };	// 最終フレームホールド用タイマー（プレイヤー毎）

//static	Vertex vdata[NUM_VERTEX] =
//{
//	//-Z面
//	{//頂点0 LEFT-TOP
//		XMFLOAT3(-0.5f, 0.5f, -0.5f),		//座標
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),	//カラー
//		XMFLOAT2(0.0f,0.0f)					//テクスチャ座標
//	},
//	{//頂点1 RIGHT-TOP
//		XMFLOAT3(0.5f, 0.5f, -0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(1.0f,0.0f)
//	},
//	{//頂点2 LEFT-BOTTOM
//		XMFLOAT3(-0.5f, -0.5f, -0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(0.0f,1.0f)
//	},
//	{//頂点3 RIGHT-BOTTOM
//		XMFLOAT3(0.5f, -0.5f, -0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(1.0f,1.0f)
//	},
//
//	//+X面
//	{//頂点4 LEFT-TOP
//		XMFLOAT3(0.5f, 0.5f, -0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(0.0f,0.0f)
//	},
//	{//頂点5 RIGHT-TOP
//		XMFLOAT3(0.5f, 0.5f, 0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(1.0f,0.0f)
//	},
//	{//頂点6 LEFT-BOTTOM
//		XMFLOAT3(0.5f, -0.5f, -0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(0.0f,1.0f)
//	},
//	{//頂点7 RIGHT-BOTTM
//		XMFLOAT3(0.5f, -0.5f, 0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(1.0f,1.0f)
//	},
//
//	//+Z面
//	{//頂点8 LEFT-TOP
//		XMFLOAT3(0.5f, 0.5f, 0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(0.0f,0.0f)
//	},
//	{//頂点9 RIGHT-TOP
//		XMFLOAT3(-0.5f, 0.5f, 0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(1.0f,0.0f)
//	},
//	{//頂点10 LEFT-BOTTOM
//		XMFLOAT3(0.5f, -0.5f, 0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(0.0f,1.0f)
//	},
//	{//頂点11 RIGHT-BOTTOM
//		XMFLOAT3(-0.5f, -0.5f, 0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(1.0f,1.0f)
//	},
//
//	//-X面
//	{//頂点12 LEFT-TOP
//		XMFLOAT3(-0.5f, 0.5f, 0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(0.0f,0.0f)
//	},
//	{//頂点13 RIGHT-TOP
//		XMFLOAT3(-0.5f, 0.5f, -0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(1.0f,0.0f)
//	},
//	{//頂点14 LEFT-BOTTOM
//		XMFLOAT3(-0.5f, -0.5f, 0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(0.0f,1.0f)
//	},
//	{//頂点15 RIGHT-BOTTOM
//		XMFLOAT3(-0.5f, -0.5f, -0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(1.0f,1.0f)
//	},
//
//	//+Y面
//	{//頂点16 LEFT-TOP
//		XMFLOAT3(-0.5f, 0.5f, 0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(0.0f,0.0f)
//	},
//	{//頂点17 RIGHT-TOP
//		XMFLOAT3(0.5f, 0.5f, 0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(1.0f,0.0f)
//	},
//	{//頂点18 LEFT-BOTTOM
//		XMFLOAT3(-0.5f, 0.5f, -0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(0.0f,1.0f)
//	},
//	{//頂点19 RIGHT-BOTTOM
//		XMFLOAT3(0.5f, 0.5f, -0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(1.0f,1.0f)
//	},
//
//	//-Y面
//	{//頂点20 LEFT-TOP
//		XMFLOAT3(-0.5f, -0.5f, -0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(0.0f,0.0f)
//	},
//	{//頂点21 RIGHT-TOP
//		XMFLOAT3(0.5f, -0.5f, -0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(1.0f,0.0f)
//	},
//	{//頂点22 LEFT-BOTTOM
//		XMFLOAT3(-0.5f, -0.5f, 0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(0.0f,1.0f)
//	},
//	{//頂点23 RIGHT-BOTTOM
//		XMFLOAT3(0.5f, -0.5f, 0.5f),
//		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
//		XMFLOAT2(1.0f,1.0f)
//	},
//};

#define COORDINATE	(0.5f)	// デフォルト (0.5f)
#define TEXCOORD	(1.0f)	// デフォルト (1.0f)

// 頂点配列
static Vertex2 vdata[NUM_VERTEX] =
{
	{// 頂点0 LEFT-TOP
		XMFLOAT3(-COORDINATE, COORDINATE, 0.0f),// 座標
		XMFLOAT3(0.0f, 0.0f, -1.0f),			// 法線ベクトル
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),		// カラー
		XMFLOAT2(0.0f, 0.0f)					// テクスチャ座標
	},
	{// 頂点1 RIGHT-TOP
		XMFLOAT3(COORDINATE, COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(TEXCOORD, 0.0f)
	},
	{// 頂点2 LEFT-BOTTOM
		XMFLOAT3(-COORDINATE, -COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f, TEXCOORD)
	},
	{// 頂点3 RIGHT-BOTTOM
		XMFLOAT3(COORDINATE, -COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(TEXCOORD, TEXCOORD)
	},
};

// インデックス配列
static UINT idxdata[6]
{
	 0, 1, 2, 2, 1, 3, // -Z面
};

//static UINT idxdata[6 * 6]
//{
//	 0,  1,  2,  2,  1,  3, // -Z面
//	 4,  5,  6,  6,  5,  7, // +X面
//	 8,  9, 10, 10,  9, 11, // +Z面
//	12, 13, 14, 14, 13, 15, // -X面
//	16, 17, 18, 18, 17, 19, // +Y面
//	20, 21, 22, 22, 21, 23, // -Y面
//};

static float top_y = 0;	// 六角形のtop-y座票のデバッグ表示

//======================================================
//	初期化関数
//======================================================
void Polygon3D_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// ポリゴン表示の初期化
	object[0].position = XMFLOAT3(-2.0f, 4.0f, 0.0f);
	object[0].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	object[0].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
	object[0].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
	object[0].maxHp = 100.0f;
	object[0].hp = object[0].maxHp;
	object[0].attack = 0.0f;
	object[0].power = 0.0f;
	object[0].speed = 0.0f;
	object[0].defense = 1.0f;
	object[0].stock = 3;
	object[0].active = true;
	object[0].isAttacking = false;
	object[0].attackTimer = 0.0f;
	object[0].isAttacked = false;
	object[0].attackedTimer = 0.0f;
	object[0].useSkill = false;
	object[0].skillTimer = 0.0f;
	object[0].useSpecial = false;
	object[0].specialTimer = 0.0f;
	object[0].isInvincible = false;
	object[0].invincibleTimer = 0.0f;
	object[0].stunGauge = 0.0f;
	object[0].isStunning = false;
	object[0].stunTimer = 0.0f;
	object[0].isDown = false;
	object[0].downTimer = 0.0f;
	object[0].lastDir = PlayerDir::Down; // 正面
	object[0].isMoving = false;
	object[0].form = Form::Normal;
	object[0].type = PlayerType::None;
	object[0].evolutionGauge = 0;
	object[0].evolutionGaugeRate = 1;
	object[0].breakCount_Glass = 0;
	object[0].breakCount_Concrete = 0;
	object[0].breakCount_Plant = 0;
	object[0].breakCount_Electric = 0;
	object[0].gl = 1.0f;
	object[0].pl = 1.0f;
	object[0].co = 1.0f;
	object[0].el = 1.0f;
	object[0].gaugeOuter = 1.0f;

	object[1].position = XMFLOAT3(1.5f, 4.0f, 2.0f);
	object[1].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	object[1].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
	object[1].maxHp = 100.0f;
	object[1].hp = object[1].maxHp;
	object[1].attack = 0.0f;
	object[1].power = 0.0f;
	object[1].speed = 0.0f;
	object[1].defense = 1.0f;
	object[1].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
	object[1].stock = 3;
	object[1].active = true;
	object[1].isAttacking = false;
	object[1].attackTimer = 0.0f;
	object[1].isAttacked = false;
	object[1].attackedTimer = 0.0f;
	object[1].useSkill = false;
	object[1].skillTimer = 0.0f;
	object[1].useSpecial = false;
	object[1].specialTimer = 0.0f;
	object[1].isInvincible = false;
	object[1].invincibleTimer = 0.0f;
	object[1].stunGauge = 0.0f;
	object[1].isStunning = false;
	object[1].stunTimer = 0.0f;
	object[1].isDown = false;
	object[1].downTimer = 0.0f;
	object[1].lastDir = PlayerDir::Down; // 正面
	object[1].isMoving = false;
	object[1].form = Form::Normal;
	object[1].type = PlayerType::None;
	object[1].evolutionGauge = 0;
	object[1].evolutionGaugeRate = 1;
	object[1].breakCount_Glass = 0;
	object[1].breakCount_Concrete = 0;
	object[1].breakCount_Plant = 0;
	object[1].breakCount_Electric = 0;
	object[1].gl = 1.0f;
	object[1].pl = 1.0f;
	object[1].co = 1.0f;
	object[1].el = 1.0f;
	object[1].gaugeOuter = 1.0f;

	object[2].position = XMFLOAT3(-4.0f, 4.0f, 0.0f);
	object[2].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	object[2].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
	object[2].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
	object[2].maxHp = 100.0f;
	object[2].hp = object[2].maxHp;
	object[2].attack = 0.0f;
	object[2].power = 0.0f;
	object[2].speed = 0.0f;
	object[2].defense = 1.0f;
	object[2].stock = 3;
	object[2].active = true;
	object[2].isAttacking = false;
	object[2].attackTimer = 0.0f;
	object[2].isAttacked = false;
	object[2].attackedTimer = 0.0f;
	object[2].useSkill = false;
	object[2].skillTimer = 0.0f;
	object[2].useSpecial = false;
	object[2].specialTimer = 0.0f;
	object[2].isInvincible = false;
	object[2].invincibleTimer = 0.0f;
	object[2].stunGauge = 0.0f;
	object[2].isStunning = false;
	object[2].stunTimer = 0.0f;
	object[2].isDown = false;
	object[2].downTimer = 0.0f;
	object[2].lastDir = PlayerDir::Down; // 正面
	object[2].isMoving = false;
	object[2].form = Form::Normal;
	object[2].type = PlayerType::None;
	object[2].evolutionGauge = 0;
	object[2].evolutionGaugeRate = 1;
	object[2].breakCount_Glass = 0;
	object[2].breakCount_Concrete = 0;
	object[2].breakCount_Plant = 0;
	object[2].breakCount_Electric = 0;
	object[2].gl = 1.0f;
	object[2].pl = 1.0f;
	object[2].co = 1.0f;
	object[2].el = 1.0f;
	object[2].gaugeOuter = 1.0f;

	object[3].position = XMFLOAT3(4.0f, 4.0f, -2.0f);
	object[3].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	object[3].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
	object[3].maxHp = 100.0f;
	object[3].hp = object[3].maxHp;
	object[3].attack = 0.0f;
	object[3].power = 0.0f;
	object[3].speed = 0.0f;
	object[3].defense = 1.0f;
	object[3].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
	object[3].stock = 3;
	object[3].active = true;
	object[3].isAttacking = false;
	object[3].attackTimer = 0.0f;
	object[3].isAttacked = false;
	object[3].attackedTimer = 0.0f;
	object[3].useSkill = false;
	object[3].skillTimer = 0.0f;
	object[3].useSpecial = false;
	object[3].specialTimer = 0.0f;
	object[3].isInvincible = false;
	object[3].invincibleTimer = 0.0f;
	object[3].stunGauge = 0.0f;
	object[3].isStunning = false;
	object[3].stunTimer = 0.0f;
	object[3].isDown = false;
	object[3].downTimer = 0.0f;
	object[3].lastDir = PlayerDir::Down; // 正面
	object[3].isMoving = false;
	object[3].form = Form::Normal;
	object[3].type = PlayerType::None;
	object[3].evolutionGauge = 0;
	object[3].evolutionGaugeRate = 1;
	object[3].breakCount_Glass = 0;
	object[3].breakCount_Concrete = 0;
	object[3].breakCount_Plant = 0;
	object[3].breakCount_Electric = 0;
	object[3].gl = 1.0f;
	object[3].pl = 1.0f;
	object[3].co = 1.0f;
	object[3].el = 1.0f;
	object[3].gaugeOuter = 1.0f;

	//頂点バッファ作成
	D3D11_BUFFER_DESC	bd;
	ZeroMemory(&bd, sizeof(bd));//0でクリア
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * NUM_VERTEX;//格納できる頂点数*頂点サイズ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	g_pDevice = pDevice;
	g_pContext = pContext;

	// テクスチャ読み込み
	LoadTextureList(pDevice);

	// インデックスバッファ作成
	{
		D3D11_BUFFER_DESC	bd;
		ZeroMemory(&bd, sizeof(bd));	// 0でクリア
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(UINT) * 6 * 6;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&bd, NULL, &g_IndexBuffer);

		// インデックスバッファへ書き込み
		D3D11_MAPPED_SUBRESOURCE   msr;
		pContext->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		UINT* index = (UINT*)msr.pData;

		// インデックスデータをバッファへコピー
		CopyMemory(&index[0], &idxdata[0], sizeof(UINT) * 6 * 6);
		pContext->Unmap(g_IndexBuffer, 0);
	}
	// デバッグレンダラー初期化
	Debug_Initialize(pDevice, pContext);

	InitializeHP(pDevice, pContext, &HPBar[0], { 160.0f,  630.0f }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);
	InitializeHP(pDevice, pContext, &HPBar[1], { 480.0f,  630.0f }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);

	// アニメーションの初期化
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		g_animFrame[i] = 0;
		g_animTimer[i] = 0.0f;
	}
}

static void LoadTextureList(ID3D11Device* pDevice)
{
	TexMetadata metadata;
	ScratchImage image;

	struct TexEntry { int idx; const wchar_t* path;};

	const TexEntry texList[] = {
		{  0, L"asset\\texture\\characterMini_v2.png"},				// 第1形態
		{  1, L"asset\\texture\\characterMidGlass_v1.png"},			// 第2形態 ガラス
		{  2, L"asset\\texture\\characterMidConcrete_v1.png" },		// 第2形態 コンクリート
		{  3, L"asset\\texture\\characterMidTree_v1.png" },			// 第2形態 植物
		{  4, L"asset\\texture\\characterMidElectricity_v1.png" },	// 第2形態 電気
		{  5, L"asset\\texture\\characterBigGlass_v1.png" },		// 第3形態 ガラス
		{  6, L"asset\\texture\\characterBigConcrete_v1.png" },		// 第3形態 コンクリート
		{  7, L"asset\\texture\\characterBigTree_v1.png" },		// 第3形態 植物
		{  8, L"asset\\texture\\characterBigElectricity_v1.png" },		// 第3形態 電気
		{  9, L"asset\\texture\\uiStockBlue_v2.png"},				// UI ストック 青
		{ 10, L"asset\\texture\\uiStockGleen_v2.png"},				// UI ストック 緑
		//{ 11, L"asset\\texture\\uiStockGleen_v2.png" },			// UI ストック 
		//{ 12, L"asset\\texture\\uiStockGleen_v2.png" },			// UI ストック 
		{ 13, L"asset\\texture\\uiLightLoopBigGlass_v1.png"},		// エフェクト ガラス
		{ 14, L"asset\\texture\\uiLightLoopBigConcrete_v1.png"},	// エフェクト コンクリート
		//{ 15, L"asset\\texture\\uiLightLoopBigGlass_v1.png"},		// エフェクト 植物
		//{ 16, L"asset\\texture\\uiLightLoopBigGlass_v1.png"},		// エフェクト 電気
	};

	for (const auto& e : texList)
	{
		// コメント化している要素は配列エントリ自体をコメントアウトしているためここには来ない。
		// （上ではコメント化行を // で無効化しているためコンパイル時に存在しません）
		HRESULT hr = LoadFromWICFile(e.path, WIC_FLAGS_NONE, &metadata, image);
		if (SUCCEEDED(hr))
		{
			if (FAILED(CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[e.idx])))
			{
				// 作成失敗時は nullptr を代入して続行
				g_Texture[e.idx] = nullptr;
			}
		}
		else
		{
			// 読み込み失敗は nullptr を代入して続行
			g_Texture[e.idx] = nullptr;
		}
	}
}

//======================================================
//	終了処理関数
//======================================================
void Polygon3D_Finalize()
{
	// シェーダーにバインドされている SRV をアンバインド（安全のため全要素分）
	const size_t TEX_COUNT = sizeof(g_Texture) / sizeof(g_Texture[0]);
	if (g_pContext)
	{
		// 固定長配列を使って確実に nullptr を渡す（API は生配列を要求）
		ID3D11ShaderResourceView* nullSRV[25] = {};
		g_pContext->PSSetShaderResources(0, static_cast<UINT>(TEX_COUNT), nullSRV);
	}

	// インデックス／頂点バッファの解放（NULL チェック後に nullptr に設定）
	if (g_IndexBuffer != nullptr)
	{
		g_IndexBuffer->Release();
		g_IndexBuffer = nullptr;
	}

	if (g_VertexBuffer != nullptr)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = nullptr;
	}

	// テクスチャ配列全要素を安全に解放（コメント化して未ロードの要素も nullptr チェックで安全）
	for (size_t i = 0; i < TEX_COUNT; ++i)
	{
		if (g_Texture[i] != nullptr)
		{
			g_Texture[i]->Release();
			g_Texture[i] = nullptr;
		}
	}

	// デバイス／コンテキストは外部管理のため解放しないが、参照はクリアしておく
	g_pContext = nullptr;
	g_pDevice = nullptr;

	// デバッグレンダラーの終了処理
	Debug_Finalize();
}

// ======================================================
// 移動関数（要変更）
// ------------------------------------------------------
// 移動ベクトルと向いている方向ベクトルは別で持った方がいい
// ======================================================
void Move(PLAYEROBJECT& object, XMFLOAT3 moveDir)
{
	// 進みたい方向（3平方）
	float length = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);

	if (length > 0.0f)
	{
		// ベクトルの正規化
		moveDir.x /= length;
		moveDir.z /= length;

		// 目標角度を求める
		float targetAngle = atan2f(moveDir.x, moveDir.z);	// ベクトルの角度
		targetAngle = XMConvertToDegrees(targetAngle);		// ラジアン -> 度

		// 差分を調整（180度超えないように）
		float diff = targetAngle - object.moveAngle;	// 角度差
		if (diff > 180.0f) diff -= 360.0f;
		if (diff < -180.0f) diff += 360.0f;

		static float angSpeed = 0.5f;

		// スムーズに補間（0.1fが補間スピード）
		object.moveAngle += diff * angSpeed;

		object.rotation.y = object.moveAngle;	// 角度の反映

		// 前進
		float rad = XMConvertToRadians(object.moveAngle);
		object.position.x += sinf(rad) * object.speed;
		object.position.z += cosf(rad) * object.speed;
	}
}

//======================================================
// 更新関数
//======================================================
void Polygon3D_Update()
{
	// 各プレイヤーに対応する発動キー
	const Keyboard_Keys_tag attackKeys[PLAYER_MAX] = { KK_SPACE, KK_ENTER };

	const Keyboard_Keys_tag specialKeys[PLAYER_MAX] = { KK_D9, KK_D0 };

	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		// -------------------------------------------------------------
		// 変身
		// -------------------------------------------------------------
		switch (object[p].form)
		{
		case Form::Normal: // 通常
			object[p].scaling.x = 0.5f;
			object[p].scaling.y = 0.5f;
			object[p].scaling.z = 0.5f;
			object[p].attack = 2.0f;
			object[p].power = 1.0f;
			object[p].speed = 0.06f;
			break;

		case Form::FirstEvolution: // 1進化
			object[p].scaling.x = 0.8f;
			object[p].scaling.y = 0.8f;
			object[p].scaling.z = 0.8f;
			object[p].attack = 3.0f;
			object[p].power = 1.5f;
			object[p].speed = 0.05f;
			break;

		case Form::SecondEvolution: // 2進化
			object[p].scaling.x = 1.2f;
			object[p].scaling.y = 1.2f;
			object[p].scaling.z = 1.2f;
			object[p].attack = 4.0f;
			object[p].power = 2.0f;
			object[p].speed = 0.04f;
			break;
		default:
			break;
		}

		// スタンゲージが最大でスタンフラグを立てる
		if (object[p].stunGauge >= STUNGAUGE_MAX)
		{
			object[p].isStunning = true;
			object[p].stunGauge = STUNGAUGE_MAX;
		}

		// スタン中の処理
		if (object[p].isStunning)
		{
			// スタンタイマーを進める
			object[p].stunTimer += DELTA_TIME;

			// スタン時間経過でスタン解除
			if (object[p].stunTimer >= STUN_TIME)
			{
				object[p].isStunning = false;	// スタン解除
				object[p].stunTimer = 0.0f;		// スタンタイマーリセット
				object[p].stunGauge = 0.0f;		// スタンゲージリセット
			}

			// スタン中は移動ベクトルを完全にゼロにする
			object[p].moveDir = { 0.0f, 0.0f, 0.0f };
		}
		else // スタンしていない場合の処理
		{
			// スタンしていない間はスタンゲージを減少させる
			object[p].stunGauge -= 0.1f / 60.0f;

			// スタンゲージが0未満にならないようにクランプ
			if (object[p].stunGauge < 0.0f)
			{
				object[p].stunGauge = 0.0f;
			}
		}

		// スタン中・ダウン中でなければ通常行動
		if(!object[p].isStunning && !object[p].isDown)
		{
			// 発動トリガー入力をチェックして攻撃フラグを立てる
			if (Keyboard_IsKeyDownTrigger(attackKeys[p]))
			{
				object[p].isAttacking = true;

				// 第2・第3形態の場合、スキル使用フラグも立てる
				if (object[p].type != PlayerType::None)
				{
					object[p].useSkill = true;
				}
			}
			if (g_Input[0].A)
			{
				object[0].isAttacking = true;

				// 第2・第3形態の場合、スキル使用フラグも立てる
				if (object[0].type != PlayerType::None)
				{
					object[0].useSkill = true;
				}
			}

			// 発動トリガー入力をチェックしてスペシャル使用フラグを立てる
			if (Keyboard_IsKeyDownTrigger(specialKeys[p]))
			{
				object[p].useSpecial = true;
			}

			// スキル中ならスキル更新処理を呼び出す
			if (object[p].useSkill)
			{
				Skill_Update(p);
			}
			// 攻撃中なら攻撃更新処理を呼び出す
			if (object[p].isAttacking)
			{
				Attack_Update(p);
			}
			// スペシャル使用中ならスペシャル更新処理を呼び出す
			if (object[p].useSpecial)
			{
				Special_Update(p);
			}

			// 現在のプレイヤー p の移動ベクトルだけをリセット
			object[p].moveDir = { 0.0f, 0.0f, 0.0f };

			// プレイヤーの番号に応じて入力キーを分ける
			if (p == 0) // プレイヤー1 (WASD)
			{
				if (g_Input[0].LStickX > 0.0f)	{object[0].moveDir.x += 1.0f; object[0].isMoving = true;}
				if (g_Input[0].LStickX < 0.0f)	{object[0].moveDir.x -= 1.0f; object[0].isMoving = true;}
				if (g_Input[0].LStickY > 0.0f)	{object[0].moveDir.z -= 1.0f; object[0].isMoving = true;}
				if (g_Input[0].LStickY < 0.0f)	{object[0].moveDir.z += 1.0f; object[0].isMoving = true;}

				if (Keyboard_IsKeyDown(KK_W))	{object[0].moveDir.z += 1.0f; object[0].isMoving = true;}
				if (Keyboard_IsKeyDown(KK_S))	{object[0].moveDir.z -= 1.0f; object[0].isMoving = true;}
				if (Keyboard_IsKeyDown(KK_A))	{object[0].moveDir.x -= 1.0f; object[0].isMoving = true;}
				if (Keyboard_IsKeyDown(KK_D))	{object[0].moveDir.x += 1.0f; object[0].isMoving = true;}
				if (object[0].moveDir.x == 0.0f && object[0].moveDir.z == 0.0f)	object[0].isMoving = false;
			}
			else if (p == 1) // プレイヤー2 (矢印キー)
			{
				if (Keyboard_IsKeyDown(KK_UP))		{object[1].moveDir.z += 1.0f; object[1].isMoving = true;}
				if (Keyboard_IsKeyDown(KK_DOWN))	{object[1].moveDir.z -= 1.0f; object[1].isMoving = true;}
				if (Keyboard_IsKeyDown(KK_LEFT))	{object[1].moveDir.x -= 1.0f; object[1].isMoving = true;}
				if (Keyboard_IsKeyDown(KK_RIGHT))	{object[1].moveDir.x += 1.0f; object[1].isMoving = true;}
				if (object[1].moveDir.x == 0.0f && object[1].moveDir.z == 0.0f)	object[1].isMoving = false;
			}

			// 現在のプレイヤー p だけを動かす
			Move(object[p], object[p].moveDir);
		}

		// HPが0以下の処理（ダウンは1度だけ）
		if (object[p].hp <= 0.0f && object[p].active && !object[p].isDown)
		{
			// ダウン状態に移行してタイマーをリセット
			object[p].isDown = true;
			object[p].downTimer = 0.0f;
		}

		// ダウン状態のタイマー更新とリスポーン判定
		if (object[p].isDown)
		{
			// 行動停止
			object[p].moveDir = { 0.0f, 0.0f, 0.0f };
			object[p].isAttacking = false;
			object[p].useSkill = false;
			object[p].useSpecial = false;

			// ダウンタイマー更新
			object[p].downTimer += DELTA_TIME;

			// プレイヤー毎のダウン時間が経過したらリスポーン処理
			if (object[p].downTimer >= DOWN_TIME)
			{
				// 残機を1つ減らす
				object[p].stock -= 1;

				if (object[p].stock > 0)
				{
					Polygon3D_Respawn(p);
				}
				else
				{
					// 残機無しで復活なし
					object[p].active = false;
					object[p].isDown = false;
					object[p].downTimer = 0.0f;
				}
			}
		}

		// 落下処理
		if (object[p].active && object[p].position.y <= -10.0f)
		{
			// 残機を一つ減らす
			object[p].stock -= 1;

			if (object[p].stock > 0)
			{
				// リスポーン（位置・ステートリセット）
				Polygon3D_Respawn(p);
			}
			else
			{
				// 残機無しで完全に非アクティブ化
				object[p].active = false;
			}
		}

		// ダメージを受けた時の処理
		if (object[p].isAttacked)
		{
			// ダメージタイマー更新
			object[p].attackedTimer += DELTA_TIME;

			// プレイヤー毎のダメージ時間が経過したらダメージ終了
			if (object[p].attackedTimer >= ATTACKED_TIME)
			{
				object[p].isAttacked = false;
				object[p].attackedTimer = 0.0f;
			}
		}

		// 進化時の無敵処理
		if (object[p].isInvincible)
		{
			// 無敵タイマー更新
			object[p].invincibleTimer += DELTA_TIME;

			// プレイヤー毎の無敵時間が経過したら無敵終了
			if (object[p].invincibleTimer >= INVINCIBLE_TIME)
			{
				object[p].isInvincible = false;
				object[p].invincibleTimer = 0.0f;
			}
		}

		// プレイヤー アニメーション更新
		g_animTimer[p] += DELTA_TIME;
		if (g_animTimer[p] >= ANIM_FRAME_TIME)
		{
			int advance = (int)(g_animTimer[p] / ANIM_FRAME_TIME);
			g_animTimer[p] -= advance * ANIM_FRAME_TIME;

			// 勝利 第1形態 13コマ(ラスト5コマ ループ) 第2形態 20コマ(ラスト9コマ ループ) 第3形態 21コマ(ラストコマ ループ)
			if (Keyboard_IsKeyDown(KK_TAB) || g_victoryState[p] != 0)
			{
				// 押下で開始
				if (Keyboard_IsKeyDown(KK_TAB) && g_victoryState[p] == 0)
				{
					g_victoryState[p] = 1;
					g_animFrame[p] = 208; // 初回再生開始フレーム
				}

				if (g_victoryState[p] == 1)
				{
					// 初回再生 フレームを単純増加
					g_animFrame[p] += advance;

					// 第1形態 220 を表示した後にループ領域へ移行する
					if (g_animFrame[p] > 220 && object[p].form == Form::Normal)
					{
						g_victoryState[p] = 2;
						g_animFrame[p] = 216; // ループ開始フレーム
					}
					// 第2形態 227 を表示した後にループ領域へ移行する
					if (g_animFrame[p] > 227 && object[p].form == Form::FirstEvolution)
					{
						g_victoryState[p] = 2;
						g_animFrame[p] = 219; // ループ開始フレーム
					}
					// 第3形態 229 を表示した後にループ領域へ移行する
					if (g_animFrame[p] > 229 && object[p].form == Form::SecondEvolution)
					{
						g_victoryState[p] = 2;
						g_animFrame[p] = 219; // ループ開始フレーム
					}
				}
				else if (g_victoryState[p] == 2)
				{
					switch (object[p].form)
					{
					case Form::Normal:			LoopRange(g_animFrame[p], 216, 5, advance);	// 第1形態 216～220をループ
						break;
					case Form::FirstEvolution:	LoopRange(g_animFrame[p], 219, 9, advance);	// 第2形態 219～227をループ
						break;
					case Form::SecondEvolution:	LoopRange(g_animFrame[p], 219, 9, advance);	// 第3形態 219～229をループ
						break;
					}
				}
			}
			// ダウン 5コマ (ダメージ 2コマ + ダウン 3コマ) 最終コマで停止
			else if (object[p].isDown == true)
			{
				// 向きに応じた開始フレームを決定
				int start = 15; // デフォルト（Down）
					 if (object[p].lastDir == PlayerDir::Up_Right)	 start = 145;
				else if (object[p].lastDir == PlayerDir::Up_Left)	 start = 93;
				else if (object[p].lastDir == PlayerDir::Down_Right) start = 197;
				else if (object[p].lastDir == PlayerDir::Down_Left)	 start = 41;
				else if (object[p].lastDir == PlayerDir::Up)		 start = 119;
				else if (object[p].lastDir == PlayerDir::Down)		 start = 15;
				else if (object[p].lastDir == PlayerDir::Right)		 start = 171;
				else if (object[p].lastDir == PlayerDir::Left)		 start = 67;

				const int count = 5;
				const int lastFrame = start + count - 1;

				// advance に対応する経過秒（g_animTimerでまとめて進めた分）
				float elapsedSec = (float)advance * ANIM_FRAME_TIME;

				// フレームが範囲外なら開始フレームに補正しタイマーリセット
				if (g_animFrame[p] < start || g_animFrame[p] > lastFrame)
				{
					g_animFrame[p] = start;
					g_downHoldTimer[p] = 0.0f;
				}

				// 最終フレーム以外なら通常進行（ループ）
				if (g_animFrame[p] != lastFrame)
				{
					LoopRange(g_animFrame[p], start, count, advance);
					g_downHoldTimer[p] = 0.0f; // 到達前はホールドタイマーをリセット
				}
				else
				{
					// 最終フレームに到達 ホールドを進める
					g_downHoldTimer[p] += elapsedSec;

					// ホールドが満了したら次に進める（ここでは1フレーム分だけ進める）
					if (g_downHoldTimer[p] >= DOWN_TIME)
					{
						g_downHoldTimer[p] = 0.0f;
						// 1フレーム分進める（ループにより start に戻る）
						LoopRange(g_animFrame[p], start, count, 1);
					}
				}
			}
			// ダメージ 3コマ
			else if (object[p].isAttacked == true || object[p].isStunning)
			{
					 if (object[p].lastDir == PlayerDir::Up_Right)	LoopRange(g_animFrame[p], 144, 3, advance);	// 右上 144～146
				else if (object[p].lastDir == PlayerDir::Up_Left)	LoopRange(g_animFrame[p],  92, 3, advance);	// 左上  92～94
				else if (object[p].lastDir == PlayerDir::Down_Right)LoopRange(g_animFrame[p], 196, 3, advance);	// 右下 196～198
				else if (object[p].lastDir == PlayerDir::Down_Left)	LoopRange(g_animFrame[p],  40, 3, advance);	// 左下  40～42
				else if (object[p].lastDir == PlayerDir::Up)		LoopRange(g_animFrame[p], 118, 3, advance);	//  上  118～120
				else if (object[p].lastDir == PlayerDir::Down)		LoopRange(g_animFrame[p],  14, 3, advance);	//  下   14～16
				else if (object[p].lastDir == PlayerDir::Right)		LoopRange(g_animFrame[p], 170, 3, advance);	//  右  170～172
				else if (object[p].lastDir == PlayerDir::Left)		LoopRange(g_animFrame[p],  66, 3, advance);	//  左   66～68
			}
			// 攻撃 6コマ
			else if (object[p].isAttacking == true)
			{
					 if (object[p].lastDir == PlayerDir::Up_Right)	LoopRange(g_animFrame[p], 150, 6, advance);	// 右上 150～155
				else if (object[p].lastDir == PlayerDir::Up_Left)	LoopRange(g_animFrame[p],  98, 6, advance);	// 左上  98～103
				else if (object[p].lastDir == PlayerDir::Down_Right)LoopRange(g_animFrame[p], 202, 6, advance);	// 右下 202～207
				else if (object[p].lastDir == PlayerDir::Down_Left)	LoopRange(g_animFrame[p],  46, 6, advance);	// 左下  46～51
				else if (object[p].lastDir == PlayerDir::Up)		LoopRange(g_animFrame[p], 124, 6, advance);	//  上  124～129
				else if (object[p].lastDir == PlayerDir::Down)		LoopRange(g_animFrame[p],  20, 6, advance);	//  下   20～25
				else if (object[p].lastDir == PlayerDir::Right)		LoopRange(g_animFrame[p], 176, 6, advance);	//  右  176～181
				else if (object[p].lastDir == PlayerDir::Left)		LoopRange(g_animFrame[p],  72, 6, advance);	//  左   72～77
			}
			// 移動 8コマ
			else if (object[p].isMoving == true)
			{
					 if (object[p].moveDir.x > 0.0f && object[p].moveDir.z > 0.0f) { LoopRange(g_animFrame[p], 136, 8, advance); object[p].lastDir = PlayerDir::Up_Right; }		// 右上 136～143
				else if (object[p].moveDir.x < 0.0f && object[p].moveDir.z > 0.0f) { LoopRange(g_animFrame[p],  84, 8, advance); object[p].lastDir = PlayerDir::Up_Left; }		// 左上  84～91
				else if (object[p].moveDir.x > 0.0f && object[p].moveDir.z < 0.0f) { LoopRange(g_animFrame[p], 188, 8, advance); object[p].lastDir = PlayerDir::Down_Right; }	// 右下 188～195
				else if (object[p].moveDir.x < 0.0f && object[p].moveDir.z < 0.0f) { LoopRange(g_animFrame[p],  32, 8, advance); object[p].lastDir = PlayerDir::Down_Left; }	// 左下  32～39
				else if (object[p].moveDir.z > 0.0f) { LoopRange(g_animFrame[p], 110, 8, advance); object[p].lastDir = PlayerDir::Up; }		// 上 110～117
				else if (object[p].moveDir.z < 0.0f) { LoopRange(g_animFrame[p],   6, 8, advance); object[p].lastDir = PlayerDir::Down; }	// 下   6～13
				else if (object[p].moveDir.x > 0.0f) { LoopRange(g_animFrame[p], 162, 8, advance); object[p].lastDir = PlayerDir::Right; }	// 右 162～169
				else if (object[p].moveDir.x < 0.0f) { LoopRange(g_animFrame[p],  58, 8, advance); object[p].lastDir = PlayerDir::Left; }	// 左  58～63
			}
			// 待機 6コマ
			else if (object[p].isMoving == false)
			{
					 if (object[p].lastDir == PlayerDir::Up_Right)	LoopRange(g_animFrame[p], 130, 6, advance);	// 右上 130～135
				else if (object[p].lastDir == PlayerDir::Up_Left)	LoopRange(g_animFrame[p],  78, 6, advance);	// 左上  78～83 
				else if (object[p].lastDir == PlayerDir::Down_Right)LoopRange(g_animFrame[p], 182, 6, advance);	// 右下 182～187		
				else if (object[p].lastDir == PlayerDir::Down_Left)	LoopRange(g_animFrame[p],  26, 6, advance);	// 左下  26～31
				else if (object[p].lastDir == PlayerDir::Up)		LoopRange(g_animFrame[p], 104, 6, advance);	//  上  104～109
				else if (object[p].lastDir == PlayerDir::Down)		LoopRange(g_animFrame[p],   0, 6, advance);	//  下    0～5
				else if (object[p].lastDir == PlayerDir::Right)		LoopRange(g_animFrame[p], 156, 6, advance);	//  右  156～161
				else if (object[p].lastDir == PlayerDir::Left)		LoopRange(g_animFrame[p],  52, 6, advance);	//  左   52～57
			}
		}
	}

	// がぶがぶとプレイヤーの当たり判定
	AttackPlayerCollisions();

	// デバッグ用 ImGui ウィンドウ
	ImGui::Begin("Player Debug");
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		// プレイヤーごとに ID を分ける（同一ラベル衝突回避）
		ImGui::PushID(p);

		ImGui::Text("Player %d", p + 1);
		ImGui::Indent();

		ImGui::SliderFloat("stunGauge", &object[p].stunGauge, 0.0f, 10.0f, "%.1f");
		ImGui::SliderFloat("invincibleTimer", &object[p].invincibleTimer, 0.0f, 3.0f, "%.1f");
		ImGui::BulletText("active            : %d", object[p].active);
		ImGui::BulletText("speed             : %.3f", object[p].speed);
		ImGui::BulletText("defense           : %.1f", object[p].defense);
		ImGui::BulletText("useSkill          : %d", object[p].useSkill);
		ImGui::BulletText("useSpecial        : %d", object[p].useSpecial);
		ImGui::BulletText("isInvincible      : %d", object[p].isInvincible);
		ImGui::BulletText("form              : %d", object[p].form);
		ImGui::BulletText("type              : %d", object[p].type);
		ImGui::BulletText("EvolutionGauge    : %d", object[p].evolutionGauge);
		ImGui::BulletText("EvolutionGaugeRate: %d", object[p].evolutionGaugeRate);
		ImGui::BulletText("1 Glass breaks    : %d", object[p].breakCount_Glass);
		ImGui::BulletText("2 Concrete breaks : %d", object[p].breakCount_Concrete);
		ImGui::BulletText("3 Plant breaks    : %d", object[p].breakCount_Plant);
		ImGui::BulletText("4 Electric breaks : %d", object[p].breakCount_Electric);

		// 履歴リストのサイズを表示
		size_t historySize = object[p].brokenHistory.size();
		ImGui::BulletText("brokenHistory Size : %zu", historySize);

		if (historySize > 0)
		{
			ImGui::Indent(); // 履歴をさらに一段インデント
			ImGui::Text("History (Latest -> Oldest):");

			// 履歴を最新（末尾）から古い方へループして表示
			for (int i = (int)historySize - 1; i >= 0; --i)
			{
				// BuildingType は enum型（整数値）なので、そのまま %d で表示可能
				// または、ImGui::Textで整形して表示する

				// 例1: 履歴のインデックスと値を直接表示
				// ImGui::BulletText("[%d]: %d", p, (int)object[p].brokenHistory[p]);

				// 例2: 履歴の値を横に並べて表示
				ImGui::SameLine(); // 同じ行に表示
				// 履歴の値（整数）を文字列に変換してから表示
				ImGui::Text("%d", (int)object[p].brokenHistory[i]);
			}

			// 履歴が横に並びすぎないよう改行
			ImGui::NewLine();
			ImGui::Unindent();
		}

		ImGui::Unindent();
		ImGui::Separator();
		ImGui::PopID();
	}

	ImGui::End();
	
	for (int p = 0; p < PLAYER_MAX; p++)
	{
		static XMFLOAT3 posBuff = object[p].position;	// デバッグ表示座標

		// 描画で使っているスプライト倍率と同じ値を物理にも使う
		const float renderScale = 2.0f; // Draw 側の spriteScale に合わせる
		// 描画スケールを反映したスケール（表示用）
		XMFLOAT3 physicsScaling = XMFLOAT3(object[p].scaling.x * renderScale, object[p].scaling.y * renderScale, object[p].scaling.z * renderScale);

		// --- プレイヤー用ヒットボックス比率（向きで長短を切り替える） ---
		// 高さは固定、水平面は向きに応じて長短を切り替える
		const float HITBOX_HEIGHT_SCALE = 1.0f;
		const float HITBOX_SHORT = 0.35f; // 向きと直交する短辺
		const float HITBOX_LONG  = 0.65f; // 向きに沿った長辺

		// 回転から前方ベクトルを算出して、どちらの軸が優勢か判定する
		float radFacing = XMConvertToRadians(object[p].rotation.y);
		float facingX = sinf(radFacing);
		float facingZ = cosf(radFacing);
		bool facingZDominant = fabsf(facingZ) >= fabsf(facingX);

		float widthScale  = facingZDominant ? HITBOX_SHORT : HITBOX_LONG; // X方向スケール
		float depthScale  = facingZDominant ? HITBOX_LONG  : HITBOX_SHORT; // Z方向スケール

		// 第2形態 第3形態はXとZ同じにする
		if (object[p].form == Form::FirstEvolution || object[p].form == Form::SecondEvolution)
		{
			widthScale = 0.25f;
			depthScale = 0.25f;
		}

		XMFLOAT3 hitboxScaling = XMFLOAT3(
			object[p].scaling.x * renderScale * widthScale,
			object[p].scaling.y * renderScale * HITBOX_HEIGHT_SCALE,
			object[p].scaling.z * renderScale * depthScale
		);

		// AABB を現在の位置・スケール（ヒットボックス）で更新しておく（衝突判定で使用）
		CalculateAABB(object[p].boundingBox, object[p].position, hitboxScaling);

		// y軸の移動量 (重力 + ジャンプ)
		// 重力加速度のない簡易的な重力
		object[p].position.y += -0.1f;

		// デバッグ出力
		if (posBuff.x != object[p].position.x ||
			posBuff.y != object[p].position.y ||
			posBuff.z != object[p].position.z)
		{
			hal::dout << "x : " << object[p].position.x << std::endl;
			hal::dout << "y : " << object[p].position.y << std::endl;
			hal::dout << "z : " << object[p].position.z << std::endl;
		}

		//hal::dout << vdata[0].position.x << std::endl;

		posBuff = object[p].position;

		// 地面の高さ（最低ライン）
		//float groundHeight = -10.0f;	// 奈落の底
		//bool isGrounded = false;		// 地面に足がついているかフラグ

		// マップデータ（地面）との当たり判定
		int fieldCount = GetFieldObjectCount();
		MAPDATA* fieldObjects = GetFieldObjects();

		for (int j = 0; j < fieldCount; ++j)
		{
			// アクティブじゃない、または no が MAX ならスキップ
			if (!fieldObjects[j].isActive || fieldObjects[j].no == FIELD::FIELD_MAX)
			{
				continue;
			}

			// --- 六角柱コライダーの準備 ---
			HexCollider hex;
			hex.center = fieldObjects[j].pos;		// -1
			hex.radius = fieldObjects[j].radius;	// 1
			hex.height = fieldObjects[j].height;	// 3.0

			// プレイヤーのAABB（体の一部）が六角柱に乗っているか
			if (CheckAABBHexCollision(object[p].boundingBox, hex))
			{
				// タイルの上面のY座標を計算
				float tileTopY = fieldObjects[j].pos.y + (hex.height / 2.0f);	// -1 + 1.5 = 0.5

				// プレイヤーの底面がタイルの上面以下か
				if (object[p].boundingBox.Min.y <= tileTopY)
				{
					const float baseHalfHeight = COORDINATE;
					// 着地では見た目の高さ（描画スケール）を基準に計算しているため physicsScaling を使用
					float halfHeight = baseHalfHeight * object[p].scaling.y * renderScale;

					// 着地させる（めり込みが起きないよう最低値として補正）
					float targetY = tileTopY + halfHeight;
					if (object[p].position.y < targetY)
					{
						object[p].position.y = targetY;
					}

					// AABB を再計算して整合性を保つ（描画スケールを考慮）
					// ヒットボックス（向きに応じた長方形）で再計算する
					CalculateAABB(object[p].boundingBox, object[p].position, hitboxScaling);

					top_y = tileTopY;

					break;
				}
			}
		}

		// -------------------------------------------------------------------------------------
		// 建物との当たり判定
		// -------------------------------------------------------------------------------------
		int buildingCount = GetBuildingCount();			// 数を取得
		Building** buildingObjects = GetBuildings();	// リストを取得

		for (int j = 0; j < buildingCount; ++j)
		{
			// アクティブでないなら無視
			if (!buildingObjects[j]->isActive)	continue;

			// y座標の調整
			// Building::Draw() で position.y + 1.0f しているので、判定用の座標も合わせる
			XMFLOAT3 colliderPos = buildingObjects[j]->position;
			colliderPos.y += 1.0f;

			// コライダーの作成と更新（補正した座標 colliderPos を使う）
			CalculateAABB(buildingObjects[j]->boundingBox, colliderPos, buildingObjects[j]->scaling);

			// プレイヤー と 建物の当たり判定
			MTV collision = CalculateAABBMTV(object[p].boundingBox, buildingObjects[j]->boundingBox);

			if (collision.isColliding)
			{
				// 衝突していたら、MTVの分だけ位置を戻す
				object[p].position.x += collision.translation.x;
				object[p].position.y += collision.translation.y;
				object[p].position.z += collision.translation.z;

				// 押し戻し後の新しいAABBを再計算（描画スケールを反映）
				// ヒットボックス（向きに応じた長方形）で再計算する
				CalculateAABB(object[p].boundingBox, object[p].position, hitboxScaling);
			}
		}

		// プレイヤーに対応する攻撃オブジェクトを PLAYER_MAX 分ループしてスケーリング同期
		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			ATTACK_OBJECT* attackObject = GetAttack(p); // GetAttack は 1-based
			if (attackObject == nullptr) continue;

			// プレイヤー側のスケールに合わせる（攻撃オブジェクトは半分）
			attackObject->scaling.x = object[p].scaling.x * 0.5f;
			attackObject->scaling.y = object[p].scaling.y * 0.5f;
			attackObject->scaling.z = object[p].scaling.z * 0.5f;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////

		// -------------------------------------------------------------
		// プレイヤーオブジェクト同士の当たり判定（PLAYER_MAX分対応）
		// -------------------------------------------------------------
		// object[p] の AABB は既に計算済み（前方で CalculateAABB(object[p].boundingBox, ... ) を呼んでいる前提）
		for (int otherIndex = p + 1; otherIndex < PLAYER_MAX; ++otherIndex)
		{
			// 非アクティブは無視
			if (!object[otherIndex].active) continue;

			// 他プレイヤーのヒットボックススケーリング（向きで長短を切り替える）
			const float HITBOX_HEIGHT_SCALE = 1.0f;
			const float HITBOX_SHORT = 0.35f;
			const float HITBOX_LONG  = 0.65f;

			// 宣言をループスコープの先頭に置く（後で再利用するため）
			XMFLOAT3 hitboxScalingOther;

			{
				float radOther = XMConvertToRadians(object[otherIndex].rotation.y);
				float otherFacingX = sinf(radOther);
				float otherFacingZ = cosf(radOther);
				bool otherFacingZDominant = fabsf(otherFacingZ) >= fabsf(otherFacingX);

				float otherWidthScale = otherFacingZDominant ? HITBOX_SHORT : HITBOX_LONG;
				float otherDepthScale = otherFacingZDominant ? HITBOX_LONG  : HITBOX_SHORT;

				// 第2形態 第3形態はXとZ同じにする
				if (object[otherIndex].form == Form::FirstEvolution || object[otherIndex].form == Form::SecondEvolution)
				{
					widthScale = 0.25f;
					depthScale = 0.25f;
				}

				hitboxScalingOther = XMFLOAT3(
					object[otherIndex].scaling.x * renderScale * otherWidthScale,
					object[otherIndex].scaling.y * renderScale * HITBOX_HEIGHT_SCALE,
					object[otherIndex].scaling.z * renderScale * otherDepthScale
				);
			}

			// 他プレイヤーの AABB を更新（ここで定義済みの hitboxScalingOther を使用）
			CalculateAABB(object[otherIndex].boundingBox, object[otherIndex].position, hitboxScalingOther);

			// 衝突チェック（ペア p <-> otherIndex を一度だけ判定）
			MTV collision_player = CalculateAABBMTV(object[p].boundingBox, object[otherIndex].boundingBox);

			if (collision_player.isColliding)
			{
				// 向きベクトルを更新（rotation.y から算出）
				{
					float rad_p = XMConvertToRadians(object[p].rotation.y);
					object[p].dir.x = sinf(rad_p);
					object[p].dir.z = cosf(rad_p);
				}
				{
					float rad_o = XMConvertToRadians(object[otherIndex].rotation.y);
					object[otherIndex].dir.x = sinf(rad_o);
					object[otherIndex].dir.z = cosf(rad_o);
				}

				// 押し戻し量 (MTV) を半分にして双方に適用
				XMFLOAT3 half_translation =
				{
					collision_player.translation.x * 0.5f,
					collision_player.translation.y * 0.5f,
					collision_player.translation.z * 0.5f
				};

				// object[p] を MTV の半分だけ押す
				object[p].position.x += half_translation.x;
				object[p].position.y += half_translation.y;
				object[p].position.z += half_translation.z;

				// object[otherIndex] を逆方向に半分だけ押す
				object[otherIndex].position.x -= half_translation.x;
				object[otherIndex].position.y -= half_translation.y;
				object[otherIndex].position.z -= half_translation.z;

				// 押し戻し後の新しいAABBを再計算 (ヒットボックスで)
				CalculateAABB(object[p].boundingBox, object[p].position, hitboxScaling);
				CalculateAABB(object[otherIndex].boundingBox, object[otherIndex].position, hitboxScalingOther);
			}
		}

		SetHPValue(&HPBar[p], (int)object[p].hp, (int)object[p].maxHp);
		UpdateHP(&HPBar[p]);
	}

	// エフェクト用タイマー更新
	//g_effectElapsed += (1.0f / 60.0f);
}

//======================================================
//	描画関数
//======================================================
void Polygon3D_Draw(bool s_IsKonamiCodeEntered)
{
	LIGHT light{};
	light.Enable = TRUE;
	// 光の向き（ワールド空間）シェーダー側で単位化して使っている想定
	light.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
	// 拡散光と環境光
	light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	Shader_SetLight(light);

	static bool input1 = false;
	// デバッグモード中のみキー入力を受け付ける
	if (s_IsKonamiCodeEntered)
	{
		if (Keyboard_IsKeyDownTrigger(KK_D1))
		{
			input1 = !input1;	// フラグ反転
		}
	}
	
	// 攻撃描画
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (object[p].isAttacking)
		{
			Attack_Draw(p);
		}
	}

	Shader_Begin(); 

	// ========================================================
	// 奥のプレイヤーが手前のプレイヤーに隠れないように描画
	// ========================================================

	// プロジェクション・ビュー行列を先に取得
	XMMATRIX projection = GetProjectionMatrix();
	XMMATRIX view = GetViewMatrix();

	// カメラ位置を算出（View の逆行列の r[3] がワールド空間のカメラ位置）
	XMMATRIX invView = XMMatrixInverse(nullptr, view);
	XMFLOAT3 camPos;
	camPos.x = invView.r[3].m128_f32[0];
	camPos.y = invView.r[3].m128_f32[1];
	camPos.z = invView.r[3].m128_f32[2];

	// プレイヤーを描画するラムダ（Projection, View をキャプチャ）
	auto DrawPlayerInternal = [&](int idx)
		{
			const float spriteScale = 2.0f; // 表示倍率

			// ワールド行列（ビルボード風の既存ロジックを踏襲）
			XMMATRIX ScalingMatrix = XMMatrixScaling(
				object[idx].scaling.x * spriteScale,
				object[idx].scaling.y * spriteScale,
				object[idx].scaling.z * spriteScale
			);

			XMMATRIX vm = GetViewMatrix();	// カメラの行列
			vm.r[3].m128_f32[0] = 0.0f;
			vm.r[3].m128_f32[1] = 0.0f;
			vm.r[3].m128_f32[2] = 0.0f;
			vm.r[3].m128_f32[3] = 1.0f;
			vm = XMMatrixTranspose(vm);
			vm.r[3].m128_f32[0] = object[idx].position.x;
			vm.r[3].m128_f32[1] = object[idx].position.y;
			vm.r[3].m128_f32[2] = object[idx].position.z;
			vm.r[3].m128_f32[3] = 1.0f;

			// World 行列（ビルボード用）をシェーダーに渡す
			XMMATRIX WorldMatrix = ScalingMatrix * vm;
			Shader_SetWorldMatrix(WorldMatrix);

			XMMATRIX WVP = ScalingMatrix * vm * view * projection;

			Shader_SetMatrix(WVP);
			Shader_Begin();
			SetBlendState(BLENDSTATE_ALPHA);

			// 頂点バッファにデータコピー（フレームに応じてUVを書き換える）
			D3D11_MAPPED_SUBRESOURCE msr;

			// コピー元のvdata をローカル配列にコピーして UV を調整
			Vertex2 localV[NUM_VERTEX];
			CopyMemory(&localV[0], &vdata[0], sizeof(Vertex2) * NUM_VERTEX);

			// 現在のフレームから UV を計算
			int frame = g_animFrame[idx];
			int col = frame % SHEET_COLS;
			int row = frame / SHEET_COLS;
			float u0 = (float)col / (float)SHEET_COLS;
			float v0 = (float)row / (float)SHEET_ROWS;
			float u1 = u0 + 1.0f / (float)SHEET_COLS;
			float v1 = v0 + 1.0f / (float)SHEET_ROWS;

			// 頂点のテクスチャ座標を上書き
			localV[0].tex = XMFLOAT2(u0, v0);	// LEFT-TOP
			localV[1].tex = XMFLOAT2(u1, v0);	// RIGHT-TOP
			localV[2].tex = XMFLOAT2(u0, v1);	// LEFT-BOTTOM
			localV[3].tex = XMFLOAT2(u1, v1);	// RIGHT-BOTTOM

			// バッファへ書き込み
			g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
			Vertex2* vertex = (Vertex2*)msr.pData;
			CopyMemory(vertex, &localV[0], sizeof(Vertex2) * NUM_VERTEX);
			g_pContext->Unmap(g_VertexBuffer, 0);

			ID3D11ShaderResourceView* srv = nullptr;

			// 形態とタイプに応じたテクスチャを設定
			switch (object[idx].form)
			{
			// 第1形態
			case Form::Normal:				srv = g_Texture[0];	break;
			// 第2形態
			case Form::FirstEvolution:
				switch (object[idx].type)
				{
				case PlayerType::Glass:		srv = g_Texture[1];	break;				
				case PlayerType::Concrete:	srv = g_Texture[2];	break;
				case PlayerType::Plant:		srv = g_Texture[3];	break;
				case PlayerType::Electric:	srv = g_Texture[4];	break;
				default: break;
				}
				break;
			// 第3形態
			case Form::SecondEvolution:
				switch (object[idx].type)
				{
				case PlayerType::Glass:		srv = g_Texture[1];	break;
				case PlayerType::Concrete:	srv = g_Texture[2];	break;
				case PlayerType::Plant:		srv = g_Texture[3];	break;
				case PlayerType::Electric:	srv = g_Texture[4];	break;
				//case PlayerType::Glass:		srv = g_Texture[5];	break;
				//case PlayerType::Concrete:	srv = g_Texture[6];	break;
				//case PlayerType::Plant:		srv = g_Texture[7];	break;
				//case PlayerType::Electric:	srv = g_Texture[8];	break;
				default: break;
				}
				break;
			}
			g_pContext->PSSetShaderResources(0, 1, &srv);

			Shader_SetColor({ 1,1,1,1 });

			// バッファセット & 描画
			UINT stride = sizeof(Vertex2);
			UINT offset = 0;
			g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
			g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			g_pContext->DrawIndexed(6, 0, 0);
		};

	// -----------------------------------
	// 透明描画のためのソート（遠い順）
	// -----------------------------------
	std::vector<std::pair<float, int>> list; // (距離二乗, index)
	list.reserve(PLAYER_MAX);

	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		float dx = object[p].position.x - camPos.x;
		float dy = object[p].position.y - camPos.y;
		float dz = object[p].position.z - camPos.z;
		float dist2 = dx * dx + dy * dy + dz * dz;
		list.emplace_back(dist2, p);
	}

	// 遠い順（大きい順）にソート
	std::sort(list.begin(), list.end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
		{
		return a.first > b.first;
		});

	// 透過レンダリング：深度テストは有効、深度書き込みは無効（SetDepthReadOnly を使用）
	SetDepthTest(true);
	SetDepthReadOnly(); // 深度テストはするが深度バッファへの書き込みはしない

	// ソート順（遠いものから描画）
	for (auto& p : list)
	{
		DrawPlayerInternal(p.second);
	}

	// 3Dオブジェクトは深度テストを有効にして描画
	SetDepthTest(false);

	if (s_IsKonamiCodeEntered)
	{
		// ------------------------------------
		// コライダーフレーム（AABB）の描画
		// ------------------------------------
		{
			// プレイヤーの描画に使われた行列をクリアする
			XMMATRIX world = XMMatrixIdentity();
			 Shader_SetMatrix(world * GetViewMatrix() * GetProjectionMatrix()); // WVP行列をIdentity * View * Projectionに設定
			//Shader_Begin(); // シェーダーを再設定

			for (int i = 0; i < PLAYER_MAX; i++)
			{
				// AABBを描画
				// AABBのMin/Maxは既にワールド座標なので、行列はリセットしたまま描画すればOK
				Debug_DrawAABB(object[i].boundingBox, XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));
			}
		}
		//s_IsKonamiCodeEntered = false;
	}
}

void Polygon3D_DrawHP()
{
	Shader_Begin();
	
	// 個別UIステータス描画
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		SetBlendState(BLENDSTATE_ALPHA);

		DrawHP(&HPBar[i], i + 2);
		XMFLOAT2 hp = HPBar[i].pos;

		Gauge_Set(i, object[i].gl, object[i].pl, object[i].co, object[i].el,
			object[i].gaugeOuter, { hp.x - GAUGE_POS_X , hp.y + GAUGE_POS_Y});

		Gauge_Draw(i);

		Shader_Begin();

		Polygon3D_DrawStock(i);
	}
}

void Polygon3D_DrawEffect()
{
	Effect_Set(g_Texture[13], { 170.0f,600.0f }, { 400.0f, 400.0f });

	//// g_Texture[4] を X 座標のみ 4 個並べて描画する
	//// basePos: 左端の位置、size: 各エフェクトのサイズ
	//// spacingX: 各インスタンスの X 間隔（表示幅 + 余白）
	//XMFLOAT2 basePos = { 170.0f, 600.0f };
	//XMFLOAT2 size = { 400.0f, 400.0f };
	//int count = 4;
	//float spacingX = 320.0f;

	//ID3D11ShaderResourceView* texToUse = (g_effectElapsed >= 5.0f) ? g_Texture[5] : g_Texture[4];

	//Effect_SetMultiple(texToUse, basePos, size, count, spacingX);
}

void Polygon3D_Respawn(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	// 残機が1つ以上ある場合
	if (object[0].active == true && playerIndex == 0)
	{
		object[0].position = XMFLOAT3(-2.0f, 4.0f, 0.0f);
		object[0].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		object[0].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
		object[0].maxHp = 100.0f;
		object[0].hp = object[0].maxHp;
		object[0].attack = 0.0f;
		object[0].power = 0.0f;
		object[0].speed = 0.0f;
		object[0].defense = 1.0f;
		object[0].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		object[0].active = true;
		object[0].isAttacking = false;
		object[0].attackTimer = 0.0f;
		object[0].isAttacked = false;
		object[0].attackedTimer = 0.0f;
		object[0].useSkill = false;
		object[0].skillTimer = 0.0f;
		object[0].useSpecial = false;
		object[0].specialTimer = 0.0f;
		object[0].isInvincible = false;
		object[0].invincibleTimer = 0.0f;
		object[0].stunGauge = 0.0f;
		object[0].isStunning = false;
		object[0].stunTimer = 0.0f;
		object[0].isDown = false;
		object[0].downTimer = 0.0f;

		object[0].lastDir = PlayerDir::Down; // 正面
		object[0].isMoving = false;
		object[0].form = Form::Normal;
		object[0].type = PlayerType::None;
		object[0].evolutionGauge = 0;
		object[0].evolutionGaugeRate = 1;
		object[0].breakCount_Glass = 0;
		object[0].breakCount_Concrete = 0;
		object[0].breakCount_Plant = 0;
		object[0].breakCount_Electric = 0;
		object[0].brokenHistory.clear();
		object[0].gl = 1.0f;
		object[0].pl = 1.0f;
		object[0].co = 1.0f;
		object[0].el = 1.0f;
		object[0].gaugeOuter = 1.0f;

		object[0].knockback_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		object[0].is_knocked_back = false;
		object[0].knockback_duration = 0.0f;
	}

	// 残機が1つ以上ある場合
	else if (object[1].active == true && playerIndex == 1)
	{
		object[1].position = XMFLOAT3(1.5f, 4.0f, 2.0f);
		object[1].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		object[1].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
		object[1].maxHp = 100.0f;
		object[1].hp = object[1].maxHp;
		object[1].attack = 0.0f;
		object[1].power = 0.0f;
		object[1].speed = 0.0f;
		object[1].defense = 1.0f;
		object[1].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		object[1].active = true;
		object[1].isAttacking = false;
		object[1].attackTimer = 0.0f;
		object[1].isAttacked = false;
		object[1].attackedTimer = 0.0f;
		object[1].useSkill = false;
		object[1].skillTimer = 0.0f;
		object[1].useSpecial = false;
		object[1].specialTimer = 0.0f;
		object[1].isInvincible = false;
		object[1].invincibleTimer = 0.0f;
		object[1].stunGauge = 0.0f;
		object[1].isStunning = false;
		object[1].stunTimer = 0.0f;
		object[1].isDown = false;
		object[1].downTimer = 0.0f;

		object[1].lastDir = PlayerDir::Down; // 正面
		object[1].isMoving = false;
		object[1].form = Form::Normal;
		object[1].type = PlayerType::None;
		object[1].evolutionGauge = 0;
		object[1].evolutionGaugeRate = 1;
		object[1].breakCount_Glass = 0;
		object[1].breakCount_Concrete = 0;
		object[1].breakCount_Plant = 0;
		object[1].breakCount_Electric = 0;
		object[1].brokenHistory.clear();
		object[1].gl = 1.0f;
		object[1].pl = 1.0f;
		object[1].co = 1.0f;
		object[1].el = 1.0f;
		object[1].gaugeOuter = 1.0f;

		object[1].knockback_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		object[1].is_knocked_back = false;
		object[1].knockback_duration = 0.0f;
	}
	// 残機が1つ以上ある場合
	else if (object[2].active == true && playerIndex == 2)
	{

		object[2].position = XMFLOAT3(-4.0f, 4.0f, 0.0f);
		object[2].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		object[2].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
		object[2].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		object[2].maxHp = 100.0f;
		object[2].hp = object[2].maxHp;
		object[2].attack = 0.0f;
		object[2].power = 0.0f;
		object[2].speed = 0.0f;
		object[2].defense = 1.0f;
		object[2].stock = 3;
		object[2].active = true;
		object[2].isAttacking = false;
		object[2].attackTimer = 0.0f;
		object[2].isAttacked = false;
		object[2].attackedTimer = 0.0f;
		object[2].useSkill = false;
		object[2].skillTimer = 0.0f;
		object[2].useSpecial = false;
		object[2].specialTimer = 0.0f;
		object[2].isInvincible = false;
		object[2].invincibleTimer = 0.0f;
		object[2].stunGauge = 0.0f;
		object[2].isStunning = false;
		object[2].stunTimer = 0.0f;
		object[2].isDown = false;
		object[2].downTimer = 0.0f;
		object[2].lastDir = PlayerDir::Down; // 正面
		object[2].isMoving = false;
		object[2].form = Form::Normal;
		object[2].type = PlayerType::None;
		object[2].evolutionGauge = 0;
		object[2].evolutionGaugeRate = 1;
		object[2].breakCount_Glass = 0;
		object[2].breakCount_Concrete = 0;
		object[2].breakCount_Plant = 0;
		object[2].breakCount_Electric = 0;
		object[2].gl = 1.0f;
		object[2].pl = 1.0f;
		object[2].co = 1.0f;
		object[2].el = 1.0f;
		object[2].gaugeOuter = 1.0f;
	}
	// 残機が1つ以上ある場合
	else if (object[3].active == true && playerIndex == 3)
	{
		object[3].position = XMFLOAT3(4.0f, 4.0f, -2.0f);
		object[3].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		object[3].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
		object[3].maxHp = 100.0f;
		object[3].hp = object[3].maxHp;
		object[3].attack = 0.0f;
		object[3].power = 0.0f;
		object[3].speed = 0.0f;
		object[3].defense = 1.0f;
		object[3].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		object[3].stock = 3;
		object[3].active = true;
		object[3].isAttacking = false;
		object[3].attackTimer = 0.0f;
		object[3].isAttacked = false;
		object[3].attackedTimer = 0.0f;
		object[3].useSkill = false;
		object[3].skillTimer = 0.0f;
		object[3].useSpecial = false;
		object[3].specialTimer = 0.0f;
		object[3].isInvincible = false;
		object[3].invincibleTimer = 0.0f;
		object[3].stunGauge = 0.0f;
		object[3].isStunning = false;
		object[3].stunTimer = 0.0f;
		object[3].isDown = false;
		object[3].downTimer = 0.0f;
		object[3].lastDir = PlayerDir::Down; // 正面
		object[3].isMoving = false;
		object[3].form = Form::Normal;
		object[3].type = PlayerType::None;
		object[3].evolutionGauge = 0;
		object[3].evolutionGaugeRate = 1;
		object[3].breakCount_Glass = 0;
		object[3].breakCount_Concrete = 0;
		object[3].breakCount_Plant = 0;
		object[3].breakCount_Electric = 0;
		object[3].gl = 1.0f;
		object[3].pl = 1.0f;
		object[3].co = 1.0f;
		object[3].el = 1.0f;
		object[3].gaugeOuter = 1.0f;
	}
}

static inline void LoopRange(int& animFrame, int start, int count, int advance)
{
	int relative = (animFrame - start + advance) % count;
	if (relative < 0) relative += count;
	animFrame = start + relative;
}

//==================================
// 残機描画
//==================================
void Polygon3D_DrawStock(int i)
{
	Shader_Begin();
	Shader_BeginUI();

	// HPバー位置取得・ゲージ座標設定
	float bx = HPBar[i].pos.x;
	float by = HPBar[i].pos.y - 5.0f;

	// プレイヤーごとのストック描画
	for (int j = 0; j < object[i].stock; j++)
	{
		// ストック描画変数
		XMFLOAT2 pos = { bx + j * 30.0f, by };	// 横並び
		XMFLOAT2 size = { 300.0f, 300.0f };

		g_pContext->PSSetShaderResources(0, 1, &g_Texture[i + 9]);
	
		SetBlendState(BLENDSTATE_ALPHA);
		DrawSprite(pos, size, color::white);
	}
}

PLAYEROBJECT* GetPlayer(int playerIndex)
{
	// 範囲チェック 0未満 または 4以上なら nullptr を返す
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX)
	{
		return nullptr;
	}

	return &object[playerIndex];
}