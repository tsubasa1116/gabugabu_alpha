//======================================================
//	gimmick.cpp
// 
//	隕石を降らせるギミック
//	player.stockが0になったときに発動可能
//======================================================

#include <DirectXMath.h>
#include <d3d11.h>
using namespace DirectX;

#include "gimmick.h"
#include "sprite.h"
#include "shader.h"
#include "Camera.h"
#include "collider.h"
#include "player.h"
#include "keyboard.h"
#include "input.h"
#include "gamepad.h"
#include "DamageText.h"
#include "Building.h"
#include "loadThread.h"
#include "model.h"

//======================================================
//	グローバル変数
//======================================================
static ID3D11Device* g_pDevice = NULL;
static ID3D11DeviceContext* g_pContext = NULL;

// 頂点バッファ
static ID3D11Buffer* g_VertexBuffer = NULL;

// インデックスバッファ
static ID3D11Buffer* g_IndexBuffer = NULL;

// テクスチャ変数
static ID3D11ShaderResourceView* g_RangeTexture[4] = { NULL };	// 範囲テクスチャ
static ID3D11ShaderResourceView* g_MeteorTexture = NULL;		// 隕石モデル用テクスチャ

// 隕石FBXモデル
static MODEL* g_MeteorModel = NULL;

// ギミック状態（プレイヤーごと）
static GIMMICK_STATE g_Gimmick[PLAYER_MAX];

// マクロ定義（範囲描画で+Y面を使うため箱の頂点データは残す）
#define METEOR_VERTEX (24)

// 隕石（箱）の頂点データ（範囲描画の+Y面のみ使用）
static Vertex2 Meteor_vdata[METEOR_VERTEX] =
{
	// -Z面 (法線: 0,0,-1)
	{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },

	// +X面 (法線: 1,0,0)
	{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },

	// +Z面 (法線: 0,0,1)
	{ XMFLOAT3(0.5f,  0.5f, 0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(-0.5f,  0.5f, 0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },

	// -X面 (法線: -1,0,0)
	{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },

	// +Y面 (法線: 0,1,0)
	{ XMFLOAT3(-0.5f, 0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(0.5f, 0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },

	// -Y面 (法線: 0,-1,0)
	{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
};

// インデックス配列
static UINT Meteor_idxdata[6 * 6] =
{
	 0,  1,  2,  2,  1,  3, // -Z面
	 4,  5,  6,  6,  5,  7, // +X面
	 8,  9, 10, 10,  9, 11, // +Z面
	12, 13, 14, 14, 13, 15, // -X面
	16, 17, 18, 18, 17, 19, // +Y面
	20, 21, 22, 22, 21, 23, // -Y面
};

// 範囲テクスチャ用頂点（ビルボード平面）
#define RANGE_VERTEX (6)
static Vertex2 Range_vdata[RANGE_VERTEX] =
{
	{ XMFLOAT3(-0.5f, 0.0f,  0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(0.5f, 0.0f,  0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(-0.5f, 0.0f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(-0.5f, 0.0f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(0.5f, 0.0f,  0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(0.5f, 0.0f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f), XMFLOAT2(1.0f, 1.0f) },
};

//======================================================
// カメラ基準でローカル入力をワールドXZ方向へ変換
//======================================================
static XMFLOAT3 MeteorToWorldDir(const XMFLOAT2& input)
{
	XMMATRIX view = GetViewMatrix();
	XMMATRIX invView = XMMatrixInverse(nullptr, view);

	XMFLOAT3 right = { invView.r[0].m128_f32[0], 0.0f, invView.r[0].m128_f32[2] };
	XMFLOAT3 forward = { invView.r[2].m128_f32[0], 0.0f, invView.r[2].m128_f32[2] };

	// 正規化
	float rl = sqrtf(right.x * right.x + right.z * right.z);
	if (rl > 0.0001f) { right.x /= rl; right.z /= rl; }
	float fl = sqrtf(forward.x * forward.x + forward.z * forward.z);
	if (fl > 0.0001f) { forward.x /= fl; forward.z /= fl; }

	XMFLOAT3 worldDir;
	worldDir.x = right.x * input.x + forward.x * input.y;
	worldDir.y = 0.0f;
	worldDir.z = right.z * input.x + forward.z * input.y;
	return worldDir;
}

//======================================================
//	初期化関数
//======================================================
void Meteor_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// ギミック状態の初期化
	for (int p = 0; p < PLAYER_MAX; p++)
	{
		g_Gimmick[p].enabled = false;
		g_Gimmick[p].cursorPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Gimmick[p].coolTimer = 0.0f;
		g_Gimmick[p].canFire = true;
		g_Gimmick[p].rangeAnimFrame = 0;
		g_Gimmick[p].rangeAnimTimer = 0.0f;

		g_Gimmick[p].meteor.position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Gimmick[p].meteor.rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Gimmick[p].meteor.scaling = XMFLOAT3(METEOR_MODEL_SCALE, METEOR_MODEL_SCALE, METEOR_MODEL_SCALE);
		g_Gimmick[p].meteor.targetPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Gimmick[p].meteor.active = false;
		g_Gimmick[p].meteor.landed = false;
	}

	// 頂点バッファ作成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(Vertex2) * METEOR_VERTEX;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);
	}

	// 範囲描画用のインデックスバッファ作成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(UINT) * 6 * 6;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&bd, NULL, &g_IndexBuffer);

		D3D11_MAPPED_SUBRESOURCE msr;
		pContext->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		UINT* index = (UINT*)msr.pData;
		CopyMemory(&index[0], &Meteor_idxdata[0], sizeof(UINT) * 6 * 6);
		pContext->Unmap(g_IndexBuffer, 0);
	}

	// 隕石FBXモデル読み込み
	g_MeteorModel = ModelLoad("asset\\model\\effectMeteo_v1.fbx");

	// テクスチャ読み込み
	Loader::AddTask([pDevice]()
		{
			TexMetadata metadata;
			ScratchImage image;

			// 隕石モデル用テクスチャ
			LoadFromWICFile(L"Asset\\Texture\\textureMeteo_v10.png", WIC_FLAGS_NONE, &metadata, image);
			CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_MeteorTexture);
			assert(g_MeteorTexture);

			// 隕石範囲表示（プレイヤーごとの色）
			LoadFromWICFile(L"Asset\\Texture\\uiSpecialRed_v2.png", WIC_FLAGS_NONE, &metadata, image);
			CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_RangeTexture[0]);
			assert(g_RangeTexture[0]);
			LoadFromWICFile(L"Asset\\Texture\\uiSpecialBlue_v2.png", WIC_FLAGS_NONE, &metadata, image);
			CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_RangeTexture[1]);
			assert(g_RangeTexture[1]);
			LoadFromWICFile(L"Asset\\Texture\\uiSpecialYellow_v3.png", WIC_FLAGS_NONE, &metadata, image);
			CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_RangeTexture[2]);
			assert(g_RangeTexture[2]);
			LoadFromWICFile(L"Asset\\Texture\\uiSpecialGreen_v2.png", WIC_FLAGS_NONE, &metadata, image);
			CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_RangeTexture[3]);
			assert(g_RangeTexture[3]);
		});
}

//======================================================
//	終了処理関数
//======================================================
void Meteor_Finalize()
{
	if (g_VertexBuffer != NULL)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
	if (g_IndexBuffer != NULL)
	{
		g_IndexBuffer->Release();
		g_IndexBuffer = NULL;
	}

	// 隕石FBXモデル解放
	ModelRelease(g_MeteorModel);
	g_MeteorModel = NULL;

	// 隕石モデル用テクスチャ解放
	if (g_MeteorTexture != NULL)
	{
		g_MeteorTexture->Release();
		g_MeteorTexture = NULL;
	}

	for (int i = 0; i < 4; i++)
	{
		if (g_RangeTexture[i] != NULL)
		{
			g_RangeTexture[i]->Release();
			g_RangeTexture[i] = NULL;
		}
	}

	g_pDevice = NULL;
	g_pContext = NULL;
}

//======================================================
//	更新関数
//======================================================
void Meteor_Update()
{
	// プレイヤーごとの移動キー
	const Keyboard_Keys_tag moveUpKeys[PLAYER_MAX] = { KK_W, KK_UP, KK_T, KK_NUMPAD8 };
	const Keyboard_Keys_tag moveDownKeys[PLAYER_MAX] = { KK_S, KK_DOWN, KK_G, KK_NUMPAD5 };
	const Keyboard_Keys_tag moveLeftKeys[PLAYER_MAX] = { KK_A, KK_LEFT, KK_F, KK_NUMPAD4 };
	const Keyboard_Keys_tag moveRightKeys[PLAYER_MAX] = { KK_D, KK_RIGHT, KK_H, KK_NUMPAD6 };

	// プレイヤーごとの攻撃キー
	const Keyboard_Keys_tag attackKeys[PLAYER_MAX] = { KK_SPACE, KK_ENTER, KK_V, KK_NUMPAD0 };

	float cursorSpeed = 3.0f * DELTA_TIME;

	for (int p = 0; p < PLAYER_MAX; p++)
	{
		PLAYEROBJECT* playerObj = GetPlayer(p);
		if (playerObj == nullptr) continue;
		PLAYEROBJECT& player = *playerObj;

		if (!player.active)
		{
			if (!g_Gimmick[p].enabled)
			{
				g_Gimmick[p].enabled = true;
				// 照準の初期位置をストック0になった瞬間のプレイヤー座標に設定
				g_Gimmick[p].cursorPos.x = player.position.x;
				g_Gimmick[p].cursorPos.y = 0.1f;
				g_Gimmick[p].cursorPos.z = player.position.z;
				g_Gimmick[p].coolTimer = 0.0f;
				g_Gimmick[p].canFire = true;
				g_Gimmick[p].rangeAnimFrame = 0;
				g_Gimmick[p].rangeAnimTimer = 0.0f;
			}
		}
		else
		{
			g_Gimmick[p].enabled = false;
			g_Gimmick[p].meteor.active = false;
			continue;
		}

		if (!g_Gimmick[p].enabled) continue;

		// ------------------------------------------
		// 範囲アニメーション更新（special.cppと同じ方式）
		// ------------------------------------------
		g_Gimmick[p].rangeAnimTimer += DELTA_TIME;
		if (g_Gimmick[p].rangeAnimTimer >= METEOR_RANGE_ANIM_TIME)
		{
			g_Gimmick[p].rangeAnimTimer -= METEOR_RANGE_ANIM_TIME;
			g_Gimmick[p].rangeAnimFrame = (g_Gimmick[p].rangeAnimFrame + 1) % METEOR_RANGE_FRAME_MAX;
		}

		// ------------------------------------------
		// ② 照準の移動（プレイヤーと同じキー/スティック）
		// ------------------------------------------
		XMFLOAT2 inputDir = { 0.0f, 0.0f };

		// キーボード入力
		if (Keyboard_IsKeyDown(moveUpKeys[p]))		inputDir.y += 1.0f;
		if (Keyboard_IsKeyDown(moveDownKeys[p]))	inputDir.y -= 1.0f;
		if (Keyboard_IsKeyDown(moveLeftKeys[p]))	inputDir.x -= 1.0f;
		if (Keyboard_IsKeyDown(moveRightKeys[p]))	inputDir.x += 1.0f;

		// コントローラーのスティック入力
		if (g_Input[p].LStickX != 0.0f || g_Input[p].LStickY != 0.0f)
		{
			inputDir.x = g_Input[p].LStickX;
			inputDir.y = g_Input[p].LStickY;
		}

		// 入力をカメラ基準でワールド方向に変換
		float inputLen = sqrtf(inputDir.x * inputDir.x + inputDir.y * inputDir.y);
		if (inputLen > 0.0f)
		{
			// 正規化（スティックの入力量を保持）
			if (inputLen > 1.0f)
			{
				inputDir.x /= inputLen;
				inputDir.y /= inputLen;
				inputLen = 1.0f;
			}

			XMFLOAT3 worldDir = MeteorToWorldDir(inputDir);
			g_Gimmick[p].cursorPos.x += worldDir.x * cursorSpeed;
			g_Gimmick[p].cursorPos.z += worldDir.z * cursorSpeed;
		}

		// ------------------------------------------
		// クールタイム管理
		// ------------------------------------------
		if (!g_Gimmick[p].canFire)
		{
			g_Gimmick[p].coolTimer += DELTA_TIME;
			if (g_Gimmick[p].coolTimer >= METEOR_COOLTIME)
			{
				g_Gimmick[p].canFire = true;
				g_Gimmick[p].coolTimer = 0.0f;
			}
		}

		// ------------------------------------------
		// ③ 攻撃入力で隕石発射
		// ------------------------------------------
		if (g_Gimmick[p].canFire && !g_Gimmick[p].meteor.active)
		{
			bool firePressed = false;

			// キーボード
			if (Keyboard_IsKeyDownTrigger(attackKeys[p])) firePressed = true;

			// コントローラー
			if (g_Input[p].A) firePressed = true;

			if (firePressed)
			{
				// 隕石を生成
				g_Gimmick[p].meteor.active = true;
				g_Gimmick[p].meteor.landed = false;
				g_Gimmick[p].meteor.targetPos = g_Gimmick[p].cursorPos;
				g_Gimmick[p].meteor.targetPos.y = 0.0f;
				g_Gimmick[p].meteor.position.x = g_Gimmick[p].cursorPos.x;
				g_Gimmick[p].meteor.position.y = METEOR_START_HEIGHT;
				g_Gimmick[p].meteor.position.z = g_Gimmick[p].cursorPos.z;
				g_Gimmick[p].meteor.rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
				g_Gimmick[p].meteor.scaling = XMFLOAT3(METEOR_MODEL_SCALE, METEOR_MODEL_SCALE, METEOR_MODEL_SCALE);

				g_Gimmick[p].canFire = false;
				g_Gimmick[p].coolTimer = 0.0f;
			}
		}

		// ------------------------------------------
		// 隕石の落下処理
		// ------------------------------------------
		if (g_Gimmick[p].meteor.active && !g_Gimmick[p].meteor.landed)
		{
			g_Gimmick[p].meteor.position.y -= METEOR_FALL_SPEED * DELTA_TIME;

			// 回転演出
			g_Gimmick[p].meteor.rotation.x += 3.0f;
			g_Gimmick[p].meteor.rotation.z += 2.0f;

			// 着弾判定（地面に到達）
			if (g_Gimmick[p].meteor.position.y <= g_Gimmick[p].meteor.targetPos.y)
			{
				g_Gimmick[p].meteor.position.y = g_Gimmick[p].meteor.targetPos.y;
				g_Gimmick[p].meteor.landed = true;

				// ⑤ 当たり判定 (scaling 1.0f)
				CalculateAABB(g_Gimmick[p].meteor.boundingBox, g_Gimmick[p].meteor.position, XMFLOAT3(1.0f, 1.0f, 1.0f));

				// 全プレイヤーとの当たり判定
				for (int target = 0; target < PLAYER_MAX; target++)
				{
					PLAYEROBJECT* targetObj = GetPlayer(target);
					if (targetObj == nullptr) continue;
					if (!targetObj->active) continue;
					if (targetObj->duringRespawn || targetObj->isEggBreaking) continue;
					if (targetObj->isInvincible) continue;

					// ターゲットのAABBを計算
					AABB targetAABB;
					CalculateAABB(targetAABB, targetObj->position, targetObj->scaling);

					// 衝突判定
					if (CheckAABBCollision(g_Gimmick[p].meteor.boundingBox, targetAABB))
					{
						// ダメージ適用
						targetObj->hp -= METEOR_DAMAGE * targetObj->defense;
						targetObj->isAttacked = true;
						targetObj->attackedTimer = 0.0f;
						targetObj->isDamageColor = true;
						targetObj->damageColorTimer = 0.0f;
					}
				}
			}
		}

		// 着弾後しばらくしたら非アクティブに
		if (g_Gimmick[p].meteor.landed)
		{
			// 即座に非アクティブ
			g_Gimmick[p].meteor.active = false;
		}
	}
}

//======================================================
//	描画関数
//======================================================
void Meteor_Draw()
{
	for (int p = 0; p < PLAYER_MAX; p++)
	{
		if (!g_Gimmick[p].enabled) continue;
		if (g_RangeTexture[p] == NULL) continue;

		// ------------------------------------------
		// ① 範囲テクスチャの常時表示（アニメーション付き）
		// ------------------------------------------
		{
			SetBlendState(BLENDSTATE_ALPHA);
			Shader_Begin();
			SetDepthTest(FALSE);

			// ライト設定
			LIGHT rangeLight{};
			rangeLight.Enable = TRUE;
			rangeLight.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
			rangeLight.Diffuse = XMFLOAT4(2.5f, 2.5f, 2.5f, 1.0f);
			rangeLight.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
			Shader_SetLight(rangeLight);

			// スプライトシートのUV計算（8x8シート）
			int frame = g_Gimmick[p].rangeAnimFrame;
			int col = frame % METEOR_RANGE_SHEET_COLS;
			int row = frame / METEOR_RANGE_SHEET_COLS;
			float u0 = (float)col / (float)METEOR_RANGE_SHEET_COLS;
			float v0 = (float)row / (float)METEOR_RANGE_SHEET_ROWS;
			float u1 = u0 + 1.0f / (float)METEOR_RANGE_SHEET_COLS;
			float v1 = v0 + 1.0f / (float)METEOR_RANGE_SHEET_ROWS;

			// 頂点データを書き込み（+Y面のUVをアニメーションフレームに合わせて書き換え）
			D3D11_MAPPED_SUBRESOURCE msr;
			g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
			Vertex2* vertex = (Vertex2*)msr.pData;
			CopyMemory(vertex, Meteor_vdata, sizeof(Vertex2) * METEOR_VERTEX);
			// +Y面（頂点16～19）のUVを上書き
			vertex[16].tex = XMFLOAT2(u0, v0);	// LEFT-TOP
			vertex[17].tex = XMFLOAT2(u1, v0);	// RIGHT-TOP
			vertex[18].tex = XMFLOAT2(u0, v1);	// LEFT-BOTTOM
			vertex[19].tex = XMFLOAT2(u1, v1);	// RIGHT-BOTTOM
			g_pContext->Unmap(g_VertexBuffer, 0);

			// ワールド行列（照準位置にスケール = 直径4 = 半径2）
			float diameter = METEOR_RANGE_RADIUS * 2.0f;
			XMMATRIX world = XMMatrixScaling(diameter, 1.0f, diameter)
				* XMMatrixTranslation(
					g_Gimmick[p].cursorPos.x,
					g_Gimmick[p].cursorPos.y,
					g_Gimmick[p].cursorPos.z);

			// WVP行列を計算してシェーダーにセット
			XMMATRIX WVP = world * GetViewMatrix() * GetProjectionMatrix();
			Shader_SetMatrix(WVP);

			// 頂点バッファ・インデックスバッファのセット
			UINT stride = sizeof(Vertex2);
			UINT offset = 0;
			g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
			g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

			SetBlendState(BLENDSTATE_ALPHA);
			g_pContext->PSSetShaderResources(0, 1, &g_RangeTexture[p]);

			// +Y面の4頂点(16,17,18,19)をTriangleStripで描画
			g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			g_pContext->Draw(4, 16);

			SetDepthTest(TRUE);
		}

		// ------------------------------------------
		// 隕石の描画（FBXモデル + 外部テクスチャ）
		// ------------------------------------------
		if (g_Gimmick[p].meteor.active && g_MeteorModel != NULL && g_MeteorModel->AiScene != NULL)
		{
			METEOR_OBJECT& meteor = g_Gimmick[p].meteor;

			// WVP行列を作成
			XMMATRIX ScalingMatrix = XMMatrixScaling(
				meteor.scaling.x, meteor.scaling.y, meteor.scaling.z);

			XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(meteor.rotation.x),
				XMConvertToRadians(meteor.rotation.y),
				XMConvertToRadians(meteor.rotation.z));

			XMMATRIX TranslationMatrix = XMMatrixTranslation(
				meteor.position.x, meteor.position.y, meteor.position.z);

			XMMATRIX world = ScalingMatrix * RotationMatrix * TranslationMatrix;
			XMMATRIX WVP = world * GetViewMatrix() * GetProjectionMatrix();

			Shader_SetMatrix(WVP);

			// ModelDrawを使わず、自前でメッシュを描画してテクスチャを確実に適用する
			Shader_Begin();
			g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			for (unsigned int m = 0; m < g_MeteorModel->AiScene->mNumMeshes; m++)
			{
				aiMesh* mesh = g_MeteorModel->AiScene->mMeshes[m];

				// 外部テクスチャを強制セット（ModelDrawの上書き問題を回避）
				if (g_MeteorTexture != NULL)
				{
					g_pContext->PSSetShaderResources(0, 1, &g_MeteorTexture);
				}

				// 頂点バッファ設定
				UINT stride = sizeof(Vertex3D);
				UINT offset = 0;
				g_pContext->IASetVertexBuffers(0, 1, &g_MeteorModel->VertexBuffer[m], &stride, &offset);

				// インデックスバッファ設定
				g_pContext->IASetIndexBuffer(g_MeteorModel->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

				// ポリゴン描画
				g_pContext->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
			}
		}
	}
}