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
#define EFFECT_TEX_MAX		(16)
#define EFFECT_MAX			(16)

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
static const int   SHEET_COLS = 8;
static const int   SHEET_ROWS = 8;

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

	hr = CreateShaderResourceView(g_pDevice,image.GetImages(), image.GetImageCount(),metadata, &g_Texture[i]);
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
	Effect_LoadTexture( 0, L"Asset\\Texture\\uiLightBigGlass_v1.png");			// 第2形態 エフェクト ガラス
	Effect_LoadTexture( 1, L"Asset\\Texture\\uiLightBigConcrete_v1.png");		// 第2形態 エフェクト コンクリート
	Effect_LoadTexture( 2, L"Asset\\Texture\\uiLightBigTree_v1.png");			// 第2形態 エフェクト 植物
	Effect_LoadTexture( 3, L"Asset\\Texture\\uiLightBigElectricity_v1.png");	// 第2形態 エフェクト 電気
	Effect_LoadTexture( 4, L"Asset\\Texture\\uiLightBigGlass_v1.png");			// 第3形態 エフェクト ガラス
	Effect_LoadTexture( 5, L"Asset\\Texture\\uiLightBigConcrete_v1.png");		// 第3形態 エフェクト コンクリート
	Effect_LoadTexture( 6, L"Asset\\Texture\\uiLightBigTree_v1.png");			// 第3形態 エフェクト 植物
	Effect_LoadTexture( 7, L"Asset\\Texture\\uiLightBigElectricity_v1.png");	// 第3形態 エフェクト 電気
	// ゲーム内
	Effect_LoadTexture( 8, L"Asset\\Texture\\effectSkillGlassConcrete_v2.png");	// スキル エフェクト ガラス・コンクリート
	Effect_LoadTexture( 9, L"Asset\\Texture\\effectSkillTree_v2.png");			// スキル エフェクト 植物
	Effect_LoadTexture(10, L"Asset\\Texture\\effectSkillElectricity_v2.png");	// スキル エフェクト 電気
	Effect_LoadTexture(11, L"Asset\\Texture\\effectHit01_v2.png");				// ヒット エフェクト コンクリートの建物・プレイヤーを攻撃した時
	Effect_LoadTexture(12, L"Asset\\Texture\\effectHit02_v2.png");				// ヒット エフェクト 電気・ガラス・植物の建物を攻撃した時
	Effect_LoadTexture(13, L"Asset\\Texture\\effectSmoke_20per.png");			// 建物 煙エフェクト 20%破壊
	Effect_LoadTexture(14, L"Asset\\Texture\\effectSmoke_50per.png");			// 建物 煙エフェクト 50%破壊
	Effect_LoadTexture(15, L"Asset\\Texture\\effectWin_v1.png");				// 撃墜 エフェクト

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
	//g_EffectTimer++;

	//if (g_EffectTimer >= EFFECT_SPEED)
	//{
	//	g_EffectTimer = 0;
	//	g_EffectFrame++;

	//	if (!g_EffectLoopFlag)
	//	{
	//		g_EffectTimer++;
	//		if (g_EffectFrame > 29)
	//		{
	//			g_EffectLoopFlag = true;
	//			g_EffectFrame = 32;
	//		}
	//		if (g_EffectFrame >= EFFECT_FRAME_MAX)
	//		{
	//			g_EffectFrame = 0;
	//		}
	//	}
	//	else
	//	{
	//		// ループ
	//		g_EffectFrame++;
	//		if (g_EffectFrame >= 61)
	//		{
	//			g_EffectFrame = 32;
	//		}
	//	}
	//}
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
//　描画（プレイヤー用）
//===============================================
void Effect_DrawForPlayer(int playerIndex, const XMFLOAT2& playerPos, const XMFLOAT2& playerSize)
{
	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	LIGHT light{};
	light.Enable = TRUE;
	// 光の向き（ワールド空間）シェーダー側で単位化して使っている想定
	light.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
	// 拡散光と環境光
	light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	Shader_SetLight(light);

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
			if (!player.active) return;

			const float spriteScale = 2.0f;	// 表示倍率

			// ワールド行列（ビルボード風の既存ロジックを踏襲）
			XMMATRIX ScalingMatrix = XMMatrixScaling(
				player.scaling.x * spriteScale,
				player.scaling.y * spriteScale,
				player.scaling.z * spriteScale
			);

			XMMATRIX vm = GetViewMatrix();	// カメラの行列
			vm.r[3].m128_f32[0] = 0.0f;
			vm.r[3].m128_f32[1] = 0.0f;
			vm.r[3].m128_f32[2] = 0.0f;
			vm.r[3].m128_f32[3] = 1.0f;
			vm = XMMatrixTranspose(vm);
			vm.r[3].m128_f32[0] = player.position.x;
			vm.r[3].m128_f32[1] = player.position.y;
			vm.r[3].m128_f32[2] = player.position.z;
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
			Vertex2 localV[PLAYER_VERTEX];
			CopyMemory(&localV[0], &effect_vdata[0], sizeof(Vertex2) * PLAYER_VERTEX);

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
			CopyMemory(vertex, &localV[0], sizeof(Vertex2) * PLAYER_VERTEX);
			g_pContext->Unmap(g_VertexBuffer, 0);

			ID3D11ShaderResourceView* srv = nullptr;

			//// 形態とタイプに応じたテクスチャを設定
			//switch (player.form)
			//{
			//	// 第1形態
			//case Form::First:					srv = g_Texture[0];	break;
			//	// 第2形態
			//case Form::Second:
			//	switch (player.type)
			//	{
			//	case PlayerType::Glass:			srv = g_Texture[1];	break;
			//	case PlayerType::Concrete:		srv = g_Texture[2];	break;
			//	case PlayerType::Plant:			srv = g_Texture[3];	break;
			//	case PlayerType::Electricity:	srv = g_Texture[4];	break;
			//	default: break;
			//	}
			//	break;
			//	// 第3形態
			//case Form::Third:
			//	switch (player.type)
			//	{
			//	case PlayerType::Glass:			srv = g_Texture[5];	break;
			//	case PlayerType::Concrete:		srv = g_Texture[6];	break;
			//	case PlayerType::Plant:			srv = g_Texture[7];	break;
			//	case PlayerType::Electricity:	srv = g_Texture[8];	break;
			//	default: break;
			//	}
			//	break;
			//default: break;
			//}

			//// スペシャル使用中は専用テクスチャ
			//if (player.useSpecial)			srv = g_Texture[9];

			//g_pContext->PSSetShaderResources(0, 1, &srv);

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
	std::vector<std::pair<float, int>> list;	// (距離二乗, index)
	list.reserve(PLAYER_MAX);

	//for (int p = 0; p < PLAYER_MAX; ++p)
	//{
	//	if (!object[p].active) continue;

	//	float dx = object[p].position.x - camPos.x;
	//	float dy = object[p].position.y - camPos.y;
	//	float dz = object[p].position.z - camPos.z;
	//	float dist2 = dx * dx + dy * dy + dz * dz;
	//	list.emplace_back(dist2, p);
	//}

	// 遠い順（大きい順）にソート
	std::sort(list.begin(), list.end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
		{
			return a.first > b.first;
		});

	// 透過レンダリング：深度テストは有効、深度書き込みは無効（SetDepthReadOnly を使用）
	SetDepthTest(true);
	SetDepthReadOnly();	// 深度テストはするが深度バッファへの書き込みはしない

	// ソート順（遠いものから描画）
	for (auto& p : list)	DrawPlayerInternal(p.second);

	// 3Dオブジェクトは深度テストを無効にして描画
	SetDepthTest(false);




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
		{ 175.0f, 620.0f }, // プレイヤー1
		{ 490.0f, 620.0f },  // プレイヤー2
		{ 805.0f, 620.0f }, // プレイヤー3
		{ 1120.0f, 620.0f }  // プレイヤー4
	};

	if (pIndex < 0 || pIndex >= 4) return;

	XMFLOAT2 targetPos = playerEffectPos[pIndex];

	for (int i = 0; i < EFFECT_MAX; ++i)
	{
		if (!effect[i].enable) continue;

		// 位置が一致するエフェクトを無効化
		if (fabsf(effect[i].pos.x - targetPos.x) < 1.0f &&fabsf(effect[i].pos.y - targetPos.y) < 1.0f)
		{
			effect[i].enable = false;
		}
	}
}