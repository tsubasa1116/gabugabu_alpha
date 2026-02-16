// Effect.cpp

#include "Effect.h"
#include "sprite.h"
#include "shader.h"
#include "color.h"
#include "player.h"
#include "Camera.h"
#include "debug_render.h"

#define EFFECT_SPRITE_X		(8)
#define EFFECT_SPRITE_Y		(8)
#define EFFECT_FRAME_MAX	(64)
#define EFFECT_SPEED		(2.5f)
#define EFFECT_TEX_MAX		(24)
#define EFFECT_MAX			(24)

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
static const float ANIM_FRAME_TIME = 0.15f;	// 1フレームあたりの秒数

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
	}

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
	Effect_LoadTexture(9, L"Asset\\Texture\\effectSkillTree_v2.png");			// スキル 植物
	Effect_LoadTexture(10, L"Asset\\Texture\\effectSkillElectricity_v2.png");	// スキル 電気
	Effect_LoadTexture(11, L"Asset\\Texture\\effectPoison_v2.png");				// 毒状態
	Effect_LoadTexture(12, L"Asset\\Texture\\effectHit01_v2.png");				// ヒット コンクリートの建物・プレイヤーを攻撃した時
	Effect_LoadTexture(13, L"Asset\\Texture\\effectHit02_v2.png");				// ヒット 電気・ガラス・植物の建物を攻撃した時
	Effect_LoadTexture(14, L"Asset\\Texture\\effectShockwave_v1.png");			// 
	Effect_LoadTexture(15, L"Asset\\Texture\\effectSmoke_20per.png");			// 建物 煙 20%破壊
	Effect_LoadTexture(16, L"Asset\\Texture\\effectSmoke_50per.png");			// 建物 煙 50%破壊
	Effect_LoadTexture(17, L"Asset\\Texture\\effectEvolution01_v1.png");		// 進化1
	Effect_LoadTexture(18, L"Asset\\Texture\\effectEvolution02_v1.png");		// 進化2 進化1の直後に使用
	Effect_LoadTexture(19, L"Asset\\Texture\\effectWin_v1.png");				// 撃墜
	Effect_LoadTexture(20, L"Asset\\Texture\\effectEgg_v3.png");				// リスポーン 卵
	//Effect_LoadTexture(21, L"Asset\\Texture\\effectEgg_v3.png");				// リスポーン 卵 影
	Effect_LoadTexture(22, L"Asset\\Texture\\effectRun_v1.png");				// 埃
	Effect_LoadTexture(23, L"Asset\\Texture\\effectVenomExplosion.png");		// 毒スペシャル

	// 頂点バッファ作成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));// 0でクリア
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * PLAYER_VERTEX;// 格納できる頂点数*頂点サイズ
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
		bd.ByteWidth = sizeof(UINT) * 6 * 6;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&bd, NULL, &g_IndexBuffer);

		// インデックスバッファへ書き込み
		D3D11_MAPPED_SUBRESOURCE msr;
		pContext->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		UINT* index = (UINT*)msr.pData;

		// インデックスデータをバッファへコピー
		CopyMemory(&index[0], &effect_idxdata[0], sizeof(UINT) * 6 * 6);
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

//===============================================
//　終了
//===============================================
void Effect_Finalize()
{
	for (int i = 0; i < EFFECT_MAX; i++)
	{
		if (g_Texture[i] && g_ReleaseOwned[i])
		{
			g_Texture[i]->Release();
		}
		g_Texture[i] = nullptr;
		g_ReleaseOwned[i] = false;
	}
}

//===============================================
//　更新
//===============================================
void Effect_Update()
{
	g_EffectTimer++;

	if (g_EffectTimer >= EFFECT_SPEED)
	{
		g_EffectTimer = 0;
		g_EffectFrame++;

		if (!g_EffectLoopFlag)
		{
			g_EffectTimer++;
			if (g_EffectFrame > 29)
			{
				g_EffectLoopFlag = true;
				g_EffectFrame = 32;
			}
			if (g_EffectFrame >= EFFECT_FRAME_MAX)
			{
				g_EffectFrame = 0;
			}
		}
		else
		{
			// ループ
			g_EffectFrame++;
			if (g_EffectFrame >= 61)
			{
				g_EffectFrame = 32;
			}
		}
	}
}

//===============================================
//　描画
//===============================================
void Effect_Draw()
{
	if (g_CurrentTexNo < 0 || g_CurrentTexNo >= EFFECT_TEX_MAX) return;
	ID3D11ShaderResourceView* tex = g_Texture[g_CurrentTexNo];
	if (!tex) return;

	int fx = g_EffectFrame % EFFECT_SPRITE_X;
	int fy = g_EffectFrame / EFFECT_SPRITE_X;

	float u = 1.0f / EFFECT_SPRITE_X;
	float v = 1.0f / EFFECT_SPRITE_Y;

	XMFLOAT2 uvMin = { fx * u, fy * v };
	XMFLOAT2 uvMax = { uvMin.x + u, uvMin.y + v };

	// シェーダーを描画パイプラインに設定
	Shader_Begin();
	Shader_BeginUI();

	Shader_SetColor({ 1.0f,1.0f,1.0f,1.0f });

	SetBlendState(BLENDSTATE_ALPHA);

	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		if (!effect[i].enable) continue;

		int texNo = effect[i].texNo;
		if (texNo < 0 || texNo >= EFFECT_TEX_MAX) continue;
		ID3D11ShaderResourceView* tex = g_Texture[texNo];
		if (!tex) continue;

		XMFLOAT2 pos = { effect[i].pos.x, effect[i].pos.y };
		XMFLOAT2 size = effect[i].size;

		g_pContext->PSSetShaderResources(0, 1, &tex);
		DrawSpriteUV(pos, size, color::white, uvMin, uvMax);
	}
}

//===============================================
//　セット関数
//===============================================
void Effect_Set(int texNo, XMFLOAT2 pos, XMFLOAT2 size)
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
	effect[slot].size = size;
	effect[slot].frameCnt = 0;
	effect[slot].texNo = texNo;
}

//===============================================
//　エフェクト消去
//===============================================
void Effect_Clear(int pIndex)
{
	// プレイヤーごとのエフェクト位置
	const XMFLOAT2 playerEffectPos[4] =
	{
		{  175.0f, 620.0f },	// プレイヤー1
		{  490.0f, 620.0f },	// プレイヤー2
		{  805.0f, 620.0f },	// プレイヤー3
		{ 1120.0f, 620.0f }		// プレイヤー4
	};

	if (pIndex < 0 || pIndex >= 4) return;

	XMFLOAT2 targetPos = playerEffectPos[pIndex];

	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		if (!effect[i].enable) continue;

		// 位置が一致するエフェクトを無効化
		if (fabsf(effect[i].pos.x - targetPos.x) < 1.0f && fabsf(effect[i].pos.y - targetPos.y) < 1.0f)
		{
			effect[i].enable = false;
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
	if (player.isInvincible)
	{
		auto& anim = g_effectAnim[playerIndex];
		anim.evolutionTimer += DELTA_TIME;
		if (anim.evolutionPhase == 0)
		{
			anim.evolutionPhase = 1;			// 進化1テクスチャ 開始
			anim.evolutionFrame = 0;
		}
		if (anim.evolutionPhase == 1)
		{
			if (anim.evolutionTimer >= ANIM_FRAME_TIME)
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
			if (anim.evolutionTimer >= ANIM_FRAME_TIME)
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

	// スキルエフェクト（進化中は更新しない）
	static bool skillFrameInitialized[PLAYER_MAX] = { false };

	if (player.useSkill && !player.isInvincible)
	{
		int skillStart = 0;
		int skillEnd = 0;
		bool useLoopRange = false;

		switch (player.type)
		{
		case PlayerType::Glass:
			skillStart = 8;
			skillEnd = 15;
			if (!skillFrameInitialized[playerIndex])
			{
				g_effectAnim[playerIndex].skillFrame = skillStart;
				skillFrameInitialized[playerIndex] = true;
			}
			break;
		case PlayerType::Concrete:
			skillStart = 0;
			skillEnd = 7;
			break;
		case PlayerType::Plant:
			useLoopRange = true;
			skillStart = 0;
			skillEnd = 54;
			break;
		case PlayerType::Electricity:
			useLoopRange = true;
			skillStart = 0;
			skillEnd = 62;
			break;
		default:
			break;
		}
		if (player.type != PlayerType::Glass)	skillFrameInitialized[playerIndex] = false;

		g_effectAnim[playerIndex].skillTimer += DELTA_TIME;
		if (g_effectAnim[playerIndex].skillTimer >= ANIM_FRAME_TIME)
		{
			g_effectAnim[playerIndex].skillTimer = 0.0f;
			if (useLoopRange)	LoopRange(g_effectAnim[playerIndex].skillFrame, skillStart, skillEnd, 1);
			else
			{
				if (g_effectAnim[playerIndex].skillFrame < skillEnd)	g_effectAnim[playerIndex].skillFrame++;
			}
		}
	}
	else	skillFrameInitialized[playerIndex] = false;

	// スペシャルエフェクト
	if (player.useSpecial)
	{
		switch (player.type)
		{
		case PlayerType::Glass:
			break;
		case PlayerType::Concrete:
			break;
		case PlayerType::Plant:
			g_effectAnim[playerIndex].specialTimer += DELTA_TIME;
			if (g_effectAnim[playerIndex].specialTimer >= ANIM_FRAME_TIME)
			{
				g_effectAnim[playerIndex].specialTimer = 0.0f;
				LoopRange(g_effectAnim[playerIndex].specialFrame, 0, 23, 1);
			}
			break;
		case PlayerType::Electricity:
			break;
		default:
			break;
		}
	}

	// 毒エフェクト
	if (player.isPoisoned)
	{
		g_effectAnim[playerIndex].poisonTimer += DELTA_TIME;
		if (g_effectAnim[playerIndex].poisonTimer >= ANIM_FRAME_TIME)
		{
			g_effectAnim[playerIndex].poisonTimer = 0.0f;
			LoopRange(g_effectAnim[playerIndex].poisonFrame, 0, 49, 1);
		}
	}

	// 被弾エフェクト
	static bool attackedFrameInitialized[PLAYER_MAX] = { false };

	if (player.isAttacked)
	{
		if (!attackedFrameInitialized[playerIndex])
		{
			g_effectAnim[playerIndex].attackedFrame = 21; // 21からスタート
			attackedFrameInitialized[playerIndex] = true;
		}
		g_effectAnim[playerIndex].attackedTimer += DELTA_TIME;
		if (g_effectAnim[playerIndex].attackedTimer >= ANIM_FRAME_TIME)
		{
			g_effectAnim[playerIndex].attackedTimer = 0.0f;
			LoopRange(g_effectAnim[playerIndex].attackedFrame, 21, 37, 1);
		}
	}
	else	attackedFrameInitialized[playerIndex] = false;

	// 回復エフェクト
	static bool healingFrameInitialized[PLAYER_MAX] = { false };

	if (player.isHealing)
	{
		if (!healingFrameInitialized[playerIndex])
		{
			g_effectAnim[playerIndex].healingFrame = 16; // 16からスタート
			healingFrameInitialized[playerIndex] = true;
		}

		g_effectAnim[playerIndex].healingTimer += DELTA_TIME;
		if (g_effectAnim[playerIndex].healingTimer >= ANIM_FRAME_TIME)
		{
			g_effectAnim[playerIndex].healingTimer = 0.0f;
			LoopRange(g_effectAnim[playerIndex].healingFrame, 16, 45, 1);
		}
	}
	else	healingFrameInitialized[playerIndex] = false;

	// リスポーン 卵 エフェクト
	static bool respawnFrameInitialized[PLAYER_MAX] = { false };

	if (player.duringRespawn)
	{
		// フラグがtrueになった瞬間だけ初期化
		if (!respawnFrameInitialized[playerIndex])
		{
			g_effectAnim[playerIndex].respawnFrame = playerIndex * 16;
			g_effectAnim[playerIndex].respawnTimer = 0.0f;
			respawnFrameInitialized[playerIndex] = true;
		}

		g_effectAnim[playerIndex].respawnTimer += DELTA_TIME;
		if (g_effectAnim[playerIndex].respawnTimer >= ANIM_FRAME_TIME)
		{
			g_effectAnim[playerIndex].respawnTimer = 0.0f;
			int start = playerIndex * 16;
			int end = start + 15;
			LoopRange(g_effectAnim[playerIndex].respawnFrame, start, end, 1);
		}
	}
	else	respawnFrameInitialized[playerIndex] = false;

	//// 移動時の埃 8コマ
	//if (player.isMoving == true)
	//{
	//	// 左下 8～15
	//		 if (player.moveDir.x < 0.0f && player.moveDir.z < 0.0f)	LoopRange(g_effectAnim[playerIndex].runDustFrame, 8, 8, 1);
	//	// 左上 24～31
	//	else if (player.moveDir.x < 0.0f && player.moveDir.z > 0.0f)	LoopRange(g_effectAnim[playerIndex].runDustFrame, 24, 8, 1);
	//	// 右上 40～47
	//	else if (player.moveDir.x > 0.0f && player.moveDir.z > 0.0f)	LoopRange(g_effectAnim[playerIndex].runDustFrame, 40, 8, 1);
	//	// 右下 56～63
	//	else if (player.moveDir.x > 0.0f && player.moveDir.z < 0.0f)	LoopRange(g_effectAnim[playerIndex].runDustFrame, 56, 8, 1);
	//	// 下   0～7
	//	else if (player.moveDir.z < 0.0f)	LoopRange(g_effectAnim[playerIndex].runDustFrame, 0, 8, 1);
	//	// 左   16～23
	//	else if (player.moveDir.x < 0.0f)	LoopRange(g_effectAnim[playerIndex].runDustFrame, 16, 8, 1);
	//	// 上   32～39
	//	else if (player.moveDir.z > 0.0f)	LoopRange(g_effectAnim[playerIndex].runDustFrame, 32, 8, 1);
	//	// 右   48～55
	//	else if (player.moveDir.x > 0.0f)	LoopRange(g_effectAnim[playerIndex].runDustFrame, 48, 8, 1);
	//}
}

// ===============================================
// プレイヤー付近に表示するエフェクト描画関数
// ===============================================
void Effect_DrawForPlayer(int playerIndex)
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

	// --- 条件に応じて複数のテクスチャを指定 ---
	std::vector<std::tuple<int, int, float, XMFLOAT3>> texNosFramesScales;

	// 進化エフェクト
	if (player.isInvincible)
	{
		const auto& anim = g_effectAnim[playerIndex];
			 if (anim.evolutionPhase == 1) texNosFramesScales.emplace_back(18, anim.evolutionFrame, 3.0f, XMFLOAT3(0,0,0));
		else if (anim.evolutionPhase == 2) texNosFramesScales.emplace_back(19, anim.evolutionFrame, 3.0f, XMFLOAT3(0,0,0));
	}
	// スキルエフェクト
	else if (player.useSkill)
	{
		int texNo = -1;
		float scale = 1.0f;
		XMFLOAT3 offset(0,0,0);
		switch (player.type)
		{
		case PlayerType::Glass:			texNo = 8;	scale = 1.0f; offset = XMFLOAT3(0,0,0); break;
		case PlayerType::Concrete:		texNo = 8;	scale = 1.0f; offset = XMFLOAT3(0,0,0); break;
		case PlayerType::Plant:			texNo = 9;	scale = 1.0f; offset = XMFLOAT3(0,0,0); break;
		case PlayerType::Electricity:	texNo = 10;	scale = 1.0f; offset = XMFLOAT3(0,0,0); break;
		default: break;
		}
		if (texNo >= 0) texNosFramesScales.emplace_back(texNo, g_effectAnim[playerIndex].skillFrame, scale, offset);
	}
	// スペシャルエフェクト
	if (player.useSpecial)
	{
		int texNo = -1;
		float scale = 1.0f;
		XMFLOAT3 offset(0,0,0);
		switch (player.type)
		{
		//case PlayerType::Glass:			texNo = 8;	scale = 1.0f; offset = XMFLOAT3(0,0,0); break;
		//case PlayerType::Concrete:		texNo = 8;	scale = 0.8f; offset = XMFLOAT3(0,0,0); break;
		case PlayerType::Plant:			texNo = 23;	scale = 9.0f; offset = XMFLOAT3(0,0,0);
			SetDepthTest(false);
			break;
		//case PlayerType::Electricity:	texNo = 10;	scale = 1.0f; offset = XMFLOAT3(0,0,0); break;
		default: break;
		}
		if (texNo >= 0) texNosFramesScales.emplace_back(texNo, g_effectAnim[playerIndex].specialFrame, scale, offset);
	}
	// 毒状態エフェクト
	if (player.isPoisoned)
	{
		texNosFramesScales.emplace_back(11, g_effectAnim[playerIndex].poisonFrame, 0.7f, XMFLOAT3(0.0f, 0.0f, 0.0f));
	}
	// 被弾エフェクト
	if (player.isAttacked)
	{
		texNosFramesScales.emplace_back(12, g_effectAnim[playerIndex].attackedFrame, 1.0f, XMFLOAT3(0.0f, 0.5f, 0.0f));
	}
	// 回復エフェクト
	if (player.isHealing)
	{
		texNosFramesScales.emplace_back(8, g_effectAnim[playerIndex].healingFrame, 1.0f, XMFLOAT3(0.0f, 0.0f, 0.0f));
	}
	// リスポーン卵エフェクト
	if (player.duringRespawn)
	{
		texNosFramesScales.emplace_back(20, g_effectAnim[playerIndex].respawnFrame, 3.0f, XMFLOAT3(0.0f, 0.0f, 0.0f));
		//texNosFramesScales.emplace_back(21, g_effectAnim[playerIndex].respawnFrame, 3.0f, XMFLOAT3(0.0f, 0.0f, 0.0f));
	}
	//// プレイヤー 走行時の埃エフェクト
	//if (player.isMoving)
	//{
	//	texNosFramesScales.emplace_back(22, g_effectAnim[playerIndex].runDustFrame, 2.0f, XMFLOAT3(0.0f, 0.3f, 0.0f));
	//}

	// 通常色を設定
	Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f }); // 通常色

	// 条件に合致したテクスチャをすべて重ねて描画
	for (const auto& texFrameScale : texNosFramesScales)
	{
		int texNo;
		int frame;
		float scale;
		XMFLOAT3 offset;
		std::tie(texNo, frame, scale, offset) = texFrameScale;
		if (texNo < 0 || texNo >= EFFECT_TEX_MAX) continue;
		ID3D11ShaderResourceView* srv = g_Texture[texNo];
		if (!srv) continue;

		if (texNo == 23) 
		{
			Shader_SetColor({ 1.0f, 1.0f, 1.0f, 0.5f }); // 半透明
		}
		else 
		{
			Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f }); // 通常
		}

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

		// サイズに倍率を掛ける
		XMMATRIX ScalingMatrix = XMMatrixScaling(
			player.scaling.x * spriteScale * scale,
			player.scaling.y * spriteScale * scale,
			player.scaling.z * spriteScale * scale
		);

		// オフセットを反映
		XMMATRIX offsetMatrix = XMMatrixTranslation(offset.x, offset.y, offset.z);
		XMMATRIX WorldMatrix = ScalingMatrix * offsetMatrix * vm;
		Shader_SetWorldMatrix(WorldMatrix);

		XMMATRIX WVP = ScalingMatrix * offsetMatrix * vm * view * projection;
		Shader_SetMatrix(WVP);

		g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		Vertex2* vertex = (Vertex2*)msr.pData;
		CopyMemory(vertex, &localV[0], sizeof(Vertex2) * PLAYER_VERTEX);
		g_pContext->Unmap(g_VertexBuffer, 0);

		g_pContext->PSSetShaderResources(0, 1, &srv);
		g_pContext->DrawIndexed(6, 0, 0);
	}
}