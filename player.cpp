// =====================================================
//	player.cpp
// 
//	§ìÒF•½‰ªéD”n			“ú•tF2026/01/27
//======================================================
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
#include <cstring> // ’Ç‰ÁFstrcmp ‚Ì‚½‚ß
#include "loadThread.h"


//======================================================
//	ƒ}ƒNƒ’è‹`
//======================================================
#define GAUGE_POS_X	(69.0f * (SCREEN_WIDTH / 1280.0f))	
#define GAUGE_POS_Y	(8.0f *  (SCREEN_HEIGHT / 720.0f))	
#define	HPBER_SIZE_X (270.0f * (SCREEN_WIDTH / 1280.0f))
#define	HPBER_SIZE_Y (270.0f * (SCREEN_HEIGHT / 720.0f))

//======================================================
//	ƒOƒ[ƒoƒ‹•Ï”
//======================================================
// ƒIƒuƒWƒFƒNƒg
PLAYEROBJECT player[PLAYER_MAX];

static ID3D11Device* g_pDevice = NULL;
static ID3D11DeviceContext* g_pContext = NULL;
static hp HPBar[PLAYER_MAX];

// ’¸“_ƒoƒbƒtƒ@
static ID3D11Buffer* g_VertexBuffer = NULL;

// ƒCƒ“ƒfƒbƒNƒXƒoƒbƒtƒ@
static ID3D11Buffer* g_IndexBuffer = NULL;

// ƒeƒNƒXƒ`ƒƒ•Ï”
static ID3D11ShaderResourceView* g_Texture[18];

// ƒvƒŒƒCƒ„[ ƒAƒjƒ[ƒVƒ‡ƒ“—p•Ï”
static const float ANIM_FRAME_TIME = 0.15f;	// 1ƒtƒŒ[ƒ€‚ ‚½‚è‚Ì•b”
static const int   SHEET_COLS = 16;
static const int   SHEET_ROWS = 16;

static int g_victoryState[PLAYER_MAX] = { 0 };			// 0 = ‚È‚µ, 1 = ‰‰ñ Ä¶’†, 2 = ƒ‹[ƒv
static float g_downHoldTimer[PLAYER_MAX] = { 0.0f };	// ÅIƒtƒŒ[ƒ€ƒz[ƒ‹ƒh—pƒ^ƒCƒ}[iƒvƒŒƒCƒ„[–ˆj

static bool g_skillAnimStarted[PLAYER_MAX] = { false, false, false, false };
static int g_skillAnimStart[PLAYER_MAX] = { 0 };	// ƒXƒLƒ‹ƒAƒjƒ[ƒVƒ‡ƒ“ŠJnƒtƒŒ[ƒ€•Û‘¶—p

static int g_specialAnimPhase[PLAYER_MAX] = { 0 };			// 0 = ‰‰ñÄ¶(0`6), 1 = ƒ‹[ƒv(4`6), 2 = I—¹‰‰o(7)
static float g_specialEndAnimTimer[PLAYER_MAX] = { 0.0f };	// I—¹ƒtƒŒ[ƒ€(7)‚Ì•\¦ƒ^ƒCƒ}[
static bool g_specialInitialize[PLAYER_MAX] = { false };

// ‡ˆÊE€–S‡‚ÌŠÇ—
static std::vector<int> g_deathOrder;	// €–S‚µ‚½ƒvƒŒƒCƒ„[‚ÌƒCƒ“ƒfƒbƒNƒXiæ‚É€‚ñ‚¾Ò‚ªæ“ªj

static int g_SE_ID[PLAYER_SE_COUNT] = { NULL };

// ’¸“_”z—ñ
static Vertex2 vdata[PLAYER_VERTEX] =
{
	{// ’¸“_0 LEFT-TOP
		XMFLOAT3(-COORDINATE, COORDINATE, 0.0f),	// À•W
		XMFLOAT3(0.0f, 0.0f, -1.0f),				// –@üƒxƒNƒgƒ‹
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),			// ƒJƒ‰[
		XMFLOAT2(0.0f, 0.0f)						// ƒeƒNƒXƒ`ƒƒÀ•W
	},
	{// ’¸“_1 RIGHT-TOP
		XMFLOAT3(COORDINATE, COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(TEXCOORD, 0.0f)
	},
	{// ’¸“_2 LEFT-BOTTOM
		XMFLOAT3(-COORDINATE, -COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f, TEXCOORD)
	},
	{// ’¸“_3 RIGHT-BOTTOM
		XMFLOAT3(COORDINATE, -COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(TEXCOORD, TEXCOORD)
	},
};

// ƒCƒ“ƒfƒbƒNƒX”z—ñ
static UINT idxdata[6]
{
	 0, 1, 2, 2, 1, 3, // -Z–Ê
};

static float top_y = 0;	// ˜ZŠpŒ`‚Ìtop-yÀ•[‚ÌƒfƒoƒbƒO•\¦

static std::atomic<int> g_loadedCount(0);                   // ‰½–‡I‚í‚Á‚½‚©ii’»—pj
static bool      s_ShowImgui = true;

//======================================================
//	‰Šú‰»ŠÖ”
//======================================================
void Player_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// ƒvƒŒƒCƒ„[•\¦‚Ì‰Šú‰»
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
		player[p].lastDir = PlayerDir::Down; // ³–Ê
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

	// ’¸“_ƒoƒbƒtƒ@ì¬
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));	// 0‚ÅƒNƒŠƒA
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * PLAYER_VERTEX;	// Ši”[‚Å‚«‚é’¸“_”*’¸“_ƒTƒCƒY
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	g_pDevice = pDevice;
	g_pContext = pContext;
	g_loadedCount = 0;

	// ƒ[ƒh‚ğ•ÊƒXƒŒƒbƒh‚ÅŠJn
	// pDevice‚ğ“n‚µAI—¹‚µ‚½‚çƒtƒ‰ƒO‚ğ—§‚Ä‚é
	Loader::AddTask([pDevice]()
	{
		LoadTextureList(pDevice);

	//// ===== GPU ƒeƒNƒXƒ`ƒƒ ƒEƒH[ƒ€ƒAƒbƒv =====
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

	// ƒCƒ“ƒfƒbƒNƒXƒoƒbƒtƒ@ì¬
	{
		D3D11_BUFFER_DESC	bd;
		ZeroMemory(&bd, sizeof(bd));	// 0‚ÅƒNƒŠƒA
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(UINT) * 6;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&bd, NULL, &g_IndexBuffer);

		// ƒCƒ“ƒfƒbƒNƒXƒoƒbƒtƒ@‚Ö‘‚«‚İ
		D3D11_MAPPED_SUBRESOURCE msr;
		pContext->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		UINT* index = (UINT*)msr.pData;

		// ƒCƒ“ƒfƒbƒNƒXƒf[ƒ^‚ğƒoƒbƒtƒ@‚ÖƒRƒs[
		CopyMemory(&index[0], &idxdata[0], sizeof(UINT) * 6);
		pContext->Unmap(g_IndexBuffer, 0);
	}
	// ƒfƒoƒbƒOƒŒƒ“ƒ_ƒ‰[‰Šú‰»
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

	// ƒAƒjƒ[ƒVƒ‡ƒ“‚Ì‰Šú‰»
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		player[p].animFrame = 0;
		player[p].animTimer = 0.0f;
		g_skillAnimStarted[p] = false;
	}

	// ‡ˆÊî•ñ‚ğ‰Šú‰»
	g_deathOrder.clear();

	// SE‚Ì‰Šú‰»
	g_SE_ID[0] = LoadAudio("asset\\Audio\\Roar_Form_Second.wav");	// i‰»Œã‚Ì™ôšK ‘æ2Œ`‘Ô
	g_SE_ID[1] = LoadAudio("asset\\Audio\\Roar_Form_Third.wav");	// i‰»Œã‚Ì™ôšK ‘æ3Œ`‘Ô
	g_SE_ID[2] = LoadAudio("asset\\Audio\\Transform.wav");			// •Ïg
	g_SE_ID[3] = LoadAudio("asset\\Audio\\EggBreaking.wav");		// —‘Š„‚ê‚é
}

static void LoadTextureList(ID3D11Device* pDevice)
{
	TexMetadata metadata;
	ScratchImage image;

	struct TexEntry { int idx; const wchar_t* path; };

	const TexEntry texList[] =
	{
		{  0, L"asset\\texture\\characterMiniRed_v2.png"},			// ‘æ1Œ`‘Ô P1 Ô
		{  1, L"asset\\texture\\characterMiniBlue_v1.png"},			// ‘æ1Œ`‘Ô P2 Â
		{  2, L"asset\\texture\\characterMiniYellow_v1.png"},		// ‘æ1Œ`‘Ô P3 ‰©
		{  3, L"asset\\texture\\characterMiniGreen_v1.png"},		// ‘æ1Œ`‘Ô P4 —Î
		{  4, L"asset\\texture\\characterMidGlass_v1.png"},			// ‘æ2Œ`‘Ô ƒKƒ‰ƒX
		{  5, L"asset\\texture\\characterMidConcrete_v1.png" },		// ‘æ2Œ`‘Ô ƒRƒ“ƒNƒŠ[ƒg
		{  6, L"asset\\texture\\characterMidTree_v1.png" },			// ‘æ2Œ`‘Ô A•¨
		{  7, L"asset\\texture\\characterMidElectricity_v1.png" },	// ‘æ2Œ`‘Ô “d‹C
		{  8, L"asset\\texture\\characterBigGlass_v2.png" },		// ‘æ3Œ`‘Ô ƒKƒ‰ƒX
		{  9, L"asset\\texture\\characterBigConcrete_v2.png" },		// ‘æ3Œ`‘Ô ƒRƒ“ƒNƒŠ[ƒg
		{ 10, L"asset\\texture\\characterBigTree_v2.png" },			// ‘æ3Œ`‘Ô A•¨
		{ 11, L"asset\\texture\\characterBigElectricity_v2.png" },	// ‘æ3Œ`‘Ô “d‹C
		{ 12, L"asset\\texture\\uiCharacterSkill_v2.png" },			// ‘æ2Œ`‘Ô ‘æ3Œ`‘Ô ƒXƒLƒ‹
		{ 13, L"asset\\texture\\characterBigSP_v4.png" },			// ‘æ3Œ`‘Ô ƒXƒyƒVƒƒƒ‹
		{ 14, L"asset\\texture\\uiStockRed_v4.png"},				// UI ƒXƒgƒbƒN Ô
		{ 15, L"asset\\texture\\uiStockBlue_v4.png"},				// UI ƒXƒgƒbƒN Â
		{ 16, L"asset\\texture\\uiStockYellow_v4.png" },			// UI ƒXƒgƒbƒN ‰©
		{ 17, L"asset\\texture\\uiStockGreen_v4.png" },				// UI ƒXƒgƒbƒN —Î
	};

	for (const auto& e : texList)
	{
		auto start = std::chrono::high_resolution_clock::now();

		// ƒRƒƒ“ƒg‰»‚µ‚Ä‚¢‚é—v‘f‚Í”z—ñƒGƒ“ƒgƒŠ©‘Ì‚ğƒRƒƒ“ƒgƒAƒEƒg‚µ‚Ä‚¢‚é‚½‚ß‚±‚±‚É‚Í—ˆ‚È‚¢B
		HRESULT hr = LoadFromWICFile(e.path, WIC_FLAGS_NONE, &metadata, image);
		if (SUCCEEDED(hr))
		{
			if (FAILED(CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[e.idx])))
			{
				// ì¬¸”s‚Í nullptr ‚ğ‘ã“ü‚µ‚Ä‘±s
				g_Texture[e.idx] = nullptr;
			}
			g_loadedCount++;

		}
		// “Ç‚İ‚İ¸”s‚Í nullptr ‚ğ‘ã“ü‚µ‚Ä‘±s
		else	g_Texture[e.idx] = nullptr;

		auto end = std::chrono::high_resolution_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		// std::wstring ‚ğ std::string ‚É•ÏŠ·‚µ‚Äo—Í
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		hal::dout << "ƒeƒNƒXƒ`ƒƒƒ[ƒh: " << conv.to_bytes(e.path) << " " << ms << " ms" << std::endl;
	}
}

void Player_Warmup()
{
	if (!g_pContext) return;

	// ===== GPU ƒeƒNƒXƒ`ƒƒ ƒEƒH[ƒ€ƒAƒbƒv =====
	const size_t TEX_COUNT = sizeof(g_Texture) / sizeof(g_Texture[0]);
	for (size_t i = 0; i < TEX_COUNT; ++i)
	{
		if (g_Texture[i] != nullptr)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[i]);
			g_pContext->DrawIndexed(0, 0, 0); 
		}
	}

	// ÅŒã‚ÉƒŠƒZƒbƒg‚µ‚Ä‚¨‚­
	ID3D11ShaderResourceView* nullSRV = nullptr;
	g_pContext->PSSetShaderResources(0, 1, &nullSRV);
}

//======================================================
//	I—¹ˆ—ŠÖ”
//======================================================
void Player_Finalize()
{
	// ƒVƒF[ƒ_[‚ÉƒoƒCƒ“ƒh‚³‚ê‚Ä‚¢‚é SRV ‚ğƒAƒ“ƒoƒCƒ“ƒhiˆÀ‘S‚Ì‚½‚ß‘S—v‘f•ªj
	const size_t TEX_COUNT = sizeof(g_Texture) / sizeof(g_Texture[0]);
	if (g_pContext)
	{
		// ŒÅ’è’·”z—ñ‚ğg‚Á‚ÄŠmÀ‚É nullptr ‚ğ“n‚·iAPI ‚Í¶”z—ñ‚ğ—v‹j
		ID3D11ShaderResourceView* nullSRV[25] = {};
		g_pContext->PSSetShaderResources(0, static_cast<UINT>(TEX_COUNT), nullSRV);
	}

	// ƒCƒ“ƒfƒbƒNƒX^’¸“_ƒoƒbƒtƒ@‚Ì‰ğ•úiNULL ƒ`ƒFƒbƒNŒã‚É nullptr ‚Éİ’èj
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

	// ƒeƒNƒXƒ`ƒƒ”z—ñ‘S—v‘f‚ğˆÀ‘S‚É‰ğ•úiƒRƒƒ“ƒg‰»‚µ‚Ä–¢ƒ[ƒh‚Ì—v‘f‚à nullptr ƒ`ƒFƒbƒN‚ÅˆÀ‘Sj
	for (size_t i = 0; i < TEX_COUNT; ++i)
	{
		if (g_Texture[i] != nullptr)
		{
			g_Texture[i]->Release();
			g_Texture[i] = nullptr;
		}
	}

	// ƒfƒoƒCƒX^ƒRƒ“ƒeƒLƒXƒg‚ÍŠO•”ŠÇ—‚Ì‚½‚ß‰ğ•ú‚µ‚È‚¢‚ªAQÆ‚ÍƒNƒŠƒA‚µ‚Ä‚¨‚­
	g_pContext = nullptr;
	g_pDevice = nullptr;

	// ƒfƒoƒbƒOƒŒƒ“ƒ_ƒ‰[‚ÌI—¹ˆ—
	Debug_Finalize();

	for (int i = 0; i < PLAYER_SE_COUNT; ++i)	UnloadAudio(g_SE_ID[i]);
}

// ======================================================
// ˆÚ“®ŠÖ”i—v•ÏXj
// ------------------------------------------------------
// ˆÚ“®ƒxƒNƒgƒ‹‚ÆŒü‚¢‚Ä‚¢‚é•ûŒüƒxƒNƒgƒ‹‚Í•Ê‚Å‚Á‚½•û‚ª‚¢‚¢
// ======================================================
// “ü—Í(ƒ[ƒJƒ‹)‚ğƒJƒƒ‰Šî€‚Åƒ[ƒ‹ƒhXZ‚Ö•ÏŠ·‚·‚éi•½–ÊˆÚ“®—pj
static inline XMFLOAT3 ToWorldMoveDirByCamera(const XMFLOAT2& input)
{
	// input.x: ‰E(+), input.y: ã(+)
	XMMATRIX view = GetViewMatrix();
	XMMATRIX invView = XMMatrixInverse(nullptr, view);

	// invView ‚Ìs‚©‚çƒJƒƒ‰²‚ğæ“¾iworldj
	XMFLOAT3 right = XMFLOAT3(invView.r[0].m128_f32[0], invView.r[0].m128_f32[1], invView.r[0].m128_f32[2]);
	XMFLOAT3 forward = XMFLOAT3(invView.r[2].m128_f32[0], invView.r[2].m128_f32[1], invView.r[2].m128_f32[2]);

	// XZ•½–Ê‚ÖË‰eiY¬•ª‚ğÌ‚Ä‚éj
	right.y = 0.0f;
	forward.y = 0.0f;

	// ³‹K‰»iƒJƒƒ‰‚ª^ã‚É‹ß‚¢“™‚Åƒ[ƒŠ„‚è‚ğ”ğ‚¯‚éj
	{
		float rl = sqrtf(right.x * right.x + right.z * right.z);
		if (rl > 0.0001f) { right.x /= rl; right.z /= rl; }
	}
	{
		float fl = sqrtf(forward.x * forward.x + forward.z * forward.z);
		if (fl > 0.0001f) { forward.x /= fl; forward.z /= fl; }
	}

	// ƒ[ƒJƒ‹“ü—Í‚ğƒ[ƒ‹ƒh‚Ö‡¬
	XMFLOAT3 worldDir;
	worldDir.x = right.x * input.x + forward.x * input.y;
	worldDir.y = 0.0f;
	worldDir.z = right.z * input.x + forward.z * input.y;
	return worldDir;
}


void Move(PLAYEROBJECT& player, XMFLOAT3 moveDir)
{
	// i‚İ‚½‚¢•ûŒüi3•½•ûj
	float length = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);

	if (length > 0.0f)
	{
		// ƒxƒNƒgƒ‹‚Ì³‹K‰»
		moveDir.x /= length;
		moveDir.z /= length;

		// –Ú•WŠp“x‚ğ‹‚ß‚é
		float targetAngle = atan2f(moveDir.x, moveDir.z);	// ƒxƒNƒgƒ‹‚ÌŠp“x
		targetAngle = XMConvertToDegrees(targetAngle);		// ƒ‰ƒWƒAƒ“ -> “x

		// ·•ª‚ğ’²®i180“x’´‚¦‚È‚¢‚æ‚¤‚Éj
		float diff = targetAngle - player.moveAngle;	// Šp“x·
		if (diff > 180.0f) diff -= 360.0f;
		if (diff < -180.0f) diff += 360.0f;

		static float angSpeed = 0.9f;

		// ƒXƒ€[ƒY‚É•âŠÔi0.1f‚ª•âŠÔƒXƒs[ƒhj
		player.moveAngle += diff * angSpeed;

		player.rotation.y = player.moveAngle;	// Šp“x‚Ì”½‰f

		// ‘Oi
		float rad = XMConvertToRadians(player.moveAngle);

		player.position.x += sinf(rad) * player.speed;
		player.position.z += cosf(rad) * player.speed;
	}
}

//======================================================
// XVŠÖ”
//======================================================
void Player_Update()
{
	// ŠeƒvƒŒƒCƒ„[‚É‘Î‰‚·‚é”­“®ƒL[
	const Keyboard_Keys_tag attackKeys[PLAYER_MAX] = { KK_SPACE, KK_ENTER, KK_V, KK_NUMPAD0 };

	const Keyboard_Keys_tag specialKeys[PLAYER_MAX] = { KK_D7, KK_D8, KK_D9, KK_D0 };

	if (Keyboard_IsKeyDownTrigger(KK_TAB))	s_ShowImgui = !s_ShowImgui;

	if (s_ShowImgui)
	{
		// ƒfƒoƒbƒO—p ImGui ƒEƒBƒ“ƒhƒE
		ImGui::Begin("Player Debug");

		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			// ƒvƒŒƒCƒ„[‚²‚Æ‚É ID ‚ğ•ª‚¯‚éi“¯ˆêƒ‰ƒxƒ‹Õ“Ë‰ñ”ğj
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

			// —š—ğƒŠƒXƒg‚ÌƒTƒCƒY‚ğ•\¦
			size_t historySize = player[p].brokenHistory.size();
			ImGui::BulletText("brokenHistory Size : %zu", historySize);

			if (historySize > 0)
			{
				ImGui::Indent(); // —š—ğ‚ğ‚³‚ç‚Éˆê’iƒCƒ“ƒfƒ“ƒg
				ImGui::Text("History (Latest -> Oldest):");

				// —š—ğ‚ğÅVi––”öj‚©‚çŒÃ‚¢•û‚Öƒ‹[ƒv‚µ‚Ä•\¦
				for (int i = (int)historySize - 1; i >= 0; --i)
				{
					// BuildingType ‚Í enumŒ^i®”’lj‚È‚Ì‚ÅA‚»‚Ì‚Ü‚Ü %d ‚Å•\¦‰Â”\
					// ‚Ü‚½‚ÍAImGui::Text‚Å®Œ`‚µ‚Ä•\¦‚·‚é

					// —á1: —š—ğ‚ÌƒCƒ“ƒfƒbƒNƒX‚Æ’l‚ğ’¼Ú•\¦
					// ImGui::BulletText("[%d]: %d", p, (int)object[p].brokenHistory[p]);

					// —á2: —š—ğ‚Ì’l‚ğ‰¡‚É•À‚×‚Ä•\¦
					ImGui::SameLine(); // “¯‚¶s‚É•\¦
					// —š—ğ‚Ì’li®”j‚ğ•¶š—ñ‚É•ÏŠ·‚µ‚Ä‚©‚ç•\¦
					ImGui::Text("%d", (int)player[p].brokenHistory[i]);
				}

				// —š—ğ‚ª‰¡‚É•À‚Ñ‚·‚¬‚È‚¢‚æ‚¤‰üs
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

		// ƒ[ƒ‹ƒhÀ•W‚ğƒXƒNƒŠ[ƒ“À•W‚É•ÏŠ·
		XMFLOAT3 worldPos = player[p].position;
		worldPos.y += 2.0f; // ƒvƒŒƒCƒ„[‚Ìã•û‚É•\¦

		XMVECTOR posVec = XMLoadFloat3(&worldPos);
		XMMATRIX view = GetViewMatrix();
		XMMATRIX proj = GetProjectionMatrix();
		XMMATRIX viewProj = view * proj;

		// ƒrƒ…[ƒ|[ƒg•ÏŠ·
		XMVECTOR screenPos = XMVector3Project
		(
			posVec,
			0.0f, 0.0f,
			SCREEN_WIDTH, SCREEN_HEIGHT,
			0.0f, 1.0f,
			proj, view,
			XMMatrixIdentity()
		);

		// Z’lƒ`ƒFƒbƒNiƒJƒƒ‰‚ÌŒã‚ë‚È‚ç•`‰æ‚µ‚È‚¢j
		float screenZ = XMVectorGetZ(screenPos);
		if (screenZ > 0.0f && screenZ < 1.0f)
		{
			float screenX = XMVectorGetX(screenPos);
			float screenY = XMVectorGetY(screenPos);

			// ƒeƒLƒXƒg•`‰æiUpdate“à‚Å‚ÍŒÄ‚Ño‚³‚È‚¢ADraw“à‚Å•`‰æ‚·‚éj
			// ‚±‚±‚Å‚ÍÀ•W‚ğ•Û‘¶‚µ‚Ä‚¨‚­
			player[p].screenPos = XMFLOAT2(screenX, screenY);
			player[p].isOnScreen = true;
		}
		else	player[p].isOnScreen = false;

		// -------------------------------------------------------------
		// •Ïg
		// -------------------------------------------------------------
		switch (player[p].form)
		{
		case Form::First:	// ‘æ1Œ`‘Ô
			player[p].scaling.x = 0.5f;
			player[p].scaling.y = 0.5f;
			player[p].scaling.z = 0.5f;
			player[p].attack = 10.0f;
			player[p].power = 0.3f;
			player[p].weight = 0.5f;
			player[p].speed = 0.07f;
			player[p].isTypeFixed = false;	// ƒXƒLƒ‹ƒN[ƒ‹ƒ^ƒCƒ€UI‚Ì•\¦‚Ég—p
			break;

		case Form::Second:	// ‘æ2Œ`‘Ô
			player[p].scaling.x = 0.8f;
			player[p].scaling.y = 0.8f;
			player[p].scaling.z = 0.8f;
			player[p].attack = 15.0f;
			player[p].power = 0.4f;
			player[p].weight = 0.6f;
			player[p].speed = 0.06f;
			player[p].isTypeFixed = true;
			break;

		case Form::Third:	// ‘æ3Œ`‘Ô
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

		// ‰ñ•œƒtƒ‰ƒO‚ÌXV
		if (player[p].isHealing)
		{
			player[p].healingTimer += DELTA_TIME;	// ‰ñ•œƒ^ƒCƒ}[‚ğXV

			if (player[p].healingTimer >= HEALING_TIME)
			{
				player[p].isHealing = false;	// ‰ñ•œI—¹
				player[p].healingTimer = 0.0f;	// ƒ^ƒCƒ}[ƒŠƒZƒbƒg
			}
		}

		// i‰»ƒtƒ‰ƒO‚ÌXV
		if (player[p].isEvolving)
		{
			player[p].evolvingTimer += DELTA_TIME;	// i‰»ƒ^ƒCƒ}[‚ğXV

			if (player[p].evolvingTimer >= EVOLVING_TIME)
			{
				player[p].isEvolving = false;	// i‰»I—¹
				player[p].evolvingTimer = 0.0f;	// ƒ^ƒCƒ}[ƒŠƒZƒbƒg
			}
		}

		// è²E€é–»E¹è ï½¦ç¸ºE®è²‚å¸›ï½°ãƒ»
		player[p].satiety -= DELTA_TIME;
		if (player[p].satiety < 0.0f)	player[p].satiety = 0.0f;
		//// –• “x‚ª1–¢–‚È‚çHP‚ğŒ¸­‚³‚¹‚é
		//if (player[p].satiety < 1.0f)	player[p].hp -= 0.05f;

		// ƒŠƒXƒ|[ƒ“ˆ—
		if (player[p].duringRespawn)
		{
			if (GetGamePhase() == PHASE_PLAY)
			{
				player[p].respawnTimer += DELTA_TIME;


				// YÀ•W‚ğ4‚ÉŒÅ’è
				player[p].position.y = 4.0f;

				// UŒ‚ƒ{ƒ^ƒ“‰Ÿ‰º‚Ü‚½‚Í5•bŒo‰ß‚Å—‰ºŠJn
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
			// y²‚ÌˆÚ“®—Ê (d—Í + ƒWƒƒƒ“ƒv)
			// d—Í‰Á‘¬“x‚Ì‚È‚¢ŠÈˆÕ“I‚Èd—Í
			player[p].position.y += -0.1f;
		}

		// —‘ƒGƒtƒFƒNƒg‚ªŠ„‚ê‚éŠÔ
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

		// “Åó‘Ô‚Ìˆ—
		if (player[p].poisonTimer > 0.0f)
		{
			// –³“G’†‚Íƒ_ƒ[ƒW‚ğ—^‚¦‚È‚¢‚ªA‚±‚±‚Åƒ‹[ƒv‚ğ”²‚¯‚È‚¢iˆÈ~‚Ì•¨—E“–‚½‚è”»’è‚ÍÀs‚·‚éj
			if (!player[p].isInvincible)
			{
				// “Åó‘Ô‚ÌŠÔAƒ_ƒ[ƒW‚ğ—^‚¦‚é
				player[p].hp -= SPECIAL_PLANT_DAMAGE * player[p].defense;
			}

			// “Åƒ^ƒCƒ}[‚ği‚ß‚é
			player[p].poisonTimer -= DELTA_TIME;

			// “Åƒ^ƒCƒ}[‚ª0‚É‚È‚Á‚½‚ç“Åó‘Ô‚ğ‰ğœ
			if (player[p].poisonTimer <= 0.0f)
			{
				player[p].isPoisoned = false;
				player[p].poisonTimer = 0.0f;
			}
		}

		// ƒXƒ^ƒ“ƒQ[ƒW‚ªÅ‘å‚ÅƒXƒ^ƒ“ƒtƒ‰ƒO‚ğ—§‚Ä‚é
		if (player[p].stunGauge >= STUNGAUGE_MAX)
		{
			player[p].isStunning = true;
			player[p].stunGauge = STUNGAUGE_MAX;
		}
		// ƒXƒ^ƒ“’†‚Ìˆ—
		if (player[p].isStunning)
		{
			// ƒXƒ^ƒ“ƒ^ƒCƒ}[‚ği‚ß‚é
			player[p].stunTimer += DELTA_TIME;

			// ŠÔŒo‰ß‚ÅƒXƒ^ƒ“‰ğœ
			if (player[p].stunTimer >= STUN_TIME)
			{
				player[p].isStunning = false;	// ƒXƒ^ƒ“‰ğœ
				player[p].stunTimer = 0.0f;		// ƒXƒ^ƒ“ƒ^ƒCƒ}[ƒŠƒZƒbƒg
				player[p].stunGauge = 0.0f;		// ƒXƒ^ƒ“ƒQ[ƒWƒŠƒZƒbƒg
			}

			// ƒXƒ^ƒ“’†‚ÍˆÚ“®ƒxƒNƒgƒ‹‚ğŠ®‘S‚Éƒ[ƒ‚É‚·‚é
			player[p].moveDir = { 0.0f, 0.0f, 0.0f };

			player[p].isMoving = false;

			player[p].isAttacking = false;
			player[p].useSkill = false;
			player[p].useSpecial = false;
		}
		else // ƒXƒ^ƒ“‚µ‚Ä‚¢‚È‚¢ê‡‚Ìˆ—
		{
			// ƒXƒ^ƒ“‚µ‚Ä‚¢‚È‚¢ŠÔ‚ÍƒXƒ^ƒ“ƒQ[ƒW‚ğŒ¸­‚³‚¹‚é
			player[p].stunGauge -= DELTA_TIME;

			// ƒXƒ^ƒ“ƒQ[ƒW‚ª0–¢–‚É‚È‚ç‚È‚¢‚æ‚¤‚ÉƒNƒ‰ƒ“ƒv
			if (player[p].stunGauge < 0.0f)	player[p].stunGauge = 0.0f;
		}

		// ƒXƒ^ƒ“’†Eƒ_ƒEƒ“’†‚Å‚È‚¯‚ê‚Î‘æ1Œ`‘Ôs“® 1ˆÊŠm’èŒã‚ÍƒAƒjƒ[ƒVƒ‡ƒ“‚Ì‚İ
		if (!player[p].isStunning && !player[p].isDown && player[p].rank != 1 && player[p].active)
		{
			if (GetGamePhase() == PHASE_PLAY)
			{
				// ”­“®ƒgƒŠƒK[“ü—Í‚ğƒ`ƒFƒbƒN‚µ‚ÄUŒ‚ƒtƒ‰ƒO‚ğ—§‚Ä‚é
				if (Keyboard_IsKeyDownTrigger(attackKeys[p]))
				{
					player[p].isAttacking = true;

					// ‘æ2E‘æ3Œ`‘Ô‚Ìê‡AƒXƒLƒ‹g—pƒtƒ‰ƒO‚à—§‚Ä‚é
					if (player[p].type != PlayerType::None)	player[p].useSkill = true;
				}
				if (g_Input[p].A)	player[p].isAttacking = true;

				// ‘æ2E‘æ3Œ`‘Ô‚Ìê‡ƒXƒLƒ‹g—pƒtƒ‰ƒO—§‚Ä‚é
				if (g_Input[p].X)	if (player[p].type != PlayerType::None)	player[p].useSkill = true;

				// ”­“®ƒgƒŠƒK[“ü—Í‚ğƒ`ƒFƒbƒN‚µ‚ÄƒXƒyƒVƒƒƒ‹g—pƒtƒ‰ƒO‚ğ—§‚Ä‚é
				if (player[p].form == Form::Third && Keyboard_IsKeyDownTrigger(specialKeys[p]))	player[p].useSpecial = true;

				// ƒ{ƒ^ƒ““ü—Í‚ğƒ`ƒFƒbƒN‚µ‚ÄƒXƒyƒVƒƒƒ‹g—pƒtƒ‰ƒO‚ğ—§‚Ä‚é
				if (player[p].form == Form::Third && g_Input[p].ZR)	player[p].useSpecial = true;

				// ƒtƒ‰ƒO‚ª—§‚Á‚½‚çXVˆ—‚ğŒÄ‚Ño‚·
				if (player[p].isAttacking)	Attack_Update(p);	// UŒ‚
				if (player[p].useSkill)		Skill_Update(p);	// ƒXƒLƒ‹
				if (player[p].useSpecial)	Special_Update(p);	// ƒXƒyƒVƒƒƒ‹

				// Œ»İ‚ÌƒvƒŒƒCƒ„[ p ‚ÌˆÚ“®ƒxƒNƒgƒ‹‚¾‚¯‚ğƒŠƒZƒbƒg
				player[p].moveDir = { 0.0f, 0.0f, 0.0f };

				XMFLOAT2 moveInput = { 0.0f, 0.0f };

				// ƒXƒyƒVƒƒƒ‹ ƒRƒ“ƒNƒŠ[ƒgg—p’†‚ÍˆÚ“®•s‰Â
				if (player[p].useSpecial && player[p].type == PlayerType::Concrete)
				{
					player[p].moveDir = { 0.0f, 0.0f, 0.0f };
					player[p].isMoving = false;
				}
				// ƒXƒyƒVƒƒƒ‹ ƒRƒ“ƒNƒŠ[ƒgg—p’†‚Å‚È‚¯‚ê‚ÎˆÚ“®ˆ—
				else
				{
					player[p].moveInput2D = { 0.0f, 0.0f };

					if (p == 0) // ƒvƒŒƒCƒ„[0 (WASD) UŒ‚ Space
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
					else if (p == 1) // ƒvƒŒƒCƒ„[1 (–îˆóƒL[) UŒ‚ Enter
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
					else if (p == 2) // ƒvƒŒƒCƒ„[2 (TFGH) UŒ‚ V
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
					if (p == 3) // ƒvƒŒƒCƒ„[3 (WASD) UŒ‚ Space
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

					// ˆÚ“®‚ÍƒJƒƒ‰Šî€‚ğƒ[ƒ‹ƒh‚É‚·‚é
					player[p].moveDir = ToWorldMoveDirByCamera(moveInput);
				}
			}

			// Œ»İ‚ÌƒvƒŒƒCƒ„[ p ‚¾‚¯‚ğ“®‚©‚·
			Move(player[p], player[p].moveDir);

			// ˆÚ“®’†‚È‚ç lastDir ‚ğXV
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

		// ƒvƒŒƒCƒ„[‚²‚Æ‚ÌƒXƒLƒ‹ƒN[ƒ‹ƒ^ƒCƒ€‚ğ–ˆƒtƒŒ[ƒ€Œ¸Z
		if (player[p].skillCoolTimer > 0.0f)
		{
			player[p].skillCoolTimer -= DELTA_TIME;
			if (player[p].skillCoolTimer < 0.0f) player[p].skillCoolTimer = 0.0f;
		}

		// HP‚ª0ˆÈ‰º‚Ìˆ—
		if (player[p].hp <= 0.0f && player[p].active && !player[p].isDown)
		{
			// ƒ_ƒEƒ“ó‘Ô‚ÉˆÚs‚µ‚Äƒ^ƒCƒ}[‚ğƒŠƒZƒbƒg
			player[p].isDown = true;
			player[p].downTimer = 0.0f;
			Effect_ClearUI(p);
		}

		// ƒ_ƒEƒ“ó‘Ô‚Ìƒ^ƒCƒ}[XV‚ÆƒŠƒXƒ|[ƒ“”»’è
		if (player[p].isDown)
		{
			// s“®’â~
			player[p].moveDir = { 0.0f, 0.0f, 0.0f };
			player[p].isAttacking = false;
			player[p].useSkill = false;
			player[p].useSpecial = false;

			// ƒ_ƒEƒ“ƒ^ƒCƒ}[XV
			player[p].downTimer += DELTA_TIME;

			// ƒvƒŒƒCƒ„[–ˆ‚Ìƒ_ƒEƒ“ŠÔ‚ªŒo‰ß‚µ‚½‚çƒŠƒXƒ|[ƒ“ˆ—
			if (player[p].downTimer >= DOWN_TIME)
			{
				// c‹@‚ğ1‚ÂŒ¸‚ç‚·
				player[p].stock -= 1;

				if (player[p].stock > 0)	Player_Respawn(p);
				else
				{
					// c‹@–³‚µ‚Å•œŠˆ‚È‚µ
					player[p].active = false;
					player[p].isDown = false;
					player[p].downTimer = 0.0f;

					// ‡ˆÊ“o˜^i“à•”‚Åd•¡“o˜^‚ğ–h~j
					Ranking(p);
				}
			}
		}

		// —‰ºˆ— ‰eƒGƒtƒFƒNƒg”ñ•\¦
		if (player[p].position.y < -1.0f)
		{
			player[p].isShadowEnabled = false;
		}

		if (player[p].active && player[p].position.y <= -10.0f)
		{
			Effect_ClearUI(p);
			// c‹@‚ğˆê‚ÂŒ¸‚ç‚·
			player[p].stock -= 1;

			// ƒŠƒXƒ|[ƒ“iˆÊ’uEƒXƒe[ƒgƒŠƒZƒbƒgj
			if (player[p].stock > 0)	Player_Respawn(p);
			else
			{
				// c‹@–³‚µ‚ÅŠ®‘S‚É”ñƒAƒNƒeƒBƒu‰»
				player[p].active = false;

				// ‡ˆÊ“o˜^
				Ranking(p);
				player[p].position.y = 0.0f;
			}
		}

		// ƒ_ƒ[ƒW‚ğó‚¯‚½‚Ìˆ—
		if (player[p].isAttacked)
		{
			// ƒ_ƒ[ƒWƒ^ƒCƒ}[XV
			player[p].attackedTimer += DELTA_TIME;

			// ƒvƒŒƒCƒ„[–ˆ‚Ìƒ_ƒ[ƒWŠÔ‚ªŒo‰ß‚µ‚½‚çƒ_ƒ[ƒWI—¹
			if (player[p].attackedTimer >= ATTACKED_TIME)
			{
				player[p].isAttacked = false;
				player[p].attackedTimer = 0.0f;
			}
		}
		// ƒ_ƒ[ƒWF‚¾‚¯‚Ìˆ—
		if (player[p].isDamageColor)
		{
			player[p].damageColorTimer += DELTA_TIME;

			if (player[p].damageColorTimer >= ATTACKED_TIME)
			{
				player[p].isDamageColor = false;
				player[p].damageColorTimer = 0.0f;
			}
		}

		// ƒ_ƒ[ƒWF‚¾‚¯‚Ìˆ—
		if (player[p].isDamageColor)
		{
			player[p].damageColorTimer += DELTA_TIME;

			if (player[p].damageColorTimer >= ATTACKED_TIME)
			{
				player[p].isDamageColor = false;
				player[p].damageColorTimer = 0.0f;
			}
		}

		// i‰»‚Ì–³“Gˆ—
		if (player[p].isInvincible)
		{
			// –³“Gƒ^ƒCƒ}[XV
			player[p].invincibleTimer += DELTA_TIME;

			// ƒvƒŒƒCƒ„[–ˆ‚Ì–³“GŠÔ‚ªŒo‰ß‚µ‚½‚ç–³“GI—¹
			if (player[p].invincibleTimer >= EVOLVING_TIME)
			{
				player[p].isInvincible = false;
				player[p].invincibleTimer = 0.0f;

				// i‰»‚Ì™ôšKSEÄ¶
					 if (player[p].form == Form::Second)PlayAudio(g_SE_ID[0], false);	// ™ôšK ‘æ2Œ`‘Ô
				else if (player[p].form == Form::Third)	PlayAudio(g_SE_ID[1], false);	// ™ôšK ‘æ3Œ`‘Ô
			}
		}

		// ==========================================================
		// ƒvƒŒƒCƒ„[ ƒAƒjƒ[ƒVƒ‡ƒ“XV
		// ==========================================================
		
		// ƒXƒLƒ‹ŠJn‚ÌƒtƒŒ[ƒ€‰Šú‰»iƒAƒjƒ[ƒVƒ‡ƒ“XVƒ^ƒCƒ~ƒ“ƒO‚ÉˆË‘¶‚µ‚È‚¢j
		if (player[p].skillAnimation && !g_skillAnimStarted[p])
		{
			// ‘®«‚²‚Æ‚ÌŠî€ƒIƒtƒZƒbƒgi‘®«1‚Â‚ ‚½‚è32ƒRƒ}j
			int typeBase = 0;
				 if (player[p].type == PlayerType::Concrete)	typeBase = 0;
			else if (player[p].type == PlayerType::Electricity)	typeBase = 32;
			else if (player[p].type == PlayerType::Glass)		typeBase = 64;
			else if (player[p].type == PlayerType::Plant)		typeBase = 96;

			// Œ`‘ÔƒIƒtƒZƒbƒgi‘æ2Œ`‘Ô: 0A‘æ3Œ`‘Ô: 128j
			int formBase = 0;
			if (player[p].form == Form::Third) formBase = 128;

			// •ûŒüƒIƒtƒZƒbƒgi1•ûŒü‚ ‚½‚è4ƒRƒ}j
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
		// ƒXƒLƒ‹I—¹‚Ìƒtƒ‰ƒOƒŠƒZƒbƒg
		if (!player[p].skillAnimation && g_skillAnimStarted[p])	g_skillAnimStarted[p] = false;

		// ƒXƒyƒVƒƒƒ‹ŠJn‚ÌƒtƒŒ[ƒ€‰Šú‰»iƒAƒjƒ[ƒVƒ‡ƒ“XVƒ^ƒCƒ~ƒ“ƒO‚ÉˆË‘¶‚µ‚È‚¢j
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
			g_specialAnimPhase[p] = 0;			// ƒtƒF[ƒYƒŠƒZƒbƒg
			g_specialEndAnimTimer[p] = 0.0f;	// I—¹‰‰oƒ^ƒCƒ}[ƒŠƒZƒbƒg
			g_specialInitialize[p] = true;
		}
		// ƒXƒyƒVƒƒƒ‹I—¹‚ÌƒtƒŒ[ƒ€ƒŠƒZƒbƒg
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
			player[p].animFrame = idleStart; // ‘Ò‹@ƒtƒŒ[ƒ€‚ÉƒŠƒZƒbƒg
		}
		// ƒKƒ‰ƒXE“d‹CEA•¨: specialTimer ‚ÉŠî‚Ã‚­ƒAƒjƒ[ƒVƒ‡ƒ“I—¹§Œä
		// ¦ useSpecial ‚Í true ‚Ì‚Ü‚Üispecial.cpp ‚Ìƒ_ƒ[ƒWˆ—“™‚ÍŒp‘±j
		if (player[p].specialAnimation)
		{
			// ƒKƒ‰ƒXE“d‹C: 0.9•b‚ÅƒtƒŒ[ƒ€7A1.0•b‚Å‘Ò‹@
			if (player[p].type == PlayerType::Glass || player[p].type == PlayerType::Electricity)
			{
				if (player[p].specialTimer >= 1.0f)
				{
					// I—¹‰‰oƒtƒF[ƒY‚ÖiI—¹ƒtƒŒ[ƒ€‚ğ•\¦‚³‚¹‚éj
					g_specialAnimPhase[p] = 2;
					g_specialEndAnimTimer[p] = 0.0f;

					// ‘®«EŒü‚«‚©‚çI—¹‰‰oƒtƒŒ[ƒ€(start + 7) ‚ğŒˆ’è‚µ‚Äİ’è
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

					player[p].animFrame = start + 7;	// I—¹‰‰oƒtƒŒ[ƒ€‚ğ•\¦
					player[p].animTimer = 0.0f;		// “¯ƒtƒŒ[ƒ€‚Åis‚µ‚È‚¢‚æ‚¤ƒŠƒZƒbƒg
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
			// A•¨: 1.0•b‚Å‘Ò‹@‚É–ß‚·i‚»‚ê‚Ü‚Å‚Í8ƒRƒ}ƒ‹[ƒvŒp‘±j
			else if (player[p].type == PlayerType::Plant)
			{
				// 1.0•bŒo‰ß‚ÅƒAƒjƒ[ƒVƒ‡ƒ“I—¹
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

		// ƒvƒŒƒCƒ„[ ƒAƒjƒ[ƒVƒ‡ƒ“XV
		player[p].animTimer += DELTA_TIME;

		// ƒGƒtƒFƒNƒg ƒAƒjƒ[ƒVƒ‡ƒ“
		Effect_UpdateForPlayer(p);

		if (player[p].animTimer >= ANIM_FRAME_TIME)
		{
			int advance = (int)(player[p].animTimer / ANIM_FRAME_TIME);
			player[p].animTimer -= advance * ANIM_FRAME_TIME;

			// Ÿ—˜ ‘æ1Œ`‘Ô 13ƒRƒ}(ƒ‰ƒXƒg5ƒRƒ} ƒ‹[ƒv) ‘æ2Œ`‘Ô 20ƒRƒ}(ƒ‰ƒXƒg9ƒRƒ} ƒ‹[ƒv) ‘æ3Œ`‘Ô 21ƒRƒ}(ƒ‰ƒXƒgƒRƒ} ƒ‹[ƒv)
			//if (Keyboard_IsKeyDown(KK_TAB) || g_victoryState[p] != 0)
			if (player[p].rank == 1 || g_victoryState[p] != 0)
			{
				//if (Keyboard_IsKeyDown(KK_TAB) && g_victoryState[p] == 0)
				if (player[p].rank == 1 && g_victoryState[p] == 0)
				{
					g_victoryState[p] = 1;
					player[p].animFrame = 208;	// ‰‰ñÄ¶ŠJnƒtƒŒ[ƒ€
				}

				if (g_victoryState[p] == 1)
				{
					// ‰‰ñÄ¶ ƒtƒŒ[ƒ€‚ğ’Pƒ‘‰Á
					player[p].animFrame += advance;

					// ‘æ1Œ`‘Ô 220 ‚ğ•\¦‚µ‚½Œã‚Éƒ‹[ƒv—Ìˆæ‚ÖˆÚs‚·‚é
					if (player[p].animFrame > 220 && player[p].form == Form::First)
					{
						g_victoryState[p] = 2;
						player[p].animFrame = 216;	// ƒ‹[ƒvŠJnƒtƒŒ[ƒ€
					}
					// ‘æ2Œ`‘Ô 227 ‚ğ•\¦‚µ‚½Œã‚Éƒ‹[ƒv—Ìˆæ‚ÖˆÚs‚·‚é
					if (player[p].animFrame > 227 && player[p].form == Form::Second)
					{
						g_victoryState[p] = 2;
						player[p].animFrame = 219;	// ƒ‹[ƒvŠJnƒtƒŒ[ƒ€
					}
					// ‘æ3Œ`‘Ô 228 ‚ğ•\¦‚µ‚½Œã‚Éƒ‹[ƒv—Ìˆæ‚ÖˆÚs‚·‚é 229ƒRƒ}–Ú‚Íg—p‚µ‚È‚¢
					if (player[p].animFrame > 228 && player[p].form == Form::Third)
					{
						g_victoryState[p] = 2;
						player[p].animFrame = 221;	// ƒ‹[ƒvŠJnƒtƒŒ[ƒ€
					}
				}
				else if (g_victoryState[p] == 2)
				{
					switch (player[p].form)
					{
					case Form::First:	LoopRange(player[p].animFrame, 216, 5, advance);	// ‘æ1Œ`‘Ô 216`220‚ğƒ‹[ƒv
						break;
					case Form::Second:	LoopRange(player[p].animFrame, 219, 9, advance);	// ‘æ2Œ`‘Ô 219`227‚ğƒ‹[ƒv
						break;
					case Form::Third:	LoopRange(player[p].animFrame, 221, 8, advance);	// ‘æ3Œ`‘Ô 221`228‚ğƒ‹[ƒv 229ƒRƒ}–Ú‚Íg—p‚µ‚È‚¢
						break;
					}
				}
			}
			// ƒ_ƒEƒ“ 5ƒRƒ} (ƒ_ƒ[ƒW 2ƒRƒ} + ƒ_ƒEƒ“ 3ƒRƒ}) ÅIƒRƒ}‚Å’â~
			else if (player[p].isDown)
			{
				// Œü‚«‚É‰‚¶‚½ŠJnƒtƒŒ[ƒ€‚ğŒˆ’è
				int start = 15; // ƒfƒtƒHƒ‹ƒgiDownj
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

				// advance ‚É‘Î‰‚·‚éŒo‰ß•big_animTimer‚Å‚Ü‚Æ‚ß‚Äi‚ß‚½•ªj
				float elapsedSec = (float)advance * ANIM_FRAME_TIME;

				// ƒtƒŒ[ƒ€‚ª”ÍˆÍŠO‚È‚çŠJnƒtƒŒ[ƒ€‚É•â³‚µƒ^ƒCƒ}[ƒŠƒZƒbƒg
				if (player[p].animFrame < start || player[p].animFrame > lastFrame)
				{
					player[p].animFrame = start;
					g_downHoldTimer[p] = 0.0f;
				}

				// ÅIƒtƒŒ[ƒ€ˆÈŠO‚È‚ç‘æ1Œ`‘Ôisiƒ‹[ƒvj
				if (player[p].animFrame != lastFrame)
				{
					LoopRange(player[p].animFrame, start, count, advance);
					g_downHoldTimer[p] = 0.0f; // “’B‘O‚Íƒz[ƒ‹ƒhƒ^ƒCƒ}[‚ğƒŠƒZƒbƒg
				}
				else
				{
					// ÅIƒtƒŒ[ƒ€‚É“’B ƒz[ƒ‹ƒh‚ği‚ß‚é
					g_downHoldTimer[p] += elapsedSec;

					// ƒz[ƒ‹ƒh‚ª–—¹‚µ‚½‚çŸ‚Éi‚ß‚éi‚±‚±‚Å‚Í1ƒtƒŒ[ƒ€•ª‚¾‚¯i‚ß‚éj
					if (g_downHoldTimer[p] >= DOWN_TIME)
					{
						g_downHoldTimer[p] = 0.0f;
						// 1ƒtƒŒ[ƒ€•ªi‚ß‚éiƒ‹[ƒv‚É‚æ‚è start ‚É–ß‚éj
						LoopRange(player[p].animFrame, start, count, 1);
					}
				}
			}
			// ƒXƒyƒVƒƒƒ‹ ƒAƒjƒ[ƒVƒ‡ƒ“
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

				// ƒKƒ‰ƒXE“d‹C: 0`6‚ğ1‰ñÄ¶ ¨ 4`6‚ğƒ‹[ƒv
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
					// phase == 2 : ƒtƒŒ[ƒ€7•\¦’† -> ‰½‚à‚µ‚È‚¢ispecialTimerƒx[ƒX‚Å§Œäj
				}
				// A•¨: ]—ˆ’Ê‚è8ƒRƒ}ƒ‹[ƒv
				if (player[p].type == PlayerType::Plant)	LoopRange(player[p].animFrame, start, 8, advance);
			}
			// ƒ_ƒ[ƒW 3ƒRƒ}
			else if (player[p].isAttacked || player[p].isStunning)
			{
					 if (player[p].lastDir == PlayerDir::Down)		LoopRange(player[p].animFrame,  14, 3, advance);	//  ‰º   14`16 
				else if (player[p].lastDir == PlayerDir::Down_Left)	LoopRange(player[p].animFrame,  40, 3, advance);	// ¶‰º  40`42
				else if (player[p].lastDir == PlayerDir::Left)		LoopRange(player[p].animFrame,  66, 3, advance);	//  ¶   66`68
				else if (player[p].lastDir == PlayerDir::Up_Left)	LoopRange(player[p].animFrame,  92, 3, advance);	// ¶ã  92`94
				else if (player[p].lastDir == PlayerDir::Up)		LoopRange(player[p].animFrame, 118, 3, advance);	//  ã  118`120
				else if (player[p].lastDir == PlayerDir::Up_Right)	LoopRange(player[p].animFrame, 144, 3, advance);	// ‰Eã 144`146
				else if (player[p].lastDir == PlayerDir::Right)		LoopRange(player[p].animFrame, 170, 3, advance);	//  ‰E  170`172
				else if (player[p].lastDir == PlayerDir::Down_Right)LoopRange(player[p].animFrame, 196, 3, advance);	// ‰E‰º 196`198
			}
			// ƒXƒLƒ‹ 4ƒRƒ}i1‰ñÄ¶EÅIƒtƒŒ[ƒ€‚Å’â~Œã‚ÉI—¹j
			else if (player[p].skillAnimation)
			{
				int start = g_skillAnimStart[p];
				const int count = 4;
				const int lastFrame = start + count - 1;

				// ”ÍˆÍŠO‚È‚çŠJnƒtƒŒ[ƒ€‚ğŒvZE•Û‘¶‚µ‚ÄƒŠƒZƒbƒg
				if (player[p].animFrame < start || player[p].animFrame > lastFrame)
				{
					// ‘®«‚²‚Æ‚ÌŠî€ƒIƒtƒZƒbƒgi‘®«1‚Â‚ ‚½‚è32ƒRƒ}j
					int typeBase = 0;
						 if (player[p].type == PlayerType::Concrete)	typeBase = 0;
					else if (player[p].type == PlayerType::Electricity)	typeBase = 32;
					else if (player[p].type == PlayerType::Glass)		typeBase = 64;
					else if (player[p].type == PlayerType::Plant)		typeBase = 96;

					// Œ`‘ÔƒIƒtƒZƒbƒgi‘æ2Œ`‘Ô: 0A‘æ3Œ`‘Ô: 128j
					int formBase = 0;
					if (player[p].form == Form::Third) formBase = 128;

					// •ûŒüƒIƒtƒZƒbƒgi1•ûŒü‚ ‚½‚è4ƒRƒ}j
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

				// lastFrame ‚ğÄŒvZistart ‚ªXV‚³‚ê‚½‰Â”\«‚ª‚ ‚é‚½‚ßj
				const int finalFrame = g_skillAnimStart[p] + count - 1;

				// ÅIƒtƒŒ[ƒ€‚É’B‚µ‚Ä‚¢‚È‚¯‚ê‚Îi‚ß‚é
				if (player[p].animFrame < finalFrame)
				{
					player[p].animFrame += advance;
					// ƒI[ƒo[ƒVƒ…[ƒg–h~iÅIƒtƒŒ[ƒ€‚ÅƒNƒ‰ƒ“ƒvj
					if (player[p].animFrame > finalFrame) player[p].animFrame = finalFrame;
				}
				else
				{
					// ÅIƒtƒŒ[ƒ€‚É’B‚µ‚½‚çƒAƒjƒ[ƒVƒ‡ƒ“I—¹
					player[p].skillAnimation = false;

					// ’ÊíƒeƒNƒXƒ`ƒƒ‚Ì‘Ò‹@ƒAƒjƒ[ƒVƒ‡ƒ“ŠJnƒtƒŒ[ƒ€‚ÉƒŠƒZƒbƒg
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
			// UŒ‚ 6ƒRƒ}
			else if (player[p].isAttacking)
			{
					 if (player[p].lastDir == PlayerDir::Down)		LoopRange(player[p].animFrame,  20, 6, advance);	//  ‰º   20`25
				else if (player[p].lastDir == PlayerDir::Down_Left)	LoopRange(player[p].animFrame,  46, 6, advance);	// ¶‰º  46`51
				else if (player[p].lastDir == PlayerDir::Left)		LoopRange(player[p].animFrame,  72, 6, advance);	//  ¶   72`77
				else if (player[p].lastDir == PlayerDir::Up_Left)	LoopRange(player[p].animFrame,  98, 6, advance);	// ¶ã  98`103
				else if (player[p].lastDir == PlayerDir::Up)		LoopRange(player[p].animFrame, 124, 6, advance);	//  ã  124`129
				else if (player[p].lastDir == PlayerDir::Up_Right)	LoopRange(player[p].animFrame, 150, 6, advance);	// ‰Eã 150`155
				else if (player[p].lastDir == PlayerDir::Right)		LoopRange(player[p].animFrame, 176, 6, advance);	//  ‰E  176`181
				else if (player[p].lastDir == PlayerDir::Down_Right)LoopRange(player[p].animFrame, 202, 6, advance);	// ‰E‰º 202`207
			}
			// ˆÚ“® 8ƒRƒ} iƒŠƒXƒ|[ƒ“’†‚ğœ‚­j
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
			// ‘Ò‹@ 6ƒRƒ}
			else if (player[p].isMoving == false)
			{
					 if (player[p].lastDir == PlayerDir::Down)		LoopRange(player[p].animFrame,   0, 6, advance);	//  ‰º    0`5
				else if (player[p].lastDir == PlayerDir::Down_Left)	LoopRange(player[p].animFrame,  26, 6, advance);	// ¶‰º  26`31
				else if (player[p].lastDir == PlayerDir::Left)		LoopRange(player[p].animFrame,  52, 6, advance);	//  ¶   52`57
				else if (player[p].lastDir == PlayerDir::Up_Left)	LoopRange(player[p].animFrame,  78, 6, advance);	// ¶ã  78`83 
				else if (player[p].lastDir == PlayerDir::Up)		LoopRange(player[p].animFrame, 104, 6, advance);	//  ã  104`109
				else if (player[p].lastDir == PlayerDir::Up_Right)	LoopRange(player[p].animFrame, 130, 6, advance);	// ‰Eã 130`135
				else if (player[p].lastDir == PlayerDir::Right)		LoopRange(player[p].animFrame, 156, 6, advance);	//  ‰E  156`161
				else if (player[p].lastDir == PlayerDir::Down_Right)LoopRange(player[p].animFrame, 182, 6, advance);	// ‰E‰º 182`187		
			}
		}

		static XMFLOAT3 posBuff = player[p].position;	// ƒfƒoƒbƒO•\¦À•W

		// •`‰æ‚Åg‚Á‚Ä‚¢‚éƒXƒvƒ‰ƒCƒg”{—¦‚Æ“¯‚¶’l‚ğ•¨—‚É‚àg‚¤
		const float renderScale = 2.0f;	// Draw ‘¤‚Ì spriteScale ‚É‡‚í‚¹‚é
		// •`‰æƒXƒP[ƒ‹‚ğ”½‰f‚µ‚½ƒXƒP[ƒ‹i•\¦—pj
		XMFLOAT3 physicsScaling = XMFLOAT3(player[p].scaling.x * renderScale, player[p].scaling.y * renderScale, player[p].scaling.z * renderScale);


		////////////////////////////////////////////////////////////////////////////////////////////
		// TODO:

		// --- ƒvƒŒƒCƒ„[—pƒqƒbƒgƒ{ƒbƒNƒX”ä—¦iŒü‚«‚Å’·’Z‚ğØ‚è‘Ö‚¦‚éj ---
		// ‚‚³‚ÍŒÅ’èA…•½–Ê‚ÍŒü‚«‚É‰‚¶‚Ä’·’Z‚ğØ‚è‘Ö‚¦‚é
		const float HITBOX_HEIGHT_SCALE = 1.0f;
		const float HITBOX_SHORT = 0.35f;	// Œü‚«‚Æ’¼Œğ‚·‚é’Z•Ó
		const float HITBOX_LONG = 0.65f;	// Œü‚«‚É‰ˆ‚Á‚½’·•Ó

		// ‰ñ“]‚©‚ç‘O•ûƒxƒNƒgƒ‹‚ğZo‚µ‚ÄA‚Ç‚¿‚ç‚Ì²‚ª—D¨‚©”»’è‚·‚é
		float radFacing = XMConvertToRadians(player[p].rotation.y);
		float facingX = sinf(radFacing);
		float facingZ = cosf(radFacing);
		bool facingZDominant = fabsf(facingZ) <= fabsf(facingX);

		float widthScale = facingZDominant ? HITBOX_SHORT : HITBOX_LONG;	// X•ûŒüƒXƒP[ƒ‹
		float depthScale = facingZDominant ? HITBOX_LONG : HITBOX_SHORT;	// Z•ûŒüƒXƒP[ƒ‹


		XMFLOAT3 hitboxScaling = XMFLOAT3
		(
			player[p].scaling.x * renderScale * widthScale,
			player[p].scaling.y * renderScale * HITBOX_HEIGHT_SCALE,
			player[p].scaling.z * renderScale * depthScale
		);


		/////////////////////////////////////////////////////////////////////////////////////
		// TODO:Œš•¨‚Æ‚Ì‚Ù‚Á‚»‚¢“–‚½‚è”»’è‚Æ‚Í•Ê‚ÉAUŒ‚‚ğH‚ç‚¤—p‚Ì‘å‚«‚ß‚Ì“–‚½‚è”»’è‚ğì‚é
		// TODO:d—Í‚ÌŒ©’¼‚µ‚ÆAƒvƒŒƒCƒ„[‚ªd—Í‚É‚æ‚è–³ŒÀ‚É€‚Ê‚Ì‚ğ–h‚®
		
		// AABB ‚ğŒ»İ‚ÌˆÊ’uEƒXƒP[ƒ‹iƒqƒbƒgƒ{ƒbƒNƒXj‚ÅXV‚µ‚Ä‚¨‚­iÕ“Ë”»’è‚Åg—pj
		CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);

		// 1. ‘¬“x‚ª‚ ‚ê‚ÎA‚»‚Ì•ª‚¾‚¯À•W‚ğ“®‚©‚·i‚±‚ê‚ªu‚Á”ò‚ñ‚Å‚¢‚évó‘Ôj
		player[p].position.x += player[p].velocity.x;
		player[p].position.y += player[p].velocity.y;
		player[p].position.z += player[p].velocity.z;

		// 2. –€C‚ÅŒ¸‘¬
		player[p].velocity.x *= 0.95f; // 1–¢–‚ğŠ|‚¯‚é‚Æ‚¾‚ñ‚¾‚ñ’x‚­‚È‚é
		player[p].velocity.z *= 0.95f;

		// 3. d—Í‚ğ‚©‚¯‚éi•‚‚©‚¹‚½ê‡j
		if (!player[p].duringRespawn)
		{
			if (player[p].position.y >= -11.0f) {
				player[p].velocity.y = 0.02f; // ‰ºŒü‚«‚Ì—Í
			}
			else {
				player[p].velocity.y = 0.0f;
			}
		}

		posBuff = player[p].position;

		// ’n–Ê‚Ì‚‚³iÅ’áƒ‰ƒCƒ“j
		//float groundHeight = -10.0f;	// “Ş—‚Ì’ê
		//bool isShadowEnabled = false;		// ’n–Ê‚É‘«‚ª‚Â‚¢‚Ä‚¢‚é‚©ƒtƒ‰ƒO

		// ƒ}ƒbƒvƒf[ƒ^i’n–Êj‚Æ‚Ì“–‚½‚è”»’è
		int fieldCount = GetFieldObjectCount();
		MAPDATA* fieldObjects = GetFieldObjects();

		for (int j = 0; j < fieldCount; ++j)
		{
			// ƒAƒNƒeƒBƒu‚¶‚á‚È‚¢A‚Ü‚½‚Í no ‚ª MAX ‚È‚çƒXƒLƒbƒv
			if (!fieldObjects[j].isActive || fieldObjects[j].no == FIELD::FIELD_MAX)
			{
				continue;
			}

			// ƒvƒŒƒCƒ„[‚ÌAABBi‘Ì‚Ìˆê•”j‚ª˜ZŠp’Œ‚Éæ‚Á‚Ä‚¢‚é‚©
			if (CheckAABBHexCollision(player[p].boundingBox, fieldObjects[j].boundingBox))
			{
				// ƒ^ƒCƒ‹‚Ìã–Ê‚ÌYÀ•W‚ğŒvZ
				float tileTopY = fieldObjects[j].pos.y + (fieldObjects[j].boundingBox.height / 2.0f);	// -1 + 1.5 = 0.5

				// ƒvƒŒƒCƒ„[‚Ì’ê–Ê‚ªƒ^ƒCƒ‹‚Ìã–ÊˆÈ‰º‚©
				if (player[p].boundingBox.Min.y <= tileTopY)
				{
					const float baseHalfHeight = COORDINATE;
					// ’…’n‚Å‚ÍŒ©‚½–Ú‚Ì‚‚³i•`‰æƒXƒP[ƒ‹j‚ğŠî€‚ÉŒvZ‚µ‚Ä‚¢‚é‚½‚ß physicsScaling ‚ğg—p
					float halfHeight = baseHalfHeight * player[p].scaling.y * renderScale;

					// ’…’n‚³‚¹‚éi‚ß‚è‚İ‚ª‹N‚«‚È‚¢‚æ‚¤Å’á’l‚Æ‚µ‚Ä•â³j
					float targetY = tileTopY + halfHeight;
					if (player[p].position.y < targetY)
					{
						player[p].position.y = targetY;
						player[p].isShadowEnabled = true; // ‰eƒGƒtƒFƒNƒg”ñ•\¦
					}

					// AABB ‚ğÄŒvZ‚µ‚Ä®‡«‚ğ•Û‚Âi•`‰æƒXƒP[ƒ‹‚ğl—¶j
					// ƒqƒbƒgƒ{ƒbƒNƒXiŒü‚«‚É‰‚¶‚½’·•ûŒ`j‚ÅÄŒvZ‚·‚é
					CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);

					top_y = tileTopY;

					break;
				}
			}
		}

		// -------------------------------------------------------------------------------------
		// Œš•¨‚Æ‚Ì“–‚½‚è”»’è
		// -------------------------------------------------------------------------------------
		int buildingCount = GetBuildingCount();			// ”‚ğæ“¾
		Building** buildingObjects = GetBuildings();	// ƒŠƒXƒg‚ğæ“¾

		for (int j = 0; j < buildingCount; ++j)
		{
			// ƒAƒNƒeƒBƒu‚Å‚È‚¢‚È‚ç–³‹
			if (!buildingObjects[j]->isActive)	continue;

			// ’Ç‰ÁFFBX–¼‚ª "togeki" ‚ÌŒš•¨‚Æ‚Í“–‚½‚è”»’è‚µ‚È‚¢
			// iPlant ƒ^ƒCƒv‚Ìƒ‚ƒfƒ‹–¼”z—ñ‚É "togeki" ‚ª‚ ‚é‘z’èj
			const char* modelName = buildingObjects[j]->GetModelName();
			if (buildingObjects[j]->GetType() == BuildingType::Plant &&
				std::strcmp(modelName, "togeki") == 0)
			{
				// ‚±‚ÌŒš•¨‚ÍÕ“Ë”»’è‚ğ–³‹
				continue;
			}

			// Œš•¨‚ª©•ª‚ÅŒvZ‚µ‚Ä‚¨‚¢‚Ä‚­‚ê‚½ AABB ‚ğ‚à‚ç‚¤‚¾‚¯I
			const AABB& bBox = buildingObjects[j]->GetAABB();

			// ”»’èI
			MTV collision = CalculateAABBMTV(player[p].boundingBox, bBox);			if (collision.isColliding)
			{
				// Õ“Ë‚µ‚Ä‚¢‚½‚çAMTV‚Ì•ª‚¾‚¯ˆÊ’u‚ğ–ß‚·
				player[p].position.x += collision.translation.x;
				player[p].position.y += collision.translation.y;
				player[p].position.z += collision.translation.z;

				// ‰Ÿ‚µ–ß‚µŒã‚ÌV‚µ‚¢AABB‚ğÄŒvZi•`‰æƒXƒP[ƒ‹‚ğ”½‰fj
				// ƒqƒbƒgƒ{ƒbƒNƒXiŒü‚«‚É‰‚¶‚½’·•ûŒ`j‚ÅÄŒvZ‚·‚é
				CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);
			}
		}

		// ƒvƒŒƒCƒ„[‚É‘Î‰‚·‚éUŒ‚ƒIƒuƒWƒFƒNƒg‚ğ PLAYER_MAX •ªƒ‹[ƒv‚µ‚ÄƒXƒP[ƒŠƒ“ƒO“¯Šú
		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			ATTACK_OBJECT* attackObject = GetAttack(p); // GetAttack ‚Í 1-based
			if (attackObject == nullptr) continue;

			// ƒvƒŒƒCƒ„[‘¤‚ÌƒXƒP[ƒ‹‚É‡‚í‚¹‚éiUŒ‚ƒIƒuƒWƒFƒNƒg‚Í”¼•ªj
			attackObject->scaling.x = player[p].scaling.x * 0.5f;
			attackObject->scaling.y = player[p].scaling.y * 0.5f;
			attackObject->scaling.z = player[p].scaling.z * 0.5f;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////
		// TODO:

		// -------------------------------------------------------------
		// ƒvƒŒƒCƒ„[ƒIƒuƒWƒFƒNƒg“¯m‚Ì“–‚½‚è”»’èiPLAYER_MAX•ª‘Î‰j
		// -------------------------------------------------------------
		for (int otherIndex = p + 1; otherIndex < PLAYER_MAX; ++otherIndex)
		{
			// ”ñƒAƒNƒeƒBƒu‚Í–³‹
			if (!player[otherIndex].active) continue;

			// ‘¼ƒvƒŒƒCƒ„[‚Ì AABB ‚ğXVi‚±‚±‚Å’è‹`Ï‚İ‚Ì hitboxScalingOther ‚ğg—pj
			CalculateAABB(player[otherIndex].boundingBox, player[otherIndex].position, hitboxScaling);

			// Õ“Ëƒ`ƒFƒbƒNiƒyƒA p <-> otherIndex ‚ğˆê“x‚¾‚¯”»’èj
			MTV collision_player = CalculateAABBMTV(player[p].boundingBox, player[otherIndex].boundingBox);

			if (collision_player.isColliding)
			{
				// Œü‚«ƒxƒNƒgƒ‹‚ğXVirotation.y ‚©‚çZoj
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

				// ‰Ÿ‚µ–ß‚µ—Ê (MTV) ‚ğ”¼•ª‚É‚µ‚Ä‘o•û‚É“K—p
				XMFLOAT3 half_translation =
				{
					collision_player.translation.x * 0.5f,
					collision_player.translation.y * 0.5f,
					collision_player.translation.z * 0.5f
				};

				// object[p] ‚ğ MTV ‚Ì”¼•ª‚¾‚¯‰Ÿ‚·
				player[p].position.x += half_translation.x;
				player[p].position.y += half_translation.y;
				player[p].position.z += half_translation.z;

				// object[otherIndex] ‚ğ‹t•ûŒü‚É”¼•ª‚¾‚¯‰Ÿ‚·
				player[otherIndex].position.x -= half_translation.x;
				player[otherIndex].position.y -= half_translation.y;
				player[otherIndex].position.z -= half_translation.z;

				// ‰Ÿ‚µ–ß‚µŒã‚ÌV‚µ‚¢AABB‚ğÄŒvZ (ƒqƒbƒgƒ{ƒbƒNƒX‚Å)
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

	// ƒvƒŒƒCƒ„[“¯m‚ÌUŒ‚”»’è
	AttackPlayerCollisions();
	//ImGui::End();
}

//======================================================
//	ƒVƒ‹ƒGƒbƒg—p•`‰æ
//======================================================
static void Player_DrawSilhouette(int p)
{
	if (!Loader::IsFinished && g_loadedCount == 0) return;
	if (!player[p].active) return;

	// ƒvƒƒWƒFƒNƒVƒ‡ƒ“Eƒrƒ…[s—ñ‚ğæ“¾
	XMMATRIX proj = GetProjectionMatrix();
	XMMATRIX view = GetViewMatrix();

	const float scale = 3.5f; // ’Êí•`‰æ‚Æ“¯‚¶”{—¦‚ğ‚©‚¯‚é

	// ƒ[ƒ‹ƒhs—ñiƒrƒ‹ƒ{[ƒhj
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

	// ƒVƒ‹ƒGƒbƒgF‚ğİ’èiƒvƒŒƒCƒ„[‚²‚Æ‚ÉˆÙ‚È‚éFj
	XMFLOAT4 silhouetteColor;
	switch (p)
	{
	case 0: silhouetteColor  = { 0.64f,  0.2f, 0.2f, 1.0f }; break; // Ô
	case 1: silhouetteColor  = {  0.0f, 0.45f, 0.7f, 1.0f }; break; // Â
	case 2: silhouetteColor  = {  0.7f,  0.7f, 0.0f, 1.0f }; break; // ‰©
	case 3: silhouetteColor  = {  0.0f,  0.6f, 0.0f, 1.0f }; break; // —Î
	default: silhouetteColor = {  1.0f,  1.0f, 1.0f, 1.0f }; break;
	}
	Shader_SetColor(silhouetteColor);

	// [“xƒeƒXƒg ‰œ‚É‚ ‚é‚¾‚¯•`‰æ‚·‚éiGreaterj
	ID3D11DeviceContext* context = Direct3D_GetDeviceContext();
	ID3D11DepthStencilState* depthStateGreater = Direct3D_GetDepthStateGreater();
	context->OMSetDepthStencilState(depthStateGreater, 0);

	// ƒVƒ‹ƒGƒbƒg—p‚Ì•`‰æƒ‚[ƒhİ’è
	Shader_SetDrawMode(1);

	// ƒeƒNƒXƒ`ƒƒİ’èi’Êí•`‰æ‚Æ“¯‚¶j
	ID3D11ShaderResourceView* srv = nullptr;
	switch (player[p].form)
	{
	// ‘æ1Œ`‘Ô
	case Form::First:
			 if (p == 0)				srv = g_Texture[0];
		else if (p == 1)				srv = g_Texture[1];
		else if (p == 2)				srv = g_Texture[2];
		else if (p == 3)				srv = g_Texture[3];
		break;
	// ‘æ2Œ`‘Ô
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
	// ‘æ3Œ`‘Ô
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

	// ƒXƒLƒ‹EƒXƒyƒVƒƒƒ‹ê—pƒeƒNƒXƒ`ƒƒ
	if (player[p].useSpecial && player[p].specialAnimation)	srv = g_Texture[13];	// ƒXƒyƒVƒƒƒ‹ƒAƒjƒ[ƒVƒ‡ƒ“Œp‘±’†‚Ì‚İ
	else if (player[p].skillAnimation)						srv = g_Texture[12];	// ƒXƒLƒ‹”­“®ƒAƒjƒ[ƒVƒ‡ƒ“

	// ’¸“_ƒoƒbƒtƒ@‚Éƒf[ƒ^ƒRƒs[iUVİ’èj
	D3D11_MAPPED_SUBRESOURCE msr;
	Vertex2 localVt[PLAYER_VERTEX];
	CopyMemory(&localVt[0], &vdata[0], sizeof(Vertex2) * PLAYER_VERTEX);

	// Œ»İ‚ÌƒAƒjƒ[ƒVƒ‡ƒ“ƒtƒŒ[ƒ€‚©‚çUVŒvZ
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

	// •`‰æ
	UINT stride = sizeof(Vertex2);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->DrawIndexed(6, 0, 0);

	// [“xƒXƒe[ƒg‚ğ–ß‚·
	ID3D11DepthStencilState* depthStateEnable = Direct3D_GetDepthStateEnable();
	context->OMSetDepthStencilState(depthStateEnable, 0);

	// •`‰æƒ‚[ƒh‚ğ’Êí‚É–ß‚·
	Shader_SetDrawMode(0);
	Shader_SetColor(color::white);
}

//======================================================
//	ƒAƒEƒgƒ‰ƒCƒ“—p•`‰æ
//======================================================
static void Player_DrawOutline(int p)
{
	if (!Loader::IsFinished && g_loadedCount == 0) return;
	if (!player[p].active) return;

	// ƒvƒƒWƒFƒNƒVƒ‡ƒ“Eƒrƒ…[s—ñ‚ğæ“¾
	XMMATRIX proj = GetProjectionMatrix();
	XMMATRIX view = GetViewMatrix();

	const float scale = 3.6f; // ’Êí•`‰æ‚è­‚µ‘å‚«‚ß‚Ì”{—¦‚ğ‚©‚¯‚é

	// ƒ[ƒ‹ƒhs—ñiƒrƒ‹ƒ{[ƒhj
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

	// ç¹§E·ç¹ï½«ç¹§E¨ç¹ãEãƒ¨æ¿¶E²ç¹§å®šï½¨E­è³å¤²E¼åŒ»ãƒ»ç¹ï½¬ç¹§E¤ç¹ï½¤ç¹ï½¼ç¸ºæ–âEç¸ºE«é€¡E°ç¸ºEªç¹§ç–ç‰¡ãƒ»ãƒ»
	XMFLOAT4 outerColor;
	switch (p)
	{
	case 0: outerColor = { 0.94f,  0.5f, 0.5f, 1.0f }; break; // è¥ï½¤
	case 1: outerColor = {  0.0f, 0.75f, 1.0f, 1.0f }; break; // é«±ãƒ»
	case 2: outerColor = {  1.0f,  1.0f, 0.3f, 1.0f }; break; // é®ŸãE
	case 3: outerColor = {  0.0f,  1.0f, 0.0f, 1.0f }; break; // é‚±ãƒ»
	default: outerColor = { 1.0f, 1.0f, 1.0f, 0.4f }; break;
	}
	Shader_SetColor(outerColor);

	// ç¹§E¢ç¹§E¦ç¹åŒ»Î›ç¹§E¤ç¹ï½³é€•ï½¨ç¸ºE®è¬ å†—åˆ¤ç¹ï½¢ç¹ï½¼ç¹èŠ½E¨E­è³ãƒ»
	Shader_SetDrawMode(2);

	// ç¹ãEã‘ç¹§E¹ç¹âEÎ•éšªE­è³å¤²E¼ç£¯Â€å£¼E¸E¸è¬ å†—åˆ¤ç¸ºE¨èœ·å¾ŒÂ§ãƒ»ãƒ»
	ID3D11ShaderResourceView* srv = nullptr;
	switch (player[p].form)
	{
	// éš¨E¬1è –ï½¢è«·ãƒ»
	case Form::First:
			 if (p == 0)				srv = g_Texture[0];
		else if (p == 1)				srv = g_Texture[1];
		else if (p == 2)				srv = g_Texture[2];
		else if (p == 3)				srv = g_Texture[3];
		break;
	// éš¨E¬2è –ï½¢è«·ãƒ»
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
	// éš¨E¬3è –ï½¢è«·ãƒ»
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

	// ç¹§E¹ç¹§E­ç¹ï½«ç¹ï½»ç¹§E¹ç¹å£¹ã™ç¹ï½£ç¹ï½«èŸE‚‰ç•‘ç¹ãEã‘ç¹§E¹ç¹âEÎE
	if (player[p].useSpecial && player[p].specialAnimation)	srv = g_Texture[13];	// ç¹§E¹ç¹å£¹ã™ç¹ï½£ç¹ï½«ç¹§E¢ç¹ä¹Î“ç¹ï½¼ç¹§E·ç¹ï½§ç¹ï½³é‚¯å’ï½¶å£»E¸E­ç¸ºE®ç¸ºE¿
	else if (player[p].skillAnimation)						srv = g_Texture[12];	// ç¹§E¹ç¹§E­ç¹ï½«é€‹ï½ºèœè¼”ã„ç¹ä¹Î“ç¹ï½¼ç¹§E·ç¹ï½§ç¹ï½³

	// é¬E‚‰ã›ç¹èEãƒ£ç¹è¼”ãƒç¸ºE«ç¹ãEãƒ»ç¹§E¿ç¹§E³ç¹æ–ãƒ»ãƒ»ãƒ»VéšªE­è³å¤²E¼ãƒ»
	D3D11_MAPPED_SUBRESOURCE msr;
	Vertex2 localVt[PLAYER_VERTEX];
	CopyMemory(&localVt[0], &vdata[0], sizeof(Vertex2) * PLAYER_VERTEX);

	// è¿´E¾è¨E¨ç¸ºE®ç¹§E¢ç¹ä¹Î“ç¹ï½¼ç¹§E·ç¹ï½§ç¹ï½³ç¹è¼”Îç¹ï½¼ç¹ï£°ç¸ºä¹ï½‰UVéšªè‚²E®ãƒ»
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

	// è¬ å†—åˆ¤
	UINT stride = sizeof(Vertex2);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pContext->DrawIndexed(6, 0, 0);

	// è¬ å†—åˆ¤ç¹ï½¢ç¹ï½¼ç¹å³¨E’é¨¾å£¼E¸E¸ç¸ºE«è¬Œï½»ç¸ºãƒ»
	Shader_SetDrawMode(0);
	Shader_SetColor(color::white);
}

//======================================================
//	ç¹åŠ±Îç¹§E¤ç¹ï½¤ç¹ï½¼è­›ï½¬è´æºç·’é€•ï½»é«¢E¢è¬¨E°
//======================================================
void Player_Draw(bool s_IsKonamiCodeEntered)
{
	if (!Loader::IsFinished && g_loadedCount == 0) return;

	// è¬¾E»è¬¦ãƒ»ãƒ»ç¹§E¹ç¹§E­ç¹ï½«ç¹ï½»ç¹§E¹ç¹å£¹ã™ç¹ï½£ç¹ï½«è¬ å†—åˆ¤
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (player[p].active && player[p].isAttacking)	Attack_Draw(p);
		//if (player[p].active && player[p].useSkill)		Skill_Draw(p);
		if (player[p].active && player[p].useSpecial)	Special_Draw(p);
	}

	LIGHT light{};
	light.Enable = TRUE;
	// èœˆå³¨ãƒ»èœ·ä»£â€³ãƒ»åŒ»Î¡ç¹ï½¼ç¹ï½«ç¹èEE©Eºé«¢é›£E¼å³¨ã™ç¹§E§ç¹ï½¼ç¹Â€ç¹ï½¼è››ï½´ç¸ºE§èœŠå€E½½æ¦Šå–§ç¸ºåŠ±â€»è´E¿ç¸ºE£ç¸ºE¦ç¸ºãƒ»E‹è« E³è³ãƒ»
	light.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
	// è«¡E¡è¬¨E£èœˆå³¨â†’è¿ºE°è EEãƒ»
	light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	Shader_SetLight(light);

	static bool input1 = false;
	// ç¹ãEãƒ°ç¹ãEã’ç¹ï½¢ç¹ï½¼ç¹æEE¸E­ç¸ºE®ç¸ºE¿ç¹§E­ç¹ï½¼èœˆï½¥èœ‰å¸™ï½’èœ¿åŠ±E è‰å€¥E ç¹§ãƒ»
	if (s_IsKonamiCodeEntered)
	{
		if (Keyboard_IsKeyDownTrigger(KK_D1)) input1 = !input1;	// ç¹è¼”Î›ç¹§E°èœ¿å´ï½»E¢
	}

	Shader_Begin();

	// ========================================================
	// è‚ï½¥ç¸ºE®ç¹åŠ±Îç¹§E¤ç¹ï½¤ç¹ï½¼ç¸ºæ¢§ç„”èœ‘é˜ªãƒ»ç¹åŠ±Îç¹§E¤ç¹ï½¤ç¹ï½¼ç¸ºE«é««E°ç¹§å¾ŒâEç¸ºãƒ»Eˆç¸ºãƒ»â†“è¬ å†—åˆ¤
	// ========================================================

	// ç¹åŠ±ÎŸç¹§E¸ç¹§E§ç¹§E¯ç¹§E·ç¹ï½§ç¹ï½³ç¹ï½»ç¹è–™Î—ç¹ï½¼é™¦æ‚ŸãEç¹§è²ãEç¸ºE«èœ¿é–€E¾ãƒ»
	XMMATRIX projection = GetProjectionMatrix();
	XMMATRIX view = GetViewMatrix();

	// ç¹§E«ç¹ï½¡ç¹ï½©è´å’²E½E®ç¹§å ¤E®æ€œãEãƒ»ãƒ»iew ç¸ºE®é¨¾ãƒ»E¡æ‚ŸãEç¸ºE® r[3] ç¸ºå¾ŒÎ¡ç¹ï½¼ç¹ï½«ç¹èEE©Eºé«¢è–™ãEç¹§E«ç¹ï½¡ç¹ï½©è´å’²E½E®ãƒ»ãƒ»
	XMMATRIX invView = XMMatrixInverse(nullptr, view);
	XMFLOAT3 camPos;
	camPos.x = invView.r[3].m128_f32[0];
	camPos.y = invView.r[3].m128_f32[1];
	camPos.z = invView.r[3].m128_f32[2];

	// ç¹åŠ±Îç¹§E¤ç¹ï½¤ç¹ï½¼ç¹§å‘ˆç·’é€•ï½»ç¸ºå¶E‹ç¹ï½©ç¹ï£°ç¹Â€ãƒ»ãƒ»rojection, View ç¹§åµãç¹ï½£ç¹åŠ±ãƒ¡ç¹ï½£ãƒ»ãƒ»
	auto DrawPlayerInternal = [&](int idx)
	{
		if (!player[idx].active) return;

		// ç¹åŠ±Îç¹§E¤ç¹ï½¤ç¹ï½¼ç¸ºE®è –ï½±ç¹§E¨ç¹è¼”ã‰ç¹§E¯ç¹åŸŸç·’é€•ï½»
		EffectShadow_DrawForPlayer(idx);

		const float spriteScale = 3.5f;	// é™¦E¨é‰ï½ºè›Ÿå’²ç´«

		// ç¹ï½¯ç¹ï½¼ç¹ï½«ç¹èŠ½E¡æ‚ŸãEãƒ»åŒ»ãƒ³ç¹ï½«ç¹æ‡Šãƒ»ç¹è›¾E¢E¨ç¸ºE®è­Œï½¢èŸE€¥ÎŸç¹§E¸ç¹ãEã‘ç¹§å®šï½¸å‰°E¥E²ãƒ»ãƒ»
		XMMATRIX ScalingMatrix = XMMatrixScaling(
			player[idx].scaling.x * spriteScale,
			player[idx].scaling.y * spriteScale,
			player[idx].scaling.z * spriteScale
		);

		XMMATRIX vm = GetViewMatrix();	// ç¹§E«ç¹ï½¡ç¹ï½©ç¸ºE®é™¦æ‚ŸãE
		vm.r[3].m128_f32[0] = 0.0f;
		vm.r[3].m128_f32[1] = 0.0f;
		vm.r[3].m128_f32[2] = 0.0f;
		vm.r[3].m128_f32[3] = 1.0f;
		vm = XMMatrixTranspose(vm);
		vm.r[3].m128_f32[0] = player[idx].position.x;
		vm.r[3].m128_f32[1] = player[idx].position.y;
		vm.r[3].m128_f32[2] = player[idx].position.z;
		vm.r[3].m128_f32[3] = 1.0f;

		// World é™¦æ‚ŸãEãƒ»åŒ»ãƒ³ç¹ï½«ç¹æ‡Šãƒ»ç¹èEç•‘ãEå³¨E’ç¹§E·ç¹§E§ç¹ï½¼ç¹Â€ç¹ï½¼ç¸ºE«è²‚ï½¡ç¸ºãƒ»
		XMMATRIX WorldMatrix = ScalingMatrix * vm;
		Shader_SetWorldMatrix(WorldMatrix);

		XMMATRIX WVP = ScalingMatrix * vm * view * projection;

		Shader_SetMatrix(WVP);
		Shader_Begin();
		SetBlendState(BLENDSTATE_ALPHA);

		// é¬E‚‰ã›ç¹èEãƒ£ç¹è¼”ãƒç¸ºE«ç¹ãEãƒ»ç¹§E¿ç¹§E³ç¹æ–ãƒ»ãƒ»åŒ»ãƒµç¹ï½¬ç¹ï½¼ç¹ï£°ç¸ºE«è ¢æ‡ŠÂ§ç¸ºE¦UVç¹§å‘ˆå¶Œç¸ºè‚´é‹¤ç¸ºåŒ»E‹ãEãƒ»
		D3D11_MAPPED_SUBRESOURCE msr;

		// ç¹§E³ç¹æ–ãƒ»èœˆãEãƒ»vdata ç¹§åµÎŸç¹ï½¼ç¹§E«ç¹ï½«é©Ÿæ¦ŠãEç¸ºE«ç¹§E³ç¹æ–ãƒ»ç¸ºåŠ±â€» UV ç¹§å®šï½ªE¿è¬¨E´
		Vertex2 localV[PLAYER_VERTEX];
		CopyMemory(&localV[0], &vdata[0], sizeof(Vertex2) * PLAYER_VERTEX);

		// è¿´E¾è¨E¨ç¸ºE®ç¹è¼”Îç¹ï½¼ç¹ï£°ç¸ºä¹ï½EUV ç¹§å®šï½¨è‚²E®ãƒ»
		int frame = player[idx].animFrame;
		int col = frame % SHEET_COLS;
		int row = frame / SHEET_COLS;
		float u0 = (float)col / (float)SHEET_COLS;
		float v0 = (float)row / (float)SHEET_ROWS;
		float u1 = u0 + 1.0f / (float)SHEET_COLS;
		float v1 = v0 + 1.0f / (float)SHEET_ROWS;

		// é¬E‚‰ã›ç¸ºE®ç¹ãEã‘ç¹§E¹ç¹âEÎ•è ï½§è®“å¶E’è³é ‘å¶Œç¸ºãƒ»
		localV[0].tex = XMFLOAT2(u0, v0);	// LEFT-TOP
		localV[1].tex = XMFLOAT2(u1, v0);	// RIGHT-TOP
		localV[2].tex = XMFLOAT2(u0, v1);	// LEFT-BOTTOM
		localV[3].tex = XMFLOAT2(u1, v1);	// RIGHT-BOTTOM

		// ç¹èEãƒ£ç¹è¼”ãƒç¸ºE¸è­–ï½¸ç¸ºå´ï½¾E¼ç¸ºE¿
		g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		Vertex2* vertex = (Vertex2*)msr.pData;
		CopyMemory(vertex, &localV[0], sizeof(Vertex2) * PLAYER_VERTEX);
		g_pContext->Unmap(g_VertexBuffer, 0);

		ID3D11ShaderResourceView* srv = nullptr;

		// è –ï½¢è«·ä¹âEç¹§E¿ç¹§E¤ç¹åŠ±â†“è ¢æ‡ŠÂ§ç¸ºæº˜ãƒ¦ç¹§E¯ç¹§E¹ç¹âEÎ•ç¹§å®šï½¨E­è³ãƒ»
		switch (player[idx].form)
		{
			// éš¨E¬1è –ï½¢è«·ãƒ»
		case Form::First:
			if (idx == 0)					srv = g_Texture[0];
			else if (idx == 1)				srv = g_Texture[1];
			else if (idx == 2)				srv = g_Texture[2];
			else if (idx == 3)				srv = g_Texture[3];
			break;
			// éš¨E¬2è –ï½¢è«·ãƒ»
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
			// éš¨E¬3è –ï½¢è«·ãƒ»
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

		// ç¹§E¹ç¹§E­ç¹ï½«ç¹ï½»ç¹§E¹ç¹å£¹ã™ç¹ï½£ç¹ï½«èŸE‚‰ç•‘ç¹ãEã‘ç¹§E¹ç¹âEÎE
		if (player[idx].useSpecial && player[idx].specialAnimation)	srv = g_Texture[13];	// ç¹§E¹ç¹å£¹ã™ç¹ï½£ç¹ï½«ç¹§E¢ç¹ä¹Î“ç¹ï½¼ç¹§E·ç¹ï½§ç¹ï½³é‚¯å’ï½¶å£»E¸E­ç¸ºE®ç¸ºE¿
		else if (player[idx].skillAnimation)						srv = g_Texture[12];	// ç¹§E¹ç¹§E­ç¹ï½«é€‹ï½ºèœè¼”ã„ç¹ä¹Î“ç¹ï½¼ç¹§E·ç¹ï½§ç¹ï½³

		g_pContext->PSSetShaderResources(0, 1, &srv);

		// ç¹åŠ±Îç¹§E¤ç¹ï½¤ç¹ï½¼ç¸ºæ–âEç¸ºE«é€¡E°ç¸ºEªç¹§ç–ç‰¡ç¹§å®šï½¨E­è³ãƒ»
		if (player[idx].isAttacked || player[idx].isDamageColor)
		{
			// ç¸ºE©ç¸ºE¡ç¹§å³¨ãƒ»ç¹§E¿ç¹§E¤ç¹æ§­ãƒ»ç¸ºæ‚Ÿè™šç¸ºãƒ»â€»ç¸ºãƒ»E‹ç¸ºãƒ»
			float currentTimer = player[idx].isAttacked ? player[idx].attackedTimer : player[idx].damageColorTimer;
			
			// è½¤E¹è²ŠãEãƒ»é¨¾æº˜ï¼E
			float speed = 40.0f; 

			// è½¤E¹è²ŠãEãƒ»è ï½¦èœ·åŒ»EãEãƒ».0fãƒ»ãƒ».0fãƒ»ãƒ»
			float blink = (sinf(currentTimer * speed) + 1.0f) * 0.5f;

			Shader_SetColorLerp(color::white, color::red, blink);

			// èœE½ªèœˆåŒ»E ç¸ºE¦è¥ï½¤ç¸ºä¸ŠâEç¹§ãƒ»
			//Shader_SetColorLerp(color::white, color::red, 0.7f); 
		}
		else if (player[idx].isPoisoned)
		{
			switch (idx)
			{
				// Lerp = 1.èµç¤¼E®è‹“ç‰¡ 2.é™¬æ†ºä¿£ç¸ºå¶E‹æ¿¶E² 3.é™¬æ†ºä¿£ç¸ºE®è ï½¦èœ·åŒ»EE
			case 0:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			case 1:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			case 2:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			case 3:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			default:	Shader_SetColor(color::white); break;
			}
		}
		else	Shader_SetColor(color::white); // é¨¾å£¼E¸E¸æ¿¶E²
		
		// ç¹èEãƒ£ç¹è¼”ãƒç¹§E»ç¹ãEãƒ¨ & è¬ å†—åˆ¤
		UINT stride = sizeof(Vertex2);
		UINT offset = 0;
		g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
		g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		g_pContext->DrawIndexed(6, 0, 0);

		// ç¹§E¨ç¹è¼”ã‰ç¹§E¯ç¹åŸŸç·’é€•ï½» ãƒ»åŒ»ãƒ»ç¹ï½¬ç¹§E¤ç¹ï½¤ç¹ï½¼ç¸ºE®è¬E¥ç‡•ãEãƒ»
		EffectFront_DrawForPlayer(idx);
	};

	// -----------------------------------
	// é¨¾ä¹—ãEè¬ å†—åˆ¤ç¸ºE®ç¸ºæº˜ï½ç¸ºE®ç¹§E½ç¹ï½¼ç¹èŒ¨E¼ç£¯â–¡ç¸ºãƒ»E°ãƒ»E¼ãƒ»
	// -----------------------------------
	std::vector<std::pair<float, int>> list;	// (éœæ™å±¬è å¾¡E¹ãƒ» index)
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

	// é©•ï£°ç¸ºãƒ»E°ãƒ»E¼äº¥E¤E§ç¸ºé˜ªEé¬EEE¼å³¨â†“ç¹§E½ç¹ï½¼ç¹ãE
	std::sort(list.begin(), list.end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
		{
			return a.first > b.first;
		});

	// é¨¾åŸâ„ƒç¹ï½¬ç¹ï½³ç¹Â€ç¹ï½ªç¹ï½³ç¹§E°ãƒ»å£½E·E±è ï½¦ç¹ãEã›ç¹åŒ»ãƒ»è­›ç‰™æŸ‘ç¸²âˆµE·E±è ï½¦è­–ï½¸ç¸ºå´ï½¾E¼ç¸ºE¿ç¸ºE¯è¾Ÿï½¡èœ‰ï½¹ãƒ»ãƒ»etDepthReadOnly ç¹§å‰E½½E¿é€•ï½¨ãƒ»ãƒ»
	SetDepthTest(true);
	SetDepthReadOnly();	// è±ºE±è ï½¦ç¹ãEã›ç¹åŒ»ãƒ»ç¸ºå¶E‹ç¸ºæ¢§E·E±è ï½¦ç¹èEãƒ£ç¹è¼”ãƒç¸ºE¸ç¸ºE®è­–ï½¸ç¸ºå´ï½¾E¼ç¸ºE¿ç¸ºE¯ç¸ºåŠ±â†‘ç¸ºãƒ»

	// ç¹§E½ç¹ï½¼ç¹ç£¯E°ãƒ»E¼ç£¯â–¡ç¸ºãƒ»E‚ç¸ºE®ç¸ºä¹ï½‰è¬ å†—åˆ¤ãƒ»ãƒ»
	for (auto& p : list)	DrawPlayerInternal(p.second);

	// 3Dç¹§Eªç¹æ‚¶ãšç¹§E§ç¹§E¯ç¹åŒ»ãƒ»è±ºE±è ï½¦ç¹ãEã›ç¹åŒ»E’è¾Ÿï½¡èœ‰ï½¹ç¸ºE«ç¸ºåŠ±â€»è¬ å†—åˆ¤
	SetDepthTest(false);

	// 3Dç¹§Eªç¹æ‚¶ãšç¹§E§ç¹§E¯ç¹èŒ¨E¼åŒ»ãƒ»ç¹ï½¬ç¹§E¤ç¹ï½¤ç¹ï½¼ãƒ»å³¨ãƒ»è¬ å†—åˆ¤ç¸ºæªï½µã‚E½ç¸ºE£ç¸ºæº·E¾ãƒ»..
	SetDepthTest(false); // ç¹§E³ç¹ï½©ç¹§E¤ç¹Â€ç¹ï½¼ç¹§å‘ˆæ€™èœ‘åŸ¼æ“E¸ºE«èœE½ºç¸ºåŠ±â—E¸ºãƒ»â†‘ç¹§å³¨EE¹§å¾Œã€’OK

	/////////////////////////////////////////////////////////////////////////////////////
	// TODO:è –è–™â—E¹§é›æEè³å£¹ãƒ»èœ¿E¯éš•é–€å–§
	if (s_IsKonamiCodeEntered)
	{
		// ç¹åŠ±Îç¹§E¤ç¹ï½¤ç¹ï½¼ç¸ºE®è¬ å†—åˆ¤ç¸ºE«è´E¿ç¹§ä¸Šï½Œç¸ºæº¯E¡æ‚ŸãEç¹§åµã‘ç¹ï½ªç¹§E¢ç¸ºå¶EE
		Shader_SetMatrix(XMMatrixIdentity() * GetViewMatrix() * GetProjectionMatrix()); // WVPé™¦æ‚ŸãEç¹§æ£šdentity * View * Projectionç¸ºE«éšªE­è³ãƒ»

		// 3. é¨¾åŸâ„ƒç¹§ãƒ»ç‰¡ç¸ºå¾Œâ™€ç¸ºä¹ï¼ ç¸ºä¸ŠâEç¹§å³¨â†‘ç¸ºãƒ»Eˆç¸ºãƒ»â†“ç¹æ‚¶Îç¹ï½³ç¹å³¨ã›ç¹ãEãƒ»ç¹åŒ»E’ç¹ï½ªç¹§E»ç¹ãEãƒ¨
		SetBlendState(BLENDSTATE_NONE); // è­«E°é‚±å£¹â†‘ç¹§å³¨ãE¹ï½«ç¹è¼”ãƒç¸ºEªç¸ºåŠ±ã€’ç¹§EK

		for (int i = 0; i < PLAYER_MAX; i++)
		{
			if (!player[i].active) continue;

			// 4. æ¿¶E²ç¹§åµãç¹ãEãƒ¨ãƒ»ç£¯æ¨æ¿¶E²ç¸ºE«ç¸ºå¶E‹ç¸ºEªç¹§è‡¥E¬E¬4è ‘å¢“çEç¸ºE®ç¹§E¢ç¹ï½«ç¹è¼”ãƒç¹§ãƒ».0fç¸ºE«ãƒ»ãƒ»E¼ãƒ»
			Shader_SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

			// 5. è¬ å†—åˆ¤ãƒ»ãƒ»
			Debug_DrawAABB(player[i].boundingBox, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
		}
	}

	// ç¹§E«ç¹ï½¡ç¹ï½©ç¸ºä¹ï½‰ç¸ºE®é¬EEåˆE¹§åµãŸç¹ï½¼ç¹åŒ»E ç¸ºæº˜ï½‚ç¸ºE®(list)ç¸ºE®é¬EEåˆE¸ºE§èœ€æ¦Šï½ºE¦è¬ å†—åˆ¤
	// p.second ç«ŠãEç¹§E½ç¹ï½¼ç¹åŸŸE¸åŒ»âˆ©ç¸ºE®ç¹åŠ±Îç¹§E¤ç¹ï½¤ç¹ï½¼ç¹§E¤ç¹ï½³ç¹ãEãƒ£ç¹§E¯ç¹§E¹
	for (auto& p : list)
	{
		Player_DrawOutline(p.second);

		DrawPlayerInternal(p.second);
	}

	// ç¹§E·ç¹ï½«ç¹§E¨ç¹ãEãƒ¨è¬ å†—åˆ¤ç¹§å®šï½¿E½èœ‰ï£°
	for (auto& p : list) Player_DrawSilhouette(p.second);

	// 3Dç¹§Eªç¹æ‚¶ãšç¹§E§ç¹§E¯ç¹åŒ»ãƒ»è±ºE±è ï½¦ç¹ãEã›ç¹åŒ»E’è¾Ÿï½¡èœ‰ï½¹ç¸ºE«ç¸ºåŠ±â€»è¬ å†—åˆ¤
	SetDepthTest(false);
}

void Player_DrawHP()
{
	Shader_Begin();

	// è›Ÿå¥æŒ¨UIç¹§E¹ç¹ãEãƒ»ç¹§E¿ç¹§E¹è¬ å†—åˆ¤
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		SetBlendState(BLENDSTATE_ALPHA);

		// ç¹åŠ±Îç¹§E¤ç¹ï½¤ç¹ï½¼ç¸ºæ¢§E­E»ç¹§è–™ã€’ç¸ºãƒ»E‹ç¸ºä¹âEç¸ºãƒ»Â°ç¹§è²æEè³ãƒ»
		bool isDead = (!player[i].active && player[i].stock <= 0);

		DrawHP(&HPBar[i], i + 2, isDead);
		

		if (isDead)
		{// è±E½»ç¹§è–™â–¡ç¸ºE¨ç¸ºé˜ªãƒ»ç¸²âˆ«ãƒ»æ¿¶E²ç¸ºE®HPç¹èEãƒ»ç¹§å‘ˆï½®ä¹ï¼ ç¸ºE¦èœˆï½¨ç¸ºE¦ç¸ºE®UIç¹§å‘ˆï½¶åŒ»â˜E
			if (!Player_CanUseSpecial(i))
			{
				Effect_Clear(i);
			}
			continue;
		}

		XMFLOAT2 hp = HPBar[i].pos;

		// ç¹§E¹ç¹§E­ç¹ï½«ç¹§E²ç¹ï½¼ç¹§E¸é™¦E¨é‰ï½ºé€•ï½¨ç¸ºE®è›Ÿï½¤ç¹§å®šï½¨è‚²E®åŠ±â˜E¹§ãƒ»
		float skillFill = 1.0f;

		// ç¹§E¹ç¹§E­ç¹ï½«è­›ï½ªè¬E€è¬–âEâ†‘ç¹§ãƒ»
		if (player[i].type == PlayerType::None)
		{
			skillFill = 0.0f;
		}
		else
		{
			// ç¹§E¯ç¹ï½¼ç¹ï½«ç¹§E¿ç¹§E¤ç¹æ§­ãƒ»ç¸ºãƒ»ç¸ºEªç¹§ç‰™èŒœé€•ï½¨èœ¿E¯é–­E½
			if (player[i].skillCoolTimer <= 0.0f)
			{
				skillFill = 1.0f;
			}
			else
			{
				// typeç¸ºE«è ¢æ‡ŠÂ§ç¸ºæº˜ã‘ç¹ï½¼ç¹ï½«ç¹§E¿ç¹§E¤ç¹ï£°ç¹§è²å™è •ãE
				float coolTime = 0.0f;
				switch (player[i].type)
				{
				case PlayerType::Glass:			coolTime = SKILL_GLASS_COOLTIME; break;
				case PlayerType::Concrete:		coolTime = SKILL_CONCRETE_COOLTIME; break;
				case PlayerType::Plant:			coolTime = SKILL_PLANT_COOLTIME; break;
				case PlayerType::Electricity:	coolTime = SKILL_ELECTRICITY_COOLTIME; break;
				default: coolTime = 0.0f; break;
				}

				// ç¹§E¯ç¹ï½¼ç¹ï½«ç¹§E¿ç¹§E¤ç¹ï£°ç¸ºãƒ»ç¸ºE®è­ã‚…ãƒ»1.0fç¹§å®šï½¿æ–âE
				if (coolTime <= 0.0f)
				{
					skillFill = 1.0f;
				}
				else
				{
					// è´E¿é€•ï½¨é€¶E´è •å¾ŒÂ€Â€skillCoolTimer == coolTime => fill = 0.0
					// ç¹§E¯ç¹ï½¼ç¹ï½«é‚¨ã‚E½ºãƒ»Â€Â€skillCoolTimer == 0 => fill = 1.0
					skillFill = 1.0f - (player[i].skillCoolTimer / coolTime);
					if (skillFill < 0.0f) skillFill = 0.0f;
					if (skillFill > 1.0f) skillFill = 1.0f;
				}
			}
		}

		// é¨¾E²è›¹æ‚¶â€²è—ï½ºè³å£¹EE¹§å¾Œâ—†ç¹§å³¨Â€âˆšã¡ç¹§E¤ç¹åŠ±ãƒ»ç¹§E²ç¹ï½¼ç¹§E¸ç¹§å‘ˆæ€™èŸE§è›Ÿï½¤ç¸ºE§é™¦E¨é‰ï½ºç¸ºå¶EE
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
			// è—ï½ºè³å£¼ç‡•ç¸ºE¯ç¹§E«ç¹§E¦ç¹ï½³ç¹åŸŸç„šç¹§åµâ—ç¸ºE®ç¸ºE¾ç¸ºE¾é™¦E¨é‰ï½ºç¸ºå¶EE
			Gauge_Set(i, player[i].breakCount_Glass, player[i].breakCount_Concrete, player[i].breakCount_Plant, player[i].breakCount_Electricity,
				player[i].evolutionGauge, skillFill, { hp.x - GAUGE_POS_X , hp.y + GAUGE_POS_Y }, player[i].type);
		}

		// ç¹§E¹ç¹å£¹ã™ç¹ï½£ç¹ï½«è´E¿é€•ï½¨èœ¿E¯é–­E½ç¸ºEªç¹§å³¨ãŠç¹è¼”ã‰ç¹§E¯ç¹åŒ»E’é™¦E¨é‰ï½ºç¸²âˆšâ—ç¸ºãƒ»ã€’ç¸ºEªç¸ºä»£EŒç¸ºE°è±¸åŒ»â˜E
		if (Player_CanUseSpecial(i))
		{
			Shader_SetColor(color::white);
			Effect_Set(24, { (hp.x + 12.0f * SCREEN_ADJUST_X), hp.y - (100.0f * SCREEN_ADJUST_Y) }, { (162.0f * SCREEN_ADJUST_X), (60.0f * SCREEN_ADJUST_Y) }, i);
		}
		if (!Player_CanUseSpecial(i))
		{
			Effect_Clear(i);
		}

		// é¨¾å£¼E¸E¸ç¹§E²ç¹ï½¼ç¹§E¸ãƒ»äº¥ãƒ»ãƒ»å¥E¤åE½¼å³¨ãƒ»èŸ¶E¸ç¸ºE«è¬ å†—åˆ¤
		// ç¹§E¹ç¹§E­ç¹ï½«ç¹§E²ç¹ï½¼ç¹§E¸ç¸ºE¯è»æ¨ŠÂ€E§é’ï½ºè³å£¹ãƒ»ç¸ºE¨ç¸ºé˜ªãƒ»ç¸ºE¿è¬ å†—åˆ¤
		Gauge_DrawBasic(i);

		// è»æ¨ŠÂ€E§é’ï½ºè³å£¹E ç¸ºE¦ç¸ºãƒ»E‹ç¸ºE¨ç¸ºé˜ªãƒ»ç¹§E¹ç¹§E­ç¹ï½«UIç¹§ã‚ˆç·’é€•ï½»
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
	// é½ãƒ»å³E¹âEã‰ç¹ãEãE0 1 2 3 è‰ï½¥èŸæ‚¶â†‘ç¹§ãƒ»return
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	// è°¿åŒºE©æº˜â€²1ç¸ºE¤è‰ï½¥è³ç¿«â‰ ç¹§å¥E°E´èœ·ãƒ»
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
		player[playerIndex].lastDir = PlayerDir::Down; // è±E½£é«±E¢
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
// è°¿åŒºE©æ»“ç·’é€•ï½»
//==================================
void Player_DrawStock(int i)
{
	Shader_Begin();
	Shader_BeginUI();

	// HPç¹èEãƒ»è´å’²E½E®èœ¿é–€E¾åŠ±ãƒ»ç¹§E²ç¹ï½¼ç¹§E¸è ï½§è®“å‘µE¨E­è³ãƒ»
	float bx = HPBar[i].pos.x - (60.0f * SCREEN_ADJUST_X);
	float by = HPBar[i].pos.y + (60.0f * SCREEN_ADJUST_Y);

	// ç¹åŠ±Îç¹§E¤ç¹ï½¤ç¹ï½¼ç¸ºæ–âEç¸ºE®ç¹§E¹ç¹åŒ»ãƒ£ç¹§E¯è¬ å†—åˆ¤
	for (int j = 0; j < player[i].stock; j++)
	{
		// ç¹§E¹ç¹åŒ»ãƒ£ç¹§E¯è¬ å†—åˆ¤èŸç”»ç„E
		XMFLOAT2 pos = { bx + (j * 30.0f * SCREEN_ADJUST_X), by };	// è®“ï½ªè³E¦ç¸ºE³
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

		// ç¹åŠ±Îç¹§E¤ç¹ï½¤ç¹ï½¼ç¸ºæ–âEç¸ºE«æ¿¶E²éšªE­è³ãƒ»
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

		// ƒtƒHƒ“ƒgƒTƒCƒY‚Ì”¼•ª’ö“x¶‚É‚¸‚ç‚·
		float offsetX = 20.0f;

		DrawTextEx(
			playerLabel,
			player[p].screenPos.x - offsetX,
			player[p].screenPos.y - 20.0f,	// ƒeƒLƒXƒg‚Ì‚‚³•ªã‚É•\¦
			35.0f,							// ƒtƒHƒ“ƒgƒTƒCƒY
			L"Impact",
			textColor
		);
		//DrawTextEx(
		//	L"    â–½ ",
		//	player[p].screenPos.x - offsetX,
		//	player[p].screenPos.y + 13.0f,	// ãƒE‚­ã‚¹ãƒˆãEé«˜ã•åˆE¸Šã«è¡¨ç¤º
		//	15.0f,							// ãƒ•ã‚©ãƒ³ãƒˆã‚µã‚¤ã‚º
		//	L"Impact",
		//	textColor
		//);
	}
}

static void Ranking(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;
	// è ç¢ã¾é€‹ï½»éª­E²é«¦E²è±E½¢
	if (player[playerIndex].rank != 0) return;

	// è±E½»è E¡é¬EEâ†“éœ‘E½èœ‰ï£°
	g_deathOrder.push_back(playerIndex);
	size_t pos = g_deathOrder.size();

	// èœˆåŒ»â†“è±E½»ç¹§è–™â–¡ç¹åŠ±Îç¹§E¤ç¹ï½¤ç¹ï½¼ç¸ºå¾¡E½æœ±E°ãƒ»E½é˜ªâ†“ç¸ºEªç¹§å…·E¼ãƒ»os=1 -> 4è´æ¾E¼ãƒ»
	player[playerIndex].rank = PLAYER_MAX - (int)(pos - 1);

	// è­›Â€è •å¾ŒãEè³Â€è Eºç¸ºæªï½¢Eºè³å£¹E ç¸ºæº˜ï½‰è°¿ä¹ï½Šç¹§ãƒ»è´é˜ªâ†“ç¸ºå¶EE
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

		// èœæ™Â€ãƒ»E¢Eºè³ãƒ»ç«ŠãESCENE_WIN ç¸ºE¸é©•ï½·é˜ï½»
		if (GetFadeState() == FADE_NONE)
		{
			XMFLOAT4 color(0.0f, 0.0f, 0.0f, 0.0f);
			SetFade(60, color, FADE_OUT, SCENE_WIN);
		}
	}
}

PLAYEROBJECT* GetPlayer(int playerIndex)
{
	// é½ãƒ»å³E¹âEã‰ç¹ãEãE0 1 2 3 è‰ï½¥èŸæ‚¶â†‘ç¹§ãƒ»nullptr ç¹§å®šï½¿æ–âE
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX)	return nullptr;

	return &player[playerIndex];
}

void TriggerbyHPShake(int playerIndex, float amplitude, float duration, float speed)
{
	// é½ãƒ»å³E¹âEã‰ç¹ãEãE
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;


	SetHPShake(&HPBar[playerIndex], amplitude, duration, speed, playerIndex + 6);

}


bool Player_CanUseSpecial(int playerIndex)
{
	// é½ãƒ»å³E¹âEã‰ç¹ãEãE
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return false;

	PLAYEROBJECT& pl = player[playerIndex];

	if (!pl.active)		return false;
	if (pl.isStunning)	return false;
	if (pl.isDown)		return false;
	if (pl.rank == 1)	return false;

	// è –ï½¢è«·ä¹â€²éš¨E¬3è –ï½¢è«·ä¹ã€’ç¸ºã‚E½‹ç¸ºè–™âE
	if (pl.form != Form::Third) return false;

	// ç¹§E¿ç¹§E¤ç¹åŠ±â€²è­›ï½ªéšªE­è³å£¹â–¡ç¸ºE¨ç¹§E¹ç¹å£¹ã™ç¹ï½£ç¹ï½«ç¸ºå¾ŒâEç¸ºãƒ»Â°ç¹§å³¨ã¡ç¹§E¤ç¹åŠ±E‚ç¹âEã‰ç¹ãEãE
	if (pl.type == PlayerType::None) return false;

	// ç¸ºå¶âˆ‹ç¸ºE¦é¨¾å£¹â–²ç¸ºæº˜ï½‰true
	return true;
}
