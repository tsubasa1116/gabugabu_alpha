//======================================================
//	polygon3D.cpp[]
// 
//	制作者：平岡颯馬	日付：2025/12/02
//======================================================
////Polygon3D.cpp

#include "d3d11.h"
#include "DirectXMath.h"
using namespace DirectX;
#include "direct3d.h"
#include "shader.h"
#include "keyboard.h"
#include "sprite.h"

#include "polygon3D.h"
#include "Camera.h"

///////////////////////////////////////
#include "field.h"
#include "collider.h"

#include <iostream>
#include "debug_ostream.h"
#include "skill.h" 
///////////////////////////////////////

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


//======================================================
//	マクロ定義
//======================================================
#define	NUM_VERTEX	(100)
#define	PLAYER_MAX	(2)

//======================================================
//	構造謡宣言
//======================================================
// オブジェクト
PLAYEROBJECT object[PLAYER_MAX];

//======================================================
//	グローバル変数
//======================================================
static	ID3D11Device* g_pDevice = NULL;
static	ID3D11DeviceContext* g_pContext = NULL;

//頂点バッファ
static	ID3D11Buffer* g_VertexBuffer = NULL;

//インデックスバッファ
static	ID3D11Buffer* g_IndexBuffer = NULL;

//テクスチャ変数
static ID3D11ShaderResourceView* g_Texture[PLAYER_MAX];

static	Vertex vdata[NUM_VERTEX] =
{
	//-Z面
	{//頂点0 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),		//座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),	//カラー
		XMFLOAT2(0.0f,0.0f)					//テクスチャ座標
	},
	{//頂点1 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{//頂点2 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	//{//頂点3 LEFT-BOTTOM
	//	XMFLOAT3(-0.5f, -0.5f, -0.5f),
	//	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	//	XMFLOAT2(0.0f,1.0f)
	//},
	//{//頂点4 RIGHT-TOP
	//	XMFLOAT3(0.5f, 0.5f, -0.5f),
	//	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	//	XMFLOAT2(1.0f,0.0f)
	//},
	{//頂点5 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	//+X面
	{//頂点6 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{//頂点7 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{//頂点8 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	//{//頂点9 LEFT-BOTTOM
	//	XMFLOAT3(0.5f, -0.5f, -0.5f),
	//	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	//	XMFLOAT2(0.0f,1.0f)
	//},
	//{//頂点10 RIGHT-TOP
	//	XMFLOAT3(0.5f, 0.5f, 0.5f),
	//	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	//	XMFLOAT2(1.0f,0.0f)
	//},
	{//頂点11 RIGHT-BOTTM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	//+Z面
	{//頂点12 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{//頂点13 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{//頂点14 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	//{//頂点15 LEFT-BOTTOM
	//	XMFLOAT3(0.5f, -0.5f, 0.5f),
	//	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	//	XMFLOAT2(0.0f,1.0f)
	//},
	//{//頂点16 RIGHT-TOP
	//	XMFLOAT3(-0.5f, 0.5f, 0.5f),
	//	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	//	XMFLOAT2(1.0f,0.0f)
	//},
	{//頂点17 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	//-X面
	{//頂点18 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{//頂点19 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{//頂点20 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	//{//頂点21 LEFT-BOTTOM
	//	XMFLOAT3(-0.5f, -0.5f, 0.5f),
	//	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	//	XMFLOAT2(0.0f,1.0f)
	//},
	//{//頂点22 RIGHT-TOP
	//	XMFLOAT3(-0.5f, 0.5f, -0.5f),
	//	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	//	XMFLOAT2(1.0f,0.0f)
	//},
	{//頂点23 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	//+Y面
	{//頂点24 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{//頂点25 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{//頂点26 LEFT-BOTTOM
		XMFLOAT3(-0.5f, 0.5f, -0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	//{//頂点27 LEFT-BOTTOM
	//	XMFLOAT3(-0.5f, 0.5f, -0.5f),
	//	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	//	XMFLOAT2(0.0f,1.0f)
	//},
	//{//頂点28 RIGHT-TOP
	//	XMFLOAT3(0.5f, 0.5f, 0.5f),
	//	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	//	XMFLOAT2(1.0f,0.0f)
	//},
	{//頂点29 RIGHT-BOTTOM
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	//-Y面
	{//頂点30 LEFT-TOP
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{//頂点31 RIGHT-TOP
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{//頂点32 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	//{//頂点33 LEFT-BOTTOM
	//	XMFLOAT3(-0.5f, -0.5f, 0.5f),
	//	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	//	XMFLOAT2(0.0f,1.0f)
	//},
	//{//頂点34 RIGHT-TOP
	//	XMFLOAT3(0.5f, -0.5f, -0.5f),
	//	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	//	XMFLOAT2(1.0f,0.0f)
	//},
	{//頂点35 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},
};

//インデックス配列
static UINT idxdata[6 * 6] = {
	0,1,2,2,1,3,		// -Z面
	4,5,6,6,5,7,		// +X面
	8,9,10,10,9,11,		// +Z面
	12,13,14,14,13,15,	// -X面
	16,17,18,18,17,19,	// +Y面
	20,21,22,22,21,23,	// -Y面
};

//======================================================
//	初期化関数
//======================================================
void Polygon3D_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// ポリゴン表示の初期化
	object[0].position = XMFLOAT3(-2.0f, 1.5f, 0.0f);
	object[0].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	object[0].scaling = XMFLOAT3(1.0f, 1.0f, 1.0f);
	object[0].speed = 0.0f;
	object[0].maxHp = 100.0f;
	object[0].hp = object[0].maxHp;
	object[0].residue = 2;
	object[0].isAttaking = false;
	object[0].attackTimer = 0.0f;
	object[0].breakCount_Glass = 0;
	object[0].breakCount_Plant = 0;
	object[0].breakCount_Concrete = 0;
	object[0].breakCount_Electricity = 0;

	object[1].position = XMFLOAT3(1.5f, 2.0f, 2.0f);
	object[1].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	object[1].scaling = XMFLOAT3(1.0f, 1.0f, 1.0f);
	object[1].speed = 0.0f;
	object[1].maxHp = 100.0f;
	object[1].hp = object[1].maxHp;
	object[1].residue = 2;
	object[1].isAttaking = false;
	object[1].attackTimer = 0.0f;
	object[1].breakCount_Glass = 0;
	object[1].breakCount_Plant = 0;
	object[1].breakCount_Concrete = 0;
	object[1].breakCount_Electricity = 0;

	//頂点バッファ作成
	D3D11_BUFFER_DESC	bd;
	ZeroMemory(&bd, sizeof(bd));//０でクリア
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

	LoadFromWICFile(L"Asset\\Texture\\TileA3.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[0]);
	assert(g_Texture[0]);

	LoadFromWICFile(L"Asset\\Texture\\texture.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[1]);
	assert(g_Texture[1]);

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
}

// ======================================================
// 移動関数（要変更）
// ------------------------------------------------------
// 移動ベクトルと向いている方向ベクトルは別で持った方がいい
// ======================================================
void Move(PLAYEROBJECT& object, XMFLOAT3 moveDir)
{
	//for (int i = 0; i < PLAYER_MAX; i++)
	//{
		// 進みたい方向（3平方）
	float length = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);

	if (length > 0.0f)
	{
		// ベクトルの正規化
		moveDir.x /= length;
		moveDir.z /= length;

		// 目標角度を求める
		float targetAngle = atan2f(moveDir.x, moveDir.z);	// ベクトルの角度
		targetAngle = XMConvertToDegrees(targetAngle);		// ラジアン → 度

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
	//	}
}

//======================================================
//	攻撃関数
//======================================================
//void Polygon3D_Attack()
//{
//	// Player1がPlayer2を攻撃する
//	if (Keyboard_IsKeyDown(KK_SPACE))
//	{
//
//	}
//
//	// Player2がPlayer1を攻撃する
//	if (Keyboard_IsKeyDown(KK_ENTER))
//	{
//
//	}
//
//}

void Polygon3D_Respawn()
{
	object[0].position = XMFLOAT3(-2.0f, 2.0f, 0.0f);
	object[0].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	object[0].scaling = XMFLOAT3(1.0f, 1.0f, 1.0f);
	object[0].speed = 0.0f;
	object[0].hp = 100.0f;
	object[0].maxHp = object[0].hp;
	object[0].residue = 2;
	object[0].isAttaking = false;
	object[0].attackTimer = 0.0f;
	object[0].breakCount_Glass = 0;
	object[0].breakCount_Plant = 0;
	object[0].breakCount_Concrete = 0;
	object[0].breakCount_Electricity = 0;

	object[1].position = XMFLOAT3(2.0f, 4.0f, 3.0f);
	object[1].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	object[1].scaling = XMFLOAT3(1.0f, 1.0f, 1.0f);
	object[1].speed = 0.0f;
	object[1].hp = 100.0f;
	object[1].maxHp = object[1].hp;
	object[1].residue = 2;
	object[1].isAttaking = false;
	object[1].attackTimer = 0.0f;
	object[1].breakCount_Glass = 0;
	object[1].breakCount_Plant = 0;
	object[1].breakCount_Concrete = 0;
	object[1].breakCount_Electricity = 0;
}

//======================================================
//	更新関数
//======================================================
void Polygon3D_Update()
{
	// プレイヤー1 スキル発動
	if (Keyboard_IsKeyDownTrigger(KK_SPACE))
	{
		object[0].isAttaking = true;
	}
	if (object[0].isAttaking == true)
	{
		Player1_Skill_Update();
	}

	// プレイヤー2 スキル発動
	if (Keyboard_IsKeyDownTrigger(KK_ENTER))
	{
		object[1].isAttaking = true;
		Player2_Skill_Update();
	}

	ImGui::Begin("Player Debug");

	// HPバー
	ImGui::SliderFloat("HP", &object[0].hp, 0.0f, object[0].maxHp);

	// 座標調整
	ImGui::Text("Position");
	ImGui::DragFloat3("pos", (float*)&object[0].position, 0.1f);

	ImGui::SliderInt("residue", &object[0].residue, 0.0f, object[0].residue);

	ImGui::End();

	object[0].moveDir = { 0.0f, 0.0f, 0.0f };	// 移動ベクトル
	object[1].moveDir = { 0.0f, 0.0f, 0.0f };	// 移動ベクトル

	if (Keyboard_IsKeyDown(KK_W))	object[0].moveDir.z += 1.0f;
	if (Keyboard_IsKeyDown(KK_S))	object[0].moveDir.z -= 1.0f;
	if (Keyboard_IsKeyDown(KK_A))	object[0].moveDir.x -= 1.0f;
	if (Keyboard_IsKeyDown(KK_D))	object[0].moveDir.x += 1.0f;

	if (Keyboard_IsKeyDown(KK_UP))		object[1].moveDir.z += 1.0f;
	if (Keyboard_IsKeyDown(KK_DOWN))	object[1].moveDir.z -= 1.0f;
	if (Keyboard_IsKeyDown(KK_LEFT))	object[1].moveDir.x -= 1.0f;
	if (Keyboard_IsKeyDown(KK_RIGHT))	object[1].moveDir.x += 1.0f;

	Move(object[0], object[0].moveDir);
	Move(object[1], object[1].moveDir);
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		static XMFLOAT3 posBuff = object[i].position;	// デバッグ表示座標

		// Y軸の移動量 (重力 + ジャンプ)
		// 重力加速度のない簡易的な重力
		//object[i].position.y += -0.1f;

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

		// -------------------------------------------------------------
		// 変身
		// -------------------------------------------------------------
		switch (object[i].form)
		{
		case Normal: // 通常
			object[i].scaling.x = 0.5f;
			object[i].scaling.y = 0.5f;
			object[i].scaling.z = 0.5f;
			object[i].speed = 0.05f;
			object[i].power = 0.8f;
			break;

		case FirstEvolution: // 1進化
			object[i].scaling.x = 1.0f;
			object[i].scaling.y = 1.0f;
			object[i].scaling.z = 1.0f;
			object[i].speed = 0.03f;
			object[i].power = 1.0f;
			break;

		case SecondEvolution: // 2進化
			object[i].scaling.x = 1.5f;
			object[i].scaling.y = 1.5f;
			object[i].scaling.z = 1.5f;
			object[i].speed = 0.02f;
			object[i].power = 1.5f;
			break;

		default:
			break;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////

		// -------------------------------------------------------------
		// 当たり判定
		// -------------------------------------------------------------
		// AABBの更新

		int fieldCount = GetFieldObjectCount();
		// 全てのフィールドオブジェクトと衝突判定を行う
		for (int j = 0; j < fieldCount; ++j)
		{
			// フィールドオブジェクトのリストを取得
			MAPDATA* fieldObjects = GetFieldObjects();

			// j番目のフィールドオブジェクトのAABBを取得
			AABB pStaticObjectAABB = fieldObjects[j].boundingBox;

			CalculateAABB(object[i].boundingBox, object[i].position, object[i].scaling);
			// プレイヤーのAABBとフィールドオブジェクトのAABBでMTVを計算
			MTV collision = CalculateAABBMTV(object[i].boundingBox, pStaticObjectAABB);	// 押し戻す量

			// 非アクティブなオブジェクトは処理をスキップ
			if (!fieldObjects[j].isActive)
			{
				continue; // 次のオブジェクトへ
			}
			//if(CheckAABBCollision(object[0].boundingBox, fieldObjects->boundingBox)&& fieldObjects->no==FIELD_BUILDING)
			if (CheckAABBCollision(object[i].boundingBox, fieldObjects[j].boundingBox) && fieldObjects[j].no == FIELD_BUILDING && Keyboard_IsKeyDown(KK_SPACE))
			{
				// 建物に衝突していて、かつスペースキーが押されていたら
				fieldObjects[j].isActive = false;
				object[i].form = (Form)((int)object[i].form + 1); // 進化
			}
			if (CheckAABBCollision(object[0].boundingBox, object[1].boundingBox) && Keyboard_IsKeyDown(KK_SPACE))
			{
				// スキル使用時
				object[i].form = (Form)((int)object[i].form - 1); // 進化
			}

			if (collision.isColliding)
			{
				//////////////////////////////////////////////
				// ↓↓↓ 無理やり押し戻しているから要修正
				//////////////////////////////////////////////
				if (fieldObjects[j].no == FIELD::FIELD_BOX)
				{
					// 衝突していたら、MTVの分だけ位置を戻す
					//object[i].position.x += collision.translation.x;
					object[i].position.y += collision.translation.y;
					//object[i].position.z += collision.translation.z;

					// 押し戻し後の新しいAABBを再計算
					// これにより、同じフレーム内で次のフィールドオブジェクトとの判定に備えます。
					CalculateAABB(object[i].boundingBox, object[i].position, object[i].scaling);
				}
				else if (fieldObjects[j].no == FIELD::FIELD_BUILDING)
				{
					object[i].position.x += collision.translation.x;
					object[i].position.y += collision.translation.y;
					object[i].position.z += collision.translation.z;

					CalculateAABB(object[i].boundingBox, object[i].position, object[i].scaling);
				}

				// デバッグ出力
				hal::dout << "衝突！押し戻し量: " << collision.overlap << " @ " <<
					(collision.translation.x != 0 ? "X軸" :
						(collision.translation.y != 0 ? "Y軸" :
							"Z軸")) << std::endl;

				// ↑↑↑　#include "debug_ostream.h"　のインクルードでデバッグ確認
			}
		}

		// Polygon3D_Update() 関数の中のフィールドとの衝突判定ループの直後に追加

		// -------------------------------------------------------------
		// プレイヤーオブジェクト同士の当たり判定
		// -------------------------------------------------------------

		// プレイヤー0 と プレイヤー1 のAABBを更新
		CalculateAABB(object[i].boundingBox, object[i].position, object[i].scaling);
		CalculateAABB(object[1].boundingBox, object[1].position, object[1].scaling);

		// プレイヤー0 (A) と プレイヤー1 (B) の衝突をチェック
		MTV collision_player = CalculateAABBMTV(object[0].boundingBox, object[1].boundingBox);

		if (collision_player.isColliding)
		{
			hal::dout << "?? プレイヤー衝突！ 互いに吹き飛ばし実行" << std::endl;
			// 吹き飛ばしの強さ（X-Z方向）
			const float knockbackPowerXZ = 0.5f; // 強さを調整
			// 吹き飛ばしの強さ（Y方向）
			const float knockbackPowerY = 0.3f; // 高さを調整

			// プレイヤー0 の向き（ラジアン）を計算
			float rad_0 = XMConvertToRadians(object[0].rotation.y);
			// プレイヤー0 の向きベクトル（X-Z平面）
			float dir0_x = sinf(rad_0);
			float dir0_z = cosf(rad_0);

			// プレイヤー1 の向き（ラジアン）を計算
			float rad_1 = XMConvertToRadians(object[1].rotation.y);
			// プレイヤー1 の向きベクトル（X-Z平面）
			float dir1_x = sinf(rad_1);
			float dir1_z = cosf(rad_1);

			if (Keyboard_IsKeyDown(KK_SPACE))
			{
				// プレイヤー1 を、プレイヤー0の向いている方向に吹き飛ばす
				object[1].position.x += dir0_x * object[0].power;
				object[1].position.z += dir0_z * object[0].power;
				object[1].position.y += object[0].power; // Y方向にも飛ばす
			}

			if (Keyboard_IsKeyDown(KK_ENTER))
			{
				// プレイヤー0 を、プレイヤー1の向いている方向に吹き飛ばす
				object[0].position.x += dir1_x * object[1].power;
				object[0].position.z += dir1_z * object[1].power;
				object[0].position.y += object[1].power; // Y方向にも飛ばす
			}
			// 衝突していた場合、MTVの半分ずつをそれぞれのオブジェクトに適用して押し戻す

			// 押し戻し量 (MTV) を半分にする
			XMFLOAT3 half_translation =
			{
				collision_player.translation.x * 0.5f,
				collision_player.translation.y * 0.5f,
				collision_player.translation.z * 0.5f
			};

			// プレイヤー0 (object[0]) を **MTVの半分だけ** 押す
			// MTVの方向 (collision_player.translation) は「AをBから押し出す方向」だから、そのまま使う
			object[0].position.x += half_translation.x;
			object[0].position.y += half_translation.y;
			object[0].position.z += half_translation.z;
			object[0].hp -= object[1].power;

			// HPが0以下
			if (object[0].hp < 0.0f)
			{
				object[0].hp = 0.0f;
				object[0].residue -= 1;
				Polygon3D_Respawn();
			}

			// プレイヤー1 (object[1]) を **MTVの逆方向の半分だけ** 押す
			// 逆方向にするために、X, Y, Z の符号を反転させる
			object[1].position.x -= half_translation.x;
			object[1].position.y -= half_translation.y;
			object[1].position.z -= half_translation.z;
			object[1].hp -= object[0].power;

			// HPが0以下
			if (object[1].hp < 0.0f)
			{
				object[1].hp = 0.0f;
				object[1].residue -= 1;
				Polygon3D_Respawn();
			}

			// 押し戻し後の新しいAABBを再計算 (次フレームや他の衝突判定に備える)
			CalculateAABB(object[0].boundingBox, object[0].position, object[0].scaling);
			CalculateAABB(object[1].boundingBox, object[1].position, object[1].scaling);

			// 吹き飛ばし後の新しいAABBを再計算 (他の衝突判定に備える)
			CalculateAABB(object[0].boundingBox, object[0].position, object[0].scaling);
			CalculateAABB(object[1].boundingBox, object[1].position, object[1].scaling);

			// -------------------------------------------------------------
			// 当たり判定 Player1とSkill2
			// -------------------------------------------------------------
			//// AABBの更新

			// フィールドオブジェクトのリストを取得
			//int Skill2Count = GetSkill1ObjectCount();

			// 全てのフィールドオブジェクトと衝突判定を行う
			//for (int i = 0; i < Skill2Count; ++i)
			//{
			//	// i番目のフィールドオブジェクトのAABBを取得
			//	// field.cppのInitializeで計算済みのため、そのまま参照
			//	AABB pStaticObjectAABB = skill[i].boundingBox;

			//	// プレイヤーのAABBとフィールドオブジェクトのAABBでMTVを計算
			//	MTV collision = CalculateAABBMTV(object[0].boundingBox, pStaticObjectAABB);

			//	if (collision.isColliding)
			//	{
			//		// 衝突していたら、MTVの分だけ位置を戻す
			//		object[0].position.x += collision.translation.x * 3.0f;
			//		object[0].position.y += collision.translation.y * 3.0f;
			//		object[0].position.z += collision.translation.z * 3.0f;

			//		// 押し戻し後の新しいAABBを再計算
			//		// これにより、同じフレーム内で次のフィールドオブジェクトとの判定に備えます。
			//		CalculateAABB(object[i].boundingBox, object[i].position, object[i].scaling);

			//		// デバッグ出力
			//		hal::dout << "衝突！押し戻し量: " << collision.overlap << " @ " << (collision.translation.x != 0 ? "X軸" : (collision.translation.y != 0 ? "Y軸" : "Z軸")) << std::endl;

			//		// ↑↑↑　#include "debug_ostream.h"　のインクルードでデバッグ確認
			//	}
			//}
		}

		///////////////////////////////////////////////////////////////////////////////////////////////

	}
}

//======================================================
//	描画関数
//======================================================
void Polygon3D_Draw()
{
	//// スキル使用時のみスキルを表示
	//if (object[0].isAttaking == true)
	//{
	//}

	//// スキル使用時のみスキルを表示
	//if (object[1].isAttaking == true)
	//{
	//}

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		//===================
		// ワールド行列の作成
		//===================

		// スケーリング行列の作成
		XMMATRIX ScalingMatrix = XMMatrixScaling(
			object[i].scaling.x,
			object[i].scaling.y,
			object[i].scaling.z
		);
		// 平向移動行列の作成
		XMMATRIX TranslationMatrix = XMMatrixTranslation(
			object[i].position.x,
			object[i].position.y,
			object[i].position.z
		);
		// 回転行列の作成
		XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(object[i].rotation.x),
			XMConvertToRadians(object[i].rotation.y),
			XMConvertToRadians(object[i].rotation.z)
		);

		// 乗算の順番に注意！！
		XMMATRIX WorldMatrix = ScalingMatrix * RotationMatrix * TranslationMatrix;

		XMMATRIX Projection = GetProjectionMatrix();// プロジェクション行列作成
		XMMATRIX View = GetViewMatrix();// ビュー行列作成
		XMMATRIX WVP = WorldMatrix * View * Projection;// 最終的な変換行列を作成　乗算の順番に注意！！

		Shader_SetMatrix(WVP);// 変換行列を頂点シェーダーへセット
		Shader_Begin();// シェーダーを描画パイプラインへ設定

		// 頂点データを頂点バッファへコピーする
		D3D11_MAPPED_SUBRESOURCE msr;
		g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		Vertex* vertex = (Vertex*)msr.pData;

		CopyMemory(&vertex[0], &vdata[0], sizeof(Vertex) * NUM_VERTEX);	// 頂点データをコピーする
		g_pContext->Unmap(g_VertexBuffer, 0);							// コピー完了
		g_pContext->PSSetShaderResources(0, 1, &g_Texture[i]);				// テクスチャをセット

		// 頂点バッファをセット
		UINT stride = sizeof(Vertex);	// 頂点1個のデータサイズ
		UINT offset = 0;

		g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
		g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);		// インデックスバッファをセット
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	// 描画するポリゴンの種類をセット　3頂点でポリゴンを1枚として表示
		g_pContext->DrawIndexed(6 * 6, 0, 0);

		// 描画リクエスト
		//g_pContext->Draw(NUM_VERTEX, 0);
	}
}

PLAYEROBJECT* GetPlayer1()
{
	return &object[0];
}

PLAYEROBJECT* GetPlayer2()
{
	return &object[1];
}