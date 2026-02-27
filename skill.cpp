// skill.cpp

#include <DirectXMath.h>
#include <d3d11.h>
using namespace DirectX;
#include "skill.h"
#include "sprite.h"
#include "shader.h"
#include "Camera.h"
#include "collider.h"
#include "field.h"
#include "building.h"
#include "debug_ostream.h"
#include "player.h"
#include "keyboard.h"
#include "Audio.h"
#include "color.h"

// グローバル変数
static ID3D11Device* g_pDevice = NULL;
static ID3D11DeviceContext* g_pContext = NULL;

// 頂点バッファ
static ID3D11Buffer* g_VertexBuffer;

// インデックスバッファ
static ID3D11Buffer* g_IndexBuffer;

// テクスチャ変数
static ID3D11ShaderResourceView* g_Skill_Texture[4];

// オブジェクト
static SKILL_OBJECT Skill[PLAYER_MAX];

static SKILL_GLASS g_SkillGlass[PLAYER_MAX];

static int g_SE_ID[SKILL_SE_COUNT] = { NULL };

// マクロ定義
#define SKILL_VERTEX (24) // 24 頂点（キューブ各面 4 頂点 × 6 面）

static Vertex2 Skill_vdata[SKILL_VERTEX] =
{
	// -Z面 (法線: 0,0,-1)
	{// 頂点0 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),		// 座標
		XMFLOAT3(0.0f, 0.0f, -1.0f),		// 法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),	// カラー
		XMFLOAT2(0.0f,0.0f)					// テクスチャ座標
	},
	{// 頂点1 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// 頂点2 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// 頂点3 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// +X面 (法線: 1,0,0)
	{// 頂点4 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// 頂点5 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// 頂点6 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// 頂点7 RIGHT-BOTTM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// +Z面 (法線: 0,0,1)
	{// 頂点8 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// 頂点9 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// 頂点10 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// 頂点11 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// -X面 (法線: -1,0,0)
	{// 頂点12 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// 頂点13 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// 頂点14 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// 頂点15 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// +Y面 (法線: 0,1,0)
	{// 頂点16 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// 頂点17 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// 頂点18 LEFT-BOTTOM
		XMFLOAT3(-0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// 頂点19 RIGHT-BOTTOM
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// -Y面 (法線: 0,-1,0)
	{// 頂点20 LEFT-TOP
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// 頂点21 RIGHT-TOP
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// 頂点22 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// 頂点23 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},
};

// インデックス配列
static UINT Skill_idxdata[6 * 6]
{
	 0,  1,  2,  2,  1,  3, // -Z面
	 4,  5,  6,  6,  5,  7, // +X面
	 8,  9, 10, 10,  9, 11, // +Z面
	12, 13, 14, 14, 13, 15, // -X面
	16, 17, 18, 18, 17, 19, // +Y面
	20, 21, 22, 22, 21, 23, // -Y面
};

void Skill_Glass_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// 構造体のインスタンス
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		for (int i = 0; i < 5; ++i)
		{
			// 各箱の初期座標を設定
			g_SkillGlass[p].boxes[i].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_SkillGlass[p].boxes[i].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_SkillGlass[p].boxes[i].scaling = XMFLOAT3(0.2f, 0.2f, 0.2f);
			// BoundingBoxの初期化などもここで行う
		}
		// その他の初期状態を設定
		g_SkillGlass[p].isActive = false;
	}

	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\SkyBlue.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Skill_Texture[0]);
	assert(g_Skill_Texture[0]);
}

void Skill_Concrete_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	for (int p = 0; p < PLAYER_MAX; p++)
	{
		Skill[p].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Skill[p].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Skill[p].scaling = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Skill_Texture[1]);
	assert(g_Skill_Texture[1]);

}

void Skill_Plant_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	for (int p = 0; p < PLAYER_MAX; p++)
	{
		Skill[p].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Skill[p].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Skill[p].scaling = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Skill_Texture[2]);
	assert(g_Skill_Texture[2]);
}

void Skill_Electricity_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	for (int p = 0; p < PLAYER_MAX; p++)
	{
		Skill[p].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Skill[p].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Skill[p].scaling = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Skill_Texture[3]);
	assert(g_Skill_Texture[3]);
}

void Skill_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点バッファ作成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex2) * SKILL_VERTEX; // 格納できる頂点数 * 頂点サイズ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// インデックスバッファ作成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(UINT) * 6 * 6; // 格納できる頂点数 * 頂点サイズ

		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&bd, NULL, &g_IndexBuffer);

		// インデックスバッファへ書き込み
		D3D11_MAPPED_SUBRESOURCE msr;
		pContext->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		UINT* index = (UINT*)msr.pData;

		// インデックスデータをバッファへコピー
		CopyMemory(&index[0], &Skill_idxdata[0], sizeof(UINT) * 6 * 6);
		pContext->Unmap(g_IndexBuffer, 0);
	}

	Skill_Glass_Initialize(pDevice, pContext);
	Skill_Concrete_Initialize(pDevice, pContext);
	Skill_Plant_Initialize(pDevice, pContext);
	Skill_Electricity_Initialize(pDevice, pContext);

	// SEの初期化
	g_SE_ID[0] = LoadAudio("asset\\Audio\\Skill_Glass_Expansion.wav");		// スキル ガラス 展開音
	g_SE_ID[1] = LoadAudio("asset\\Audio\\Skill_Concrete_Expansion.wav");	// スキル コンクリート 展開音
	g_SE_ID[2] = LoadAudio("asset\\Audio\\Skill_Plant.wav");				// スキル 植物
	g_SE_ID[3] = LoadAudio("asset\\Audio\\Skill_Electricity.wav");			// スキル 電気
}

void Skill_Finalize()
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

	for (int i = 0; i < 4; i++)
	{
		if (g_Skill_Texture[i])
		{
			g_Skill_Texture[i]->Release();
			g_Skill_Texture[i] = NULL;
		}
	}

	for (int i = 0; i < SKILL_SE_COUNT; ++i)	UnloadAudio(g_SE_ID[i]);
}

void Skill_Glass_Update(int playerIndex)
{
	// 範囲チェック 0 1 2 3 以外なら return
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// クールタイムが残っている場合は起動をキャンセル
	if (player.useSkill && player.skillCoolTimer > 0.0f)
	{
		player.useSkill = false;
		player.skillTimer = 0.0f;
		return;
	}

	// スキル発動の最初のフレームだけSEを再生
	if (player.skillTimer == 0.0f)
	{
		PlayAudio(g_SE_ID[0], false);
		player.skillAnimation = true;
	}

	// スキルタイマー更新
	player.skillTimer += DELTA_TIME;

	// ここで Radius の値を動的に計算する
	float dynamicRadius = player.scaling.x; // scalingは等しいのでy,zでも可

	// 5つの箱に対応する相対角度 (度)
	const float RelativeAngles[5] = { 20.0f, 130.0f, 180.0f, 220.0f, 290.0f };

	// 5つの箱のプレイヤーからの高さオフセット
	const float High[5] = { 0.0f, 0.5f, 0.1f, 0.2f, 0.3f };

	// 5つの箱の回転角度 (度)
	const float Rot[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	// 5つの箱のスケーリング値
	const float Scal[5] = { 0.1f, 0.3f, 0.15f, 0.4f, 0.2f };

	// プレイヤーの現在の回転角度 (ラジアン)
	float playerYaw = XMConvertToRadians(player.rotation.y);

	SKILL_GLASS& skillGlass = g_SkillGlass[playerIndex];

	// 箱の位置・回転・スケールを更新しつつ AABB を更新
	for (int i = 0; i < 5; ++i)
	{
		float relativeRad = XMConvertToRadians(RelativeAngles[i]);
		float finalAngle = playerYaw + relativeRad;

		// 座標オフセットの計算
		float offsetX = dynamicRadius * cosf(finalAngle);
		float offsetZ = dynamicRadius * sinf(finalAngle);

		// 箱の座標を設定
		skillGlass.boxes[i].position.x = player.position.x + offsetX;
		skillGlass.boxes[i].position.y = player.position.y + High[i];
		skillGlass.boxes[i].position.z = player.position.z + offsetZ;
		skillGlass.boxes[i].rotation = XMFLOAT3(Rot[i], Rot[i], Rot[i]);
		skillGlass.boxes[i].scaling = XMFLOAT3(Scal[i], Scal[i], Scal[i]);

		// 箱の AABB を更新
		CalculateAABB(skillGlass.boxes[i].boundingBox, skillGlass.boxes[i].position, skillGlass.boxes[i].scaling);
	}

	// -------------------------------
	// 当たり判定：箱 vs 他プレイヤー
	// 仕様：ガラススキルは「ダメージあり」「押し返しなし」
	// -------------------------------
	// プレイヤー側で使っている描画スケール・ヒットボックス比率と合わせる（attack.cpp と同等）
	const float RENDER_SCALE = 2.0f;
	const float HITBOX_WIDTH_SCALE = 0.6f;
	const float HITBOX_HEIGHT_SCALE = 1.0f;
	const float HITBOX_DEPTH_SCALE = 0.6f;

	// 各箱について他プレイヤーと当たり判定
	for (int b = 0; b < 5; ++b)
	{
		SKILL_OBJECT& box = skillGlass.boxes[b];

		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			if (p == playerIndex) continue; // 自分は無視

			PLAYEROBJECT* otherPlayerObject = GetPlayer(p);
			if (otherPlayerObject == nullptr) continue;
			PLAYEROBJECT& otherPlayer = *otherPlayerObject;

			if (!otherPlayer.active) continue;		// 非アクティブは無視
			if (otherPlayer.isInvincible) continue;	// 無敵中は無視

			// リスポーン中や卵割れ中はダメージを受けないよう無視する
			if (otherPlayer.duringRespawn || otherPlayer.isEggBreaking) continue;

			// otherPlayer 用のヒットボックススケールを攻撃判定と合わせて計算して AABB を作る
			XMFLOAT3 otherPlayerHitboxScaling =
			{
				otherPlayer.scaling.x * RENDER_SCALE * HITBOX_WIDTH_SCALE,
				otherPlayer.scaling.y * RENDER_SCALE * HITBOX_HEIGHT_SCALE,
				otherPlayer.scaling.z * RENDER_SCALE * HITBOX_DEPTH_SCALE
			};
			CalculateAABB(otherPlayer.boundingBox, otherPlayer.position, otherPlayerHitboxScaling);

			// box の AABB 再更新
			CalculateAABB(box.boundingBox, box.position, box.scaling);

			// 判定（otherPlayer AABB と 箱 AABB）
			MTV col = CalculateAABBMTV(otherPlayer.boundingBox, box.boundingBox);

			// 当たってもアニメーションはなし
			if (col.isColliding)
			{
				// ダメージのみ（ノックバックは与えない） 防御率でダメージ軽減
				otherPlayer.hp -= SKILL_GLASS_DAMAGE * otherPlayer.defense;
				// HPが0以下にならないように
				if (otherPlayer.hp < 0.0f) otherPlayer.hp = 0.0f;

				otherPlayer.isDamageColor = true;	// ダメージカラーON
				otherPlayer.damageColorTimer = 0.0f;// ダメージカラータイマーリセット

				// スタンゲージ増加
				otherPlayer.stunGauge += 0.03f;
			}
		}
	}

	// スキルの効果時間が経過したらスキル終了
	if (player.skillTimer >= SKILL_GLASS_TIME)
	{
		player.useSkill = false;
		player.skillTimer = 0.0f;
		player.skillCoolTimer = SKILL_GLASS_COOLTIME;
	}
}

void Skill_Concrete_Update(int playerIndex)
{
	// 範囲チェック 0 1 2 3 以外なら return
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	SKILL_OBJECT& skillConcrete = Skill[playerIndex];

	// クールタイムが残っている場合は起動をキャンセル
	if (player.useSkill && player.skillCoolTimer > 0.0f)
	{
		player.useSkill = false;
		player.skillTimer = 0.0f;
		return;
	}

	// スキル発動の最初のフレームだけSEを再生
	if (player.skillTimer == 0.0f)
	{
		PlayAudio(g_SE_ID[1], false);
		player.skillAnimation = true;
	}

	// スキルタイマー更新
	player.skillTimer += DELTA_TIME;

	// スキル効果： ダメージ0.8倍 (デフォルトは1.0f)
	player.defense = 0.8f;

	// スキルの効果時間が経過したらスキル終了
	if (player.skillTimer >= SKILL_CONCRETE_TIME)
	{
		player.defense = 1.0f;
		player.useSkill = false;
		player.skillTimer = 0.0f;
		player.skillCoolTimer = SKILL_CONCRETE_COOLTIME;
	}
}

void Skill_Plant_Update(int playerIndex)
{
	// 範囲チェック 0 1 2 3 以外なら return
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// クールタイムが残っている場合は起動をキャンセル
	if (player.useSkill && player.skillCoolTimer > 0.0f)
	{
		player.useSkill = false;
		player.skillTimer = 0.0f;
		return;
	}

	// スキル発動の最初のフレームだけSEを再生
	if (player.skillTimer == 0.0f)
	{
		PlayAudio(g_SE_ID[2], false);
		player.skillAnimation = true;
	}

	// スキルタイマー更新
	player.skillTimer += DELTA_TIME;

	// スキル効果：進化ゲージ2倍（デフォルトは1）
	player.evolutionGaugeRate = 0.6f;

	// スキルの効果時間が経過したらスキル終了
	if (player.skillTimer >= SKILL_PLANT_TIME)
	{
		player.evolutionGaugeRate = 0.3f;
		player.useSkill = false;
		player.skillTimer = 0.0f;
		player.skillCoolTimer = SKILL_PLANT_COOLTIME;
	}
}

void Skill_Electricity_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// クールタイムが残っている場合は起動をキャンセル
	if (player.useSkill && player.skillCoolTimer > 0.0f)
	{
		player.useSkill = false;
		player.skillTimer = 0.0f;
		return;
	}

	float baseSpeed = 0.06f;								// 第1形態
	if (player.form == Form::Second) baseSpeed = 0.05f;		// 第2形態
	else if (player.form == Form::Third) baseSpeed = 0.04f;	// 第3形態

	// スキル発動の最初のフレームだけSEを再生
	if (player.skillTimer == 0.0f)
	{
		PlayAudio(g_SE_ID[3], false);
		player.skillAnimation = true;
	}

	// スキルタイマー更新
	player.skillTimer += DELTA_TIME;

	// スキル効果：スピード1.5倍
	player.speed = baseSpeed * 1.5f;
	
	// スキルの効果時間が経過したらスキル終了
	if (player.skillTimer >= SKILL_ELECTRICITY_TIME)
	{
		player.speed = baseSpeed * 1.0f;
		player.useSkill = false;
		player.skillTimer = 0.0f;
		player.skillCoolTimer = SKILL_ELECTRICITY_COOLTIME;
	}
}

void Skill_Update(int playerIndex)
{
	// 範囲チェック 0 1 2 3 以外なら return
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// スキル使用中かつスタン中でない場合に更新処理を行う
	if (player.useSkill && !player.isStunning)
	{
		switch (player.type)
		{
		case PlayerType::Glass:			Skill_Glass_Update(playerIndex);		break;
		case PlayerType::Concrete:		Skill_Concrete_Update(playerIndex);		break;
		case PlayerType::Plant:			Skill_Plant_Update(playerIndex);		break;
		case PlayerType::Electricity:	Skill_Electricity_Update(playerIndex);	break;
		default: break;
		}
	}
}

// Glass専用描画 (5つの箱をループで描画)
void Skill_Glass_Draw(int playerIndex)
{
	// Glass専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Skill_Texture[0];
	g_pContext->PSSetShaderResources(0, 1, &tex);

	SKILL_GLASS& skillGlass = g_SkillGlass[playerIndex];

	// GlassSkill構造体（5つの箱）を使ってループ描画
	for (int i = 0; i < 5; ++i)
	{
		SKILL_OBJECT& box = skillGlass.boxes[i];

		// --- ワールド行列計算 ---
		XMMATRIX WorldMatrix =
			XMMatrixScaling(box.scaling.x, box.scaling.y, box.scaling.z) *
			XMMatrixRotationRollPitchYaw(XMConvertToRadians(box.rotation.x), XMConvertToRadians(box.rotation.y), XMConvertToRadians(box.rotation.z)) *
			XMMatrixTranslation(box.position.x, box.position.y, box.position.z);

		// 行列セット
		XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
		Shader_SetMatrix(WVP);

		// 描画実行
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		g_pContext->DrawIndexed(6 * 6, 0, 0);
	}
}

// Concrete専用描画
void Skill_Concrete_Draw(int playerIndex)
{
	// Concrete専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Skill_Texture[1];
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// Concrete用の座標計算
	SKILL_OBJECT& skillConcrete = Skill[playerIndex];

	XMMATRIX WorldMatrix =
		XMMatrixScaling(skillConcrete.scaling.x, skillConcrete.scaling.y, skillConcrete.scaling.z) *
		XMMatrixRotationRollPitchYaw(XMConvertToRadians(skillConcrete.rotation.x), XMConvertToRadians(skillConcrete.rotation.y), XMConvertToRadians(skillConcrete.rotation.z)) *
		XMMatrixTranslation(skillConcrete.position.x, skillConcrete.position.y, skillConcrete.position.z);

	XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(WVP);

	// 描画実行
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pContext->DrawIndexed(6 * 6, 0, 0);
}

// Plant専用描画
void Skill_Plant_Draw(int playerIndex)
{
	// Plant専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Skill_Texture[1];
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// Plant用の座標計算
	SKILL_OBJECT& skillPlant = Skill[playerIndex];

	XMMATRIX WorldMatrix =
		XMMatrixScaling(skillPlant.scaling.x, skillPlant.scaling.y, skillPlant.scaling.z) *
		XMMatrixRotationRollPitchYaw(XMConvertToRadians(skillPlant.rotation.x), XMConvertToRadians(skillPlant.rotation.y), XMConvertToRadians(skillPlant.rotation.z)) *
		XMMatrixTranslation(skillPlant.position.x, skillPlant.position.y, skillPlant.position.z);

	XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(WVP);

	// 描画実行
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pContext->DrawIndexed(6 * 6, 0, 0);
}

// Electirc専用描画
void Skill_Electricity_Draw(int playerIndex)
{
	// Electirc専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Skill_Texture[1];
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// Electirc用の座標計算
	SKILL_OBJECT& skillElectricity = Skill[playerIndex];

	XMMATRIX WorldMatrix =
		XMMatrixScaling(skillElectricity.scaling.x, skillElectricity.scaling.y, skillElectricity.scaling.z) *
		XMMatrixRotationRollPitchYaw(XMConvertToRadians(skillElectricity.rotation.x), XMConvertToRadians(skillElectricity.rotation.y), XMConvertToRadians(skillElectricity.rotation.z)) *
		XMMatrixTranslation(skillElectricity.position.x, skillElectricity.position.y, skillElectricity.position.z);

	XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(WVP);

	// 描画実行
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pContext->DrawIndexed(6 * 6, 0, 0);

}

void Skill_Draw(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// そのプレイヤーがスキルを使っているかチェック
	if (!player.useSkill) return;

	// プレイヤーがスタンしていない場合のみ描画
	if (player.isStunning == false)
	{
		// ライトを設定
		LIGHT light{};
		light.Enable = TRUE;
		light.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
		light.Diffuse = XMFLOAT4(1.5f, 1.5f, 1.5f, 1.0f);
		light.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		Shader_SetLight(light);

		// 1. 共通設定 (パイプラインステートの設定)
		//    これを親で一度だけやることで処理落ちを防ぐ

		// シェーダー開始
		Shader_Begin();

		// ブレンドステート
		SetBlendState(BLENDSTATE_NONE); // または BLENDSTATE_ALPHA
		Shader_SetColor(color::white);

		// 頂点バッファ・インデックスバッファのセット
		UINT stride = sizeof(Vertex2);
		UINT offset = 0;
		g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
		g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 頂点データ書き込み
		D3D11_MAPPED_SUBRESOURCE msr;
		g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		Vertex2* vertex = (Vertex2*)msr.pData;
		CopyMemory(&vertex[0], &Skill_vdata[0], sizeof(Vertex2) * SKILL_VERTEX);
		g_pContext->Unmap(g_VertexBuffer, 0);

		// プレイヤーのタイプに合わせて子関数を呼ぶ
		switch (player.type)
		{
		case PlayerType::Glass:			Skill_Glass_Draw(playerIndex);			break;
		case PlayerType::Concrete:		Skill_Concrete_Draw(playerIndex);		break;
		case PlayerType::Plant:			Skill_Plant_Draw(playerIndex);			break;
		case PlayerType::Electricity:	Skill_Electricity_Draw(playerIndex);	break;
		default: break;
		}

		// 3. 後始末
		SetBlendState(BLENDSTATE_ALPHA);
	}
}

SKILL_OBJECT* GetSkill(int playerIndex)
{
	// 範囲チェック 0 1 2 3 以外なら nullptr を返す
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX)	return nullptr;

	return &Skill[playerIndex];
}

