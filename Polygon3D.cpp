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
static ID3D11ShaderResourceView* g_Texture[6];

// エフェクト デバッグ用タイマー
static float g_effectElapsed = 0.0f; // 秒単位での経過時間を保持

// プレイヤー アニメーション用変数
static int   g_animFrame[PLAYER_MAX];
static float g_animTimer[PLAYER_MAX];
static const float ANIM_FRAME_TIME = 0.15f; // 1フレームあたりの秒数
static const int   SHEET_COLS = 16;
static const int   SHEET_ROWS = 16;

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

#define POSITION	(0.5f)	// デフォルト (0.5f)
#define TEXCOORD	(1.0f)	// デフォルト (1.0f)

// 頂点配列
static Vertex2 vdata[NUM_VERTEX] =
{
	{// 頂点0 LEFT-TOP
		XMFLOAT3(-POSITION, POSITION, 0.0f),	// 座標
		XMFLOAT3(0.0f, 0.0f, -1.0f),			// 法線ベクトル
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),		// カラー
		XMFLOAT2(0.0f, 0.0f)					// テクスチャ座標
	},
	{// 頂点1 RIGHT-TOP
		XMFLOAT3(POSITION, POSITION, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(TEXCOORD, 0.0f)
	},
	{// 頂点2 LEFT-BOTTOM
		XMFLOAT3(-POSITION, -POSITION, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f, TEXCOORD)
	},
	{// 頂点3 RIGHT-BOTTOM
		XMFLOAT3(POSITION, -POSITION, 0.0f),
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
	object[0].speed = 0.0f;
	object[0].defense = 1.0f;
	object[0].stock = 3;
	object[0].active = true;
	object[0].isAttacking = false;
	object[0].attackTimer = 0.0f;
	object[0].useSkill = false;
	object[0].skillTimer = 0.0f;
	object[0].useSpecial = false;
	object[0].specialTimer = 0.0f;
	object[0].isInvincible = false;
	object[0].invincibleTimer = 0.0f;
	object[0].stunGauge = 0.0f;
	object[0].isStunning = false;
	object[0].stunTimer = 0.0f;
	object[0].standbyDir = PlayerDir::Down; // 正面
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
	object[1].speed = 0.0f;
	object[1].defense = 1.0f;
	object[1].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
	object[1].stock = 3;
	object[1].active = true;
	object[1].isAttacking = false;
	object[1].attackTimer = 0.0f;
	object[1].useSkill = false;
	object[1].skillTimer = 0.0f;
	object[1].useSpecial = false;
	object[1].specialTimer = 0.0f;
	object[1].isInvincible = false;
	object[1].invincibleTimer = 0.0f;
	object[1].stunGauge = 0.0f;
	object[1].isStunning = false;
	object[1].stunTimer = 0.0f;
	object[1].standbyDir = PlayerDir::Down; // 正面
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
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"asset\\texture\\characterMini_v2.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[0]);
	assert(g_Texture[0]);

	LoadFromWICFile(L"asset\\texture\\kai_walk_01.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[1]);
	assert(g_Texture[1]);

	LoadFromWICFile(L"asset\\texture\\uiStockBlue_v2.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[2]);
	assert(g_Texture[2]);

	LoadFromWICFile(L"asset\\texture\\uiStockGleen_v2.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[3]);
	assert(g_Texture[3]);

	LoadFromWICFile(L"asset\\texture\\uiLightLoopBigConcrete_v1.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[4]);
	assert(g_Texture[4]);

	LoadFromWICFile(L"asset\\texture\\uiLightLoopBigGlass_v1.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[5]);
	assert(g_Texture[5]);

	//インデックスバッファ作成
	{
		D3D11_BUFFER_DESC	bd;
		ZeroMemory(&bd, sizeof(bd));//０でクリア
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(UINT) * 6 * 6;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&bd, NULL, &g_IndexBuffer);

		//インデックスバッファへ書き込み
		D3D11_MAPPED_SUBRESOURCE   msr;
		pContext->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		UINT* index = (UINT*)msr.pData;

		//インデックスデータをバッファへコピー
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

//======================================================
//	終了処理関数
//======================================================
void Polygon3D_Finalize()
{
	if (g_IndexBuffer != NULL)
	{
		g_IndexBuffer->Release();
		g_IndexBuffer = NULL;
	}

	if (g_VertexBuffer != NULL)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	//テクスチャの解放
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		g_Texture[i]->Release();
		g_Texture[i] = NULL;
	}

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

	const float deltaTime = 1.0f / 60.0f; // デルタタイム

	for (int p = 0; p < PLAYER_MAX; ++p)
	{
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
			object[p].stunTimer += 1.0f / 60.0f;

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

			// 攻撃中なら攻撃更新処理を呼び出す
			if (object[p].isAttacking)
			{
				Attack_Update(p);
			}
			// スキル中ならスキル更新処理を呼び出す
			if (object[p].useSkill)
			{
				Skill_Update(p);
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

		// 進化時の無敵タイマー更新
		if (object[p].isInvincible)
		{
			object[p].invincibleTimer += 1.0f / 60.0f;
			if (object[p].invincibleTimer >= INVINCIBLE_TIME)
			{
				object[p].isInvincible = false;
				object[p].invincibleTimer = 0.0f;
			}
		}

		// プレイヤー アニメーション更新
		g_animTimer[p] += deltaTime;
		if (g_animTimer[p] >= ANIM_FRAME_TIME)
		{
			int advance = (int)(g_animTimer[p] / ANIM_FRAME_TIME);
			g_animTimer[p] -= advance * ANIM_FRAME_TIME;

			// 汎用ループ関数（start から count フレームを循環）
			auto loopRange = [&](int start, int count)
				{
					int relative = (g_animFrame[p] - start + advance) % count;
					if (relative < 0) relative += count;
					g_animFrame[p] = start + relative;
				};

			// 移動 8コマ
			if (object[p].isMoving)
			{
					 if (object[p].moveDir.x > 0.0f && object[p].moveDir.z > 0.0f) { loopRange(136, 8); object[p].standbyDir = PlayerDir::Up_Right; }	// 右上 136～143
				else if (object[p].moveDir.x < 0.0f && object[p].moveDir.z > 0.0f) { loopRange(84, 8);	object[p].standbyDir = PlayerDir::Up_Left; }	// 左上  84～91
				else if (object[p].moveDir.x > 0.0f && object[p].moveDir.z < 0.0f) { loopRange(188, 8); object[p].standbyDir = PlayerDir::Down_Right; }	// 右下 188～195
				else if (object[p].moveDir.x < 0.0f && object[p].moveDir.z < 0.0f) { loopRange(32, 8); object[p].standbyDir = PlayerDir::Down_Left; }	// 左下  32～39
				else if (object[p].moveDir.z > 0.0f) { loopRange(110, 8); object[p].standbyDir = PlayerDir::Up; }		// 上 110～117
				else if (object[p].moveDir.z < 0.0f) { loopRange(6, 8); object[p].standbyDir = PlayerDir::Down; }		// 下   6～13
				else if (object[p].moveDir.x > 0.0f) { loopRange(162, 8); object[p].standbyDir = PlayerDir::Right; }	// 右 162～169
				else if (object[p].moveDir.x < 0.0f) { loopRange(58, 8); object[p].standbyDir = PlayerDir::Left; }		// 左  58～63
			}

			// 待機 6コマ
			if (object[p].isMoving == false)
			{
					 if (object[p].standbyDir == PlayerDir::Up_Right)	loopRange(130, 6);	// 右上 130～135
				else if (object[p].standbyDir == PlayerDir::Up_Left)	loopRange(78, 6);	// 左上  78～83 
				else if (object[p].standbyDir == PlayerDir::Down_Right)	loopRange(182, 6);	// 右下 182～187		
				else if (object[p].standbyDir == PlayerDir::Down_Left)	loopRange(26, 6);	// 左下  26～31
				else if (object[p].standbyDir == PlayerDir::Up)			loopRange(104, 6);	//  上  104～109
				else if (object[p].standbyDir == PlayerDir::Down)		loopRange(0, 6);	//  下    0～5
				else if (object[p].standbyDir == PlayerDir::Right)		loopRange(156, 6);	//  右  156～161
				else if (object[p].standbyDir == PlayerDir::Left)		loopRange(52, 6);	//  左   52～57
			}
		}
	}

	// がぶがぶとプレイヤーの当たり判定（object[0] <-> Attack2、object[1] <-> Attack1）
	AttackPlayerCollisions();

	// デバッグ用 ImGui ウィンドウ
	ImGui::Begin("Player Debug");
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		// プレイヤーごとに ID を分ける（同一ラベル衝突回避）
		ImGui::PushID(p);

		ImGui::Text("Player %d", p + 1);
		ImGui::Indent();

		// evolutionGauge breakCount を列挙して表示
		ImGui::SliderFloat("stunGauge", &object[p].stunGauge, 0.0f, 10.0f, "%.1f");
		ImGui::SliderFloat("invincibleTimer", &object[p].invincibleTimer, 0.0f, 3.0f, "%.1f");
		ImGui::BulletText("active            : %d", object[p].active);
		ImGui::BulletText("useSkill          : %d", object[p].useSkill);
		ImGui::BulletText("isInvincible      : %d", object[p].isInvincible);
		ImGui::BulletText("form              : %d", object[p].form);
		ImGui::BulletText("type              : %d", object[p].type);
		ImGui::BulletText("EvolutionGauge    : %d", object[p].evolutionGauge);
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
				// ImGui::BulletText("[%d]: %d", i, (int)object[p].brokenHistory[i]);

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
	
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		static XMFLOAT3 posBuff = object[i].position;	// デバッグ表示座標

		// 描画で使っているスプライト倍率と同じ値を物理にも使う
		const float renderScale = 2.0f; // Draw 側の spriteScale に合わせる
		// AABB と着地処理で使うスケーリング（描画スケールを反映）
		XMFLOAT3 physicsScaling = XMFLOAT3(object[i].scaling.x * renderScale, object[i].scaling.y * renderScale, object[i].scaling.z * renderScale);

		// AABB を現在の位置・スケール（表示スケールを反映したもの）で更新しておく
		CalculateAABB(object[i].boundingBox, object[i].position, physicsScaling);

		// y軸の移動量 (重力 + ジャンプ)
		// 重力加速度のない簡易的な重力
		object[i].position.y += -0.1f;

		// デバッグ出力
		if (posBuff.x != object[i].position.x ||
			posBuff.y != object[i].position.y ||
			posBuff.z != object[i].position.z)
		{
			hal::dout << "x : " << object[i].position.x << std::endl;
			hal::dout << "y : " << object[i].position.y << std::endl;
			hal::dout << "z : " << object[i].position.z << std::endl;
		}

		//hal::dout << vdata[0].position.x << std::endl;

		posBuff = object[i].position;

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
			if (CheckAABBHexCollision(object[i].boundingBox, hex))
			{
				// タイルの上面のY座標を計算
				float tileTopY = fieldObjects[j].pos.y + (hex.height / 2.0f);	// -1 + 1.5 = 0.5

				// プレイヤーの底面がタイルの上面以下か
				if (object[i].boundingBox.Min.y <= tileTopY)
				{
					// 頂点定義が -0.5..+0.5 を基準にしているので baseHalfHeight = 0.5
					const float baseHalfHeight = POSITION; // POSITION はファイル上で 0.5f
					// 描画スケールを反映した半高で着地位置を計算
					float halfHeight = baseHalfHeight * object[i].scaling.y * renderScale;

					// 着地させる（めり込みが起きないよう最低値として補正）
					float targetY = tileTopY + halfHeight;
					if (object[i].position.y < targetY)
					{
						object[i].position.y = targetY;
					}

					// AABB を再計算して整合性を保つ（描画スケールを考慮）
					CalculateAABB(object[i].boundingBox, object[i].position, physicsScaling);

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
			MTV collision = CalculateAABBMTV(object[i].boundingBox, buildingObjects[j]->boundingBox);

			if (collision.isColliding)
			{
				// 衝突していたら、MTVの分だけ位置を戻す
				object[i].position.x += collision.translation.x;
				object[i].position.y += collision.translation.y;
				object[i].position.z += collision.translation.z;

				// 押し戻し後の新しいAABBを再計算（描画スケールを反映）
				CalculateAABB(object[i].boundingBox, object[i].position, physicsScaling);
			}
		}

		// -------------------------------------------------------------
		// 変身
		// -------------------------------------------------------------
		switch (object[i].form)
		{
		case Form::Normal: // 通常
			object[i].scaling.x = 0.5f;
			object[i].scaling.y = 0.5f;
			object[i].scaling.z = 0.5f;
			object[i].speed = 0.06f;
			object[i].power = 0.8f;
			break;

		case Form::FirstEvolution: // 1進化
			object[i].scaling.x = 0.8f;
			object[i].scaling.y = 0.8f;
			object[i].scaling.z = 0.8f;
			object[i].speed = 0.05f;
			object[i].power = 1.0f;
			break;

		case Form::SecondEvolution: // 2進化
			object[i].scaling.x = 1.2f;
			object[i].scaling.y = 1.2f;
			object[i].scaling.z = 1.2f;
			object[i].speed = 0.04f;
			object[i].power = 1.5f;
			break;

		default:
			break;
		}

		ATTACK_OBJECT* attack1 = GetAttack(1);
		ATTACK_OBJECT* attack2 = GetAttack(2);

		// プレイヤー i に対応するスキル（i==0 -> attack1, i==1 -> attack2）をプレイヤーのフォームに合わせてスケーリング同期
		ATTACK_OBJECT* attackForPlayer = (i == 0) ? attack1 : ((i == 1) ? attack2 : nullptr);
		if (attackForPlayer != nullptr)
		{
			// プレイヤーと同じスケールにする（攻撃オブジェクトは半分）
			attackForPlayer->scaling.x = object[i].scaling.x / 2;
			attackForPlayer->scaling.y = object[i].scaling.y / 2;
			attackForPlayer->scaling.z = object[i].scaling.z / 2;
		}
		///////////////////////////////////////////////////////////////////////////////////////////////

		// -------------------------------------------------------------
		// プレイヤーオブジェクト同士の当たり判定
		// -------------------------------------------------------------

		// 他プレイヤーの physicsScaling を作る（両者とも表示スケールを反映して AABB を扱う）
		int otherIndex = (i == 0) ? 1 : 0;
		XMFLOAT3 physicsScalingOther = XMFLOAT3(object[otherIndex].scaling.x * renderScale, object[otherIndex].scaling.y * renderScale, object[otherIndex].scaling.z * renderScale);

		// プレイヤー0 と プレイヤー1 のAABBを更新（表示スケール反映）
		CalculateAABB(object[i].boundingBox, object[i].position, physicsScaling);
		CalculateAABB(object[1].boundingBox, object[1].position, physicsScalingOther);

		// プレイヤー0 (A) と プレイヤー1 (B) の衝突をチェック
		MTV collision_player = CalculateAABBMTV(object[0].boundingBox, object[1].boundingBox);

		if (collision_player.isColliding)
		{
			//hal::dout << " プレイヤー衝突！ 互いに吹き飛ばし実行" << std::endl;
			// 吹き飛ばしの強さ（XZ方向）
			float knockbackPowerXZ = 0.5f; // 強さを調整
			// 吹き飛ばしの強さ（Y方向）
			float knockbackPowerY = 0.3f; // 高さを調整

			// プレイヤー0 の向き（ラジアン）を計算
			float rad_0 = XMConvertToRadians(object[0].rotation.y);
			// プレイヤー0 の向きベクトル（XZ平面）
			object[0].dir.x = sinf(rad_0);
			object[0].dir.z = cosf(rad_0);

			// プレイヤー1 の向き（ラジアン）を計算
			float rad_1 = XMConvertToRadians(object[1].rotation.y);
			// プレイヤー1 の向きベクトル（XZ平面）
			object[1].dir.x = sinf(rad_1);
			object[1].dir.z = cosf(rad_1);

			// 押し戻し量 (MTV) を半分にする
			XMFLOAT3 half_translation =
			{
				collision_player.translation.x * 0.5f,
				collision_player.translation.y * 0.5f,
				collision_player.translation.z * 0.5f
			};

			// プレイヤー0 (object[0]) を MTVの半分だけ 押す
			// MTVの方向 (collision_player.translation) は「AをBから押し出す方向」だから、そのまま使う
			object[0].position.x += half_translation.x;
			object[0].position.y += half_translation.y;
			object[0].position.z += half_translation.z;

			// プレイヤー1 (object[1]) を MTVの逆方向の半分だけ 押す
			// 逆方向にするために、X, Y, Z の符号を反転させる
			object[1].position.x -= half_translation.x;
			object[1].position.y -= half_translation.y;
			object[1].position.z -= half_translation.z;

			// 押し戻し後の新しいAABBを再計算 (次フレームや他の衝突判定に備える)
			CalculateAABB(object[0].boundingBox, object[0].position, physicsScaling);
			CalculateAABB(object[1].boundingBox, object[1].position, physicsScalingOther);

			// 吹き飛ばし後の新しいAABBを再計算 (他の衝突判定に備える)
			CalculateAABB(object[0].boundingBox, object[0].position, physicsScaling);
			CalculateAABB(object[1].boundingBox, object[1].position, physicsScalingOther);
		}

		if (object[i].hp <= 0 && object[i].active)
		{
			object[i].stock--;

			// 残機があれば復活
			if (object[i].stock > 0)
			{
				// リスポーン
				Polygon3D_Respawn(i);
			}
			else
			{
				// 残機無しで死亡
				object[i].active = false;
			}
		}

		SetHPValue(&HPBar[i], (int)object[i].hp, (int)object[i].maxHp);
		UpdateHP(&HPBar[i]);
	}

	// -------------------------------------------------------------
	// 当たり判定 Player1とAttack2
	// -------------------------------------------------------------
	//// AABBの更新

	for (int idx = 0; idx < PLAYER_MAX; ++idx)
	{
		CheckRespawnPlayer(idx);
	}

	// エフェクト用タイマー更新
	//g_effectElapsed += (1.0f / 60.0f);
}

//======================================================
//	描画関数
//======================================================
void Polygon3D_Draw(bool s_IsKonamiCodeEntered)
{
	static bool input1 = false;
	// デバッグモード中のみキー入力を受け付ける
	if (s_IsKonamiCodeEntered)
	{
		if (Keyboard_IsKeyDownTrigger(KK_D1))
		{
			input1 = !input1;	// フラグ反転
		}
	}
	
	//Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	 
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

			g_pContext->PSSetShaderResources(0, 1, &g_Texture[0]);
			Shader_SetColor({ 1,1,1,1 });


			// バッファセット & 描画
			UINT stride = sizeof(Vertex2);
			UINT offset = 0;
			g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
			g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			g_pContext->DrawIndexed(6, 0, 0);
		};

	// -------------------------
	// 透明描画のためのソート（遠い順）
	// -------------------------
	std::vector<std::pair<float, int>> list; // (距離二乗, index)
	list.reserve(PLAYER_MAX);

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		float dx = object[i].position.x - camPos.x;
		float dy = object[i].position.y - camPos.y;
		float dz = object[i].position.z - camPos.z;
		float dist2 = dx * dx + dy * dy + dz * dz;
		list.emplace_back(dist2, i);
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
	Effect_Set(g_Texture[4], { 170.0f,600.0f }, { 400.0f, 400.0f });

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
		object[0].speed = 0.0f;
		object[0].defense = 1.0f;
		object[0].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		object[0].active = true;
		object[0].isAttacking = false;
		object[0].attackTimer = 0.0f;
		object[0].useSkill = false;
		object[0].skillTimer = 0.0f;
		object[0].useSpecial = false;
		object[0].specialTimer = 0.0f;
		object[0].isInvincible = false;
		object[0].invincibleTimer = 0.0f;
		object[0].stunGauge = 0.0f;
		object[0].isStunning = false;
		object[0].stunTimer = 0.0f;

		object[0].standbyDir = PlayerDir::Down; // 正面
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
		object[1].hp = object[0].maxHp;
		object[1].speed = 0.0f;
		object[1].defense = 1.0f;
		object[1].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		object[1].active = true;
		object[1].isAttacking = false;
		object[1].attackTimer = 0.0f;
		object[1].useSkill = false;
		object[1].skillTimer = 0.0f;
		object[1].useSpecial = false;
		object[1].specialTimer = 0.0f;
		object[1].isInvincible = false;
		object[1].invincibleTimer = 0.0f;
		object[1].stunGauge = 0.0f;
		object[1].isStunning = false;
		object[1].stunTimer = 0.0f;

		object[1].standbyDir = PlayerDir::Down; // 正面
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
}

void AttackPlayerCollisions()
{
	// Attack / Player オブジェクト取得
	ATTACK_OBJECT* attack1 = GetAttack(1);
	ATTACK_OBJECT* attack2 = GetAttack(2);
	PLAYEROBJECT* p1 = &object[0];
	PLAYEROBJECT* p2 = &object[1];

	// AABB を最新化
	CalculateAABB(p1->boundingBox, p1->position, p1->scaling);
	CalculateAABB(p2->boundingBox, p2->position, p2->scaling);
	CalculateAABB(attack1->boundingBox, attack1->position, attack1->scaling);
	CalculateAABB(attack2->boundingBox, attack2->position, attack2->scaling);

	// ------------------------
	// object[0] と Attack2 の当たり判定
	// （Attack2 は object[1] のスキル）
	// ------------------------
	if (object[1].isAttacking)
	{
		MTV col = CalculateAABBMTV(p1->boundingBox, attack2->boundingBox);

		// 当たっている かつ object[0]が無敵でない
		if (col.isColliding && !object[0].isInvincible)
		{
			hal::dout << "Attack2 hit object[0] overlap=" << col.overlap << std::endl;

			// ノックバック
			p1->position.x += p2->dir.x * p2->power;
			//p1->position.y += p2->power / 3;
			p1->position.z += p2->dir.z * p2->power;

			// ダメージ 攻撃を受ける側の防御力でダメージを軽減
			p1->hp -= p2->power * p1->defense;
			// スタンゲージ増加
			p1->stunGauge += 0.5f;

			//// ヒットでスキルを消す（1回ヒット）
			//object[1].isAttacking = false;
			//object[1].attackTimer = 0.0f;

			// AABB を更新
			CalculateAABB(p1->boundingBox, p1->position, p1->scaling);
			CalculateAABB(attack2->boundingBox, attack2->position, attack2->scaling);
		}
	}

	// ------------------------
	// object[1] と Attack1 の当たり判定
	// （Attack1 は object[0] のスキル）
	// ------------------------
	if (object[0].isAttacking)
	{
		MTV col = CalculateAABBMTV(p2->boundingBox, attack1->boundingBox);
		
		// 当たっている かつ object[1]が無敵でない
		if (col.isColliding && !object[1].isInvincible)
		{
			hal::dout << "Attack1 hit object[1] overlap=" << col.overlap << std::endl;

			// ノックバック
			p2->position.x += p1->dir.x * p1->power;
			//p2->position.y += p1->power;
			p2->position.z += p1->dir.z * p1->power;

			// ダメージ 攻撃を受ける側の防御力でダメージを軽減
			p2->hp -= p1->power * p2->defense;
			// スタンゲージ増加
			p2->stunGauge += 0.5f;

			//// ヒットでスキルを消す
			//object[0].isAttacking = false;
			//object[0].attackTimer = 0.0f;

			// AABB を更新
			CalculateAABB(p2->boundingBox, p2->position, p2->scaling);
			CalculateAABB(attack1->boundingBox, attack1->position, attack1->scaling);
		}
	}
}

static void CheckRespawnPlayer(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	bool needRespawn = false;

	// HP <= 0 または落下判定でリスポーン
	if (object[playerIndex].hp <= 0.0f)
	{
		object[playerIndex].hp = 0.0f;
		needRespawn = true;
	}

	if (object[playerIndex].position.y < -10.0f)
	{
		needRespawn = true;
	}

	if(needRespawn)	
	{
		// 残機を1減らす
		object[playerIndex].stock -= 1;

		// 個別リスポーン処理
		Polygon3D_Respawn(playerIndex);
	}
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

		g_pContext->PSSetShaderResources(0, 1, &g_Texture[i + 2]);
	
		SetBlendState(BLENDSTATE_ALPHA);
		DrawSprite(pos, size, color::white);
	}
}

PLAYEROBJECT* GetPlayer(int playerIndex)
{
	if (playerIndex > PLAYER_MAX || playerIndex <= 0)
	{
		return nullptr;
	}

	return &object[playerIndex - 1];
}