// Effect.cpp

#include "Effect.h"
#include "sprite.h"
#include "shader.h"
#include "color.h"
#include "player.h"
#include "Camera.h"
#include "debug_render.h"
#include "Building.h"
#include "Audio.h"
#include "imgui.h"
#include "loadThread.h"

#define EFFECT_SPRITE_X		(8)
#define EFFECT_SPRITE_Y		(8)
#define EFFECT_FRAME_MAX	(64)
#define EFFECT_SPEED		(2.5f)
#define EFFECT_TEX_MAX		(30)
#define EFFECT_MAX			(30)

// 頂点配列
static Vertex2 effect_vdata[PLAYER_VERTEX] =
{
	{// 頂点0 LEFT-TOP
		XMFLOAT3(-COORDINATE, COORDINATE, 0.0f),	// 座標
		XMFLOAT3(0.0f, 0.0f, -1.0f),				// 法線ベクトル
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),			// カラー
		XMFLOAT2(0.0f, 0.0f)						// テクスチャ座標
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
static UINT effect_idxdata[6]
{
	 0, 1, 2, 2, 1, 3, // -Z面
};

// グローバル変数
// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// 頂点バッファ
static ID3D11Buffer* g_VertexBuffer = NULL;

// インデックスバッファ
static ID3D11Buffer* g_IndexBuffer = NULL;

static ID3D11ShaderResourceView* g_Texture[EFFECT_TEX_MAX] = {};
static bool g_ReleaseOwned[EFFECT_TEX_MAX] = {};
static int g_CurrentTexNo = 0;

static EFFECT effect[EFFECT_MAX];

static int g_EffectFrame = 0;
static int g_EffectTimer = 0;

static bool g_EffectLoopFlag = false;

static int   g_animFrame[PLAYER_MAX] = { 0 };
static float g_animTimer[PLAYER_MAX] = { 0.0f };
static const float ANIM_FRAME_TIME = 0.16f;	// 1フレームあたりの秒数

PLAYER_EFFECT_ANIM g_PlayerEffectAnim[PLAYER_MAX];
BUILDING_EFFECT_ANIM g_BuildingEffectAnim[BUILDING_EFFECT_MAX];

//static int g_SE_ID[10] = { NULL };

// テクスチャ番号ごとの設定
static EffectConfig g_EffectConfigs[EFFECT_TEX_MAX] = {
   // max, loopS, loopE, isLoop, speed, sprintY, scaleMin, scaleMax, scaleSpeed
	 { 32,     0,    30,   true,  1.5f,       8,     0.0f,     0.0f,       0.0f },
	 { 32,     0,    30,   true,  1.5f,       8,     0.0f,     0.0f,       0.0f },
	 { 32,     0,    30,   true,  1.5f,       8,     0.0f,     0.0f,       0.0f },
	 { 32,     0,    30,   true,  1.5f,       8,     0.0f,     0.0f,       0.0f },
	 { 32,     0,    30,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },
	 { 32,     0,    30,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },
	 { 32,     0,    30,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },
	 { 32,     0,    30,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },
	 { 64,     -1,   -1,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },  // スキル ガラス・コンクリート
	 { 64,     -1,   -1,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },  // スキル 植物
	 { 64,     -1,   -1,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },  // スキル 電気
	 { 64,     -1,   -1,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },  // 毒状態
	 { 64,     -1,   -1,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },  // ヒット コンクリートの建物・プレイヤーを攻撃した時
	 { 64,     -1,   -1,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },  // ヒット 電気・ガラス・植物の建物を攻撃した時
	 { 64,     -1,   -1,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },  // スペシャル コンクリート 地面の衝撃波
	 { 64,     -1,   -1,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },  // スペシャル 電気 衝撃波
	 { 64,     -1,   -1,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },  // 建物 煙 20%破壊
	 { 64,     -1,   -1,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },  // 建物 煙 50%破壊
	 { 64,     -1,   -1,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },  // 進化1
	 { 64,     -1,   -1,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },  // 進化2 進化1の直後に使用
	 { 64,     -1,   -1,   true,  1.0f,       8,     0.0f,     0.0f,       0.0f },  // 撃墜
	 { 32,     0,    30,   true,  0.8f,       4,     0.0f,     0.0f,       0.0f }, // UI 毒状態
	 { 32,     0,    30,   true,  0.8f,       4,     0.0f,     0.0f,       0.0f },
	 { 32,     0,    30,   true,  0.8f,       4,     0.0f,     0.0f,       0.0f },
	 { 1,      0,     0,   true,  0.0f,       1,     0.9f,     1.0f,       2.5f },
	 { 32,     0,    30,   true,  0.8f,       4,     0.0f,     0.0f,       0.0f },
	 { 1,      0,     0,   true,  0.0f,       1,     0.9f,     1.0f,       2.5f }
};


//===============================================
//　テクスチャセット用関数
//===============================================
static void Effect_LoadTexture(int i, const wchar_t* num)
{
	assert(i >= 0 && i < EFFECT_TEX_MAX);
	assert(g_pDevice);

	TexMetadata metadata{};
	ScratchImage image{};
	HRESULT hr = LoadFromWICFile(num, WIC_FLAGS_NONE, &metadata, image);
	assert(SUCCEEDED(hr));

	hr = CreateShaderResourceView(g_pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[i]);
	assert(SUCCEEDED(hr));
	assert(g_Texture[i]);

	g_ReleaseOwned[i] = true; // ← 追加：自前で読み込んだテクスチャは解放対象
}

//===============================================
//　初期化
//===============================================
void Effect_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		effect[i].enable = false;
		effect[i].pos = XMFLOAT3(0, 0, 0);
		effect[i].size = XMFLOAT2(0, 0);
		effect[i].frameCnt = 0;
		effect[i].texNo = 0;
		effect[i].playerIndex = -1;
		effect[i].scaleTimer = 0.0f;
		effect[i].scaleGrowing = true;
	}

	Loader::AddTask([pDevice]()
	{
	// UI画面
	Effect_LoadTexture(0, L"Asset\\Texture\\uiLightBigGlass_v1.png");			// 第2形態 ガラス
	Effect_LoadTexture(1, L"Asset\\Texture\\uiLightBigConcrete_v1.png");		// 第2形態 コンクリート
	Effect_LoadTexture(2, L"Asset\\Texture\\uiLightBigTree_v1.png");			// 第2形態 植物
	Effect_LoadTexture(3, L"Asset\\Texture\\uiLightBigElectricity_v1.png");		// 第2形態 電気
	Effect_LoadTexture(4, L"Asset\\Texture\\uiLightBigGlass_v1.png");			// 第3形態 ガラス
	Effect_LoadTexture(5, L"Asset\\Texture\\uiLightBigConcrete_v1.png");		// 第3形態 コンクリート
	Effect_LoadTexture(6, L"Asset\\Texture\\uiLightBigTree_v1.png");			// 第3形態 植物
	Effect_LoadTexture(7, L"Asset\\Texture\\uiLightBigElectricity_v1.png");		// 第3形態 電気
	// ゲーム内
	Effect_LoadTexture(8, L"Asset\\Texture\\effectSkillGlassConcrete_v4.png");	// スキル ガラス・コンクリート 回復
	Effect_LoadTexture(9, L"Asset\\Texture\\effectSkillTree_v4.png");			// スキル 植物
	Effect_LoadTexture(10, L"Asset\\Texture\\effectSkillElectricity_v2.png");	// スキル 電気
	Effect_LoadTexture(11, L"Asset\\Texture\\effectPoison_v3.png");				// 毒・Aボタン・プレイヤーの影
	Effect_LoadTexture(12, L"Asset\\Texture\\effectHit01_v4.png");				// ヒット コンクリート 建物・プレイヤーを攻撃した時 スタン
	Effect_LoadTexture(13, L"Asset\\Texture\\effectHit02_v2.png");				// ヒット 電気・ガラス・植物 建物を攻撃した時
	Effect_LoadTexture(14, L"Asset\\Texture\\effectShockwave_v1.png");			// 
	Effect_LoadTexture(15, L"Asset\\Texture\\effectSmoke_20per.png");			// 建物 煙 20%破壊
	Effect_LoadTexture(16, L"Asset\\Texture\\effectSmoke_50per.png");			// 建物 煙 50%破壊
	Effect_LoadTexture(17, L"Asset\\Texture\\effectEvolution01_v1.png");		// 進化1
	Effect_LoadTexture(18, L"Asset\\Texture\\effectEvolution02_v1.png");		// 進化2 進化1の直後に使用
	Effect_LoadTexture(19, L"Asset\\Texture\\effectWin_v1.png");				// 撃墜
	Effect_LoadTexture(20, L"Asset\\Texture\\effectEgg_v3.png");				// リスポーン 卵
	Effect_LoadTexture(21, L"Asset\\Texture\\effectVenomExplosion_v2.png");		// スペシャル 植物 毒煙

		Effect_LoadTexture(22, L"Asset\\Texture\\uiPoison_vx.png");
		Effect_LoadTexture(23, L"Asset\\Texture\\uiOrbit_v1.png");
		Effect_LoadTexture(24, L"Asset\\Texture\\special.png");
		Effect_LoadTexture(25, L"Asset\\Texture\\effectSmork02_v1.png");

	});

	//if (!g_isPlayerLoadingFinished && g_loadedCount == 0) return;
	

	// 頂点バッファ作成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));// 0でクリア
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex2) * PLAYER_VERTEX;// 格納できる頂点数*頂点サイズ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	g_pDevice = pDevice;
	g_pContext = pContext;

	// インデックスバッファ作成
	{
		D3D11_BUFFER_DESC	bd;
		ZeroMemory(&bd, sizeof(bd));	// 0でクリア
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(UINT) * 6;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&bd, NULL, &g_IndexBuffer);

		// インデックスバッファへ書き込み
		D3D11_MAPPED_SUBRESOURCE msr;
		pContext->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		UINT* index = (UINT*)msr.pData;

		// インデックスデータをバッファへコピー
		CopyMemory(&index[0], &effect_idxdata[0], sizeof(UINT) * 6);
		pContext->Unmap(g_IndexBuffer, 0);
	}
	// デバッグレンダラー初期化 
	Debug_Initialize(pDevice, pContext);

	// アニメーションの初期化
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		g_animFrame[i] = 0;
		g_animTimer[i] = 0.0f;
	}
}

void Effect_Warmup()
{
	if (!g_pContext) return;

	// ===== GPU テクスチャ ウォームアップ =====
	const size_t TEX_COUNT = sizeof(g_Texture) / sizeof(g_Texture[0]);
	for (size_t i = 0; i < TEX_COUNT; ++i)
	{
		if (g_Texture[i] != nullptr)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[i]);
			g_pContext->DrawIndexed(0, 0, 0);
		}
	}

	// 最後にリセットしておく
	ID3D11ShaderResourceView* nullSRV = nullptr;
	g_pContext->PSSetShaderResources(0, 1, &nullSRV);
}

//===============================================
//　終了
//===============================================
void Effect_Finalize()
{
	// テクスチャの解放
	for (int i = 0; i < EFFECT_TEX_MAX; i++)
	{
		if (g_Texture[i] && g_ReleaseOwned[i])
		{
			g_Texture[i]->Release();
		}
		g_Texture[i] = nullptr;
		g_ReleaseOwned[i] = false;
	}

	//for (int i = 0; i < 4; ++i)	UnloadAudio(g_SE_ID[i]);
}

//===============================================
//　更新
//===============================================
void Effect_Update()
{
	const float deltaTime = 1.0f / 60.0f; // フレーム時間

	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		if (!effect[i].enable) continue;

		int texNo = effect[i].texNo;
		const EffectConfig& config = g_EffectConfigs[texNo];

		// フレームを進める
		effect[i].frameCnt += (1.0f / EFFECT_SPEED) * config.speed;

		// ループor終了判定
		if (config.isLoop)
		{
			if (effect[i].frameCnt >= config.loopEnd)
			{
				effect[i].frameCnt = (float)config.loopStart;
			}
		}
		else
		{
			// ループしない時は、設定された最大を超えたら消滅
			if (effect[i].frameCnt >= config.loopEnd)
			{
				effect[i].enable = false; // 再生終了
			}
		}

		// スケーリングアニメーション
		if (config.scaleSpeed > 0.0f)
		{
			// サイン波で滑らかに拡大縮小
			effect[i].scaleTimer += deltaTime * config.scaleSpeed;
			float t = (sinf(effect[i].scaleTimer) + 1.0f) * 0.5f; // 0.0～1.0
			float scale = config.scaleMin + (config.scaleMax - config.scaleMin) * t;

			effect[i].size.x = effect[i].baseSize.x * scale;
			effect[i].size.y = effect[i].baseSize.y * scale;
		}
	}
}

//===============================================
//　描画
//===============================================
void Effect_Draw()
{
	if (!Loader::IsFinished) return;

	Shader_Begin();
	Shader_BeginUI();
	SetBlendState(BLENDSTATE_ALPHA);

	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		if (!effect[i].enable) continue;

		int texNo = effect[i].texNo;
		if (texNo < 0 || texNo >= EFFECT_TEX_MAX) continue;
		ID3D11ShaderResourceView* tex = g_Texture[texNo];
		if (!tex) continue;

		const EffectConfig& config = g_EffectConfigs[texNo];
		int currentFrame = (int)effect[i].frameCnt;

		if (config.spriteY == 1)
		{
			// UV座標全体を使用
			XMFLOAT2 uvMin = { 0.0f, 0.0f };
			XMFLOAT2 uvMax = { 1.0f, 1.0f };

			XMFLOAT2 pos = { effect[i].pos.x, effect[i].pos.y };
			XMFLOAT2 size = effect[i].size;

			g_pContext->PSSetShaderResources(0, 1, &tex);
			DrawSpriteUV(pos, size, color::white, uvMin, uvMax);
		}
		else
		{
			int fx = currentFrame % EFFECT_SPRITE_X;
			int fy = currentFrame / EFFECT_SPRITE_X;

			float u = 1.0f / (float)EFFECT_SPRITE_X;
			float v = 1.0f / (float)config.spriteY;

			XMFLOAT2 uvMin = { fx * u, fy * v };
			XMFLOAT2 uvMax = { uvMin.x + u, uvMin.y + v };

			XMFLOAT2 pos = { effect[i].pos.x, effect[i].pos.y };
			XMFLOAT2 size = effect[i].size;

			g_pContext->PSSetShaderResources(0, 1, &tex);
			DrawSpriteUV(pos, size, color::white, uvMin, uvMax);
		}
	}
}

// ===============================================
// プレイヤー付近に表示するエフェクト更新関数
// ===============================================
void Effect_UpdateForPlayer(int playerIndex)
{
	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// 進化エフェクト
	static bool evolutuionFrameInitialized[PLAYER_MAX] = { false };

	if (player.isEvolving)
	{
		if (!evolutuionFrameInitialized[playerIndex])
		{
			g_PlayerEffectAnim[playerIndex].evolutionFrame = 0;
			evolutuionFrameInitialized[playerIndex] = true;
		}

		auto& anim = g_PlayerEffectAnim[playerIndex];
		anim.evolutionTimer += DELTA_TIME;
		if (anim.evolutionPhase == 0)
		{
			anim.evolutionPhase = 1;			// 進化1テクスチャ 開始
			anim.evolutionFrame = 0;
		}
		if (anim.evolutionPhase == 1)
		{
			if (anim.evolutionTimer >= 0.03f)
			{
				anim.evolutionTimer = 0.0f;
				anim.evolutionFrame++;
				if (anim.evolutionFrame > 63)
				{
					anim.evolutionPhase = 2;	// 進化2テクスチャ 開始
					anim.evolutionFrame = 0;
				}
			}
		}
		else if (anim.evolutionPhase == 2)
		{
			if (anim.evolutionTimer >= 0.03f)
			{
				anim.evolutionTimer = 0.0f;
				anim.evolutionFrame++;
				if (anim.evolutionFrame > 47)
				{
					anim.evolutionPhase = 3;	// 終了
					anim.evolutionFrame = 0;
				}
			}
		}
	}
	else
	{
		evolutuionFrameInitialized[playerIndex] = false;
		g_PlayerEffectAnim[playerIndex].evolutionPhase = 0;
		g_PlayerEffectAnim[playerIndex].evolutionFrame = 0;
	}

	// スキルエフェクト （進化エフェクト中は更新しない）
	static bool skillFrameInitialized[PLAYER_MAX] = { false };

	if (player.useSkill && !player.isEvolving)
	{
		int skillStart = 0;
		int skillEnd = 0;
		bool useLoopRange = false;

		// 再生間隔（デフォルトは ANIM_FRAME_TIME。植物のみ高速化）
		float skillFrameInterval = ANIM_FRAME_TIME;

		switch (player.type)
		{
		case PlayerType::Glass:
			skillStart = 8;
			skillEnd = 15;
			if (!skillFrameInitialized[playerIndex])
			{
				g_PlayerEffectAnim[playerIndex].skillFrame = skillStart;
				skillFrameInitialized[playerIndex] = true;
			}
			break;
		case PlayerType::Concrete:
			skillStart = 0;
			skillEnd = 7;
			if (!skillFrameInitialized[playerIndex])
			{
				g_PlayerEffectAnim[playerIndex].skillFrame = skillStart;
				skillFrameInitialized[playerIndex] = true;
			}
			break;
		case PlayerType::Plant:
			skillFrameInterval = 0.15f;
			skillStart = 0;
			skillEnd = 63;
			if (!skillFrameInitialized[playerIndex])
			{
				g_PlayerEffectAnim[playerIndex].skillFrame = skillStart;
				skillFrameInitialized[playerIndex] = true;
			}
			break;
		case PlayerType::Electricity:
			// 電気は従来どおりループ範囲で再生
			useLoopRange = true;
			skillStart = 0;
			skillEnd = 62;
			// electricity は初期化を行わない既存の挙動を維持
			break;
		default:
			break;
		}

		// Glass / Concrete / Plant は個別初期化を保持するためここでの一括リセットを避ける
		if (player.type != PlayerType::Glass && player.type != PlayerType::Concrete && player.type != PlayerType::Plant)
			skillFrameInitialized[playerIndex] = false;

		g_PlayerEffectAnim[playerIndex].skillTimer += DELTA_TIME;
		if (g_PlayerEffectAnim[playerIndex].skillTimer >= skillFrameInterval)
		{
			g_PlayerEffectAnim[playerIndex].skillTimer = 0.0f;
			if (useLoopRange)	LoopRange(g_PlayerEffectAnim[playerIndex].skillFrame, skillStart, skillEnd, 1);
			else
			{
				if (g_PlayerEffectAnim[playerIndex].skillFrame < skillEnd)	g_PlayerEffectAnim[playerIndex].skillFrame++;
			}
		}
	}
	else	skillFrameInitialized[playerIndex] = false;	// スキルが終了したら初期化フラグをリセット

	// スペシャルエフェクト
	static bool specialFrameInitialized[PLAYER_MAX] = { false };

	if (player.useSpecial)
	{
		if (!specialFrameInitialized[playerIndex])
		{
			g_PlayerEffectAnim[playerIndex].specialFrame = 0;
			specialFrameInitialized[playerIndex] = true;
		}

		switch (player.type)
		{
		case PlayerType::Glass:
			break;
		case PlayerType::Concrete:
			break;
		case PlayerType::Plant:
			g_PlayerEffectAnim[playerIndex].specialTimer += DELTA_TIME;
			if (g_PlayerEffectAnim[playerIndex].specialTimer >= 0.017f)
			{
				g_PlayerEffectAnim[playerIndex].specialTimer = 0.0f;
				LoopRange(g_PlayerEffectAnim[playerIndex].specialFrame, 0, 63, 1);
			}
			break;
		case PlayerType::Electricity:
			break;
		default:
			break;
		}
	}
	else	specialFrameInitialized[playerIndex] = false;

	// 毒エフェクト
	if (player.isPoisoned)
	{
		g_PlayerEffectAnim[playerIndex].poisonTimer += DELTA_TIME;
		if (g_PlayerEffectAnim[playerIndex].poisonTimer >= ANIM_FRAME_TIME)
		{
			g_PlayerEffectAnim[playerIndex].poisonTimer = 0.0f;
			LoopRange(g_PlayerEffectAnim[playerIndex].poisonFrame, 0, 49, 1);
		}
	}

	// 死亡時の爆発エフェクト
	static bool explosionFrameInitialized[PLAYER_MAX] = { false };
	static bool explosionFinished[PLAYER_MAX] = { false };
	static bool cameraFocusStarted[PLAYER_MAX] = { false };

	bool isDeathConfirmed = (player.isDown && player.stock <= 1);

	if (isDeathConfirmed && !explosionFinished[playerIndex])
	{
		// 爆発エフェクト開始
		if (!explosionFrameInitialized[playerIndex])
		{
			g_PlayerEffectAnim[playerIndex].explosionFrame = 0;
			g_PlayerEffectAnim[playerIndex].explosionTimer = 0.0f;
			explosionFrameInitialized[playerIndex] = true;
		}
		// カメラがまだフォーカスしていない時はフォーカスを開始
		if (!cameraFocusStarted[playerIndex])
		{
			// カメラフォーカス開始
			Camera_FocusOnPlayer(playerIndex, 10.0f);
			cameraFocusStarted[playerIndex] = true;
		}

		g_PlayerEffectAnim[playerIndex].explosionTimer += DELTA_TIME;

		// 0.1秒ごとにフレームを進める
		if (g_PlayerEffectAnim[playerIndex].explosionTimer >= 0.1f)
		{
			g_PlayerEffectAnim[playerIndex].explosionTimer = 0.0f;
			g_PlayerEffectAnim[playerIndex].explosionFrame++;

			// エフェクトが最後まで再生されたら終了
			if (g_PlayerEffectAnim[playerIndex].explosionFrame >= 29)
			{
				g_PlayerEffectAnim[playerIndex].explosionFrame = 29;
				explosionFinished[playerIndex] = true;

				// エフェクト終了と同時にカメラフォーカスを解除
				if (cameraFocusStarted[playerIndex])
				{
					Camera_ReturnToNormal();
					cameraFocusStarted[playerIndex] = false;
				}
			}
		}
	}
	else if (player.active && player.stock > 1)
	{
		// リスポーンしたらリセット
		explosionFrameInitialized[playerIndex] = false;
		explosionFinished[playerIndex] = false;
		cameraFocusStarted[playerIndex] = false;
		g_PlayerEffectAnim[playerIndex].explosionFrame = 0;
	}

	// 被弾エフェクト
	static bool attackedFrameInitialized[PLAYER_MAX] = { false };

	if (player.isAttacked)
	{
		if (!attackedFrameInitialized[playerIndex])
		{
			g_PlayerEffectAnim[playerIndex].attackedFrame = 20; // 20からスタート
			attackedFrameInitialized[playerIndex] = true;
		}
		g_PlayerEffectAnim[playerIndex].attackedTimer += DELTA_TIME;
		if (g_PlayerEffectAnim[playerIndex].attackedTimer >= 0.05f)
		{
			g_PlayerEffectAnim[playerIndex].attackedTimer = 0.0f;
			LoopRange(g_PlayerEffectAnim[playerIndex].attackedFrame, 20, 18, 1);
		}
	}
	else	attackedFrameInitialized[playerIndex] = false;

	// スタンエフェクト
	static bool stunFrameInitialized[PLAYER_MAX] = { false };

	if (player.isStunning)
	{
		if (!stunFrameInitialized[playerIndex])
		{
			g_PlayerEffectAnim[playerIndex].stunFrame = 38; // 38からスタート
			stunFrameInitialized[playerIndex] = true;
		}
		g_PlayerEffectAnim[playerIndex].stunTimer += DELTA_TIME;
		if (g_PlayerEffectAnim[playerIndex].stunTimer >= 0.05f)
		{
			g_PlayerEffectAnim[playerIndex].stunTimer = 0.0f;
			LoopRange(g_PlayerEffectAnim[playerIndex].stunFrame, 38, 26, 1);
		}
	}
	else	stunFrameInitialized[playerIndex] = false;

	// 回復エフェクト （進化エフェクト中は更新しない）
	static bool healingFrameInitialized[PLAYER_MAX] = { false };

	if (player.isHealing && !player.isEvolving)
	{
		if (!healingFrameInitialized[playerIndex])
		{
			g_PlayerEffectAnim[playerIndex].healingFrame = 16; // 16からスタート
			healingFrameInitialized[playerIndex] = true;
		}

		g_PlayerEffectAnim[playerIndex].healingTimer += DELTA_TIME;
		if (g_PlayerEffectAnim[playerIndex].healingTimer >= ANIM_FRAME_TIME)
		{
			g_PlayerEffectAnim[playerIndex].healingTimer = 0.0f;
			LoopRange(g_PlayerEffectAnim[playerIndex].healingFrame, 16, 45, 1);
		}
	}
	else	healingFrameInitialized[playerIndex] = false;

	// リスポーン卵エフェクト
	static bool respawnFrameInitialized[PLAYER_MAX] = { false };
	static bool eggBreakingFrameInitialized[PLAYER_MAX] = { false };
	static bool eggBreakingFinished[PLAYER_MAX] = { false };

	// isEggBreakingがtrueのとき、各プレイヤーごとに一回だけ再生
	if (player.isEggBreaking && !eggBreakingFinished[playerIndex])
	{
		int start = playerIndex * 16 + 12;
		int end = start + 3;

		if (!eggBreakingFrameInitialized[playerIndex])
		{
			g_PlayerEffectAnim[playerIndex].respawnFrame = start;
			g_PlayerEffectAnim[playerIndex].respawnTimer = 0.0f;
			eggBreakingFrameInitialized[playerIndex] = true;
		}

		g_PlayerEffectAnim[playerIndex].respawnTimer += DELTA_TIME;
		if (g_PlayerEffectAnim[playerIndex].respawnTimer >= 0.1f)
		{
			g_PlayerEffectAnim[playerIndex].respawnTimer = 0.0f;
			g_PlayerEffectAnim[playerIndex].respawnFrame++;
			if (g_PlayerEffectAnim[playerIndex].respawnFrame > end)
			{
				g_PlayerEffectAnim[playerIndex].respawnFrame = end;
				eggBreakingFinished[playerIndex] = true;
			}
		}
		respawnFrameInitialized[playerIndex] = false; // ここでリセット
	}
	else if (player.duringRespawn)
	{
		eggBreakingFrameInitialized[playerIndex] = false;
		eggBreakingFinished[playerIndex] = false;

		// 各プレイヤーごとにループ範囲を決定
		int start = playerIndex * 16;
		int end = start + 11;

		if (!respawnFrameInitialized[playerIndex])
		{
			g_PlayerEffectAnim[playerIndex].respawnFrame = start;
			g_PlayerEffectAnim[playerIndex].respawnTimer = 0.0f;
			respawnFrameInitialized[playerIndex] = true;
		}

		g_PlayerEffectAnim[playerIndex].respawnTimer += DELTA_TIME;
		if (g_PlayerEffectAnim[playerIndex].respawnTimer >= 0.1f)
		{
			g_PlayerEffectAnim[playerIndex].respawnTimer = 0.0f;
			// ループ処理
			g_PlayerEffectAnim[playerIndex].respawnFrame++;
			if (g_PlayerEffectAnim[playerIndex].respawnFrame > end)
			{
				g_PlayerEffectAnim[playerIndex].respawnFrame = start;
			}
		}
	}
	else
	{
		respawnFrameInitialized[playerIndex] = false;
		eggBreakingFrameInitialized[playerIndex] = false;
		eggBreakingFinished[playerIndex] = false;
	}
}

//===============================================
// エフェクトセット
//===============================================
void Effect_Set(int texNo, XMFLOAT2 pos, XMFLOAT2 size, int playerIndex)
{
	if (texNo < 0 || texNo >= EFFECT_TEX_MAX) return;
	if (!g_Texture[texNo]) return;

	// 同じtexNo,playerIndexが既にあるならそれを再利用
	int slot = -1;
	bool isExisting = false; // 既存スロットかどうか

	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		if (!effect[i].enable) continue;
		if (effect[i].texNo != texNo) continue;
		if (effect[i].playerIndex != playerIndex) continue;
		slot = i;
		isExisting = true; // 既存のエフェクトみーっけ！
		break;
	}

	// 空きがない場合は空きスロットを探す
	if (slot < 0)
	{
		for (int i = 0; i < EFFECT_MAX; ++i)
		{
			if (!effect[i].enable)
			{
				slot = i;
				break;
			}
		}
	}

	if (slot < 0) return;

	effect[slot].enable = true;
	effect[slot].pos = XMFLOAT3(pos.x, pos.y, 0.0f);
	effect[slot].size = size;
	effect[slot].baseSize = size;
	effect[slot].texNo = texNo;
	effect[slot].playerIndex = playerIndex;
	effect[slot].scaleGrowing = true;

	// 新規作成の時だけframeCntとscaleTimerをリセット
	if (!isExisting)
	{
		effect[slot].frameCnt = 0.0f;
		effect[slot].scaleTimer = 0.0f;
	}
}


//===============================================
// エフェクト消去
//===============================================
void Effect_Clear(int pIndex)
{
	if (pIndex < 0 || pIndex >= PLAYER_MAX) return;

	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		if (!effect[i].enable) continue;

		// エフェクトが指定プレイヤーのものなら消去
		if (effect[i].playerIndex == pIndex)
		{
			effect[i].enable = false;
			effect[i].pos = XMFLOAT3(0, 0, 0);
			effect[i].size = XMFLOAT2(0, 0);
			effect[i].frameCnt = 0;
			effect[i].texNo = 0;
			effect[i].playerIndex = -1;  // 無効な値にリセット
		}
	}
}

//===============================================
// プレイヤーUIセット関数
//===============================================
void Effect_SetUI(int texNo, XMFLOAT2 pos, XMFLOAT2 size)
{
	if (texNo < 0 || texNo >= EFFECT_TEX_MAX) return;
	if (!g_Texture[texNo]) return;

	// 空きを探す
	int slot = -1;
	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		if (!effect[i].enable)
		{
			slot = i;
			break;
		}
	}

	if (slot < 0) return;

	effect[slot].enable = true;
	effect[slot].pos = XMFLOAT3(pos.x, pos.y, 0.0f);
	effect[slot].size = XMFLOAT2((size.x * SCREEN_ADJUST_X), (size.y * SCREEN_ADJUST_Y));
	effect[slot].frameCnt = 0;
	effect[slot].texNo = texNo;
}

//===============================================
// プレイヤーUIエフェクト消去
//===============================================
void Effect_ClearUI(int pIndex)
{
	float screenX = SCREEN_ADJUST_X;
	float screenY = 620.0f * SCREEN_ADJUST_Y;

	// プレイヤーごとのエフェクト位置
	const XMFLOAT2 playerEffectPos[4] =
	{
			{  170.0f * screenX, screenY },	// プレイヤー1
			{  490.0f * screenX, screenY },	// プレイヤー2
			{  810.0f * screenX, screenY },	// プレイヤー3
			{ 1130.0f * screenX, screenY }	// プレイヤー4
	};

	if (pIndex < 0 || pIndex >= 4) return;

	XMFLOAT2 targetPos = playerEffectPos[pIndex];

	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		if (!effect[i].enable) continue;

		if (fabsf(effect[i].pos.x - targetPos.x) < 1.0f && fabsf(effect[i].pos.y - targetPos.y) < 1.0f)
		{
			effect[i].enable = false;
		}
	}
}

// ===============================================
// プレイヤーの手前に表示するエフェクト描画関数
// ===============================================
void EffectFront_DrawForPlayer(int playerIndex)
{
	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	LIGHT light{};
	light.Enable = TRUE;
	light.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
	light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	Shader_SetLight(light);

	Shader_Begin();

	XMMATRIX projection = GetProjectionMatrix();
	XMMATRIX view = GetViewMatrix();

	const float spriteScale = 2.0f;
	XMMATRIX ScalingMatrix = XMMatrixScaling
	(
		player.scaling.x * spriteScale,
		player.scaling.y * spriteScale,
		player.scaling.z * spriteScale
	);

	XMMATRIX vm = GetViewMatrix();
	vm.r[3].m128_f32[0] = 0.0f;
	vm.r[3].m128_f32[1] = 0.0f;
	vm.r[3].m128_f32[2] = 0.0f;
	vm.r[3].m128_f32[3] = 1.0f;
	vm = XMMatrixTranspose(vm);
	vm.r[3].m128_f32[0] = player.position.x;
	vm.r[3].m128_f32[1] = player.position.y;
	vm.r[3].m128_f32[2] = player.position.z;
	vm.r[3].m128_f32[3] = 1.0f;

	XMMATRIX WorldMatrix = ScalingMatrix * vm;
	Shader_SetWorldMatrix(WorldMatrix);

	XMMATRIX WVP = ScalingMatrix * vm * view * projection;
	Shader_SetMatrix(WVP);

	// 頂点バッファにデータコピー
	D3D11_MAPPED_SUBRESOURCE msr;
	Vertex2 localV[PLAYER_VERTEX];
	CopyMemory(&localV[0], &effect_vdata[0], sizeof(Vertex2) * PLAYER_VERTEX);

	int frame = g_animFrame[playerIndex];
	int col = frame % EFFECT_SPRITE_X;
	int row = frame / EFFECT_SPRITE_X;
	float u0 = (float)col / (float)EFFECT_SPRITE_X;
	float v0 = (float)row / (float)EFFECT_SPRITE_Y;
	float u1 = u0 + 1.0f / (float)EFFECT_SPRITE_X;
	float v1 = v0 + 1.0f / (float)EFFECT_SPRITE_Y;

	localV[0].tex = XMFLOAT2(u0, v0);
	localV[1].tex = XMFLOAT2(u1, v0);
	localV[2].tex = XMFLOAT2(u0, v1);
	localV[3].tex = XMFLOAT2(u1, v1);

	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex2* vertex = (Vertex2*)msr.pData;
	CopyMemory(vertex, &localV[0], sizeof(Vertex2) * PLAYER_VERTEX);
	g_pContext->Unmap(g_VertexBuffer, 0);

	UINT stride = sizeof(Vertex2);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// --- 固定長配列で動的メモリ確保を排除 ---
	struct EffectEntry { int texNo; int frame; float scale; XMFLOAT3 offset; };
	EffectEntry entries[8]; // 最大同時8レイヤーで十分
	int entryCount = 0;

	auto addEntry = [&](int texNo, int f, float s, XMFLOAT3 o)
		{
			if (entryCount < 8) entries[entryCount++] = { texNo, f, s, o };
		};

	// 進化エフェクト
	if (player.isEvolving)
	{
		const auto& anim = g_PlayerEffectAnim[playerIndex];
		if (anim.evolutionPhase == 1) addEntry(17, anim.evolutionFrame, 3.0f, XMFLOAT3(0, -0.2f, 0));
		else if (anim.evolutionPhase == 2) addEntry(18, anim.evolutionFrame, 3.0f, XMFLOAT3(0, -0.2f, 0));
	}
	// 進化中はスキルエフェクト・回復エフェクト非表示
	else
	{
		// スキルエフェクト
		if (player.useSkill)
		{
			int texNo = -1;
			float scale = 1.0f;
			XMFLOAT3 ofs(0, 0, 0);
			switch (player.type)
			{
			case PlayerType::Glass:			texNo = 8;	scale = 1.5f; ofs = XMFLOAT3(0, 0, 0); break;
			case PlayerType::Concrete:		texNo = 8;	scale = 1.5f; ofs = XMFLOAT3(0, 0, 0); break;
			case PlayerType::Plant:			texNo = 9;	scale = 1.5f; ofs = XMFLOAT3(0, 0, 0); break;
			case PlayerType::Electricity:	texNo = 10;	scale = 1.5f; ofs = XMFLOAT3(0, 0, 0); break;
			default: break;
			}
			if (texNo >= 0) addEntry(texNo, g_PlayerEffectAnim[playerIndex].skillFrame, scale, ofs);
		}

		// 回復エフェクト
		if (player.isHealing)
		{
			addEntry(8, g_PlayerEffectAnim[playerIndex].healingFrame, 1.75f, XMFLOAT3(0.0f, 0.0f, 0.0f));
		}
	}
	// スペシャルエフェクト
	if (player.useSpecial)
	{
		int texNo = -1;
		float scale = 1.0f;
		XMFLOAT3 ofs(0, 0, 0);
		switch (player.type)
		{
		case PlayerType::Plant:			texNo = 21;	scale = 5.0f; ofs = XMFLOAT3(0, 0, 0);
			SetDepthTest(false);
			break;
		default: break;
		}
		if (texNo >= 0) addEntry(texNo, g_PlayerEffectAnim[playerIndex].specialFrame, scale, ofs);
	}
	// 死亡エフェクト
	if (player.isDown && player.stock <= 1)
	{
		addEntry(25, g_PlayerEffectAnim[playerIndex].explosionFrame, 3.0f, XMFLOAT3(0.0f, 0.0f, 0.0f));
	}
	// 毒状態エフェクト
	if (player.isPoisoned)
	{
		addEntry(11, g_PlayerEffectAnim[playerIndex].poisonFrame, 2.0f, XMFLOAT3(0.0f, 0.0f, 0.0f));
	}
	// 被弾エフェクト
	if (player.isAttacked)
	{
		addEntry(12, g_PlayerEffectAnim[playerIndex].attackedFrame, 1.2f, XMFLOAT3(0.0f, 0.5f, 0.0f));
	}
	// スタンエフェクト
	if (player.isStunning)
	{
		addEntry(12, g_PlayerEffectAnim[playerIndex].stunFrame, 2.2f, XMFLOAT3(0.0f, 0.0f, 0.0f));
	}
	// リスポーン卵エフェクト
	if (player.isEggBreaking)	// 卵割れエフェクト
	{
		addEntry(20, g_PlayerEffectAnim[playerIndex].respawnFrame, 5.0f, XMFLOAT3(0.0f, 0.0f, 0.0f));
	}
	else if (player.duringRespawn)	// 卵エフェクト
	{
		addEntry(20, g_PlayerEffectAnim[playerIndex].respawnFrame, 5.0f, XMFLOAT3(0.0f, 0.0f, 0.0f));
	}
	// 通常色を設定
	Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	// 条件に合致したテクスチャをすべて重ねて描画
	for (int e = 0; e < entryCount; ++e)
	{
		const EffectEntry& entry = entries[e];
		if (entry.texNo < 0 || entry.texNo >= EFFECT_TEX_MAX) continue;
		ID3D11ShaderResourceView* srv = g_Texture[entry.texNo];
		if (!srv) continue;

		Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

		int col = entry.frame % EFFECT_SPRITE_X;
		int row = entry.frame / EFFECT_SPRITE_X;
		float u0 = (float)col / (float)EFFECT_SPRITE_X;
		float v0 = (float)row / (float)EFFECT_SPRITE_Y;
		float u1 = u0 + 1.0f / (float)EFFECT_SPRITE_X;
		float v1 = v0 + 1.0f / (float)EFFECT_SPRITE_Y;

		localV[0].tex = XMFLOAT2(u0, v0);
		localV[1].tex = XMFLOAT2(u1, v0);
		localV[2].tex = XMFLOAT2(u0, v1);
		localV[3].tex = XMFLOAT2(u1, v1);

		XMMATRIX ScalingMatrix = XMMatrixScaling(
			player.scaling.x * spriteScale * entry.scale,
			player.scaling.y * spriteScale * entry.scale,
			player.scaling.z * spriteScale * entry.scale
		);

		XMMATRIX offsetMatrix = XMMatrixTranslation(entry.offset.x, entry.offset.y, entry.offset.z);
		XMMATRIX WorldMatrix = ScalingMatrix * offsetMatrix * vm;
		Shader_SetWorldMatrix(WorldMatrix);

		XMMATRIX WVP = ScalingMatrix * offsetMatrix * vm * view * projection;
		Shader_SetMatrix(WVP);

		g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		Vertex2* vertex = (Vertex2*)msr.pData;
		CopyMemory(vertex, &localV[0], sizeof(Vertex2) * PLAYER_VERTEX);
		g_pContext->Unmap(g_VertexBuffer, 0);

		g_pContext->PSSetShaderResources(0, 1, &srv);
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		g_pContext->DrawIndexed(6, 0, 0);
	}
}

// ==============================================
// プレイヤーの影エフェクト描画関数
// ==============================================
void EffectShadow_DrawForPlayer(int playerIndex)
{
	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// 影を非表示にする条件
	if (!player.duringRespawn && !player.isEggBreaking && !player.isShadowEnabled)	return;

	// シェーダーを設定
	Shader_Begin();

	// 影のY座標（地面の高さ）を決める
	float groundY = 0.0f;

	// 影の位置
	XMFLOAT3 shadowPos(player.position.x, groundY + 0.5f, player.position.z - 0.3f);

	// 影のワールド行列（XZ平面に平行、回転なし）
	XMMATRIX ScalingMatrix;

	float shadowScaling_x = player.scaling.x;
	float shadowScaling_z = player.scaling.z;

	if (player.duringRespawn)	ScalingMatrix = XMMatrixScaling(shadowScaling_x += 2.0f, 1.0f, shadowScaling_z += 6.0f);
	else						ScalingMatrix = XMMatrixScaling(shadowScaling_x, 1.0f, shadowScaling_z);

	XMMATRIX TranslationMatrix = XMMatrixTranslation(shadowPos.x + 0.2f, shadowPos.y, shadowPos.z);
	XMMATRIX WorldMatrix = ScalingMatrix * TranslationMatrix;

	XMMATRIX view = GetViewMatrix();
	XMMATRIX projection = GetProjectionMatrix();
	XMMATRIX WVP = WorldMatrix * view * projection;

	Shader_SetWorldMatrix(WorldMatrix);
	Shader_SetMatrix(WVP);

	// 頂点バッファにデータコピー
	D3D11_MAPPED_SUBRESOURCE msr;
	Vertex2 localV[PLAYER_VERTEX];
	CopyMemory(&localV[0], &effect_vdata[0], sizeof(Vertex2) * PLAYER_VERTEX);

	int frame = 51;
	int col = frame % EFFECT_SPRITE_X;
	int row = frame / EFFECT_SPRITE_X;
	float u0 = (float)col / (float)EFFECT_SPRITE_X;
	float v0 = (float)row / (float)EFFECT_SPRITE_Y;
	float u1 = u0 + 1.0f / (float)EFFECT_SPRITE_X;
	float v1 = v0 + 1.0f / (float)EFFECT_SPRITE_Y;

	localV[0].tex = XMFLOAT2(u0, v0);
	localV[1].tex = XMFLOAT2(u1, v0);
	localV[2].tex = XMFLOAT2(u0, v1);
	localV[3].tex = XMFLOAT2(u1, v1);

	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex2* vertex = (Vertex2*)msr.pData;
	CopyMemory(vertex, &localV[0], sizeof(Vertex2) * PLAYER_VERTEX);
	g_pContext->Unmap(g_VertexBuffer, 0);

	UINT stride = sizeof(Vertex2);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 影テクスチャ
	int texNo = 11;
	ID3D11ShaderResourceView* srv = g_Texture[texNo];
	if (!srv) return;

	Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	g_pContext->PSSetShaderResources(0, 1, &srv);
	g_pContext->DrawIndexed(6, 0, 0);
}

// ===============================================
// 建物付近に表示するエフェクト更新関数
// ===============================================
void Effect_UpdateForBuilding(int buildingIndex)
{
	int buildingCount = GetBuildingCount();
	Building** building = GetBuildings();
	if (!building) return;
	if (buildingIndex < 0 || buildingIndex >= buildingCount) return;
	if (buildingIndex >= BUILDING_EFFECT_MAX) return; // g_BuildingEffectAnim の配列サイズガード

	auto& anim = g_BuildingEffectAnim[buildingIndex];

	if (building[buildingIndex]->isDestroyed)
	{
		// 再生完了済みなら何もしない（1回だけ再生）
		if (anim.hitFinished) return;

		// 初回：建物タイプに応じたフレーム範囲を設定して再生開始
		if (!anim.hitPlaying)
		{
			switch (building[buildingIndex]->type)
			{
			case BuildingType::Concrete:	anim.hitStartFrame = 0;  anim.hitEndFrame = 19; break;	// コンクリート 0～19
			case BuildingType::Electricity:	anim.hitStartFrame = 0;  anim.hitEndFrame = 19; break;	// 電気         0～19
			case BuildingType::Glass:		anim.hitStartFrame = 20; anim.hitEndFrame = 39; break;	// ガラス       20～39
			case BuildingType::Plant:		anim.hitStartFrame = 40; anim.hitEndFrame = 59; break;	// 植物         40～59
			default: return;
			}
			anim.hitFrame = anim.hitStartFrame;
			anim.hitTimer = 0.0f;
			anim.hitPlaying = true;
		}

		// フレーム進行
		anim.hitTimer += DELTA_TIME;
		if (anim.hitTimer >= 0.05f)
		{
			anim.hitTimer = 0.0f;
			anim.hitFrame++;
			if (anim.hitFrame > anim.hitEndFrame)
			{
				// 最終フレームで停止し、再生完了
				anim.hitFrame = anim.hitEndFrame;
				anim.hitPlaying = false;
				anim.hitFinished = true;
			}
		}
	}
	else
	{
		// 破壊状態が解除されたらリセット
		anim.hitFrame = 0;
		anim.hitTimer = 0.0f;
		anim.hitPlaying = false;
		anim.hitFinished = false;
		anim.hitStartFrame = 0;
		anim.hitEndFrame = 0;
	}
}

// ===============================================
// 建物付近に表示するエフェクト描画関数
// ===============================================
void Effect_DrawForBuilding(int buildingIndex)
{
	int buildingCount = GetBuildingCount();
	Building** building = GetBuildings();
	if (!building) return;
	if (buildingIndex < 0 || buildingIndex >= buildingCount) return;
	if (buildingIndex >= BUILDING_EFFECT_MAX) return; // g_BuildingEffectAnim の配列サイズガード

	auto& anim = g_BuildingEffectAnim[buildingIndex];

	// 再生中のみ描画
	if (!anim.hitPlaying) return;

	// シェーダーを設定（頂点シェーダー未設定エラーの修正）
	Shader_Begin();

	int texNo = (building[buildingIndex]->type == BuildingType::Concrete) ? 12 : 13;

	XMFLOAT3 pos = building[buildingIndex]->position;
	float scale = 5.0f;

	int col = anim.hitFrame % EFFECT_SPRITE_X;
	int row = anim.hitFrame / EFFECT_SPRITE_X;
	float u0 = (float)col / (float)EFFECT_SPRITE_X;
	float v0 = (float)row / (float)EFFECT_SPRITE_Y;
	float u1 = u0 + 1.0f / (float)EFFECT_SPRITE_X;
	float v1 = v0 + 1.0f / (float)EFFECT_SPRITE_Y;

	Vertex2 localV[PLAYER_VERTEX];
	CopyMemory(&localV[0], &effect_vdata[0], sizeof(Vertex2) * PLAYER_VERTEX);
	localV[0].tex = XMFLOAT2(u0, v0);
	localV[1].tex = XMFLOAT2(u1, v0);
	localV[2].tex = XMFLOAT2(u0, v1);
	localV[3].tex = XMFLOAT2(u1, v1);

	XMMATRIX ScalingMatrix = XMMatrixScaling(scale, scale, scale);

	// ★ ビルボード処理（カメラに常に正対させる）
	XMMATRIX vm = GetViewMatrix();
	vm.r[3].m128_f32[0] = 0.0f;
	vm.r[3].m128_f32[1] = 0.0f;
	vm.r[3].m128_f32[2] = 0.0f;
	vm.r[3].m128_f32[3] = 1.0f;
	vm = XMMatrixTranspose(vm);

	vm.r[3].m128_f32[0] = pos.x;
	vm.r[3].m128_f32[1] = pos.y + 2.0f;  // 建物の少し上に配置
	vm.r[3].m128_f32[2] = pos.z;
	vm.r[3].m128_f32[3] = 1.0f;

	XMMATRIX WorldMatrix = ScalingMatrix * vm;
	Shader_SetWorldMatrix(WorldMatrix);

	XMMATRIX view = GetViewMatrix();
	XMMATRIX projection = GetProjectionMatrix();
	XMMATRIX WVP = WorldMatrix * view * projection;
	Shader_SetMatrix(WVP);

	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex2* vertex = (Vertex2*)msr.pData;
	CopyMemory(vertex, &localV[0], sizeof(Vertex2) * PLAYER_VERTEX);
	g_pContext->Unmap(g_VertexBuffer, 0);

	UINT stride = sizeof(Vertex2);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	g_pContext->PSSetShaderResources(0, 1, &g_Texture[texNo]);
	g_pContext->DrawIndexed(6, 0, 0);
}

//// ===============================================
//// 建物付近に表示するエフェクト一括更新関数
//// ===============================================
//void Effect_UpdateAllBuildings()
//{
//	int buildingCount = GetBuildingCount();
//	Building** building = GetBuildings();
//	if (!building) return;
//	if (buildingCount <= 0) return;
//
//	int loopCount = (buildingCount < BUILDING_EFFECT_MAX) ? buildingCount : BUILDING_EFFECT_MAX;
//
//	for (int i = 0; i < loopCount; ++i)
//	{
//		auto& anim = g_BuildingEffectAnim[i];
//
//		if (building[i]->isDestroyed)
//		{
//			// 再生完了済みならスキップ
//			if (anim.hitFinished) continue;
//
//			// 初回：建物タイプに応じたフレーム範囲を設定して再生開始
//			if (!anim.hitPlaying)
//			{
//				switch (building[i]->type)
//				{
//				case BuildingType::Concrete:	anim.hitStartFrame = 0;  anim.hitEndFrame = 19; break;
//				case BuildingType::Electricity:	anim.hitStartFrame = 0;  anim.hitEndFrame = 19; break;
//				case BuildingType::Glass:		anim.hitStartFrame = 20; anim.hitEndFrame = 39; break;
//				case BuildingType::Plant:		anim.hitStartFrame = 40; anim.hitEndFrame = 59; break;
//				default: continue;
//				}
//				anim.hitFrame = anim.hitStartFrame;
//				anim.hitTimer = 0.0f;
//				anim.hitPlaying = true;
//			}
//
//			// フレーム進行
//			anim.hitTimer += DELTA_TIME;
//			if (anim.hitTimer >= 0.05f)
//			{
//				anim.hitTimer = 0.0f;
//				anim.hitFrame++;
//				if (anim.hitFrame > anim.hitEndFrame)
//				{
//					anim.hitFrame = anim.hitEndFrame;
//					anim.hitPlaying = false;
//					anim.hitFinished = true;
//				}
//			}
//		}
//		else
//		{
//			// 破壊状態が解除されたらリセット
//			anim.hitFrame = 0;
//			anim.hitTimer = 0.0f;
//			anim.hitPlaying = false;
//			anim.hitFinished = false;
//			anim.hitStartFrame = 0;
//			anim.hitEndFrame = 0;
//		}
//	}
//}

//// ===============================================
//// 建物エフェクト一括描画関数
//// ===============================================
//void Effect_DrawAllBuildings()
//{
//	int buildingCount = GetBuildingCount();
//	Building** building = GetBuildings();
//	if (!building) return;
//	if (buildingCount <= 0) return;
//
//	int loopCount = (buildingCount < BUILDING_EFFECT_MAX) ? buildingCount : BUILDING_EFFECT_MAX;
//
//	// ビュー・射影行列はループ外で1回だけ取得
//	XMMATRIX view = GetViewMatrix();
//	XMMATRIX projection = GetProjectionMatrix();
//
//	// ビルボード用の回転行列もループ外で1回だけ計算
//	XMMATRIX billboardRot = view;
//	billboardRot.r[3].m128_f32[0] = 0.0f;
//	billboardRot.r[3].m128_f32[1] = 0.0f;
//	billboardRot.r[3].m128_f32[2] = 0.0f;
//	billboardRot.r[3].m128_f32[3] = 1.0f;
//	billboardRot = XMMatrixTranspose(billboardRot);
//
//	// パイプライン設定もループ外で1回だけ
//	UINT stride = sizeof(Vertex2);
//	UINT offset = 0;
//	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
//	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
//	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
//
//	for (int i = 0; i < loopCount; ++i)
//	{
//		auto& anim = g_BuildingEffectAnim[i];
//
//		// 再生中でなければスキップ（早期リターン）
//		if (!anim.hitPlaying) continue;
//
//		int texNo = (building[i]->type == BuildingType::Concrete) ? 12 : 13;
//
//		XMFLOAT3 pos = building[i]->position;
//		float scale = 5.0f;
//
//		int col = anim.hitFrame % EFFECT_SPRITE_X;
//		int row = anim.hitFrame / EFFECT_SPRITE_X;
//		float u0 = (float)col / (float)EFFECT_SPRITE_X;
//		float v0 = (float)row / (float)EFFECT_SPRITE_Y;
//		float u1 = u0 + 1.0f / (float)EFFECT_SPRITE_X;
//		float v1 = v0 + 1.0f / (float)EFFECT_SPRITE_Y;
//
//		Vertex2 localV[PLAYER_VERTEX];
//		CopyMemory(&localV[0], &effect_vdata[0], sizeof(Vertex2) * PLAYER_VERTEX);
//		localV[0].tex = XMFLOAT2(u0, v0);
//		localV[1].tex = XMFLOAT2(u1, v0);
//		localV[2].tex = XMFLOAT2(u0, v1);
//		localV[3].tex = XMFLOAT2(u1, v1);
//
//		XMMATRIX ScalingMatrix = XMMatrixScaling(scale, scale, scale);
//
//		// ビルボード：ループ外で計算済みの回転行列に位置だけ設定
//		XMMATRIX vm = billboardRot;
//		vm.r[3].m128_f32[0] = pos.x;
//		vm.r[3].m128_f32[1] = pos.y + 2.0f;
//		vm.r[3].m128_f32[2] = pos.z;
//		vm.r[3].m128_f32[3] = 1.0f;
//
//		XMMATRIX WorldMatrix = ScalingMatrix * vm;
//		Shader_SetWorldMatrix(WorldMatrix);
//
//		XMMATRIX WVP = WorldMatrix * view * projection;
//		Shader_SetMatrix(WVP);
//
//		D3D11_MAPPED_SUBRESOURCE msr;
//		g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
//		Vertex2* vertex = (Vertex2*)msr.pData;
//		CopyMemory(vertex, &localV[0], sizeof(Vertex2) * PLAYER_VERTEX);
//		g_pContext->Unmap(g_VertexBuffer, 0);
//
//		g_pContext->PSSetShaderResources(0, 1, &g_Texture[texNo]);
//		g_pContext->DrawIndexed(6, 0, 0);
//	}
//}