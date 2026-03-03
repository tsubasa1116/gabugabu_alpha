// =====================================================
//	player.cpp
// 
//	蛻ｶ菴懆・ｼ壼ｹｳ蟯｡鬚ｯ鬥ｬ			譌･莉假ｼ・026/01/27
//======================================================

/////////////////////////////////////////////////////////////////////////
// TODO: カミナリをプレイヤーの位置を参照して発生させる
// TODO: ドッスンで建物を壊す＆プレイヤーを吹っ飛ばす
// TODO: ヒットストップ
// TODO: プレイヤーの当たり判定を直し、吹っ飛びを滑らかにする

#include <d3d11.h>
#include <iostream>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "shader.h"
#include "keyboard.h"
#include "sprite.h"
#include "color.h"
#include "hp.h"
#include "gauge.h"
#include "Effect.h"
#include "player.h"
#include "Camera.h"
#include "input.h"
#include "skill.h"
#include "special.h"
#include "field.h"
#include "collider.h"
#include "debug_render.h"
#include "debug_ostream.h"
#include "attack.h" 
#include "DamageText.h"
#include "makeText.h"
#include "Audio.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "fade.h"
#include <chrono>
#include <codecvt>
#include <vector>
#include <algorithm>
#include <cstring> // 霑ｽ蜉・嘖trcmp 縺ｮ縺溘ａ
#include "loadThread.h"


//======================================================
//	繝槭け繝ｭ螳夂ｾｩ
//======================================================
#define GAUGE_POS_X	(69.0f * (SCREEN_WIDTH / 1280.0f))	
#define GAUGE_POS_Y	(8.0f *  (SCREEN_HEIGHT / 720.0f))	
#define	HPBER_SIZE_X (270.0f * (SCREEN_WIDTH / 1280.0f))
#define	HPBER_SIZE_Y (270.0f * (SCREEN_HEIGHT / 720.0f))

//======================================================
//	繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚
//======================================================
// 繧ｪ繝悶ず繧ｧ繧ｯ繝・
PLAYEROBJECT player[PLAYER_MAX];

static ID3D11Device* g_pDevice = NULL;
static ID3D11DeviceContext* g_pContext = NULL;
static hp HPBar[PLAYER_MAX];

// 鬆らせ繝舌ャ繝輔ぃ
static ID3D11Buffer* g_VertexBuffer = NULL;

// 繧､繝ｳ繝・ャ繧ｯ繧ｹ繝舌ャ繝輔ぃ
static ID3D11Buffer* g_IndexBuffer = NULL;

// 繝・け繧ｹ繝√Ε螟画焚
static ID3D11ShaderResourceView* g_Texture[18];

// 繝励Ξ繧､繝､繝ｼ 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ逕ｨ螟画焚
static const float ANIM_FRAME_TIME = 0.15f;	// 1繝輔Ξ繝ｼ繝縺ゅ◆繧翫・遘呈焚
static const int   SHEET_COLS = 16;
static const int   SHEET_ROWS = 16;

static int g_victoryState[PLAYER_MAX] = { 0 };			// 0 = 縺ｪ縺・ 1 = 蛻晏屓 蜀咲函荳ｭ, 2 = 繝ｫ繝ｼ繝・
static float g_downHoldTimer[PLAYER_MAX] = { 0.0f };	// 譛邨ゅヵ繝ｬ繝ｼ繝繝帙・繝ｫ繝臥畑繧ｿ繧､繝槭・・医・繝ｬ繧､繝､繝ｼ豈趣ｼ・

static bool g_skillAnimStarted[PLAYER_MAX] = { false, false, false, false };
static int g_skillAnimStart[PLAYER_MAX] = { 0 };	// 繧ｹ繧ｭ繝ｫ繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ髢句ｧ九ヵ繝ｬ繝ｼ繝菫晏ｭ倡畑

static int g_specialAnimPhase[PLAYER_MAX] = { 0 };			// 0 = 蛻晏屓蜀咲函(0・・), 1 = 繝ｫ繝ｼ繝・4・・), 2 = 邨ゆｺ・ｼ泌・(7)
static float g_specialEndAnimTimer[PLAYER_MAX] = { 0.0f };	// 邨ゆｺ・ヵ繝ｬ繝ｼ繝(7)縺ｮ陦ｨ遉ｺ繧ｿ繧､繝槭・
static bool g_specialInitialize[PLAYER_MAX] = { false };

// 鬆・ｽ阪・豁ｻ莠｡鬆・・邂｡逅・
static std::vector<int> g_deathOrder;	// 豁ｻ莠｡縺励◆繝励Ξ繧､繝､繝ｼ縺ｮ繧､繝ｳ繝・ャ繧ｯ繧ｹ・亥・縺ｫ豁ｻ繧薙□閠・′蜈磯ｭ・・

static int g_SE_ID[PLAYER_SE_COUNT] = { NULL };

// 鬆らせ驟榊・
static Vertex2 vdata[PLAYER_VERTEX] =
{
	{// 鬆らせ0 LEFT-TOP
		XMFLOAT3(-COORDINATE, COORDINATE, 0.0f),	// 蠎ｧ讓・
		XMFLOAT3(0.0f, 0.0f, -1.0f),				// 豕慕ｷ壹・繧ｯ繝医Ν
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),			// 繧ｫ繝ｩ繝ｼ
		XMFLOAT2(0.0f, 0.0f)						// 繝・け繧ｹ繝√Ε蠎ｧ讓・
	},
	{// 鬆らせ1 RIGHT-TOP
		XMFLOAT3(COORDINATE, COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(TEXCOORD, 0.0f)
	},
	{// 鬆らせ2 LEFT-BOTTOM
		XMFLOAT3(-COORDINATE, -COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f, TEXCOORD)
	},
	{// 鬆らせ3 RIGHT-BOTTOM
		XMFLOAT3(COORDINATE, -COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(TEXCOORD, TEXCOORD)
	},
};

// 繧､繝ｳ繝・ャ繧ｯ繧ｹ驟榊・
static UINT idxdata[6]
{
	 0, 1, 2, 2, 1, 3, // -Z髱｢
};

static float top_y = 0;	// 蜈ｭ隗貞ｽ｢縺ｮtop-y蠎ｧ逾ｨ縺ｮ繝・ヰ繝・げ陦ｨ遉ｺ

static std::atomic<int> g_loadedCount(0);                   // 菴墓椢邨ゅｏ縺｣縺溘°・磯ｲ謐礼畑・・
static bool      s_ShowImgui = true;

//======================================================
//	蛻晄悄蛹夜未謨ｰ
//======================================================
void Player_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// 繝励Ξ繧､繝､繝ｼ陦ｨ遉ｺ縺ｮ蛻晄悄蛹・
	player[0].position = XMFLOAT3(-6.0f, 4.0f, -3.0f);
	player[1].position = XMFLOAT3(4.5f, 4.0f, 5.0f);
	player[2].position = XMFLOAT3(-7.0f, 4.0f, -6.0f);
	player[3].position = XMFLOAT3(7.0f, 4.0f, 4.0f);


	//player[0].type = PlayerType::Glass;
	//player[1].type = PlayerType::Concrete;
	//player[2].type = PlayerType::Plant;
	//player[3].type = PlayerType::Electricity;

	for (int p = 0; p < PLAYER_MAX; p++)
	{
		player[p].form = Form::First;
		//player[p].form = Form::Second;
		//player[p].form = Form::Third;

		player[p].type = PlayerType::None;
		//player[p].type = PlayerType::Glass;
		//player[p].type = PlayerType::Concrete;
		//player[p].type = PlayerType::Plant;
		//player[p].type = PlayerType::Electricity;

		player[p].velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[p].oldPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[p].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[p].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
		player[p].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[p].hp = PLAYER_MAX_HP;
		player[p].attack = 0.0f;
		player[p].power = 0.0f;
		player[p].speed = 0.0f;
		player[p].defense = 1.0f;
		player[p].stock = 3;
		player[p].rank = 0;
		player[p].active = true;
		player[p].satiety = 0.0f;
		player[p].isAttacking = false;
		player[p].attackTimer = 0.0f;
		player[p].isAttacked = false;
		player[p].attackedTimer = 0.0f;
		player[p].isDamageColor = false;
		player[p].damageColorTimer = 0.0f;
		player[p].isHealing = false;
		player[p].healingTimer = 0.0f;
		player[p].isEvolving = false;
		player[p].evolvingTimer = 0.0f;
		player[p].useSkill = false;
		player[p].skillTimer = 0.0f;
		player[p].skillCoolTimer = 0.0f;
		player[p].skillAnimation = false;
		player[p].useSpecial = false;
		player[p].specialTimer = 0.0f;
		player[p].specialAnimation = false;
		player[p].isInvincible = false;
		player[p].invincibleTimer = 0.0f;
		player[p].stunGauge = 0.0f;
		player[p].isStunning = false;
		player[p].stunTimer = 0.0f;
		player[p].isDown = false;
		player[p].downTimer = 0.0f;
		player[p].isPoisoned = false;
		player[p].poisonTimer = 0.0f;
		player[p].duringRespawn = true;
		player[p].respawnTimer = 0.0f;
		player[p].isEggBreaking = false;
		player[p].eggBreakingTimer = 0.0f;
		player[p].lastDir = PlayerDir::Down; // 豁｣髱｢
		player[p].isMoving = false;
		player[p].isShadowEnabled = false;
		player[p].evolutionGauge = 0.0f;
		player[p].evolutionGaugeRate = PLAYER_EVOLUTION_GAUGE_RATE;
		player[p].breakCount_Glass = 0;
		player[p].breakCount_Concrete = 0;
		player[p].breakCount_Plant = 0;
		player[p].breakCount_Electricity = 0;
		player[p].isTypeFixed = false;
	}

	// 鬆らせ繝舌ャ繝輔ぃ菴懈・
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));	// 0縺ｧ繧ｯ繝ｪ繧｢
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * PLAYER_VERTEX;	// 譬ｼ邏阪〒縺阪ｋ鬆らせ謨ｰ*鬆らせ繧ｵ繧､繧ｺ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	g_pDevice = pDevice;
	g_pContext = pContext;
	g_loadedCount = 0;

	// 繝ｭ繝ｼ繝峨ｒ蛻･繧ｹ繝ｬ繝・ラ縺ｧ髢句ｧ・
	// pDevice繧呈ｸ｡縺励∫ｵゆｺ・＠縺溘ｉ繝輔Λ繧ｰ繧堤ｫ九※繧・
	Loader::AddTask([pDevice]()
	{
		LoadTextureList(pDevice);

	//// ===== GPU 繝・け繧ｹ繝√Ε 繧ｦ繧ｩ繝ｼ繝繧｢繝・・ =====
	//{
	//	const size_t TEX_COUNT = sizeof(g_Texture) / sizeof(g_Texture[0]);
	//	for (size_t i = 0; i < TEX_COUNT; ++i)
	//	{
	//		if (g_Texture[i] != nullptr)
	//		{
	//			g_pContext->PSSetShaderResources(0, 1, &g_Texture[i]);
	//			g_pContext->DrawIndexed(0, 0, 0);
	//		}
	//	}
	//	ID3D11ShaderResourceView* nullSRV = nullptr;
	//	g_pContext->PSSetShaderResources(0, 1, &nullSRV);
	//}
		});

	// 繧､繝ｳ繝・ャ繧ｯ繧ｹ繝舌ャ繝輔ぃ菴懈・
	{
		D3D11_BUFFER_DESC	bd;
		ZeroMemory(&bd, sizeof(bd));	// 0縺ｧ繧ｯ繝ｪ繧｢
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(UINT) * 6;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&bd, NULL, &g_IndexBuffer);

		// 繧､繝ｳ繝・ャ繧ｯ繧ｹ繝舌ャ繝輔ぃ縺ｸ譖ｸ縺崎ｾｼ縺ｿ
		D3D11_MAPPED_SUBRESOURCE msr;
		pContext->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		UINT* index = (UINT*)msr.pData;

		// 繧､繝ｳ繝・ャ繧ｯ繧ｹ繝・・繧ｿ繧偵ヰ繝・ヵ繧｡縺ｸ繧ｳ繝斐・
		CopyMemory(&index[0], &idxdata[0], sizeof(UINT) * 6);
		pContext->Unmap(g_IndexBuffer, 0);
	}
	// 繝・ヰ繝・げ繝ｬ繝ｳ繝繝ｩ繝ｼ蛻晄悄蛹・
	Debug_Initialize(pDevice, pContext);

	float screenX = SCREEN_ADJUST_X;
	float screenY = 650.0f * SCREEN_ADJUST_Y;

	InitializeHP(pDevice, pContext, &HPBar[0], { 160.0f * screenX, screenY }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);
	InitializeHP(pDevice, pContext, &HPBar[1], { 480.0f * screenX, screenY }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);
	InitializeHP(pDevice, pContext, &HPBar[2], { 800.0f * screenX, screenY }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);
	InitializeHP(pDevice, pContext, &HPBar[3], { 1120.0f * screenX, screenY }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);

	HPBar[0].gaugeIndex = 0;
	HPBar[1].gaugeIndex = 1;
	HPBar[2].gaugeIndex = 2;
	HPBar[3].gaugeIndex = 3;

	SetDeathHP(&HPBar[0], 6);
	SetDeathHP(&HPBar[1], 7);
	SetDeathHP(&HPBar[2], 8);
	SetDeathHP(&HPBar[3], 9);

	// 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ縺ｮ蛻晄悄蛹・
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		player[p].animFrame = 0;
		player[p].animTimer = 0.0f;
		g_skillAnimStarted[p] = false;
	}

	// 鬆・ｽ肴ュ蝣ｱ繧貞・譛溷喧
	g_deathOrder.clear();

	// SE縺ｮ蛻晄悄蛹・
	g_SE_ID[0] = LoadAudio("asset\\Audio\\Roar_Form_Second.wav");	// 騾ｲ蛹門ｾ後・蜥・動 隨ｬ2蠖｢諷・
	g_SE_ID[1] = LoadAudio("asset\\Audio\\Roar_Form_Third.wav");	// 騾ｲ蛹門ｾ後・蜥・動 隨ｬ3蠖｢諷・
	g_SE_ID[2] = LoadAudio("asset\\Audio\\Transform.wav");			// 螟芽ｺｫ
	g_SE_ID[3] = LoadAudio("asset\\Audio\\EggBreaking.wav");		// 蜊ｵ蜑ｲ繧後ｋ
}

static void LoadTextureList(ID3D11Device* pDevice)
{
	TexMetadata metadata;
	ScratchImage image;

	struct TexEntry { int idx; const wchar_t* path; };

	const TexEntry texList[] =
	{
		{  0, L"asset\\texture\\characterMiniRed_v2.png"},			// 隨ｬ1蠖｢諷・P1 襍､
		{  1, L"asset\\texture\\characterMiniBlue_v1.png"},			// 隨ｬ1蠖｢諷・P2 髱・
		{  2, L"asset\\texture\\characterMiniYellow_v1.png"},		// 隨ｬ1蠖｢諷・P3 鮟・
		{  3, L"asset\\texture\\characterMiniGreen_v1.png"},		// 隨ｬ1蠖｢諷・P4 邱・
		{  4, L"asset\\texture\\characterMidGlass_v1.png"},			// 隨ｬ2蠖｢諷・繧ｬ繝ｩ繧ｹ
		{  5, L"asset\\texture\\characterMidConcrete_v1.png" },		// 隨ｬ2蠖｢諷・繧ｳ繝ｳ繧ｯ繝ｪ繝ｼ繝・
		{  6, L"asset\\texture\\characterMidTree_v1.png" },			// 隨ｬ2蠖｢諷・讀咲黄
		{  7, L"asset\\texture\\characterMidElectricity_v1.png" },	// 隨ｬ2蠖｢諷・髮ｻ豌・
		{  8, L"asset\\texture\\characterBigGlass_v2.png" },		// 隨ｬ3蠖｢諷・繧ｬ繝ｩ繧ｹ
		{  9, L"asset\\texture\\characterBigConcrete_v2.png" },		// 隨ｬ3蠖｢諷・繧ｳ繝ｳ繧ｯ繝ｪ繝ｼ繝・
		{ 10, L"asset\\texture\\characterBigTree_v2.png" },			// 隨ｬ3蠖｢諷・讀咲黄
		{ 11, L"asset\\texture\\characterBigElectricity_v2.png" },	// 隨ｬ3蠖｢諷・髮ｻ豌・
		{ 12, L"asset\\texture\\uiCharacterSkill_v2.png" },			// 隨ｬ2蠖｢諷・隨ｬ3蠖｢諷・繧ｹ繧ｭ繝ｫ
		{ 13, L"asset\\texture\\characterBigSP_v4.png" },			// 隨ｬ3蠖｢諷・繧ｹ繝壹す繝｣繝ｫ
		{ 14, L"asset\\texture\\uiStockRed_v4.png"},				// UI 繧ｹ繝医ャ繧ｯ 襍､
		{ 15, L"asset\\texture\\uiStockBlue_v4.png"},				// UI 繧ｹ繝医ャ繧ｯ 髱・
		{ 16, L"asset\\texture\\uiStockYellow_v4.png" },			// UI 繧ｹ繝医ャ繧ｯ 鮟・
		{ 17, L"asset\\texture\\uiStockGreen_v4.png" },				// UI 繧ｹ繝医ャ繧ｯ 邱・
	};

	for (const auto& e : texList)
	{
		auto start = std::chrono::high_resolution_clock::now();

		// 繧ｳ繝｡繝ｳ繝亥喧縺励※縺・ｋ隕∫ｴ縺ｯ驟榊・繧ｨ繝ｳ繝医Μ閾ｪ菴薙ｒ繧ｳ繝｡繝ｳ繝医い繧ｦ繝医＠縺ｦ縺・ｋ縺溘ａ縺薙％縺ｫ縺ｯ譚･縺ｪ縺・・
		HRESULT hr = LoadFromWICFile(e.path, WIC_FLAGS_NONE, &metadata, image);
		if (SUCCEEDED(hr))
		{
			if (FAILED(CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[e.idx])))
			{
				// 菴懈・螟ｱ謨玲凾縺ｯ nullptr 繧剃ｻ｣蜈･縺励※邯夊｡・
				g_Texture[e.idx] = nullptr;
			}
			g_loadedCount++;

		}
		// 隱ｭ縺ｿ霎ｼ縺ｿ螟ｱ謨励・ nullptr 繧剃ｻ｣蜈･縺励※邯夊｡・
		else	g_Texture[e.idx] = nullptr;

		auto end = std::chrono::high_resolution_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		// std::wstring 繧・std::string 縺ｫ螟画鋤縺励※蜃ｺ蜉・
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		hal::dout << "繝・け繧ｹ繝√Ε繝ｭ繝ｼ繝・ " << conv.to_bytes(e.path) << " " << ms << " ms" << std::endl;
	}
}

void Player_Warmup()
{
	if (!g_pContext) return;

	// ===== GPU 繝・け繧ｹ繝√Ε 繧ｦ繧ｩ繝ｼ繝繧｢繝・・ =====
	const size_t TEX_COUNT = sizeof(g_Texture) / sizeof(g_Texture[0]);
	for (size_t i = 0; i < TEX_COUNT; ++i)
	{
		if (g_Texture[i] != nullptr)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[i]);
			g_pContext->DrawIndexed(0, 0, 0); 
		}
	}

	// 譛蠕後↓繝ｪ繧ｻ繝・ヨ縺励※縺翫￥
	ID3D11ShaderResourceView* nullSRV = nullptr;
	g_pContext->PSSetShaderResources(0, 1, &nullSRV);
}

//======================================================
//	邨ゆｺ・・逅・未謨ｰ
//======================================================
void Player_Finalize()
{
	// 繧ｷ繧ｧ繝ｼ繝繝ｼ縺ｫ繝舌う繝ｳ繝峨＆繧後※縺・ｋ SRV 繧偵い繝ｳ繝舌う繝ｳ繝会ｼ亥ｮ牙・縺ｮ縺溘ａ蜈ｨ隕∫ｴ蛻・ｼ・
	const size_t TEX_COUNT = sizeof(g_Texture) / sizeof(g_Texture[0]);
	if (g_pContext)
	{
		// 蝗ｺ螳夐聞驟榊・繧剃ｽｿ縺｣縺ｦ遒ｺ螳溘↓ nullptr 繧呈ｸ｡縺呻ｼ・PI 縺ｯ逕滄・蛻励ｒ隕∵ｱゑｼ・
		ID3D11ShaderResourceView* nullSRV[25] = {};
		g_pContext->PSSetShaderResources(0, static_cast<UINT>(TEX_COUNT), nullSRV);
	}

	// 繧､繝ｳ繝・ャ繧ｯ繧ｹ・城らせ繝舌ャ繝輔ぃ縺ｮ隗｣謾ｾ・・ULL 繝√ぉ繝・け蠕後↓ nullptr 縺ｫ險ｭ螳夲ｼ・
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

	// 繝・け繧ｹ繝√Ε驟榊・蜈ｨ隕∫ｴ繧貞ｮ牙・縺ｫ隗｣謾ｾ・医さ繝｡繝ｳ繝亥喧縺励※譛ｪ繝ｭ繝ｼ繝峨・隕∫ｴ繧・nullptr 繝√ぉ繝・け縺ｧ螳牙・・・
	for (size_t i = 0; i < TEX_COUNT; ++i)
	{
		if (g_Texture[i] != nullptr)
		{
			g_Texture[i]->Release();
			g_Texture[i] = nullptr;
		}
	}

	// 繝・ヰ繧､繧ｹ・上さ繝ｳ繝・く繧ｹ繝医・螟夜Κ邂｡逅・・縺溘ａ隗｣謾ｾ縺励↑縺・′縲∝盾辣ｧ縺ｯ繧ｯ繝ｪ繧｢縺励※縺翫￥
	g_pContext = nullptr;
	g_pDevice = nullptr;

	// 繝・ヰ繝・げ繝ｬ繝ｳ繝繝ｩ繝ｼ縺ｮ邨ゆｺ・・逅・
	Debug_Finalize();

	for (int i = 0; i < PLAYER_SE_COUNT; ++i)	UnloadAudio(g_SE_ID[i]);
}

// ======================================================
// 遘ｻ蜍暮未謨ｰ・郁ｦ∝､画峩・・
// ------------------------------------------------------
// 遘ｻ蜍輔・繧ｯ繝医Ν縺ｨ蜷代＞縺ｦ縺・ｋ譁ｹ蜷代・繧ｯ繝医Ν縺ｯ蛻･縺ｧ謖√▲縺滓婿縺後＞縺・
// ======================================================
// 蜈･蜉・繝ｭ繝ｼ繧ｫ繝ｫ)繧偵き繝｡繝ｩ蝓ｺ貅悶〒繝ｯ繝ｼ繝ｫ繝厩Z縺ｸ螟画鋤縺吶ｋ・亥ｹｳ髱｢遘ｻ蜍慕畑・・
static inline XMFLOAT3 ToWorldMoveDirByCamera(const XMFLOAT2& input)
{
	// input.x: 蜿ｳ(+), input.y: 荳・+)
	XMMATRIX view = GetViewMatrix();
	XMMATRIX invView = XMMatrixInverse(nullptr, view);

	// invView 縺ｮ陦後°繧峨き繝｡繝ｩ霆ｸ繧貞叙蠕暦ｼ・orld・・
	XMFLOAT3 right = XMFLOAT3(invView.r[0].m128_f32[0], invView.r[0].m128_f32[1], invView.r[0].m128_f32[2]);
	XMFLOAT3 forward = XMFLOAT3(invView.r[2].m128_f32[0], invView.r[2].m128_f32[1], invView.r[2].m128_f32[2]);

	// XZ蟷ｳ髱｢縺ｸ蟆・ｽｱ・・謌仙・繧呈昏縺ｦ繧具ｼ・
	right.y = 0.0f;
	forward.y = 0.0f;

	// 豁｣隕丞喧・医き繝｡繝ｩ縺檎悄荳翫↓霑代＞遲峨〒繧ｼ繝ｭ蜑ｲ繧翫ｒ驕ｿ縺代ｋ・・
	{
		float rl = sqrtf(right.x * right.x + right.z * right.z);
		if (rl > 0.0001f) { right.x /= rl; right.z /= rl; }
	}
	{
		float fl = sqrtf(forward.x * forward.x + forward.z * forward.z);
		if (fl > 0.0001f) { forward.x /= fl; forward.z /= fl; }
	}

	// 繝ｭ繝ｼ繧ｫ繝ｫ蜈･蜉帙ｒ繝ｯ繝ｼ繝ｫ繝峨∈蜷域・
	XMFLOAT3 worldDir;
	worldDir.x = right.x * input.x + forward.x * input.y;
	worldDir.y = 0.0f;
	worldDir.z = right.z * input.x + forward.z * input.y;
	return worldDir;
}


void Move(PLAYEROBJECT& player, XMFLOAT3 moveDir)
{
	// 騾ｲ縺ｿ縺溘＞譁ｹ蜷托ｼ・蟷ｳ譁ｹ・・
	float length = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);

	if (length > 0.0f)
	{
		// 繝吶け繝医Ν縺ｮ豁｣隕丞喧
		moveDir.x /= length;
		moveDir.z /= length;

		// 逶ｮ讓呵ｧ貞ｺｦ繧呈ｱゅａ繧・
		float targetAngle = atan2f(moveDir.x, moveDir.z);	// 繝吶け繝医Ν縺ｮ隗貞ｺｦ
		targetAngle = XMConvertToDegrees(targetAngle);		// 繝ｩ繧ｸ繧｢繝ｳ -> 蠎ｦ

		// 蟾ｮ蛻・ｒ隱ｿ謨ｴ・・80蠎ｦ雜・∴縺ｪ縺・ｈ縺・↓・・
		float diff = targetAngle - player.moveAngle;	// 隗貞ｺｦ蟾ｮ
		if (diff > 180.0f) diff -= 360.0f;
		if (diff < -180.0f) diff += 360.0f;

		static float angSpeed = 0.9f;

		// 繧ｹ繝繝ｼ繧ｺ縺ｫ陬憺俣・・.1f縺瑚｣憺俣繧ｹ繝斐・繝会ｼ・
		player.moveAngle += diff * angSpeed;

		player.rotation.y = player.moveAngle;	// 隗貞ｺｦ縺ｮ蜿肴丐

		// 蜑埼ｲ
		float rad = XMConvertToRadians(player.moveAngle);

		player.position.x += sinf(rad) * player.speed;
		player.position.z += cosf(rad) * player.speed;
	}
}

//======================================================
// 譖ｴ譁ｰ髢｢謨ｰ
//======================================================
void Player_Update()
{
	// 蜷・・繝ｬ繧､繝､繝ｼ縺ｫ蟇ｾ蠢懊☆繧狗匱蜍輔く繝ｼ
	const Keyboard_Keys_tag attackKeys[PLAYER_MAX] = { KK_SPACE, KK_ENTER, KK_V, KK_NUMPAD0 };

	const Keyboard_Keys_tag specialKeys[PLAYER_MAX] = { KK_D7, KK_D8, KK_D9, KK_D0 };

	if (Keyboard_IsKeyDownTrigger(KK_TAB))	s_ShowImgui = !s_ShowImgui;

	if (s_ShowImgui)
	{
		// 繝・ヰ繝・げ逕ｨ ImGui 繧ｦ繧｣繝ｳ繝峨え
		ImGui::Begin("Player Debug");

		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			// 繝励Ξ繧､繝､繝ｼ縺斐→縺ｫ ID 繧貞・縺代ｋ・亥酔荳繝ｩ繝吶Ν陦晉ｪ∝屓驕ｿ・・
			ImGui::PushID(p);
			ImGui::Text("Player %d", p + 1);
			ImGui::Indent();

			ImGui::SliderFloat("poisonTimer", &player[p].poisonTimer, 0.0f, 5.0f);
			ImGui::SliderFloat("specialTimer", &player[p].specialTimer, 0.0f, 10.0f);
			ImGui::SliderFloat("stunGauge", &player[p].stunGauge, 0.0f, 10.0f);
			ImGui::SliderFloat("satiety", &player[p].satiety, 0.0f, 6.0f);
			ImGui::BulletText("position.y        : %.2f", player[p].position.y);
			ImGui::BulletText("isEggBreaking     : %d", player[p].isEggBreaking);
			ImGui::BulletText("isShadowEnabled   : %d", player[p].isShadowEnabled);
			ImGui::BulletText("isHealing         : %d", player[p].isHealing);
			ImGui::BulletText("isPoisoned        : %d", player[p].isPoisoned);
			ImGui::BulletText("isInvincible      : %d", player[p].isInvincible);
			ImGui::BulletText("useSkill          : %d", player[p].useSkill);
			ImGui::BulletText("EvolutionGauge    : %.1f", player[p].evolutionGauge);
			ImGui::BulletText("EvolutionGaugeRate: %.1f", player[p].evolutionGaugeRate);

			if (ImGui::Button("hp -1"))			player[p].hp -= 0.1f;
			if (ImGui::Button("gl +1"))			player[p].breakCount_Glass += 1;
			else if (ImGui::Button("pl +1"))	player[p].breakCount_Plant += 1;
			else if (ImGui::Button("co +1"))	player[p].breakCount_Concrete += 1;
			else if (ImGui::Button("el +1"))	player[p].breakCount_Electricity += 1;

			ImGui::SliderFloat("HP", &player[p].hp, 0.0f, 500.0f);
			ImGui::SliderFloat("Outer", &player[p].evolutionGauge, 0.0f, 1.0f);
			ImGui::BulletText("2 Concrete breaks : %d", player[p].breakCount_Concrete);
			ImGui::BulletText("3 Plant breaks    : %d", player[p].breakCount_Plant);
			ImGui::BulletText("4 Electricity breaks : %d", player[p].breakCount_Electricity);

			// 螻･豁ｴ繝ｪ繧ｹ繝医・繧ｵ繧､繧ｺ繧定｡ｨ遉ｺ
			size_t historySize = player[p].brokenHistory.size();
			ImGui::BulletText("brokenHistory Size : %zu", historySize);

			if (historySize > 0)
			{
				ImGui::Indent(); // 螻･豁ｴ繧偵＆繧峨↓荳谿ｵ繧､繝ｳ繝・Φ繝・
				ImGui::Text("History (Latest -> Oldest):");

				// 螻･豁ｴ繧呈怙譁ｰ・域忰蟆ｾ・峨°繧牙商縺・婿縺ｸ繝ｫ繝ｼ繝励＠縺ｦ陦ｨ遉ｺ
				for (int i = (int)historySize - 1; i >= 0; --i)
				{
					// BuildingType 縺ｯ enum蝙具ｼ域紛謨ｰ蛟､・峨↑縺ｮ縺ｧ縲√◎縺ｮ縺ｾ縺ｾ %d 縺ｧ陦ｨ遉ｺ蜿ｯ閭ｽ
					// 縺ｾ縺溘・縲！mGui::Text縺ｧ謨ｴ蠖｢縺励※陦ｨ遉ｺ縺吶ｋ

					// 萓・: 螻･豁ｴ縺ｮ繧､繝ｳ繝・ャ繧ｯ繧ｹ縺ｨ蛟､繧堤峩謗･陦ｨ遉ｺ
					// ImGui::BulletText("[%d]: %d", p, (int)object[p].brokenHistory[p]);

					// 萓・: 螻･豁ｴ縺ｮ蛟､繧呈ｨｪ縺ｫ荳ｦ縺ｹ縺ｦ陦ｨ遉ｺ
					ImGui::SameLine(); // 蜷後§陦後↓陦ｨ遉ｺ
					// 螻･豁ｴ縺ｮ蛟､・域紛謨ｰ・峨ｒ譁・ｭ怜・縺ｫ螟画鋤縺励※縺九ｉ陦ｨ遉ｺ
					ImGui::Text("%d", (int)player[p].brokenHistory[i]);
				}

				// 螻･豁ｴ縺梧ｨｪ縺ｫ荳ｦ縺ｳ縺吶℃縺ｪ縺・ｈ縺・隼陦・
				ImGui::NewLine();
				ImGui::Unindent();
			}

			ImGui::Unindent();
			ImGui::Separator();
			ImGui::PopID();
		}
		ImGui::End();
	}
	
	for (int p = 0; p < PLAYER_MAX; ++p)
	{

		if (!player[p].active) continue;

		// 繝ｯ繝ｼ繝ｫ繝牙ｺｧ讓吶ｒ繧ｹ繧ｯ繝ｪ繝ｼ繝ｳ蠎ｧ讓吶↓螟画鋤
		XMFLOAT3 worldPos = player[p].position;
		worldPos.y += 2.0f; // 繝励Ξ繧､繝､繝ｼ縺ｮ荳頑婿縺ｫ陦ｨ遉ｺ

		XMVECTOR posVec = XMLoadFloat3(&worldPos);
		XMMATRIX view = GetViewMatrix();
		XMMATRIX proj = GetProjectionMatrix();
		XMMATRIX viewProj = view * proj;

		// 繝薙Η繝ｼ繝昴・繝亥､画鋤
		XMVECTOR screenPos = XMVector3Project
		(
			posVec,
			0.0f, 0.0f,
			SCREEN_WIDTH, SCREEN_HEIGHT,
			0.0f, 1.0f,
			proj, view,
			XMMatrixIdentity()
		);

		// Z蛟､繝√ぉ繝・け・医き繝｡繝ｩ縺ｮ蠕後ｍ縺ｪ繧画緒逕ｻ縺励↑縺・ｼ・
		float screenZ = XMVectorGetZ(screenPos);
		if (screenZ > 0.0f && screenZ < 1.0f)
		{
			float screenX = XMVectorGetX(screenPos);
			float screenY = XMVectorGetY(screenPos);

			// 繝・く繧ｹ繝域緒逕ｻ・・pdate蜀・〒縺ｯ蜻ｼ縺ｳ蜃ｺ縺輔↑縺・．raw蜀・〒謠冗判縺吶ｋ・・
			// 縺薙％縺ｧ縺ｯ蠎ｧ讓吶ｒ菫晏ｭ倥＠縺ｦ縺翫￥
			player[p].screenPos = XMFLOAT2(screenX, screenY);
			player[p].isOnScreen = true;
		}
		else	player[p].isOnScreen = false;

		// -------------------------------------------------------------
		// 螟芽ｺｫ
		// -------------------------------------------------------------
		switch (player[p].form)
		{
		case Form::First:	// 隨ｬ1蠖｢諷・
			player[p].scaling.x = 0.5f;
			player[p].scaling.y = 0.5f;
			player[p].scaling.z = 0.5f;
			player[p].attack = 10.0f;
			player[p].power = 0.3f;
			player[p].weight = 0.5f;
			player[p].speed = 0.07f;
			player[p].isTypeFixed = false;	// 繧ｹ繧ｭ繝ｫ繧ｯ繝ｼ繝ｫ繧ｿ繧､繝UI縺ｮ陦ｨ遉ｺ縺ｫ菴ｿ逕ｨ
			break;

		case Form::Second:	// 隨ｬ2蠖｢諷・
			player[p].scaling.x = 0.8f;
			player[p].scaling.y = 0.8f;
			player[p].scaling.z = 0.8f;
			player[p].attack = 15.0f;
			player[p].power = 0.4f;
			player[p].weight = 0.6f;
			player[p].speed = 0.06f;
			player[p].isTypeFixed = true;
			break;

		case Form::Third:	// 隨ｬ3蠖｢諷・
			player[p].scaling.x = 1.2f;
			player[p].scaling.y = 1.2f;
			player[p].scaling.z = 1.2f;
			player[p].attack = 20.0f;
			player[p].power = 0.5f;
			player[p].weight = 0.7f;
			player[p].speed = 0.05f;
			player[p].isTypeFixed = true;
			break;
		default:
			break;
		}

		// 蝗槫ｾｩ繝輔Λ繧ｰ縺ｮ譖ｴ譁ｰ
		if (player[p].isHealing)
		{
			player[p].healingTimer += DELTA_TIME;	// 蝗槫ｾｩ繧ｿ繧､繝槭・繧呈峩譁ｰ

			if (player[p].healingTimer >= HEALING_TIME)
			{
				player[p].isHealing = false;	// 蝗槫ｾｩ邨ゆｺ・
				player[p].healingTimer = 0.0f;	// 繧ｿ繧､繝槭・繝ｪ繧ｻ繝・ヨ
			}
		}

		// 騾ｲ蛹悶ヵ繝ｩ繧ｰ縺ｮ譖ｴ譁ｰ
		if (player[p].isEvolving)
		{
			player[p].evolvingTimer += DELTA_TIME;	// 騾ｲ蛹悶ち繧､繝槭・繧呈峩譁ｰ

			if (player[p].evolvingTimer >= EVOLVING_TIME)
			{
				player[p].isEvolving = false;	// 騾ｲ蛹也ｵゆｺ・
				player[p].evolvingTimer = 0.0f;	// 繧ｿ繧､繝槭・繝ｪ繧ｻ繝・ヨ
			}
		}

		// 貅閻ｹ蠎ｦ縺ｮ貂帛ｰ・
		player[p].satiety -= DELTA_TIME;
		if (player[p].satiety < 0.0f)	player[p].satiety = 0.0f;
		//// 貅閻ｹ蠎ｦ縺・譛ｪ貅縺ｪ繧羽P繧呈ｸ帛ｰ代＆縺帙ｋ
		//if (player[p].satiety < 1.0f)	player[p].hp -= 0.05f;

		// 繝ｪ繧ｹ繝昴・繝ｳ蜃ｦ逅・
		if (player[p].duringRespawn)
		{
			if (GetGamePhase() == PHASE_PLAY)
			{
				player[p].respawnTimer += DELTA_TIME;


				// Y蠎ｧ讓吶ｒ4縺ｫ蝗ｺ螳・
				player[p].position.y = 4.0f;

				// 謾ｻ謦・・繧ｿ繝ｳ謚ｼ荳九∪縺溘・5遘堤ｵ碁℃縺ｧ關ｽ荳矩幕蟋・
				if (g_Input[p].A || Keyboard_IsKeyDownTrigger(attackKeys[p]) || player[p].respawnTimer >= 5.0f)
				{
					player[p].duringRespawn = false;
					player[p].respawnTimer = 0.0f;
					player[p].isInvincible = true;
					player[p].invincibleTimer = 0.0f;
					player[p].isEggBreaking = true;
					player[p].eggBreakingTimer = 0.0f;
				}
			}
		}
		else
		{
			// y霆ｸ縺ｮ遘ｻ蜍暮㍼ (驥榊鴨 + 繧ｸ繝｣繝ｳ繝・
			// 驥榊鴨蜉騾溷ｺｦ縺ｮ縺ｪ縺・ｰ｡譏鍋噪縺ｪ驥榊鴨
			player[p].position.y += -0.1f;
		}

		// 蜊ｵ繧ｨ繝輔ぉ繧ｯ繝医′蜑ｲ繧後ｋ譎る俣
		if (player[p].isEggBreaking)
		{
			if (player[p].eggBreakingTimer == 0.0f)	PlayAudio(g_SE_ID[3], false);

			player[p].eggBreakingTimer += DELTA_TIME;

			if (player[p].eggBreakingTimer >= EGG_BREAKING_TIME)
			{
				player[p].isEggBreaking = false;
				player[p].eggBreakingTimer = 0.0f;
			}
		}

		// 豈堤憾諷九・蜃ｦ逅・
		if (player[p].poisonTimer > 0.0f)
		{
			// 辟｡謨ｵ荳ｭ縺ｯ繝繝｡繝ｼ繧ｸ繧剃ｸ弱∴縺ｪ縺・′縲√％縺薙〒繝ｫ繝ｼ繝励ｒ謚懊￠縺ｪ縺・ｼ井ｻ･髯阪・迚ｩ逅・・蠖薙◆繧雁愛螳壹・螳溯｡後☆繧具ｼ・
			if (!player[p].isInvincible)
			{
				// 豈堤憾諷九・髢薙√ム繝｡繝ｼ繧ｸ繧剃ｸ弱∴繧・
				player[p].hp -= SPECIAL_PLANT_DAMAGE * player[p].defense;
			}

			// 豈偵ち繧､繝槭・繧帝ｲ繧√ｋ
			player[p].poisonTimer -= DELTA_TIME;

			// 豈偵ち繧､繝槭・縺・縺ｫ縺ｪ縺｣縺溘ｉ豈堤憾諷九ｒ隗｣髯､
			if (player[p].poisonTimer <= 0.0f)
			{
				player[p].isPoisoned = false;
				player[p].poisonTimer = 0.0f;
			}
		}

		// 繧ｹ繧ｿ繝ｳ繧ｲ繝ｼ繧ｸ縺梧怙螟ｧ縺ｧ繧ｹ繧ｿ繝ｳ繝輔Λ繧ｰ繧堤ｫ九※繧・
		if (player[p].stunGauge >= STUNGAUGE_MAX)
		{
			player[p].isStunning = true;
			player[p].stunGauge = STUNGAUGE_MAX;
		}
		// 繧ｹ繧ｿ繝ｳ荳ｭ縺ｮ蜃ｦ逅・
		if (player[p].isStunning)
		{
			// 繧ｹ繧ｿ繝ｳ繧ｿ繧､繝槭・繧帝ｲ繧√ｋ
			player[p].stunTimer += DELTA_TIME;

			// 譎る俣邨碁℃縺ｧ繧ｹ繧ｿ繝ｳ隗｣髯､
			if (player[p].stunTimer >= STUN_TIME)
			{
				player[p].isStunning = false;	// 繧ｹ繧ｿ繝ｳ隗｣髯､
				player[p].stunTimer = 0.0f;		// 繧ｹ繧ｿ繝ｳ繧ｿ繧､繝槭・繝ｪ繧ｻ繝・ヨ
				player[p].stunGauge = 0.0f;		// 繧ｹ繧ｿ繝ｳ繧ｲ繝ｼ繧ｸ繝ｪ繧ｻ繝・ヨ
			}

			// 繧ｹ繧ｿ繝ｳ荳ｭ縺ｯ遘ｻ蜍輔・繧ｯ繝医Ν繧貞ｮ悟・縺ｫ繧ｼ繝ｭ縺ｫ縺吶ｋ
			player[p].moveDir = { 0.0f, 0.0f, 0.0f };

			player[p].isMoving = false;

			player[p].isAttacking = false;
			player[p].useSkill = false;
			player[p].useSpecial = false;
		}
		else // 繧ｹ繧ｿ繝ｳ縺励※縺・↑縺・ｴ蜷医・蜃ｦ逅・
		{
			// 繧ｹ繧ｿ繝ｳ縺励※縺・↑縺・俣縺ｯ繧ｹ繧ｿ繝ｳ繧ｲ繝ｼ繧ｸ繧呈ｸ帛ｰ代＆縺帙ｋ
			player[p].stunGauge -= DELTA_TIME;

			// 繧ｹ繧ｿ繝ｳ繧ｲ繝ｼ繧ｸ縺・譛ｪ貅縺ｫ縺ｪ繧峨↑縺・ｈ縺・↓繧ｯ繝ｩ繝ｳ繝・
			if (player[p].stunGauge < 0.0f)	player[p].stunGauge = 0.0f;
		}

		// 繧ｹ繧ｿ繝ｳ荳ｭ繝ｻ繝繧ｦ繝ｳ荳ｭ縺ｧ縺ｪ縺代ｌ縺ｰ隨ｬ1蠖｢諷玖｡悟虚 1菴咲｢ｺ螳壼ｾ後・繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ縺ｮ縺ｿ
		if (!player[p].isStunning && !player[p].isDown && player[p].rank != 1 && player[p].active)
		{
			if (GetGamePhase() == PHASE_PLAY)
			{
				// 逋ｺ蜍輔ヨ繝ｪ繧ｬ繝ｼ蜈･蜉帙ｒ繝√ぉ繝・け縺励※謾ｻ謦・ヵ繝ｩ繧ｰ繧堤ｫ九※繧・
				if (Keyboard_IsKeyDownTrigger(attackKeys[p]))
				{
					player[p].isAttacking = true;

					// 隨ｬ2繝ｻ隨ｬ3蠖｢諷九・蝣ｴ蜷医√せ繧ｭ繝ｫ菴ｿ逕ｨ繝輔Λ繧ｰ繧らｫ九※繧・
					if (player[p].type != PlayerType::None)	player[p].useSkill = true;
				}
				if (g_Input[p].A)	player[p].isAttacking = true;

				// 隨ｬ2繝ｻ隨ｬ3蠖｢諷九・蝣ｴ蜷医せ繧ｭ繝ｫ菴ｿ逕ｨ繝輔Λ繧ｰ遶九※繧・
				if (g_Input[p].X)	if (player[p].type != PlayerType::None)	player[p].useSkill = true;

				// 逋ｺ蜍輔ヨ繝ｪ繧ｬ繝ｼ蜈･蜉帙ｒ繝√ぉ繝・け縺励※繧ｹ繝壹す繝｣繝ｫ菴ｿ逕ｨ繝輔Λ繧ｰ繧堤ｫ九※繧・
				if (player[p].form == Form::Third && Keyboard_IsKeyDownTrigger(specialKeys[p]))	player[p].useSpecial = true;

				// 繝懊ち繝ｳ蜈･蜉帙ｒ繝√ぉ繝・け縺励※繧ｹ繝壹す繝｣繝ｫ菴ｿ逕ｨ繝輔Λ繧ｰ繧堤ｫ九※繧・
				if (player[p].form == Form::Third && g_Input[p].ZR)	player[p].useSpecial = true;

				// 繝輔Λ繧ｰ縺檎ｫ九▲縺溘ｉ譖ｴ譁ｰ蜃ｦ逅・ｒ蜻ｼ縺ｳ蜃ｺ縺・
				if (player[p].isAttacking)	Attack_Update(p);	// 謾ｻ謦・
				if (player[p].useSkill)		Skill_Update(p);	// 繧ｹ繧ｭ繝ｫ
				if (player[p].useSpecial)	Special_Update(p);	// 繧ｹ繝壹す繝｣繝ｫ

				// 迴ｾ蝨ｨ縺ｮ繝励Ξ繧､繝､繝ｼ p 縺ｮ遘ｻ蜍輔・繧ｯ繝医Ν縺縺代ｒ繝ｪ繧ｻ繝・ヨ
				player[p].moveDir = { 0.0f, 0.0f, 0.0f };

				XMFLOAT2 moveInput = { 0.0f, 0.0f };

				// 繧ｹ繝壹す繝｣繝ｫ 繧ｳ繝ｳ繧ｯ繝ｪ繝ｼ繝井ｽｿ逕ｨ荳ｭ縺ｯ遘ｻ蜍穂ｸ榊庄
				if (player[p].useSpecial && player[p].type == PlayerType::Concrete)
				{
					player[p].moveDir = { 0.0f, 0.0f, 0.0f };
					player[p].isMoving = false;
				}
				// 繧ｹ繝壹す繝｣繝ｫ 繧ｳ繝ｳ繧ｯ繝ｪ繝ｼ繝井ｽｿ逕ｨ荳ｭ縺ｧ縺ｪ縺代ｌ縺ｰ遘ｻ蜍募・逅・
				else
				{
					player[p].moveInput2D = { 0.0f, 0.0f };

					if (p == 0) // 繝励Ξ繧､繝､繝ｼ0 (WASD) 謾ｻ謦・Space
					{
						if (g_Input[0].LStickY < 0.0f) { moveInput.y += 1.0f; player[0].isMoving = true; }
						if (g_Input[0].LStickY > 0.0f) { moveInput.y -= 1.0f; player[0].isMoving = true; }
						if (g_Input[0].LStickX < 0.0f) { moveInput.x -= 1.0f; player[0].isMoving = true; }
						if (g_Input[0].LStickX > 0.0f) { moveInput.x += 1.0f; player[0].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_W)) { moveInput.y += 1.0f; player[0].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_S)) { moveInput.y -= 1.0f; player[0].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_A)) { moveInput.x -= 1.0f; player[0].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_D)) { moveInput.x += 1.0f; player[0].isMoving = true; }
						if (moveInput.x == 0.0f && moveInput.y == 0.0f)	player[0].isMoving = false;
					}
					else if (p == 1) // 繝励Ξ繧､繝､繝ｼ1 (遏｢蜊ｰ繧ｭ繝ｼ) 謾ｻ謦・Enter
					{
						if (g_Input[1].LStickY < 0.0f) { moveInput.y += 1.0f; player[1].isMoving = true; }
						if (g_Input[1].LStickY > 0.0f) { moveInput.y -= 1.0f; player[1].isMoving = true; }
						if (g_Input[1].LStickX < 0.0f) { moveInput.x -= 1.0f; player[1].isMoving = true; }
						if (g_Input[1].LStickX > 0.0f) { moveInput.x += 1.0f; player[1].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_UP)) { moveInput.y += 1.0f; player[1].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_DOWN)) { moveInput.y -= 1.0f; player[1].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_LEFT)) { moveInput.x -= 1.0f; player[1].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_RIGHT)) { moveInput.x += 1.0f; player[1].isMoving = true; }
						if (moveInput.x == 0.0f && moveInput.y == 0.0f)	player[1].isMoving = false;
					}
					else if (p == 2) // 繝励Ξ繧､繝､繝ｼ2 (TFGH) 謾ｻ謦・V
					{
						if (g_Input[2].LStickY < 0.0f) { moveInput.y += 1.0f; player[2].isMoving = true; }
						if (g_Input[2].LStickY > 0.0f) { moveInput.y -= 1.0f; player[2].isMoving = true; }
						if (g_Input[2].LStickX < 0.0f) { moveInput.x -= 1.0f; player[2].isMoving = true; }
						if (g_Input[2].LStickX > 0.0f) { moveInput.x += 1.0f; player[2].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_T)) { moveInput.y += 1.0f; player[2].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_G)) { moveInput.y -= 1.0f; player[2].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_F)) { moveInput.x -= 1.0f; player[2].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_H)) { moveInput.x += 1.0f; player[2].isMoving = true; }
						if (moveInput.x == 0.0f && moveInput.y == 0.0f)	player[2].isMoving = false;
					}
					if (p == 3) // 繝励Ξ繧､繝､繝ｼ3 (WASD) 謾ｻ謦・Space
					{
						if (g_Input[3].LStickY < 0.0f) { moveInput.y += 1.0f; player[3].isMoving = true; }
						if (g_Input[3].LStickY > 0.0f) { moveInput.y -= 1.0f; player[3].isMoving = true; }
						if (g_Input[3].LStickX < 0.0f) { moveInput.x -= 1.0f; player[3].isMoving = true; }
						if (g_Input[3].LStickX > 0.0f) { moveInput.x += 1.0f; player[3].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_NUMPAD8)) { moveInput.y += 1.0f; player[3].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_NUMPAD5)) { moveInput.y -= 1.0f; player[3].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_NUMPAD4)) { moveInput.x -= 1.0f; player[3].isMoving = true; }
						if (Keyboard_IsKeyDown(KK_NUMPAD6)) { moveInput.x += 1.0f; player[3].isMoving = true; }
						if (moveInput.x == 0.0f && moveInput.y == 0.0f)	player[3].isMoving = false;
					}
					player[p].moveInput2D = moveInput;

					// 遘ｻ蜍輔・繧ｫ繝｡繝ｩ蝓ｺ貅悶ｒ繝ｯ繝ｼ繝ｫ繝峨↓縺吶ｋ
					player[p].moveDir = ToWorldMoveDirByCamera(moveInput);
				}
			}

			// 迴ｾ蝨ｨ縺ｮ繝励Ξ繧､繝､繝ｼ p 縺縺代ｒ蜍輔°縺・
			Move(player[p], player[p].moveDir);

			// 遘ｻ蜍穂ｸｭ縺ｪ繧・lastDir 繧呈峩譁ｰ
			if (player[p].isMoving)
			{
				float dx = player[p].moveInput2D.x;
				float dz = player[p].moveInput2D.y;

					 if (dx < 0.0f && dz < 0.0f) player[p].lastDir = PlayerDir::Down_Left;
				else if (dx < 0.0f && dz > 0.0f) player[p].lastDir = PlayerDir::Up_Left;
				else if (dx > 0.0f && dz > 0.0f) player[p].lastDir = PlayerDir::Up_Right;
				else if (dx > 0.0f && dz < 0.0f) player[p].lastDir = PlayerDir::Down_Right;
				else if (dz < 0.0f)              player[p].lastDir = PlayerDir::Down;
				else if (dx < 0.0f)              player[p].lastDir = PlayerDir::Left;
				else if (dz > 0.0f)              player[p].lastDir = PlayerDir::Up;
				else if (dx > 0.0f)              player[p].lastDir = PlayerDir::Right;
			}
		}

		// 繝励Ξ繧､繝､繝ｼ縺斐→縺ｮ繧ｹ繧ｭ繝ｫ繧ｯ繝ｼ繝ｫ繧ｿ繧､繝繧呈ｯ弱ヵ繝ｬ繝ｼ繝貂帷ｮ・
		if (player[p].skillCoolTimer > 0.0f)
		{
			player[p].skillCoolTimer -= DELTA_TIME;
			if (player[p].skillCoolTimer < 0.0f) player[p].skillCoolTimer = 0.0f;
		}

		// HP縺・莉･荳九・蜃ｦ逅・
		if (player[p].hp <= 0.0f && player[p].active && !player[p].isDown)
		{
			// 繝繧ｦ繝ｳ迥ｶ諷九↓遘ｻ陦後＠縺ｦ繧ｿ繧､繝槭・繧偵Μ繧ｻ繝・ヨ
			player[p].isDown = true;
			player[p].downTimer = 0.0f;
			Effect_ClearUI(p);
		}

		// 繝繧ｦ繝ｳ迥ｶ諷九・繧ｿ繧､繝槭・譖ｴ譁ｰ縺ｨ繝ｪ繧ｹ繝昴・繝ｳ蛻､螳・
		if (player[p].isDown)
		{
			// 陦悟虚蛛懈ｭ｢
			player[p].moveDir = { 0.0f, 0.0f, 0.0f };
			player[p].isAttacking = false;
			player[p].useSkill = false;
			player[p].useSpecial = false;

			// 繝繧ｦ繝ｳ繧ｿ繧､繝槭・譖ｴ譁ｰ
			player[p].downTimer += DELTA_TIME;

			// 繝励Ξ繧､繝､繝ｼ豈弱・繝繧ｦ繝ｳ譎る俣縺檎ｵ碁℃縺励◆繧峨Μ繧ｹ繝昴・繝ｳ蜃ｦ逅・
			if (player[p].downTimer >= DOWN_TIME)
			{
				// 谿区ｩ溘ｒ1縺､貂帙ｉ縺・
				player[p].stock -= 1;

				if (player[p].stock > 0)	Player_Respawn(p);
				else
				{
					// 谿区ｩ溽┌縺励〒蠕ｩ豢ｻ縺ｪ縺・
					player[p].active = false;
					player[p].isDown = false;
					player[p].downTimer = 0.0f;

					// 鬆・ｽ咲匳骭ｲ・亥・驛ｨ縺ｧ驥崎､・匳骭ｲ繧帝亟豁｢・・
					Ranking(p);
				}
			}
		}

		// 關ｽ荳句・逅・蠖ｱ繧ｨ繝輔ぉ繧ｯ繝磯撼陦ｨ遉ｺ
		if (player[p].position.y < -1.0f)
		{
			player[p].isShadowEnabled = false;
		}

		if (player[p].active && player[p].position.y <= -10.0f)
		{
			Effect_ClearUI(p);
			// 谿区ｩ溘ｒ荳縺､貂帙ｉ縺・
			player[p].stock -= 1;

			// 繝ｪ繧ｹ繝昴・繝ｳ・井ｽ咲ｽｮ繝ｻ繧ｹ繝・・繝医Μ繧ｻ繝・ヨ・・
			if (player[p].stock > 0)	Player_Respawn(p);
			else
			{
				// 谿区ｩ溽┌縺励〒螳悟・縺ｫ髱槭い繧ｯ繝・ぅ繝門喧
				player[p].active = false;

				// 鬆・ｽ咲匳骭ｲ
				Ranking(p);
				player[p].position.y = 0.0f;
			}
		}

		// 繝繝｡繝ｼ繧ｸ繧貞女縺代◆譎ゅ・蜃ｦ逅・
		if (player[p].isAttacked)
		{
			// 繝繝｡繝ｼ繧ｸ繧ｿ繧､繝槭・譖ｴ譁ｰ
			player[p].attackedTimer += DELTA_TIME;

			// 繝励Ξ繧､繝､繝ｼ豈弱・繝繝｡繝ｼ繧ｸ譎る俣縺檎ｵ碁℃縺励◆繧峨ム繝｡繝ｼ繧ｸ邨ゆｺ・
			if (player[p].attackedTimer >= ATTACKED_TIME)
			{
				player[p].isAttacked = false;
				player[p].attackedTimer = 0.0f;
			}
		}
		// 繝繝｡繝ｼ繧ｸ濶ｲ縺縺代・蜃ｦ逅・
		if (player[p].isDamageColor)
		{
			player[p].damageColorTimer += DELTA_TIME;

			if (player[p].damageColorTimer >= ATTACKED_TIME)
			{
				player[p].isDamageColor = false;
				player[p].damageColorTimer = 0.0f;
			}
		}

		// 繝繝｡繝ｼ繧ｸ濶ｲ縺縺代・蜃ｦ逅・
		if (player[p].isDamageColor)
		{
			player[p].damageColorTimer += DELTA_TIME;

			if (player[p].damageColorTimer >= ATTACKED_TIME)
			{
				player[p].isDamageColor = false;
				player[p].damageColorTimer = 0.0f;
			}
		}

		// 騾ｲ蛹匁凾縺ｮ辟｡謨ｵ蜃ｦ逅・
		if (player[p].isInvincible)
		{
			// 辟｡謨ｵ繧ｿ繧､繝槭・譖ｴ譁ｰ
			player[p].invincibleTimer += DELTA_TIME;

			// 繝励Ξ繧､繝､繝ｼ豈弱・辟｡謨ｵ譎る俣縺檎ｵ碁℃縺励◆繧臥┌謨ｵ邨ゆｺ・
			if (player[p].invincibleTimer >= EVOLVING_TIME)
			{
				player[p].isInvincible = false;
				player[p].invincibleTimer = 0.0f;

				// 騾ｲ蛹匁凾縺ｮ蜥・動SE蜀咲函
					 if (player[p].form == Form::Second)PlayAudio(g_SE_ID[0], false);	// 蜥・動 隨ｬ2蠖｢諷・
				else if (player[p].form == Form::Third)	PlayAudio(g_SE_ID[1], false);	// 蜥・動 隨ｬ3蠖｢諷・
			}
		}

		// ==========================================================
		// 繝励Ξ繧､繝､繝ｼ 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ譖ｴ譁ｰ
		// ==========================================================
		
		// 繧ｹ繧ｭ繝ｫ髢句ｧ区凾縺ｮ繝輔Ξ繝ｼ繝蛻晄悄蛹厄ｼ医い繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ譖ｴ譁ｰ繧ｿ繧､繝溘Φ繧ｰ縺ｫ萓晏ｭ倥＠縺ｪ縺・ｼ・
		if (player[p].skillAnimation && !g_skillAnimStarted[p])
		{
			// 螻樊ｧ縺斐→縺ｮ蝓ｺ貅悶が繝輔そ繝・ヨ・亥ｱ樊ｧ1縺､縺ゅ◆繧・2繧ｳ繝橸ｼ・
			int typeBase = 0;
				 if (player[p].type == PlayerType::Concrete)	typeBase = 0;
			else if (player[p].type == PlayerType::Electricity)	typeBase = 32;
			else if (player[p].type == PlayerType::Glass)		typeBase = 64;
			else if (player[p].type == PlayerType::Plant)		typeBase = 96;

			// 蠖｢諷九が繝輔そ繝・ヨ・育ｬｬ2蠖｢諷・ 0縲∫ｬｬ3蠖｢諷・ 128・・
			int formBase = 0;
			if (player[p].form == Form::Third) formBase = 128;

			// 譁ｹ蜷代が繝輔そ繝・ヨ・・譁ｹ蜷代≠縺溘ｊ4繧ｳ繝橸ｼ・
			int dirOffset = 0;
				 if (player[p].lastDir == PlayerDir::Down)		dirOffset = 0;
			else if (player[p].lastDir == PlayerDir::Down_Left)	dirOffset = 4;
			else if (player[p].lastDir == PlayerDir::Left)		dirOffset = 8;
			else if (player[p].lastDir == PlayerDir::Up_Left)	dirOffset = 12;
			else if (player[p].lastDir == PlayerDir::Up)		dirOffset = 16;
			else if (player[p].lastDir == PlayerDir::Up_Right)	dirOffset = 20;
			else if (player[p].lastDir == PlayerDir::Right)		dirOffset = 24;
			else if (player[p].lastDir == PlayerDir::Down_Right)dirOffset = 28;

			int start = formBase + typeBase + dirOffset;
			g_skillAnimStart[p] = start;
			player[p].animFrame = start;
			g_skillAnimStarted[p] = true;
		}
		// 繧ｹ繧ｭ繝ｫ邨ゆｺ・凾縺ｮ繝輔Λ繧ｰ繝ｪ繧ｻ繝・ヨ
		if (!player[p].skillAnimation && g_skillAnimStarted[p])	g_skillAnimStarted[p] = false;

		// 繧ｹ繝壹す繝｣繝ｫ髢句ｧ区凾縺ｮ繝輔Ξ繝ｼ繝蛻晄悄蛹厄ｼ医い繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ譖ｴ譁ｰ繧ｿ繧､繝溘Φ繧ｰ縺ｫ萓晏ｭ倥＠縺ｪ縺・ｼ・
		if (player[p].specialAnimation && !g_specialInitialize[p])
		{
			int type = -1;
			if (player[p].type == PlayerType::Concrete)	type = 0;
			else if (player[p].type == PlayerType::Electricity)	type = 1;
			else if (player[p].type == PlayerType::Glass)		type = 2;
			else if (player[p].type == PlayerType::Plant)		type = 3;

			int start = type * 64;
			if (player[p].lastDir == PlayerDir::Down)		start += 0;
			else if (player[p].lastDir == PlayerDir::Down_Left)	start += 8;
			else if (player[p].lastDir == PlayerDir::Left)		start += 16;
			else if (player[p].lastDir == PlayerDir::Up_Left)	start += 24;
			else if (player[p].lastDir == PlayerDir::Up)		start += 32;
			else if (player[p].lastDir == PlayerDir::Up_Right)	start += 40;
			else if (player[p].lastDir == PlayerDir::Right)		start += 48;
			else if (player[p].lastDir == PlayerDir::Down_Right)start += 56;

			player[p].animFrame = start;
			g_specialAnimPhase[p] = 0;			// 繝輔ぉ繝ｼ繧ｺ繝ｪ繧ｻ繝・ヨ
			g_specialEndAnimTimer[p] = 0.0f;	// 邨ゆｺ・ｼ泌・繧ｿ繧､繝槭・繝ｪ繧ｻ繝・ヨ
			g_specialInitialize[p] = true;
		}
		// 繧ｹ繝壹す繝｣繝ｫ邨ゆｺ・凾縺ｮ繝輔Ξ繝ｼ繝繝ｪ繧ｻ繝・ヨ
		else if (!player[p].specialAnimation && g_specialInitialize[p])
		{
			g_specialInitialize[p] = false;
			int idleStart = 0;
			if (player[p].lastDir == PlayerDir::Down)		idleStart = 0;
			else if (player[p].lastDir == PlayerDir::Down_Left)	idleStart = 26;
			else if (player[p].lastDir == PlayerDir::Left)		idleStart = 52;
			else if (player[p].lastDir == PlayerDir::Up_Left)	idleStart = 78;
			else if (player[p].lastDir == PlayerDir::Up)		idleStart = 104;
			else if (player[p].lastDir == PlayerDir::Up_Right)	idleStart = 130;
			else if (player[p].lastDir == PlayerDir::Right)		idleStart = 156;
			else if (player[p].lastDir == PlayerDir::Down_Right)idleStart = 182;
			player[p].animFrame = idleStart; // 蠕・ｩ溘ヵ繝ｬ繝ｼ繝縺ｫ繝ｪ繧ｻ繝・ヨ
		}
		// 繧ｬ繝ｩ繧ｹ繝ｻ髮ｻ豌励・讀咲黄: specialTimer 縺ｫ蝓ｺ縺･縺上い繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ邨ゆｺ・宛蠕｡
		// 窶ｻ useSpecial 縺ｯ true 縺ｮ縺ｾ縺ｾ・・pecial.cpp 縺ｮ繝繝｡繝ｼ繧ｸ蜃ｦ逅・ｭ峨・邯咏ｶ夲ｼ・
		if (player[p].specialAnimation)
		{
			// 繧ｬ繝ｩ繧ｹ繝ｻ髮ｻ豌・ 0.9遘偵〒繝輔Ξ繝ｼ繝7縲・.0遘偵〒蠕・ｩ・
			if (player[p].type == PlayerType::Glass || player[p].type == PlayerType::Electricity)
			{
				if (player[p].specialTimer >= 1.0f)
				{
					// 邨ゆｺ・ｼ泌・繝輔ぉ繝ｼ繧ｺ縺ｸ・育ｵゆｺ・ヵ繝ｬ繝ｼ繝繧定｡ｨ遉ｺ縺輔○繧具ｼ・
					g_specialAnimPhase[p] = 2;
					g_specialEndAnimTimer[p] = 0.0f;

					// 螻樊ｧ繝ｻ蜷代″縺九ｉ邨ゆｺ・ｼ泌・繝輔Ξ繝ｼ繝(start + 7) 繧呈ｱｺ螳壹＠縺ｦ險ｭ螳・
					int type = -1;
					if (player[p].type == PlayerType::Concrete)		type = 0;
					else if (player[p].type == PlayerType::Electricity)	type = 1;
					else if (player[p].type == PlayerType::Glass)		type = 2;
					else if (player[p].type == PlayerType::Plant)		type = 3;

					int start = type * 64;
					if (player[p].lastDir == PlayerDir::Down)		start += 0;
					else if (player[p].lastDir == PlayerDir::Down_Left)	start += 8;
					else if (player[p].lastDir == PlayerDir::Left)		start += 16;
					else if (player[p].lastDir == PlayerDir::Up_Left)	start += 24;
					else if (player[p].lastDir == PlayerDir::Up)		start += 32;
					else if (player[p].lastDir == PlayerDir::Up_Right)	start += 40;
					else if (player[p].lastDir == PlayerDir::Right)		start += 48;
					else if (player[p].lastDir == PlayerDir::Down_Right)start += 56;

					player[p].animFrame = start + 7;	// 邨ゆｺ・ｼ泌・繝輔Ξ繝ｼ繝繧定｡ｨ遉ｺ
					player[p].animTimer = 0.0f;		// 蜷後ヵ繝ｬ繝ｼ繝縺ｧ騾ｲ陦後＠縺ｪ縺・ｈ縺・Μ繧ｻ繝・ヨ
				}
				else if (player[p].specialTimer >= 0.9f && g_specialAnimPhase[p] != 2)
				{
					g_specialAnimPhase[p] = 2;

					int type = -1;
					if (player[p].type == PlayerType::Concrete)	type = 0;
					else if (player[p].type == PlayerType::Electricity)	type = 1;
					else if (player[p].type == PlayerType::Glass)		type = 2;
					else if (player[p].type == PlayerType::Plant)		type = 3;

					int start = type * 64;
					if (player[p].lastDir == PlayerDir::Down)		start += 0;
					else if (player[p].lastDir == PlayerDir::Down_Left)	start += 8;
					else if (player[p].lastDir == PlayerDir::Left)		start += 16;
					else if (player[p].lastDir == PlayerDir::Up_Left)	start += 24;
					else if (player[p].lastDir == PlayerDir::Up)		start += 32;
					else if (player[p].lastDir == PlayerDir::Up_Right)	start += 40;
					else if (player[p].lastDir == PlayerDir::Right)		start += 48;
					else if (player[p].lastDir == PlayerDir::Down_Right)start += 56;

					player[p].animFrame = start + 7;
				}
			}
			// 讀咲黄: 1.0遘偵〒蠕・ｩ溘↓謌ｻ縺呻ｼ医◎繧後∪縺ｧ縺ｯ8繧ｳ繝槭Ν繝ｼ繝礼ｶ咏ｶ夲ｼ・
			else if (player[p].type == PlayerType::Plant)
			{
				// 1.0遘堤ｵ碁℃縺ｧ繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ邨ゆｺ・
				if (player[p].specialTimer >= 1.0f && player[p].specialAnimation)
				{
					g_specialAnimPhase[p] = 0;
					g_specialEndAnimTimer[p] = 0.0f;

					int idleStart = 0;
						 if (player[p].lastDir == PlayerDir::Down)		idleStart = 0;
					else if (player[p].lastDir == PlayerDir::Down_Left)	idleStart = 26;
					else if (player[p].lastDir == PlayerDir::Left)		idleStart = 52;
					else if (player[p].lastDir == PlayerDir::Up_Left)	idleStart = 78;
					else if (player[p].lastDir == PlayerDir::Up)		idleStart = 104;
					else if (player[p].lastDir == PlayerDir::Up_Right)	idleStart = 130;
					else if (player[p].lastDir == PlayerDir::Right)		idleStart = 156;
					else if (player[p].lastDir == PlayerDir::Down_Right)idleStart = 182;

					player[p].animFrame = idleStart;
				}
			}
		}

		// 繝励Ξ繧､繝､繝ｼ 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ譖ｴ譁ｰ
		player[p].animTimer += DELTA_TIME;

		// 繧ｨ繝輔ぉ繧ｯ繝・繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ
		Effect_UpdateForPlayer(p);

		if (player[p].animTimer >= ANIM_FRAME_TIME)
		{
			int advance = (int)(player[p].animTimer / ANIM_FRAME_TIME);
			player[p].animTimer -= advance * ANIM_FRAME_TIME;

			// 蜍晏茜 隨ｬ1蠖｢諷・13繧ｳ繝・繝ｩ繧ｹ繝・繧ｳ繝・繝ｫ繝ｼ繝・ 隨ｬ2蠖｢諷・20繧ｳ繝・繝ｩ繧ｹ繝・繧ｳ繝・繝ｫ繝ｼ繝・ 隨ｬ3蠖｢諷・21繧ｳ繝・繝ｩ繧ｹ繝医さ繝・繝ｫ繝ｼ繝・
			//if (Keyboard_IsKeyDown(KK_TAB) || g_victoryState[p] != 0)
			if (player[p].rank == 1 || g_victoryState[p] != 0)
			{
				//if (Keyboard_IsKeyDown(KK_TAB) && g_victoryState[p] == 0)
				if (player[p].rank == 1 && g_victoryState[p] == 0)
				{
					g_victoryState[p] = 1;
					player[p].animFrame = 208;	// 蛻晏屓蜀咲函髢句ｧ九ヵ繝ｬ繝ｼ繝
				}

				if (g_victoryState[p] == 1)
				{
					// 蛻晏屓蜀咲函 繝輔Ξ繝ｼ繝繧貞腰邏泌｢怜刈
					player[p].animFrame += advance;

					// 隨ｬ1蠖｢諷・220 繧定｡ｨ遉ｺ縺励◆蠕後↓繝ｫ繝ｼ繝鈴伜沺縺ｸ遘ｻ陦後☆繧・
					if (player[p].animFrame > 220 && player[p].form == Form::First)
					{
						g_victoryState[p] = 2;
						player[p].animFrame = 216;	// 繝ｫ繝ｼ繝鈴幕蟋九ヵ繝ｬ繝ｼ繝
					}
					// 隨ｬ2蠖｢諷・227 繧定｡ｨ遉ｺ縺励◆蠕後↓繝ｫ繝ｼ繝鈴伜沺縺ｸ遘ｻ陦後☆繧・
					if (player[p].animFrame > 227 && player[p].form == Form::Second)
					{
						g_victoryState[p] = 2;
						player[p].animFrame = 219;	// 繝ｫ繝ｼ繝鈴幕蟋九ヵ繝ｬ繝ｼ繝
					}
					// 隨ｬ3蠖｢諷・228 繧定｡ｨ遉ｺ縺励◆蠕後↓繝ｫ繝ｼ繝鈴伜沺縺ｸ遘ｻ陦後☆繧・229繧ｳ繝樒岼縺ｯ菴ｿ逕ｨ縺励↑縺・
					if (player[p].animFrame > 228 && player[p].form == Form::Third)
					{
						g_victoryState[p] = 2;
						player[p].animFrame = 221;	// 繝ｫ繝ｼ繝鈴幕蟋九ヵ繝ｬ繝ｼ繝
					}
				}
				else if (g_victoryState[p] == 2)
				{
					switch (player[p].form)
					{
					case Form::First:	LoopRange(player[p].animFrame, 216, 5, advance);	// 隨ｬ1蠖｢諷・216・・20繧偵Ν繝ｼ繝・
						break;
					case Form::Second:	LoopRange(player[p].animFrame, 219, 9, advance);	// 隨ｬ2蠖｢諷・219・・27繧偵Ν繝ｼ繝・
						break;
					case Form::Third:	LoopRange(player[p].animFrame, 221, 8, advance);	// 隨ｬ3蠖｢諷・221・・28繧偵Ν繝ｼ繝・229繧ｳ繝樒岼縺ｯ菴ｿ逕ｨ縺励↑縺・
						break;
					}
				}
			}
			// 繝繧ｦ繝ｳ 5繧ｳ繝・(繝繝｡繝ｼ繧ｸ 2繧ｳ繝・+ 繝繧ｦ繝ｳ 3繧ｳ繝・ 譛邨ゅさ繝槭〒蛛懈ｭ｢
			else if (player[p].isDown)
			{
				// 蜷代″縺ｫ蠢懊§縺滄幕蟋九ヵ繝ｬ繝ｼ繝繧呈ｱｺ螳・
				int start = 15; // 繝・ヵ繧ｩ繝ｫ繝茨ｼ・own・・
					 if (player[p].lastDir == PlayerDir::Down)		 start = 15;
				else if (player[p].lastDir == PlayerDir::Down_Left)	 start = 41;
				else if (player[p].lastDir == PlayerDir::Left)		 start = 67;
				else if (player[p].lastDir == PlayerDir::Up_Left)	 start = 93;
				else if (player[p].lastDir == PlayerDir::Up)		 start = 119;
				else if (player[p].lastDir == PlayerDir::Up_Right)	 start = 145;
				else if (player[p].lastDir == PlayerDir::Right)		 start = 171;
				else if (player[p].lastDir == PlayerDir::Down_Right) start = 197;

				const int count = 5;
				const int lastFrame = start + count - 1;

				// advance 縺ｫ蟇ｾ蠢懊☆繧狗ｵ碁℃遘抵ｼ・_animTimer縺ｧ縺ｾ縺ｨ繧√※騾ｲ繧√◆蛻・ｼ・
				float elapsedSec = (float)advance * ANIM_FRAME_TIME;

				// 繝輔Ξ繝ｼ繝縺檎ｯ・峇螟悶↑繧蛾幕蟋九ヵ繝ｬ繝ｼ繝縺ｫ陬懈ｭ｣縺励ち繧､繝槭・繝ｪ繧ｻ繝・ヨ
				if (player[p].animFrame < start || player[p].animFrame > lastFrame)
				{
					player[p].animFrame = start;
					g_downHoldTimer[p] = 0.0f;
				}

				// 譛邨ゅヵ繝ｬ繝ｼ繝莉･螟悶↑繧臥ｬｬ1蠖｢諷矩ｲ陦鯉ｼ医Ν繝ｼ繝暦ｼ・
				if (player[p].animFrame != lastFrame)
				{
					LoopRange(player[p].animFrame, start, count, advance);
					g_downHoldTimer[p] = 0.0f; // 蛻ｰ驕泌燕縺ｯ繝帙・繝ｫ繝峨ち繧､繝槭・繧偵Μ繧ｻ繝・ヨ
				}
				else
				{
					// 譛邨ゅヵ繝ｬ繝ｼ繝縺ｫ蛻ｰ驕・繝帙・繝ｫ繝峨ｒ騾ｲ繧√ｋ
					g_downHoldTimer[p] += elapsedSec;

					// 繝帙・繝ｫ繝峨′貅莠・＠縺溘ｉ谺｡縺ｫ騾ｲ繧√ｋ・医％縺薙〒縺ｯ1繝輔Ξ繝ｼ繝蛻・□縺鷹ｲ繧√ｋ・・
					if (g_downHoldTimer[p] >= DOWN_TIME)
					{
						g_downHoldTimer[p] = 0.0f;
						// 1繝輔Ξ繝ｼ繝蛻・ｲ繧√ｋ・医Ν繝ｼ繝励↓繧医ｊ start 縺ｫ謌ｻ繧具ｼ・
						LoopRange(player[p].animFrame, start, count, 1);
					}
				}
			}
			// 繧ｹ繝壹す繝｣繝ｫ 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ
			else if (player[p].specialAnimation)
			{
				int type = -1;
					 if (player[p].type == PlayerType::Concrete)	type = 0;
				else if (player[p].type == PlayerType::Electricity)	type = 1;
				else if (player[p].type == PlayerType::Glass)		type = 2;
				else if (player[p].type == PlayerType::Plant)		type = 3;

				int start = type * 64;
					 if (player[p].lastDir == PlayerDir::Down)		start += 0;
				else if (player[p].lastDir == PlayerDir::Down_Left)	start += 8;
				else if (player[p].lastDir == PlayerDir::Left)		start += 16;
				else if (player[p].lastDir == PlayerDir::Up_Left)	start += 24;
				else if (player[p].lastDir == PlayerDir::Up)		start += 32;
				else if (player[p].lastDir == PlayerDir::Up_Right)	start += 40;
				else if (player[p].lastDir == PlayerDir::Right)		start += 48;
				else if (player[p].lastDir == PlayerDir::Down_Right)start += 56;

				// 繧ｬ繝ｩ繧ｹ繝ｻ髮ｻ豌・ 0・・繧・蝗槫・逕・竊・4・・繧偵Ν繝ｼ繝・
				if (player[p].type == PlayerType::Glass || player[p].type == PlayerType::Electricity)
				{
					if (g_specialAnimPhase[p] == 0)
					{
						player[p].animFrame += advance;
						if (player[p].animFrame > start + 6)
						{
							g_specialAnimPhase[p] = 1;
							player[p].animFrame = start + 4;
						}
					}
					else if (g_specialAnimPhase[p] == 1)	LoopRange(player[p].animFrame, start + 4, 3, advance);
					// phase == 2 : 繝輔Ξ繝ｼ繝7陦ｨ遉ｺ荳ｭ -> 菴輔ｂ縺励↑縺・ｼ・pecialTimer繝吶・繧ｹ縺ｧ蛻ｶ蠕｡・・
				}
				// 讀咲黄: 蠕捺擂騾壹ｊ8繧ｳ繝槭Ν繝ｼ繝・
				if (player[p].type == PlayerType::Plant)	LoopRange(player[p].animFrame, start, 8, advance);
			}
			// 繝繝｡繝ｼ繧ｸ 3繧ｳ繝・
			else if (player[p].isAttacked || player[p].isStunning)
			{
					 if (player[p].lastDir == PlayerDir::Down)		LoopRange(player[p].animFrame,  14, 3, advance);	//  荳・  14・・6 
				else if (player[p].lastDir == PlayerDir::Down_Left)	LoopRange(player[p].animFrame,  40, 3, advance);	// 蟾ｦ荳・ 40・・2
				else if (player[p].lastDir == PlayerDir::Left)		LoopRange(player[p].animFrame,  66, 3, advance);	//  蟾ｦ   66・・8
				else if (player[p].lastDir == PlayerDir::Up_Left)	LoopRange(player[p].animFrame,  92, 3, advance);	// 蟾ｦ荳・ 92・・4
				else if (player[p].lastDir == PlayerDir::Up)		LoopRange(player[p].animFrame, 118, 3, advance);	//  荳・ 118・・20
				else if (player[p].lastDir == PlayerDir::Up_Right)	LoopRange(player[p].animFrame, 144, 3, advance);	// 蜿ｳ荳・144・・46
				else if (player[p].lastDir == PlayerDir::Right)		LoopRange(player[p].animFrame, 170, 3, advance);	//  蜿ｳ  170・・72
				else if (player[p].lastDir == PlayerDir::Down_Right)LoopRange(player[p].animFrame, 196, 3, advance);	// 蜿ｳ荳・196・・98
			}
			// 繧ｹ繧ｭ繝ｫ 4繧ｳ繝橸ｼ・蝗槫・逕溘・譛邨ゅヵ繝ｬ繝ｼ繝縺ｧ蛛懈ｭ｢蠕後↓邨ゆｺ・ｼ・
			else if (player[p].skillAnimation)
			{
				int start = g_skillAnimStart[p];
				const int count = 4;
				const int lastFrame = start + count - 1;

				// 遽・峇螟悶↑繧蛾幕蟋九ヵ繝ｬ繝ｼ繝繧定ｨ育ｮ励・菫晏ｭ倥＠縺ｦ繝ｪ繧ｻ繝・ヨ
				if (player[p].animFrame < start || player[p].animFrame > lastFrame)
				{
					// 螻樊ｧ縺斐→縺ｮ蝓ｺ貅悶が繝輔そ繝・ヨ・亥ｱ樊ｧ1縺､縺ゅ◆繧・2繧ｳ繝橸ｼ・
					int typeBase = 0;
						 if (player[p].type == PlayerType::Concrete)	typeBase = 0;
					else if (player[p].type == PlayerType::Electricity)	typeBase = 32;
					else if (player[p].type == PlayerType::Glass)		typeBase = 64;
					else if (player[p].type == PlayerType::Plant)		typeBase = 96;

					// 蠖｢諷九が繝輔そ繝・ヨ・育ｬｬ2蠖｢諷・ 0縲∫ｬｬ3蠖｢諷・ 128・・
					int formBase = 0;
					if (player[p].form == Form::Third) formBase = 128;

					// 譁ｹ蜷代が繝輔そ繝・ヨ・・譁ｹ蜷代≠縺溘ｊ4繧ｳ繝橸ｼ・
					int dirOffset = 0;
						 if (player[p].lastDir == PlayerDir::Down)		dirOffset = 0;
					else if (player[p].lastDir == PlayerDir::Down_Left)	dirOffset = 4;
					else if (player[p].lastDir == PlayerDir::Left)		dirOffset = 8;
					else if (player[p].lastDir == PlayerDir::Up_Left)	dirOffset = 12;
					else if (player[p].lastDir == PlayerDir::Up)		dirOffset = 16;
					else if (player[p].lastDir == PlayerDir::Up_Right)	dirOffset = 20;
					else if (player[p].lastDir == PlayerDir::Right)		dirOffset = 24;
					else if (player[p].lastDir == PlayerDir::Down_Right)dirOffset = 28;

					start = formBase + typeBase + dirOffset;
					g_skillAnimStart[p] = start;
					player[p].animFrame = start;
				}

				// lastFrame 繧貞・險育ｮ暦ｼ・tart 縺梧峩譁ｰ縺輔ｌ縺溷庄閭ｽ諤ｧ縺後≠繧九◆繧・ｼ・
				const int finalFrame = g_skillAnimStart[p] + count - 1;

				// 譛邨ゅヵ繝ｬ繝ｼ繝縺ｫ驕斐＠縺ｦ縺・↑縺代ｌ縺ｰ騾ｲ繧√ｋ
				if (player[p].animFrame < finalFrame)
				{
					player[p].animFrame += advance;
					// 繧ｪ繝ｼ繝舌・繧ｷ繝･繝ｼ繝磯亟豁｢・域怙邨ゅヵ繝ｬ繝ｼ繝縺ｧ繧ｯ繝ｩ繝ｳ繝暦ｼ・
					if (player[p].animFrame > finalFrame) player[p].animFrame = finalFrame;
				}
				else
				{
					// 譛邨ゅヵ繝ｬ繝ｼ繝縺ｫ驕斐＠縺溘ｉ繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ邨ゆｺ・
					player[p].skillAnimation = false;

					// 騾壼ｸｸ繝・け繧ｹ繝√Ε縺ｮ蠕・ｩ溘い繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ髢句ｧ九ヵ繝ｬ繝ｼ繝縺ｫ繝ｪ繧ｻ繝・ヨ
					int idleStart = 0;
						 if (player[p].lastDir == PlayerDir::Down)		idleStart = 0;
					else if (player[p].lastDir == PlayerDir::Down_Left)	idleStart = 26;
					else if (player[p].lastDir == PlayerDir::Left)		idleStart = 52;
					else if (player[p].lastDir == PlayerDir::Up_Left)	idleStart = 78;
					else if (player[p].lastDir == PlayerDir::Up)		idleStart = 104;
					else if (player[p].lastDir == PlayerDir::Up_Right)	idleStart = 130;
					else if (player[p].lastDir == PlayerDir::Right)		idleStart = 156;
					else if (player[p].lastDir == PlayerDir::Down_Right)idleStart = 182;

						 player[p].animFrame = idleStart;
				}
			}
			// 謾ｻ謦・6繧ｳ繝・
			else if (player[p].isAttacking)
			{
					 if (player[p].lastDir == PlayerDir::Down)		LoopRange(player[p].animFrame,  20, 6, advance);	//  荳・  20・・5
				else if (player[p].lastDir == PlayerDir::Down_Left)	LoopRange(player[p].animFrame,  46, 6, advance);	// 蟾ｦ荳・ 46・・1
				else if (player[p].lastDir == PlayerDir::Left)		LoopRange(player[p].animFrame,  72, 6, advance);	//  蟾ｦ   72・・7
				else if (player[p].lastDir == PlayerDir::Up_Left)	LoopRange(player[p].animFrame,  98, 6, advance);	// 蟾ｦ荳・ 98・・03
				else if (player[p].lastDir == PlayerDir::Up)		LoopRange(player[p].animFrame, 124, 6, advance);	//  荳・ 124・・29
				else if (player[p].lastDir == PlayerDir::Up_Right)	LoopRange(player[p].animFrame, 150, 6, advance);	// 蜿ｳ荳・150・・55
				else if (player[p].lastDir == PlayerDir::Right)		LoopRange(player[p].animFrame, 176, 6, advance);	//  蜿ｳ  176・・81
				else if (player[p].lastDir == PlayerDir::Down_Right)LoopRange(player[p].animFrame, 202, 6, advance);	// 蜿ｳ荳・202・・07
			}
			// 遘ｻ蜍・8繧ｳ繝・・医Μ繧ｹ繝昴・繝ｳ荳ｭ繧帝勁縺擾ｼ・
			else if (!player[p].duringRespawn && player[p].isMoving)
			{
				float dx = player[p].moveInput2D.x;
				float dz = player[p].moveInput2D.y;

					 if (dx < 0.0f && dz < 0.0f)LoopRange(player[p].animFrame,  32, 8, advance);
				else if (dx < 0.0f && dz > 0.0f)LoopRange(player[p].animFrame,  84, 8, advance);
				else if (dx > 0.0f && dz > 0.0f)LoopRange(player[p].animFrame, 136, 8, advance);
				else if (dx > 0.0f && dz < 0.0f)LoopRange(player[p].animFrame, 188, 8, advance);
				else if (dz < 0.0f)				LoopRange(player[p].animFrame,   6, 8, advance);
				else if (dx < 0.0f)				LoopRange(player[p].animFrame,  58, 8, advance);
				else if (dz > 0.0f)				LoopRange(player[p].animFrame, 110, 8, advance);
				else if (dx > 0.0f)				LoopRange(player[p].animFrame, 162, 8, advance);
			}
			// 蠕・ｩ・6繧ｳ繝・
			else if (player[p].isMoving == false)
			{
					 if (player[p].lastDir == PlayerDir::Down)		LoopRange(player[p].animFrame,   0, 6, advance);	//  荳・   0・・
				else if (player[p].lastDir == PlayerDir::Down_Left)	LoopRange(player[p].animFrame,  26, 6, advance);	// 蟾ｦ荳・ 26・・1
				else if (player[p].lastDir == PlayerDir::Left)		LoopRange(player[p].animFrame,  52, 6, advance);	//  蟾ｦ   52・・7
				else if (player[p].lastDir == PlayerDir::Up_Left)	LoopRange(player[p].animFrame,  78, 6, advance);	// 蟾ｦ荳・ 78・・3 
				else if (player[p].lastDir == PlayerDir::Up)		LoopRange(player[p].animFrame, 104, 6, advance);	//  荳・ 104・・09
				else if (player[p].lastDir == PlayerDir::Up_Right)	LoopRange(player[p].animFrame, 130, 6, advance);	// 蜿ｳ荳・130・・35
				else if (player[p].lastDir == PlayerDir::Right)		LoopRange(player[p].animFrame, 156, 6, advance);	//  蜿ｳ  156・・61
				else if (player[p].lastDir == PlayerDir::Down_Right)LoopRange(player[p].animFrame, 182, 6, advance);	// 蜿ｳ荳・182・・87		
			}
		}

		static XMFLOAT3 posBuff = player[p].position;	// 繝・ヰ繝・げ陦ｨ遉ｺ蠎ｧ讓・

		// 謠冗判縺ｧ菴ｿ縺｣縺ｦ縺・ｋ繧ｹ繝励Λ繧､繝亥咲紫縺ｨ蜷後§蛟､繧堤黄逅・↓繧ゆｽｿ縺・
		const float renderScale = 2.0f;	// Draw 蛛ｴ縺ｮ spriteScale 縺ｫ蜷医ｏ縺帙ｋ
		// 謠冗判繧ｹ繧ｱ繝ｼ繝ｫ繧貞渚譏縺励◆繧ｹ繧ｱ繝ｼ繝ｫ・郁｡ｨ遉ｺ逕ｨ・・
		XMFLOAT3 physicsScaling = XMFLOAT3(player[p].scaling.x * renderScale, player[p].scaling.y * renderScale, player[p].scaling.z * renderScale);


		////////////////////////////////////////////////////////////////////////////////////////////
		// TODO:

		// --- 繝励Ξ繧､繝､繝ｼ逕ｨ繝偵ャ繝医・繝・け繧ｹ豈皮紫・亥髄縺阪〒髟ｷ遏ｭ繧貞・繧頑崛縺医ｋ・・---
		// 鬮倥＆縺ｯ蝗ｺ螳壹∵ｰｴ蟷ｳ髱｢縺ｯ蜷代″縺ｫ蠢懊§縺ｦ髟ｷ遏ｭ繧貞・繧頑崛縺医ｋ
		const float HITBOX_HEIGHT_SCALE = 1.0f;
		const float HITBOX_SHORT = 0.35f;	// 蜷代″縺ｨ逶ｴ莠､縺吶ｋ遏ｭ霎ｺ
		const float HITBOX_LONG = 0.65f;	// 蜷代″縺ｫ豐ｿ縺｣縺滄聞霎ｺ

		// 蝗櫁ｻ｢縺九ｉ蜑肴婿繝吶け繝医Ν繧堤ｮ怜・縺励※縲√←縺｡繧峨・霆ｸ縺悟━蜍｢縺句愛螳壹☆繧・
		float radFacing = XMConvertToRadians(player[p].rotation.y);
		float facingX = sinf(radFacing);
		float facingZ = cosf(radFacing);
		bool facingZDominant = fabsf(facingZ) <= fabsf(facingX);

		float widthScale = facingZDominant ? HITBOX_SHORT : HITBOX_LONG;	// X譁ｹ蜷代せ繧ｱ繝ｼ繝ｫ
		float depthScale = facingZDominant ? HITBOX_LONG : HITBOX_SHORT;	// Z譁ｹ蜷代せ繧ｱ繝ｼ繝ｫ

		XMFLOAT3 hitboxScaling = XMFLOAT3
		(
			player[p].scaling.x * renderScale * widthScale,
			player[p].scaling.y * renderScale * HITBOX_HEIGHT_SCALE,
			player[p].scaling.z * renderScale * depthScale
		);


		/////////////////////////////////////////////////////////////////////////////////////
		// TODO:蟒ｺ迚ｩ縺ｨ縺ｮ縺ｻ縺｣縺昴＞蠖薙◆繧雁愛螳壹→縺ｯ蛻･縺ｫ縲∵判謦・ｒ鬟溘ｉ縺・畑縺ｮ螟ｧ縺阪ａ縺ｮ蠖薙◆繧雁愛螳壹ｒ菴懊ｋ
		// TODO:驥榊鴨縺ｮ隕狗峩縺励→縲√・繝ｬ繧､繝､繝ｼ縺碁㍾蜉帙↓繧医ｊ辟｡髯舌↓豁ｻ縺ｬ縺ｮ繧帝亟縺・
		
		// AABB 繧堤樟蝨ｨ縺ｮ菴咲ｽｮ繝ｻ繧ｹ繧ｱ繝ｼ繝ｫ・医ヲ繝・ヨ繝懊ャ繧ｯ繧ｹ・峨〒譖ｴ譁ｰ縺励※縺翫￥・郁｡晉ｪ∝愛螳壹〒菴ｿ逕ｨ・・
		CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);

		// 1. 騾溷ｺｦ縺後≠繧後・縲√◎縺ｮ蛻・□縺大ｺｧ讓吶ｒ蜍輔°縺呻ｼ医％繧後′縲悟聖縺｣鬟帙ｓ縺ｧ縺・ｋ縲咲憾諷具ｼ・
		player[p].position.x += player[p].velocity.x;
		player[p].position.y += player[p].velocity.y;
		player[p].position.z += player[p].velocity.z;

		// 2. 鞫ｩ謫ｦ縺ｧ貂幃・
		player[p].velocity.x *= 0.95f; // 1譛ｪ貅繧呈寺縺代ｋ縺ｨ縺繧薙□繧馴≦縺上↑繧・
		player[p].velocity.z *= 0.95f;

		// 3. 驥榊鴨繧偵°縺代ｋ・域ｵｮ縺九○縺溷ｴ蜷茨ｼ・
		if (!player[p].duringRespawn)
		{
			if (player[p].position.y >= -11.0f) {
				player[p].velocity.y = 0.02f; // 荳句髄縺阪・蜉・
			}
			else {
				player[p].velocity.y = 0.0f;
			}
		}

		posBuff = player[p].position;

		// 蝨ｰ髱｢縺ｮ鬮倥＆・域怙菴弱Λ繧､繝ｳ・・
		//float groundHeight = -10.0f;	// 螂郁誠縺ｮ蠎・
		//bool isShadowEnabled = false;		// 蝨ｰ髱｢縺ｫ雜ｳ縺後▽縺・※縺・ｋ縺九ヵ繝ｩ繧ｰ

		// 繝槭ャ繝励ョ繝ｼ繧ｿ・亥慍髱｢・峨→縺ｮ蠖薙◆繧雁愛螳・
		int fieldCount = GetFieldObjectCount();
		MAPDATA* fieldObjects = GetFieldObjects();

		for (int j = 0; j < fieldCount; ++j)
		{
			// 繧｢繧ｯ繝・ぅ繝悶§繧・↑縺・√∪縺溘・ no 縺・MAX 縺ｪ繧峨せ繧ｭ繝・・
			if (!fieldObjects[j].isActive || fieldObjects[j].no == FIELD::FIELD_MAX)
			{
				continue;
			}

			// 繝励Ξ繧､繝､繝ｼ縺ｮAABB・井ｽ薙・荳驛ｨ・峨′蜈ｭ隗呈浤縺ｫ荵励▲縺ｦ縺・ｋ縺・
			if (CheckAABBHexCollision(player[p].boundingBox, fieldObjects[j].boundingBox))
			{
				// 繧ｿ繧､繝ｫ縺ｮ荳企擇縺ｮY蠎ｧ讓吶ｒ險育ｮ・
				float tileTopY = fieldObjects[j].pos.y + (fieldObjects[j].boundingBox.height / 2.0f);	// -1 + 1.5 = 0.5

				// 繝励Ξ繧､繝､繝ｼ縺ｮ蠎暮擇縺後ち繧､繝ｫ縺ｮ荳企擇莉･荳九°
				if (player[p].boundingBox.Min.y <= tileTopY)
				{
					const float baseHalfHeight = COORDINATE;
					// 逹蝨ｰ縺ｧ縺ｯ隕九◆逶ｮ縺ｮ鬮倥＆・域緒逕ｻ繧ｹ繧ｱ繝ｼ繝ｫ・峨ｒ蝓ｺ貅悶↓險育ｮ励＠縺ｦ縺・ｋ縺溘ａ physicsScaling 繧剃ｽｿ逕ｨ
					float halfHeight = baseHalfHeight * player[p].scaling.y * renderScale;

					// 逹蝨ｰ縺輔○繧具ｼ医ａ繧願ｾｼ縺ｿ縺瑚ｵｷ縺阪↑縺・ｈ縺・怙菴主､縺ｨ縺励※陬懈ｭ｣・・
					float targetY = tileTopY + halfHeight;
					if (player[p].position.y < targetY)
					{
						player[p].position.y = targetY;
						player[p].isShadowEnabled = true; // 蠖ｱ繧ｨ繝輔ぉ繧ｯ繝磯撼陦ｨ遉ｺ
					}

					// AABB 繧貞・險育ｮ励＠縺ｦ謨ｴ蜷域ｧ繧剃ｿ昴▽・域緒逕ｻ繧ｹ繧ｱ繝ｼ繝ｫ繧定・・・・
					// 繝偵ャ繝医・繝・け繧ｹ・亥髄縺阪↓蠢懊§縺滄聞譁ｹ蠖｢・峨〒蜀崎ｨ育ｮ励☆繧・
					CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);

					top_y = tileTopY;

					break;
				}
			}
		}

		// -------------------------------------------------------------------------------------
		// 蟒ｺ迚ｩ縺ｨ縺ｮ蠖薙◆繧雁愛螳・
		// -------------------------------------------------------------------------------------
		int buildingCount = GetBuildingCount();			// 謨ｰ繧貞叙蠕・
		Building** buildingObjects = GetBuildings();	// 繝ｪ繧ｹ繝医ｒ蜿門ｾ・

		for (int j = 0; j < buildingCount; ++j)
		{
			// 繧｢繧ｯ繝・ぅ繝悶〒縺ｪ縺・↑繧臥┌隕・
			if (!buildingObjects[j]->isActive)	continue;

			// 霑ｽ蜉・哥BX蜷阪′ "togeki" 縺ｮ蟒ｺ迚ｩ縺ｨ縺ｯ蠖薙◆繧雁愛螳壹＠縺ｪ縺・
			// ・・lant 繧ｿ繧､繝励・繝｢繝・Ν蜷埼・蛻励↓ "togeki" 縺後≠繧区Φ螳夲ｼ・
			const char* modelName = buildingObjects[j]->GetModelName();
			if (buildingObjects[j]->GetType() == BuildingType::Plant &&
				std::strcmp(modelName, "togeki") == 0)
			{
				// 縺薙・蟒ｺ迚ｩ縺ｯ陦晉ｪ∝愛螳壹ｒ辟｡隕・
				continue;
			}

			// 蟒ｺ迚ｩ縺瑚・蛻・〒險育ｮ励＠縺ｦ縺翫＞縺ｦ縺上ｌ縺・AABB 繧偵ｂ繧峨≧縺縺托ｼ・
			const AABB& bBox = buildingObjects[j]->GetAABB();

			// 蛻､螳夲ｼ・
			MTV collision = CalculateAABBMTV(player[p].boundingBox, bBox);			if (collision.isColliding)
			{
				// 陦晉ｪ√＠縺ｦ縺・◆繧峨｀TV縺ｮ蛻・□縺台ｽ咲ｽｮ繧呈綾縺・
				player[p].position.x += collision.translation.x;
				player[p].position.y += collision.translation.y;
				player[p].position.z += collision.translation.z;

				// 謚ｼ縺玲綾縺怜ｾ後・譁ｰ縺励＞AABB繧貞・險育ｮ暦ｼ域緒逕ｻ繧ｹ繧ｱ繝ｼ繝ｫ繧貞渚譏・・
				// 繝偵ャ繝医・繝・け繧ｹ・亥髄縺阪↓蠢懊§縺滄聞譁ｹ蠖｢・峨〒蜀崎ｨ育ｮ励☆繧・
				CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);
			}
		}

		// 繝励Ξ繧､繝､繝ｼ縺ｫ蟇ｾ蠢懊☆繧区判謦・が繝悶ず繧ｧ繧ｯ繝医ｒ PLAYER_MAX 蛻・Ν繝ｼ繝励＠縺ｦ繧ｹ繧ｱ繝ｼ繝ｪ繝ｳ繧ｰ蜷梧悄
		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			ATTACK_OBJECT* attackObject = GetAttack(p); // GetAttack 縺ｯ 1-based
			if (attackObject == nullptr) continue;

			// 繝励Ξ繧､繝､繝ｼ蛛ｴ縺ｮ繧ｹ繧ｱ繝ｼ繝ｫ縺ｫ蜷医ｏ縺帙ｋ・域判謦・が繝悶ず繧ｧ繧ｯ繝医・蜊雁・・・
			attackObject->scaling.x = player[p].scaling.x * 0.5f;
			attackObject->scaling.y = player[p].scaling.y * 0.5f;
			attackObject->scaling.z = player[p].scaling.z * 0.5f;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////
		// TODO:

		// -------------------------------------------------------------
		// 繝励Ξ繧､繝､繝ｼ繧ｪ繝悶ず繧ｧ繧ｯ繝亥酔螢ｫ縺ｮ蠖薙◆繧雁愛螳夲ｼ・LAYER_MAX蛻・ｯｾ蠢懶ｼ・
		// -------------------------------------------------------------
		for (int otherIndex = p + 1; otherIndex < PLAYER_MAX; ++otherIndex)
		{
			// 髱槭い繧ｯ繝・ぅ繝悶・辟｡隕・
			if (!player[otherIndex].active) continue;

			// 莉悶・繝ｬ繧､繝､繝ｼ縺ｮ AABB 繧呈峩譁ｰ・医％縺薙〒螳夂ｾｩ貂医∩縺ｮ hitboxScalingOther 繧剃ｽｿ逕ｨ・・
			CalculateAABB(player[otherIndex].boundingBox, player[otherIndex].position, hitboxScaling);

			// 陦晉ｪ√メ繧ｧ繝・け・医・繧｢ p <-> otherIndex 繧剃ｸ蠎ｦ縺縺大愛螳夲ｼ・
			MTV collision_player = CalculateAABBMTV(player[p].boundingBox, player[otherIndex].boundingBox);

			if (collision_player.isColliding)
			{
				// 蜷代″繝吶け繝医Ν繧呈峩譁ｰ・・otation.y 縺九ｉ邂怜・・・
				{
					float rad_p = XMConvertToRadians(player[p].rotation.y);
					player[p].dir.x = sinf(rad_p);
					player[p].dir.z = cosf(rad_p);
				}


				{
					float rad_o = XMConvertToRadians(player[otherIndex].rotation.y);
					player[otherIndex].dir.x = sinf(rad_o);
					player[otherIndex].dir.z = cosf(rad_o);
				}

				// 謚ｼ縺玲綾縺鈴㍼ (MTV) 繧貞濠蛻・↓縺励※蜿梧婿縺ｫ驕ｩ逕ｨ
				XMFLOAT3 half_translation =
				{
					collision_player.translation.x * 0.5f,
					collision_player.translation.y * 0.5f,
					collision_player.translation.z * 0.5f
				};

				// object[p] 繧・MTV 縺ｮ蜊雁・縺縺第款縺・
				player[p].position.x += half_translation.x;
				player[p].position.y += half_translation.y;
				player[p].position.z += half_translation.z;

				// object[otherIndex] 繧帝・婿蜷代↓蜊雁・縺縺第款縺・
				player[otherIndex].position.x -= half_translation.x;
				player[otherIndex].position.y -= half_translation.y;
				player[otherIndex].position.z -= half_translation.z;

				// 謚ｼ縺玲綾縺怜ｾ後・譁ｰ縺励＞AABB繧貞・險育ｮ・(繝偵ャ繝医・繝・け繧ｹ縺ｧ)
				CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);
				CalculateAABB(player[otherIndex].boundingBox, player[otherIndex].position, hitboxScaling);
			}
		}

		SetHPValue(&HPBar[p], (int)player[p].hp, (int)PLAYER_MAX_HP);
		UpdateHP(&HPBar[p]);

		if (&HPBar[p])
		{
			SetHPOutline(&HPBar[p], player[p].type);
		}
	}

	// 繝励Ξ繧､繝､繝ｼ蜷悟｣ｫ縺ｮ謾ｻ謦・愛螳・
	AttackPlayerCollisions();
	//ImGui::End();
}

//======================================================
//	繧ｷ繝ｫ繧ｨ繝・ヨ逕ｨ謠冗判
//======================================================
static void Player_DrawSilhouette(int p)
{
	if (!Loader::IsFinished && g_loadedCount == 0) return;
	if (!player[p].active) return;

	// 繝励Ο繧ｸ繧ｧ繧ｯ繧ｷ繝ｧ繝ｳ繝ｻ繝薙Η繝ｼ陦悟・繧貞叙蠕・
	XMMATRIX proj = GetProjectionMatrix();
	XMMATRIX view = GetViewMatrix();

	const float scale = 3.5f; // 騾壼ｸｸ謠冗判縺ｨ蜷後§蛟咲紫繧偵°縺代ｋ

	// 繝ｯ繝ｼ繝ｫ繝芽｡悟・・医ン繝ｫ繝懊・繝会ｼ・
	XMMATRIX scalingMatrix = XMMatrixScaling(
		player[p].scaling.x * scale,
		player[p].scaling.y * scale,
		player[p].scaling.z * scale
	);

	XMMATRIX viewMatrix = GetViewMatrix();
	viewMatrix.r[3].m128_f32[0] = 0.0f;
	viewMatrix.r[3].m128_f32[1] = 0.0f;
	viewMatrix.r[3].m128_f32[2] = 0.0f;
	viewMatrix.r[3].m128_f32[3] = 1.0f;
	viewMatrix = XMMatrixTranspose(viewMatrix);
	viewMatrix.r[3].m128_f32[0] = player[p].position.x;
	viewMatrix.r[3].m128_f32[1] = player[p].position.y;
	viewMatrix.r[3].m128_f32[2] = player[p].position.z;
	viewMatrix.r[3].m128_f32[3] = 1.0f;

	XMMATRIX worldMatrix = scalingMatrix * viewMatrix;
	Shader_SetWorldMatrix(worldMatrix);

	XMMATRIX wvp = scalingMatrix * viewMatrix * view * proj;
	Shader_SetMatrix(wvp);

	Shader_Begin();
	SetBlendState(BLENDSTATE_ALPHA);

	// 繧ｷ繝ｫ繧ｨ繝・ヨ濶ｲ繧定ｨｭ螳夲ｼ医・繝ｬ繧､繝､繝ｼ縺斐→縺ｫ逡ｰ縺ｪ繧玖牡・・
	XMFLOAT4 silhouetteColor;
	switch (p)
	{
	case 0: silhouetteColor  = { 0.64f,  0.2f, 0.2f, 1.0f }; break; // 襍､
	case 1: silhouetteColor  = {  0.0f, 0.45f, 0.7f, 1.0f }; break; // 髱・
	case 2: silhouetteColor  = {  0.7f,  0.7f, 0.0f, 1.0f }; break; // 鮟・
	case 3: silhouetteColor  = {  0.0f,  0.6f, 0.0f, 1.0f }; break; // 邱・
	default: silhouetteColor = {  1.0f,  1.0f, 1.0f, 1.0f }; break;
	}
	Shader_SetColor(silhouetteColor);

	// 豺ｱ蠎ｦ繝・せ繝・螂･縺ｫ縺ゅｋ譎ゅ□縺第緒逕ｻ縺吶ｋ・・reater・・
	ID3D11DeviceContext* context = Direct3D_GetDeviceContext();
	ID3D11DepthStencilState* depthStateGreater = Direct3D_GetDepthStateGreater();
	context->OMSetDepthStencilState(depthStateGreater, 0);

	// 繧ｷ繝ｫ繧ｨ繝・ヨ逕ｨ縺ｮ謠冗判繝｢繝ｼ繝芽ｨｭ螳・
	Shader_SetDrawMode(1);

	// 繝・け繧ｹ繝√Ε險ｭ螳夲ｼ磯壼ｸｸ謠冗判縺ｨ蜷後§・・
	ID3D11ShaderResourceView* srv = nullptr;
	switch (player[p].form)
	{
	// 隨ｬ1蠖｢諷・
	case Form::First:
			 if (p == 0)				srv = g_Texture[0];
		else if (p == 1)				srv = g_Texture[1];
		else if (p == 2)				srv = g_Texture[2];
		else if (p == 3)				srv = g_Texture[3];
		break;
	// 隨ｬ2蠖｢諷・
	case Form::Second:
		switch (player[p].type)
		{
		case PlayerType::Glass:			srv = g_Texture[4];	break;
		case PlayerType::Concrete:		srv = g_Texture[5];	break;
		case PlayerType::Plant:			srv = g_Texture[6];	break;
		case PlayerType::Electricity:	srv = g_Texture[7];	break;
		default: break;
		}
		break;
	// 隨ｬ3蠖｢諷・
	case Form::Third:
		switch (player[p].type)
		{
		case PlayerType::Glass:			srv = g_Texture[8];		break;
		case PlayerType::Concrete:		srv = g_Texture[9];		break;
		case PlayerType::Plant:			srv = g_Texture[10];	break;
		case PlayerType::Electricity:	srv = g_Texture[11];	break;
		default: break;
		}
		break;
	default: break;
	}

	// 繧ｹ繧ｭ繝ｫ繝ｻ繧ｹ繝壹す繝｣繝ｫ蟆ら畑繝・け繧ｹ繝√Ε
	if (player[p].useSpecial && player[p].specialAnimation)	srv = g_Texture[13];	// 繧ｹ繝壹す繝｣繝ｫ繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ邯咏ｶ壻ｸｭ縺ｮ縺ｿ
	else if (player[p].skillAnimation)						srv = g_Texture[12];	// 繧ｹ繧ｭ繝ｫ逋ｺ蜍輔い繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ

	// 鬆らせ繝舌ャ繝輔ぃ縺ｫ繝・・繧ｿ繧ｳ繝斐・・・V險ｭ螳夲ｼ・
	D3D11_MAPPED_SUBRESOURCE msr;
	Vertex2 localVt[PLAYER_VERTEX];
	CopyMemory(&localVt[0], &vdata[0], sizeof(Vertex2) * PLAYER_VERTEX);

	// 迴ｾ蝨ｨ縺ｮ繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ繝輔Ξ繝ｼ繝縺九ｉUV險育ｮ・
	int frame = player[p].animFrame;
	int col = frame % SHEET_COLS;
	int row = frame / SHEET_COLS;
	float u0 = (float)col / (float)SHEET_COLS;
	float v0 = (float)row / (float)SHEET_ROWS;
	float u1 = u0 + 1.0f / (float)SHEET_COLS;
	float v1 = v0 + 1.0f / (float)SHEET_ROWS;

	localVt[0].tex = XMFLOAT2(u0, v0);
	localVt[1].tex = XMFLOAT2(u1, v0);
	localVt[2].tex = XMFLOAT2(u0, v1);
	localVt[3].tex = XMFLOAT2(u1, v1);

	context->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex2* vertex = (Vertex2*)msr.pData;
	CopyMemory(vertex, &localVt[0], sizeof(Vertex2) * PLAYER_VERTEX);
	context->Unmap(g_VertexBuffer, 0);

	context->PSSetShaderResources(0, 1, &srv);

	// 謠冗判
	UINT stride = sizeof(Vertex2);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->DrawIndexed(6, 0, 0);

	// 豺ｱ蠎ｦ繧ｹ繝・・繝医ｒ謌ｻ縺・
	ID3D11DepthStencilState* depthStateEnable = Direct3D_GetDepthStateEnable();
	context->OMSetDepthStencilState(depthStateEnable, 0);

	// 謠冗判繝｢繝ｼ繝峨ｒ騾壼ｸｸ縺ｫ謌ｻ縺・
	Shader_SetDrawMode(0);
	Shader_SetColor(color::white);
}

//======================================================
//	繧｢繧ｦ繝医Λ繧､繝ｳ逕ｨ謠冗判
//======================================================
static void Player_DrawOutline(int p)
{
	if (!Loader::IsFinished && g_loadedCount == 0) return;
	if (!player[p].active) return;

	// 繝励Ο繧ｸ繧ｧ繧ｯ繧ｷ繝ｧ繝ｳ繝ｻ繝薙Η繝ｼ陦悟・繧貞叙蠕・
	XMMATRIX proj = GetProjectionMatrix();
	XMMATRIX view = GetViewMatrix();

	const float scale = 3.6f; // 騾壼ｸｸ謠冗判繧雁ｰ代＠螟ｧ縺阪ａ縺ｮ蛟咲紫繧偵°縺代ｋ

	// 繝ｯ繝ｼ繝ｫ繝芽｡悟・・医ン繝ｫ繝懊・繝会ｼ・
	XMMATRIX scalingMatrix = XMMatrixScaling(
		player[p].scaling.x * scale,
		player[p].scaling.y * scale,
		player[p].scaling.z * scale
	);

	XMMATRIX viewMatrix = GetViewMatrix();
	viewMatrix.r[3].m128_f32[0] = 0.0f;
	viewMatrix.r[3].m128_f32[1] = 0.0f;
	viewMatrix.r[3].m128_f32[2] = 0.0f;
	viewMatrix.r[3].m128_f32[3] = 1.0f;
	viewMatrix = XMMatrixTranspose(viewMatrix);
	viewMatrix.r[3].m128_f32[0] = player[p].position.x;
	viewMatrix.r[3].m128_f32[1] = player[p].position.y;
	viewMatrix.r[3].m128_f32[2] = player[p].position.z;
	viewMatrix.r[3].m128_f32[3] = 1.0f;

	XMMATRIX worldMatrix = scalingMatrix * viewMatrix;
	Shader_SetWorldMatrix(worldMatrix);

	XMMATRIX wvp = scalingMatrix * viewMatrix * view * proj;
	Shader_SetMatrix(wvp);

	Shader_Begin();
	SetBlendState(BLENDSTATE_ALPHA);

	// 繧ｷ繝ｫ繧ｨ繝・ヨ濶ｲ繧定ｨｭ螳夲ｼ医・繝ｬ繧､繝､繝ｼ縺斐→縺ｫ逡ｰ縺ｪ繧玖牡・・
	XMFLOAT4 outerColor;
	switch (p)
	{
	case 0: outerColor = { 0.94f,  0.5f, 0.5f, 1.0f }; break; // 襍､
	case 1: outerColor = {  0.0f, 0.75f, 1.0f, 1.0f }; break; // 髱・
	case 2: outerColor = {  1.0f,  1.0f, 0.3f, 1.0f }; break; // 鮟・
	case 3: outerColor = {  0.0f,  1.0f, 0.0f, 1.0f }; break; // 邱・
	default: outerColor = { 1.0f, 1.0f, 1.0f, 0.4f }; break;
	}
	Shader_SetColor(outerColor);

	// 繧｢繧ｦ繝医Λ繧､繝ｳ逕ｨ縺ｮ謠冗判繝｢繝ｼ繝芽ｨｭ螳・
	Shader_SetDrawMode(2);

	// 繝・け繧ｹ繝√Ε險ｭ螳夲ｼ磯壼ｸｸ謠冗判縺ｨ蜷後§・・
	ID3D11ShaderResourceView* srv = nullptr;
	switch (player[p].form)
	{
	// 隨ｬ1蠖｢諷・
	case Form::First:
			 if (p == 0)				srv = g_Texture[0];
		else if (p == 1)				srv = g_Texture[1];
		else if (p == 2)				srv = g_Texture[2];
		else if (p == 3)				srv = g_Texture[3];
		break;
	// 隨ｬ2蠖｢諷・
	case Form::Second:
		switch (player[p].type)
		{
		case PlayerType::Glass:			srv = g_Texture[4];	break;
		case PlayerType::Concrete:		srv = g_Texture[5];	break;
		case PlayerType::Plant:			srv = g_Texture[6];	break;
		case PlayerType::Electricity:	srv = g_Texture[7];	break;
		default: break;
		}
		break;
	// 隨ｬ3蠖｢諷・
	case Form::Third:
		switch (player[p].type)
		{
		case PlayerType::Glass:			srv = g_Texture[8];		break;
		case PlayerType::Concrete:		srv = g_Texture[9];		break;
		case PlayerType::Plant:			srv = g_Texture[10];	break;
		case PlayerType::Electricity:	srv = g_Texture[11];	break;
		default: break;
		}
		break;
	}

	// 繧ｹ繧ｭ繝ｫ繝ｻ繧ｹ繝壹す繝｣繝ｫ蟆ら畑繝・け繧ｹ繝√Ε
	if (player[p].useSpecial && player[p].specialAnimation)	srv = g_Texture[13];	// 繧ｹ繝壹す繝｣繝ｫ繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ邯咏ｶ壻ｸｭ縺ｮ縺ｿ
	else if (player[p].skillAnimation)						srv = g_Texture[12];	// 繧ｹ繧ｭ繝ｫ逋ｺ蜍輔い繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ

	// 鬆らせ繝舌ャ繝輔ぃ縺ｫ繝・・繧ｿ繧ｳ繝斐・・・V險ｭ螳夲ｼ・
	D3D11_MAPPED_SUBRESOURCE msr;
	Vertex2 localVt[PLAYER_VERTEX];
	CopyMemory(&localVt[0], &vdata[0], sizeof(Vertex2) * PLAYER_VERTEX);

	// 迴ｾ蝨ｨ縺ｮ繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ繝輔Ξ繝ｼ繝縺九ｉUV險育ｮ・
	int frame = player[p].animFrame;
	int col = frame % SHEET_COLS;
	int row = frame / SHEET_COLS;
	float u0 = (float)col / (float)SHEET_COLS;
	float v0 = (float)row / (float)SHEET_ROWS;
	float u1 = u0 + 1.0f / (float)SHEET_COLS;
	float v1 = v0 + 1.0f / (float)SHEET_ROWS;

	localVt[0].tex = XMFLOAT2(u0, v0);
	localVt[1].tex = XMFLOAT2(u1, v0);
	localVt[2].tex = XMFLOAT2(u0, v1);
	localVt[3].tex = XMFLOAT2(u1, v1);

	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex2* vertex = (Vertex2*)msr.pData;
	CopyMemory(vertex, &localVt[0], sizeof(Vertex2) * PLAYER_VERTEX);
	g_pContext->Unmap(g_VertexBuffer, 0);

	g_pContext->PSSetShaderResources(0, 1, &srv);

	// 謠冗判
	UINT stride = sizeof(Vertex2);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pContext->DrawIndexed(6, 0, 0);

	// 謠冗判繝｢繝ｼ繝峨ｒ騾壼ｸｸ縺ｫ謌ｻ縺・
	Shader_SetDrawMode(0);
	Shader_SetColor(color::white);
}

//======================================================
//	繝励Ξ繧､繝､繝ｼ譛ｬ菴捺緒逕ｻ髢｢謨ｰ
//======================================================
void Player_Draw(bool s_IsKonamiCodeEntered)
{
	if (!Loader::IsFinished && g_loadedCount == 0) return;

	// 謾ｻ謦・・繧ｹ繧ｭ繝ｫ繝ｻ繧ｹ繝壹す繝｣繝ｫ謠冗判
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (player[p].active && player[p].isAttacking)	Attack_Draw(p);
		//if (player[p].active && player[p].useSkill)		Skill_Draw(p);
		if (player[p].active && player[p].useSpecial)	Special_Draw(p);
	}

	LIGHT light{};
	light.Enable = TRUE;
	// 蜈峨・蜷代″・医Ρ繝ｼ繝ｫ繝臥ｩｺ髢難ｼ峨す繧ｧ繝ｼ繝繝ｼ蛛ｴ縺ｧ蜊倅ｽ榊喧縺励※菴ｿ縺｣縺ｦ縺・ｋ諠ｳ螳・
	light.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
	// 諡｡謨｣蜈峨→迺ｰ蠅・・
	light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	Shader_SetLight(light);

	static bool input1 = false;
	// 繝・ヰ繝・げ繝｢繝ｼ繝我ｸｭ縺ｮ縺ｿ繧ｭ繝ｼ蜈･蜉帙ｒ蜿励￠莉倥￠繧・
	if (s_IsKonamiCodeEntered)
	{
		if (Keyboard_IsKeyDownTrigger(KK_D1)) input1 = !input1;	// 繝輔Λ繧ｰ蜿崎ｻ｢
	}

	Shader_Begin();

	// ========================================================
	// 螂･縺ｮ繝励Ξ繧､繝､繝ｼ縺梧焔蜑阪・繝励Ξ繧､繝､繝ｼ縺ｫ髫繧後↑縺・ｈ縺・↓謠冗判
	// ========================================================

	// 繝励Ο繧ｸ繧ｧ繧ｯ繧ｷ繝ｧ繝ｳ繝ｻ繝薙Η繝ｼ陦悟・繧貞・縺ｫ蜿門ｾ・
	XMMATRIX projection = GetProjectionMatrix();
	XMMATRIX view = GetViewMatrix();

	// 繧ｫ繝｡繝ｩ菴咲ｽｮ繧堤ｮ怜・・・iew 縺ｮ騾・｡悟・縺ｮ r[3] 縺後Ρ繝ｼ繝ｫ繝臥ｩｺ髢薙・繧ｫ繝｡繝ｩ菴咲ｽｮ・・
	XMMATRIX invView = XMMatrixInverse(nullptr, view);
	XMFLOAT3 camPos;
	camPos.x = invView.r[3].m128_f32[0];
	camPos.y = invView.r[3].m128_f32[1];
	camPos.z = invView.r[3].m128_f32[2];

	// 繝励Ξ繧､繝､繝ｼ繧呈緒逕ｻ縺吶ｋ繝ｩ繝繝・・rojection, View 繧偵く繝｣繝励メ繝｣・・
	auto DrawPlayerInternal = [&](int idx)
	{
		if (!player[idx].active) return;

		// 繝励Ξ繧､繝､繝ｼ縺ｮ蠖ｱ繧ｨ繝輔ぉ繧ｯ繝域緒逕ｻ
		EffectShadow_DrawForPlayer(idx);

		const float spriteScale = 3.5f;	// 陦ｨ遉ｺ蛟咲紫

		// 繝ｯ繝ｼ繝ｫ繝芽｡悟・・医ン繝ｫ繝懊・繝蛾｢ｨ縺ｮ譌｢蟄倥Ο繧ｸ繝・け繧定ｸ剰･ｲ・・
		XMMATRIX ScalingMatrix = XMMatrixScaling(
			player[idx].scaling.x * spriteScale,
			player[idx].scaling.y * spriteScale,
			player[idx].scaling.z * spriteScale
		);

		XMMATRIX vm = GetViewMatrix();	// 繧ｫ繝｡繝ｩ縺ｮ陦悟・
		vm.r[3].m128_f32[0] = 0.0f;
		vm.r[3].m128_f32[1] = 0.0f;
		vm.r[3].m128_f32[2] = 0.0f;
		vm.r[3].m128_f32[3] = 1.0f;
		vm = XMMatrixTranspose(vm);
		vm.r[3].m128_f32[0] = player[idx].position.x;
		vm.r[3].m128_f32[1] = player[idx].position.y;
		vm.r[3].m128_f32[2] = player[idx].position.z;
		vm.r[3].m128_f32[3] = 1.0f;

		// World 陦悟・・医ン繝ｫ繝懊・繝臥畑・峨ｒ繧ｷ繧ｧ繝ｼ繝繝ｼ縺ｫ貂｡縺・
		XMMATRIX WorldMatrix = ScalingMatrix * vm;
		Shader_SetWorldMatrix(WorldMatrix);

		XMMATRIX WVP = ScalingMatrix * vm * view * projection;

		Shader_SetMatrix(WVP);
		Shader_Begin();
		SetBlendState(BLENDSTATE_ALPHA);

		// 鬆らせ繝舌ャ繝輔ぃ縺ｫ繝・・繧ｿ繧ｳ繝斐・・医ヵ繝ｬ繝ｼ繝縺ｫ蠢懊§縺ｦUV繧呈嶌縺肴鋤縺医ｋ・・
		D3D11_MAPPED_SUBRESOURCE msr;

		// 繧ｳ繝斐・蜈・・vdata 繧偵Ο繝ｼ繧ｫ繝ｫ驟榊・縺ｫ繧ｳ繝斐・縺励※ UV 繧定ｪｿ謨ｴ
		Vertex2 localV[PLAYER_VERTEX];
		CopyMemory(&localV[0], &vdata[0], sizeof(Vertex2) * PLAYER_VERTEX);

		// 迴ｾ蝨ｨ縺ｮ繝輔Ξ繝ｼ繝縺九ｉ UV 繧定ｨ育ｮ・
		int frame = player[idx].animFrame;
		int col = frame % SHEET_COLS;
		int row = frame / SHEET_COLS;
		float u0 = (float)col / (float)SHEET_COLS;
		float v0 = (float)row / (float)SHEET_ROWS;
		float u1 = u0 + 1.0f / (float)SHEET_COLS;
		float v1 = v0 + 1.0f / (float)SHEET_ROWS;

		// 鬆らせ縺ｮ繝・け繧ｹ繝√Ε蠎ｧ讓吶ｒ荳頑嶌縺・
		localV[0].tex = XMFLOAT2(u0, v0);	// LEFT-TOP
		localV[1].tex = XMFLOAT2(u1, v0);	// RIGHT-TOP
		localV[2].tex = XMFLOAT2(u0, v1);	// LEFT-BOTTOM
		localV[3].tex = XMFLOAT2(u1, v1);	// RIGHT-BOTTOM

		// 繝舌ャ繝輔ぃ縺ｸ譖ｸ縺崎ｾｼ縺ｿ
		g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		Vertex2* vertex = (Vertex2*)msr.pData;
		CopyMemory(vertex, &localV[0], sizeof(Vertex2) * PLAYER_VERTEX);
		g_pContext->Unmap(g_VertexBuffer, 0);

		ID3D11ShaderResourceView* srv = nullptr;

		// 蠖｢諷九→繧ｿ繧､繝励↓蠢懊§縺溘ユ繧ｯ繧ｹ繝√Ε繧定ｨｭ螳・
		switch (player[idx].form)
		{
			// 隨ｬ1蠖｢諷・
		case Form::First:
			if (idx == 0)					srv = g_Texture[0];
			else if (idx == 1)				srv = g_Texture[1];
			else if (idx == 2)				srv = g_Texture[2];
			else if (idx == 3)				srv = g_Texture[3];
			break;
			// 隨ｬ2蠖｢諷・
		case Form::Second:
			switch (player[idx].type)
			{
			case PlayerType::Glass:			srv = g_Texture[4];	break;
			case PlayerType::Concrete:		srv = g_Texture[5];	break;
			case PlayerType::Plant:			srv = g_Texture[6];	break;
			case PlayerType::Electricity:	srv = g_Texture[7];	break;
			default: break;
			}
			break;
			// 隨ｬ3蠖｢諷・
		case Form::Third:
			switch (player[idx].type)
			{
			case PlayerType::Glass:			srv = g_Texture[8];		break;
			case PlayerType::Concrete:		srv = g_Texture[9];		break;
			case PlayerType::Plant:			srv = g_Texture[10];	break;
			case PlayerType::Electricity:	srv = g_Texture[11];	break;
			default: break;
			}
			break;
		}

		// 繧ｹ繧ｭ繝ｫ繝ｻ繧ｹ繝壹す繝｣繝ｫ蟆ら畑繝・け繧ｹ繝√Ε
		if (player[idx].useSpecial && player[idx].specialAnimation)	srv = g_Texture[13];	// 繧ｹ繝壹す繝｣繝ｫ繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ邯咏ｶ壻ｸｭ縺ｮ縺ｿ
		else if (player[idx].skillAnimation)						srv = g_Texture[12];	// 繧ｹ繧ｭ繝ｫ逋ｺ蜍輔い繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ

		g_pContext->PSSetShaderResources(0, 1, &srv);

		// 繝励Ξ繧､繝､繝ｼ縺斐→縺ｫ逡ｰ縺ｪ繧玖牡繧定ｨｭ螳・
		if (player[idx].isAttacked || player[idx].isDamageColor)
		{
			// 縺ｩ縺｡繧峨・繧ｿ繧､繝槭・縺悟虚縺・※縺・ｋ縺・
			float currentTimer = player[idx].isAttacked ? player[idx].attackedTimer : player[idx].damageColorTimer;
			
			// 轤ｹ貊・・騾溘＆
			float speed = 40.0f; 

			// 轤ｹ貊・・蠎ｦ蜷医＞・・.0f・・.0f・・
			float blink = (sinf(currentTimer * speed) + 1.0f) * 0.5f;

			Shader_SetColorLerp(color::white, color::red, blink);

			// 蜆ｪ蜈医＠縺ｦ襍､縺上☆繧・
			//Shader_SetColorLerp(color::white, color::red, 0.7f); 
		}
		else if (player[idx].isPoisoned)
		{
			switch (idx)
			{
				// Lerp = 1.荵礼ｮ苓牡 2.陬憺俣縺吶ｋ濶ｲ 3.陬憺俣縺ｮ蠎ｦ蜷医＞
			case 0:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			case 1:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			case 2:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			case 3:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			default:	Shader_SetColor(color::white); break;
			}
		}
		else	Shader_SetColor(color::white); // 騾壼ｸｸ濶ｲ
		
		// 繝舌ャ繝輔ぃ繧ｻ繝・ヨ & 謠冗判
		UINT stride = sizeof(Vertex2);
		UINT offset = 0;
		g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
		g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		g_pContext->DrawIndexed(6, 0, 0);

		// 繧ｨ繝輔ぉ繧ｯ繝域緒逕ｻ ・医・繝ｬ繧､繝､繝ｼ縺ｮ謇句燕・・
		EffectFront_DrawForPlayer(idx);
	};

	// -----------------------------------
	// 騾乗・謠冗判縺ｮ縺溘ａ縺ｮ繧ｽ繝ｼ繝茨ｼ磯□縺・・ｼ・
	// -----------------------------------
	std::vector<std::pair<float, int>> list;	// (霍晞屬莠御ｹ・ index)
	list.reserve(PLAYER_MAX);

	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (!player[p].active) continue;

		float dx = player[p].position.x - camPos.x;
		float dy = player[p].position.y - camPos.y;
		float dz = player[p].position.z - camPos.z;
		float dist2 = dx * dx + dy * dy + dz * dz;
		list.emplace_back(dist2, p);
	}

	// 驕縺・・ｼ亥､ｧ縺阪＞鬆・ｼ峨↓繧ｽ繝ｼ繝・
	std::sort(list.begin(), list.end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
		{
			return a.first > b.first;
		});

	// 騾城℃繝ｬ繝ｳ繝繝ｪ繝ｳ繧ｰ・壽ｷｱ蠎ｦ繝・せ繝医・譛牙柑縲∵ｷｱ蠎ｦ譖ｸ縺崎ｾｼ縺ｿ縺ｯ辟｡蜉ｹ・・etDepthReadOnly 繧剃ｽｿ逕ｨ・・
	SetDepthTest(true);
	SetDepthReadOnly();	// 豺ｱ蠎ｦ繝・せ繝医・縺吶ｋ縺梧ｷｱ蠎ｦ繝舌ャ繝輔ぃ縺ｸ縺ｮ譖ｸ縺崎ｾｼ縺ｿ縺ｯ縺励↑縺・

	// 繧ｽ繝ｼ繝磯・ｼ磯□縺・ｂ縺ｮ縺九ｉ謠冗判・・
	for (auto& p : list)	DrawPlayerInternal(p.second);

	// 3D繧ｪ繝悶ず繧ｧ繧ｯ繝医・豺ｱ蠎ｦ繝・せ繝医ｒ辟｡蜉ｹ縺ｫ縺励※謠冗判
	SetDepthTest(false);

	// 3D繧ｪ繝悶ず繧ｧ繧ｯ繝茨ｼ医・繝ｬ繧､繝､繝ｼ・峨・謠冗判縺檎ｵゅｏ縺｣縺溷ｾ・..
	SetDepthTest(false); // 繧ｳ繝ｩ繧､繝繝ｼ繧呈怙蜑埼擇縺ｫ蜃ｺ縺励◆縺・↑繧峨％繧後〒OK

	/////////////////////////////////////////////////////////////////////////////////////
	// TODO:蠖薙◆繧雁愛螳壹・蜿ｯ隕門喧
	if (s_IsKonamiCodeEntered)
	{
		// 繝励Ξ繧､繝､繝ｼ縺ｮ謠冗判縺ｫ菴ｿ繧上ｌ縺溯｡悟・繧偵け繝ｪ繧｢縺吶ｋ
		Shader_SetMatrix(XMMatrixIdentity() * GetViewMatrix() * GetProjectionMatrix()); // WVP陦悟・繧棚dentity * View * Projection縺ｫ險ｭ螳・

		// 3. 騾城℃繧・牡縺後♀縺九＠縺上↑繧峨↑縺・ｈ縺・↓繝悶Ξ繝ｳ繝峨せ繝・・繝医ｒ繝ｪ繧ｻ繝・ヨ
		SetBlendState(BLENDSTATE_NONE); // 譫邱壹↑繧峨い繝ｫ繝輔ぃ縺ｪ縺励〒繧０K

		for (int i = 0; i < PLAYER_MAX; i++)
		{
			if (!player[i].active) continue;

			// 4. 濶ｲ繧偵そ繝・ヨ・磯搨濶ｲ縺ｫ縺吶ｋ縺ｪ繧臥ｬｬ4蠑墓焚縺ｮ繧｢繝ｫ繝輔ぃ繧・.0f縺ｫ・・ｼ・
			Shader_SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// 5. 謠冗判・・
			Debug_DrawAABB(player[i].boundingBox, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
		}
	}

	// 繧ｫ繝｡繝ｩ縺九ｉ縺ｮ鬆・分繧偵た繝ｼ繝医＠縺溘ｂ縺ｮ(list)縺ｮ鬆・分縺ｧ蜀榊ｺｦ謠冗判
	// p.second 竊・繧ｽ繝ｼ繝域ｸ医∩縺ｮ繝励Ξ繧､繝､繝ｼ繧､繝ｳ繝・ャ繧ｯ繧ｹ
	for (auto& p : list)
	{
		Player_DrawOutline(p.second);

		DrawPlayerInternal(p.second);
	}

	// 繧ｷ繝ｫ繧ｨ繝・ヨ謠冗判繧定ｿｽ蜉
	for (auto& p : list) Player_DrawSilhouette(p.second);

	// 3D繧ｪ繝悶ず繧ｧ繧ｯ繝医・豺ｱ蠎ｦ繝・せ繝医ｒ辟｡蜉ｹ縺ｫ縺励※謠冗判
	SetDepthTest(false);
}

void Player_DrawHP()
{
	Shader_Begin();

	// 蛟句挨UI繧ｹ繝・・繧ｿ繧ｹ謠冗判
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		SetBlendState(BLENDSTATE_ALPHA);

		// 繝励Ξ繧､繝､繝ｼ縺梧ｭｻ繧薙〒縺・ｋ縺九←縺・°繧貞愛螳・
		bool isDead = (!player[i].active && player[i].stock <= 0);

		DrawHP(&HPBar[i], i + 2, isDead);
		

		if (isDead)
		{// 豁ｻ繧薙□縺ｨ縺阪・縲∫・濶ｲ縺ｮHP繝舌・繧呈ｮ九＠縺ｦ蜈ｨ縺ｦ縺ｮUI繧呈ｶ医☆
			if (!Player_CanUseSpecial(i))
			{
				Effect_Clear(i);
			}
			continue;
		}

		XMFLOAT2 hp = HPBar[i].pos;

		// 繧ｹ繧ｭ繝ｫ繧ｲ繝ｼ繧ｸ陦ｨ遉ｺ逕ｨ縺ｮ蛟､繧定ｨ育ｮ励☆繧・
		float skillFill = 1.0f;

		// 繧ｹ繧ｭ繝ｫ譛ｪ謇謖√↑繧・
		if (player[i].type == PlayerType::None)
		{
			skillFill = 0.0f;
		}
		else
		{
			// 繧ｯ繝ｼ繝ｫ繧ｿ繧､繝槭・縺・縺ｪ繧牙茜逕ｨ蜿ｯ閭ｽ
			if (player[i].skillCoolTimer <= 0.0f)
			{
				skillFill = 1.0f;
			}
			else
			{
				// type縺ｫ蠢懊§縺溘け繝ｼ繝ｫ繧ｿ繧､繝繧貞叙蠕・
				float coolTime = 0.0f;
				switch (player[i].type)
				{
				case PlayerType::Glass:			coolTime = SKILL_GLASS_COOLTIME; break;
				case PlayerType::Concrete:		coolTime = SKILL_CONCRETE_COOLTIME; break;
				case PlayerType::Plant:			coolTime = SKILL_PLANT_COOLTIME; break;
				case PlayerType::Electricity:	coolTime = SKILL_ELECTRICITY_COOLTIME; break;
				default: coolTime = 0.0f; break;
				}

				// 繧ｯ繝ｼ繝ｫ繧ｿ繧､繝縺・縺ｮ譎ゅ・1.0f繧定ｿ斐☆
				if (coolTime <= 0.0f)
				{
					skillFill = 1.0f;
				}
				else
				{
					// 菴ｿ逕ｨ逶ｴ蠕後skillCoolTimer == coolTime => fill = 0.0
					// 繧ｯ繝ｼ繝ｫ邨ゆｺ・skillCoolTimer == 0 => fill = 1.0
					skillFill = 1.0f - (player[i].skillCoolTimer / coolTime);
					if (skillFill < 0.0f) skillFill = 0.0f;
					if (skillFill > 1.0f) skillFill = 1.0f;
				}
			}
		}

		// 騾ｲ蛹悶′蝗ｺ螳壹＆繧後◆繧峨√ち繧､繝励・繧ｲ繝ｼ繧ｸ繧呈怙螟ｧ蛟､縺ｧ陦ｨ遉ｺ縺吶ｋ
		if (player[i].isTypeFixed)
		{
			float glass = 0.0f;
			float concrete = 0.0f;
			float plant = 0.0f;
			float electricity = 0.0f;

			switch (player[i].type)
			{
			case PlayerType::Glass:			glass		= 1.0f;	break;
			case PlayerType::Concrete:		concrete	= 1.0f;	break;
			case PlayerType::Plant:			plant		= 1.0f;	break;
			case PlayerType::Electricity:	electricity = 1.0f;	break;
			default: break;
			}

			Gauge_Set(i, glass, concrete, plant, electricity,
				player[i].evolutionGauge, skillFill, { hp.x - GAUGE_POS_X , hp.y + GAUGE_POS_Y }, player[i].type);
		}
		else
		{
			// 蝗ｺ螳壼燕縺ｯ繧ｫ繧ｦ繝ｳ繝域焚繧偵◎縺ｮ縺ｾ縺ｾ陦ｨ遉ｺ縺吶ｋ
			Gauge_Set(i, player[i].breakCount_Glass, player[i].breakCount_Concrete, player[i].breakCount_Plant, player[i].breakCount_Electricity,
				player[i].evolutionGauge, skillFill, { hp.x - GAUGE_POS_X , hp.y + GAUGE_POS_Y }, player[i].type);
		}

		// 繧ｹ繝壹す繝｣繝ｫ菴ｿ逕ｨ蜿ｯ閭ｽ縺ｪ繧峨お繝輔ぉ繧ｯ繝医ｒ陦ｨ遉ｺ縲√◎縺・〒縺ｪ縺代ｌ縺ｰ豸医☆
		if (Player_CanUseSpecial(i))
		{
			Shader_SetColor(color::white);
			Effect_Set(24, { (hp.x + 12.0f * SCREEN_ADJUST_X), hp.y - (100.0f * SCREEN_ADJUST_Y) }, { (162.0f * SCREEN_ADJUST_X), (60.0f * SCREEN_ADJUST_Y) }, i);
		}
		if (!Player_CanUseSpecial(i))
		{
			Effect_Clear(i);
		}

		// 騾壼ｸｸ繧ｲ繝ｼ繧ｸ・亥・・句､厄ｼ峨・蟶ｸ縺ｫ謠冗判
		// 繧ｹ繧ｭ繝ｫ繧ｲ繝ｼ繧ｸ縺ｯ螻樊ｧ遒ｺ螳壹・縺ｨ縺阪・縺ｿ謠冗判
		Gauge_DrawBasic(i);

		// 螻樊ｧ遒ｺ螳壹＠縺ｦ縺・ｋ縺ｨ縺阪・繧ｹ繧ｭ繝ｫUI繧よ緒逕ｻ
		if (player[i].isTypeFixed)
		{
			Gauge_DrawSkill(i);
		}

		Shader_Begin();

		Player_DrawStock(i);
	}
}

void Player_Respawn(int playerIndex)
{
	// 遽・峇繝√ぉ繝・け 0 1 2 3 莉･螟悶↑繧・return
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	// 谿区ｩ溘′1縺､莉･荳翫≠繧句ｴ蜷・
	if (player[playerIndex].active == true)
	{
		player[playerIndex].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[playerIndex].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
		player[playerIndex].hp = PLAYER_MAX_HP;
		player[playerIndex].attack = 0.0f;
		player[playerIndex].power = 0.0f;
		player[playerIndex].speed = 0.0f;
		player[playerIndex].defense = 1.0f;
		player[playerIndex].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[playerIndex].active = true;
		player[playerIndex].satiety = 0.0f;
		player[playerIndex].isAttacking = false;
		player[playerIndex].attackTimer = 0.0f;
		player[playerIndex].isAttacked = false;
		player[playerIndex].attackedTimer = 0.0f;
		player[playerIndex].isDamageColor = false;
		player[playerIndex].damageColorTimer = 0.0f;
		player[playerIndex].isHealing = false;
		player[playerIndex].healingTimer = 0.0f;
		player[playerIndex].isEvolving = false;
		player[playerIndex].evolvingTimer = 0.0f;
		player[playerIndex].useSkill = false;
		player[playerIndex].skillTimer = 0.0f;
		player[playerIndex].skillCoolTimer = 0.0f;
		player[playerIndex].skillAnimation = false;
		player[playerIndex].useSpecial = false;
		player[playerIndex].specialTimer = 0.0f;
		player[playerIndex].specialAnimation = false;
		player[playerIndex].isInvincible = false;
		player[playerIndex].invincibleTimer = 0.0f;
		player[playerIndex].stunGauge = 0.0f;
		player[playerIndex].isStunning = false;
		player[playerIndex].stunTimer = 0.0f;
		player[playerIndex].isDown = false;
		player[playerIndex].downTimer = 0.0f;
		player[playerIndex].isPoisoned = false;
		player[playerIndex].poisonTimer = 0.0f;
		player[playerIndex].duringRespawn = true;
		player[playerIndex].respawnTimer = 0.0f;
		player[playerIndex].isEggBreaking = false;
		player[playerIndex].eggBreakingTimer = 0.0f;
		player[playerIndex].lastDir = PlayerDir::Down; // 豁｣髱｢
		player[playerIndex].isMoving = false;
		player[playerIndex].isShadowEnabled = true;
		player[playerIndex].form = Form::First;
		player[playerIndex].type = PlayerType::None;
		player[playerIndex].evolutionGauge = 0;
		player[playerIndex].evolutionGaugeRate = PLAYER_EVOLUTION_GAUGE_RATE;
		player[playerIndex].breakCount_Glass = 0;
		player[playerIndex].breakCount_Concrete = 0;
		player[playerIndex].breakCount_Plant = 0;
		player[playerIndex].breakCount_Electricity = 0;
		player[playerIndex].brokenHistory.clear();
		player[playerIndex].knockback_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[playerIndex].is_knocked_back = false;
		player[playerIndex].knockback_duration = 0.0f;
		player[playerIndex].isTypeFixed = false;
	}

	if (playerIndex == 0) player[0].position = XMFLOAT3(-4.0f, 4.0f, 0.0f);
	if (playerIndex == 1) player[1].position = XMFLOAT3(1.5f, 4.0f, 2.0f);
	if (playerIndex == 2) player[2].position = XMFLOAT3(-4.0f, 4.0f, -3.0f);
	if (playerIndex == 3) player[3].position = XMFLOAT3(4.0f, 4.0f, 1.0f);
}

inline void LoopRange(int& animFrame, int start, int count, int advance)
{
	int relative = (animFrame - start + advance) % count;
	if (relative < 0) relative += count;
	animFrame = start + relative;
}

//==================================
// 谿区ｩ滓緒逕ｻ
//==================================
void Player_DrawStock(int i)
{
	Shader_Begin();
	Shader_BeginUI();

	// HP繝舌・菴咲ｽｮ蜿門ｾ励・繧ｲ繝ｼ繧ｸ蠎ｧ讓呵ｨｭ螳・
	float bx = HPBar[i].pos.x - (60.0f * SCREEN_ADJUST_X);
	float by = HPBar[i].pos.y + (60.0f * SCREEN_ADJUST_Y);

	// 繝励Ξ繧､繝､繝ｼ縺斐→縺ｮ繧ｹ繝医ャ繧ｯ謠冗判
	for (int j = 0; j < player[i].stock; j++)
	{
		// 繧ｹ繝医ャ繧ｯ謠冗判螟画焚
		XMFLOAT2 pos = { bx + (j * 30.0f * SCREEN_ADJUST_X), by };	// 讓ｪ荳ｦ縺ｳ
		XMFLOAT2 size = { (260.0f * SCREEN_ADJUST_X), (260.0f * SCREEN_ADJUST_Y) };

		g_pContext->PSSetShaderResources(0, 1, &g_Texture[i + 14]);

		SetBlendState(BLENDSTATE_ALPHA);
		DrawSprite(pos, size, color::white);
	}
}

void Player_DrawText()
{
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (!player[p].active || !player[p].isOnScreen) continue;

		wchar_t playerLabel[8];
		swprintf_s(playerLabel, L"%dP", p + 1);

		// 繝励Ξ繧､繝､繝ｼ縺斐→縺ｫ濶ｲ險ｭ螳・
		TextColor textColor;
		switch (p)
		{
		case 0:
			textColor = TextColor::P1color;
			break;
		case 1:
			textColor = TextColor::P2color;
			break;
		case 2:
			textColor = TextColor::P3color;
			break;
		case 3:
			textColor = TextColor::P4color;
			break;
		default:
			textColor = TextColor::White;
			break;
		}

		// 繝輔か繝ｳ繝医し繧､繧ｺ縺ｮ蜊雁・遞句ｺｦ蟾ｦ縺ｫ縺壹ｉ縺・
		float offsetX = 15.0f;

		DrawTextEx(
			playerLabel,
			player[p].screenPos.x - offsetX,
			player[p].screenPos.y - 10.0f,	// 繝・く繧ｹ繝医・鬮倥＆蛻・ｸ翫↓陦ｨ遉ｺ
			40.0f,							// 繝輔か繝ｳ繝医し繧､繧ｺ
			L"Impact",
			textColor
		);
		//DrawTextEx(
		//	L"    笆ｽ ",
		//	player[p].screenPos.x - offsetX,
		//	player[p].screenPos.y + 13.0f,	// 繝・く繧ｹ繝医・鬮倥＆蛻・ｸ翫↓陦ｨ遉ｺ
		//	15.0f,							// 繝輔か繝ｳ繝医し繧､繧ｺ
		//	L"Impact",
		//	textColor
		//);
	}
}

static void Ranking(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;
	// 莠碁㍾逋ｻ骭ｲ髦ｲ豁｢
	if (player[playerIndex].rank != 0) return;

	// 豁ｻ莠｡鬆・↓霑ｽ蜉
	g_deathOrder.push_back(playerIndex);
	size_t pos = g_deathOrder.size();

	// 蜈医↓豁ｻ繧薙□繝励Ξ繧､繝､繝ｼ縺御ｽ朱・ｽ阪↓縺ｪ繧具ｼ・os=1 -> 4菴搾ｼ・
	player[playerIndex].rank = PLAYER_MAX - (int)(pos - 1);

	// 譛蠕後・荳莠ｺ縺檎｢ｺ螳壹＠縺溘ｉ谿九ｊ繧・菴阪↓縺吶ｋ
	if (g_deathOrder.size() == (size_t)(PLAYER_MAX - 1))
	{
		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			if (player[p].rank == 0)
			{
				player[p].rank = 1;
				break;
			}
		}

		// 蜍晁・｢ｺ螳・竊・SCENE_WIN 縺ｸ驕ｷ遘ｻ
		if (GetFadeState() == FADE_NONE)
		{
			XMFLOAT4 color(0.0f, 0.0f, 0.0f, 0.0f);
			SetFade(60, color, FADE_OUT, SCENE_WIN);
		}
	}
}

PLAYEROBJECT* GetPlayer(int playerIndex)
{
	// 遽・峇繝√ぉ繝・け 0 1 2 3 莉･螟悶↑繧・nullptr 繧定ｿ斐☆
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX)	return nullptr;

	return &player[playerIndex];
}

void TriggerbyHPShake(int playerIndex, float amplitude, float duration, float speed)
{
	// 遽・峇繝√ぉ繝・け
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;


	SetHPShake(&HPBar[playerIndex], amplitude, duration, speed, playerIndex + 6);

}


bool Player_CanUseSpecial(int playerIndex)
{
	// 遽・峇繝√ぉ繝・け
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return false;

	PLAYEROBJECT& pl = player[playerIndex];

	if (!pl.active)		return false;
	if (pl.isStunning)	return false;
	if (pl.isDown)		return false;
	if (pl.rank == 1)	return false;

	// 蠖｢諷九′隨ｬ3蠖｢諷九〒縺ゅｋ縺薙→
	if (pl.form != Form::Third) return false;

	// 繧ｿ繧､繝励′譛ｪ險ｭ螳壹□縺ｨ繧ｹ繝壹す繝｣繝ｫ縺後↑縺・°繧峨ち繧､繝励ｂ繝√ぉ繝・け
	if (pl.type == PlayerType::None) return false;

	// 縺吶∋縺ｦ騾壹▲縺溘ｉtrue
	return true;
}
