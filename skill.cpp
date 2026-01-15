// skill.cpp

#include "DirectXMath.h"
#include "d3d11.h"
using namespace DirectX;

#include "skill.h"
#include "sprite.h"
#include "shader.h"
#include "Camera.h"
#include "collider.h"
#include "field.h"
#include "building.h"
#include "debug_ostream.h"
#include "Polygon3D.h"
#include "keyboard.h"

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

// マクロ定義
#define NUM_VERTEX (24) // 24 頂点（キューブ各面 4 頂点 × 6 面）

static Vertex2 Skill_vdata[NUM_VERTEX] =
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

void Skill_Electric_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
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
	bd.ByteWidth = sizeof(Vertex2) * NUM_VERTEX; // 格納できる頂点数 * 頂点サイズ
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
	Skill_Electric_Initialize(pDevice, pContext);
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
}

void Skill_Glass_Update(int playerIndex)
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

	// ここで Radius の値を動的に計算する
	float dynamicRadius = player.scaling.x; // scalingは等しいのでy,zでも可

	// 5つの箱に対応する相対角度 (度)
	const float RelativeAngles[5] = { 20.0f, 130.0f, 180.0f, 220.0f, 290.0f };

	// 5つの箱のプレイヤーからの高さオフセット
	const float High[5] = { 0.0f, 0.5f, -0.1f, 0.2f, 0.3f };

	// 5つの箱の回転角度 (度)
	const float Rot[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	// 5つの箱のスケーリング値
	const float Scal[5] = { 0.05f, 0.15f, 0.075f, 0.2f, 0.1f };

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

		for (int def = 0; def < PLAYER_MAX; ++def)
		{
			if (def == playerIndex) continue; // 自分は無視

			PLAYEROBJECT* defender = GetPlayer(def);
			if (defender == nullptr) continue;

			if (!defender->active) continue;      // 非アクティブは無視
			if (defender->isInvincible) continue; // 無敵中は無視

			// defender 用のヒットボックススケールを攻撃判定と合わせて計算して AABB を作る
			XMFLOAT3 defenderHitboxScaling =
			{
				defender->scaling.x * RENDER_SCALE * HITBOX_WIDTH_SCALE,
				defender->scaling.y * RENDER_SCALE * HITBOX_HEIGHT_SCALE,
				defender->scaling.z * RENDER_SCALE * HITBOX_DEPTH_SCALE
			};
			CalculateAABB(defender->boundingBox, defender->position, defenderHitboxScaling);

			// box の AABB 再更新
			CalculateAABB(box.boundingBox, box.position, box.scaling);

			// 判定（defender AABB と 箱 AABB）
			MTV col = CalculateAABBMTV(defender->boundingBox, box.boundingBox);

			// 当たってもアニメーションはなし
			if (col.isColliding)
			{
				// ダメージのみ（ノックバックは与えない）
				float damage = 0.01f;

				// 防御率でダメージ軽減
				defender->hp -= damage * defender->defense;
				// HPが0以下にならないように
				if (defender->hp < 0.0f) defender->hp = 0.0f;

				// スタンゲージ増加
				defender->stunGauge += 0.01f;
			}
		}
	}

	// スキルタイマー更新
	player.skillTimer += DELTA_TIME;

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
	// 範囲チェック
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

	// スキルタイマー更新
	player.skillTimer += DELTA_TIME;

	// スキル効果： ダメージ0.8倍 (デフォルトは1.0f)
	player.defense = 0.8f;

	// スキルの初期位置をプレイヤーの位置に設定
	skillConcrete.position.x = player.position.x;
	skillConcrete.position.y = player.position.y;
	skillConcrete.position.z = player.position.z;

	// スキルタイマー更新
	player.skillTimer += DELTA_TIME;

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

	// スキルタイマー更新
	player.skillTimer += DELTA_TIME;

	// スキル効果：進化ゲージ2倍（デフォルトは1）
	player.evolutionGaugeRate = 2;

	// スキルの効果時間が経過したらスキル終了
	if (player.skillTimer >= SKILL_PLANT_TIME)
	{
		player.evolutionGaugeRate = 1;
		player.useSkill = false;
		player.skillTimer = 0.0f;
		player.skillCoolTimer = SKILL_PLANT_COOLTIME;
	}
}

void Skill_Electric_Update(int playerIndex)
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

	float baseSpeed = 0.06f; // Normal
	if (player.form == Form::FirstEvolution) baseSpeed = 0.05f;
	else if (player.form == Form::SecondEvolution) baseSpeed = 0.04f;

	// スキルタイマー更新
	player.skillTimer += DELTA_TIME;

	// スキル効果：スピード1.5倍
	player.speed = baseSpeed * 1.5f;
	
	// スキルの効果時間が経過したらスキル終了
	if (player.skillTimer >= SKILL_ELECTRIC_TIME)
	{
		player.speed = baseSpeed * 1.0f;
		player.useSkill = false;
		player.skillTimer = 0.0f;
		player.skillCoolTimer = SKILL_ELECTRIC_COOLTIME;
	}
}

void Skill_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// スキル使用中かつスタン中でない場合に更新処理を行う
	if (player.useSkill && !player.isStunning)
	{
		switch (player.type)
		{
		case PlayerType::Glass:		Skill_Glass_Update(playerIndex);	break;
		case PlayerType::Concrete:	Skill_Concrete_Update(playerIndex);	break;
		case PlayerType::Plant:		Skill_Plant_Update(playerIndex);	break;
		case PlayerType::Electric:	Skill_Electric_Update(playerIndex);	break;
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
	g_pContext->DrawIndexed(6 * 6, 0, 0);
}

// Electirc専用描画
void Skill_Electric_Draw(int playerIndex)
{
	// Electirc専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Skill_Texture[1];
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// Electirc用の座標計算
	SKILL_OBJECT& skillElectric = Skill[playerIndex];

	XMMATRIX WorldMatrix =
		XMMatrixScaling(skillElectric.scaling.x, skillElectric.scaling.y, skillElectric.scaling.z) *
		XMMatrixRotationRollPitchYaw(XMConvertToRadians(skillElectric.rotation.x), XMConvertToRadians(skillElectric.rotation.y), XMConvertToRadians(skillElectric.rotation.z)) *
		XMMatrixTranslation(skillElectric.position.x, skillElectric.position.y, skillElectric.position.z);

	XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(WVP);

	// 描画実行
	g_pContext->DrawIndexed(6 * 6, 0, 0);

}

void Skill_Draw()
{
	// ライトを設定（Polygon3D::Draw と同様のライト）
	LIGHT light{};
	light.Enable = TRUE;
	light.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
	light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	Shader_SetLight(light);

	// 1. 共通設定 (パイプラインステートの設定)
	//    これを親で一度だけやることで処理落ちを防ぐ

	// シェーダー開始
	Shader_Begin();

	// ブレンドステート
	SetBlendState(BLENDSTATE_NONE); // または BLENDSTATE_ALPHA
	Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

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
	CopyMemory(&vertex[0], &Skill_vdata[0], sizeof(Vertex2) * NUM_VERTEX);
	g_pContext->Unmap(g_VertexBuffer, 0);

	// 2. プレイヤーごとの振り分け処理
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		PLAYEROBJECT* playerObject = GetPlayer(p);
		if (playerObject == nullptr) continue;
		PLAYEROBJECT& player = *playerObject;

		// そのプレイヤーがスキルを使っているかチェック
		if (!player.useSkill) continue;

		// プレイヤーがスタンしていない場合のみ描画
		if (player.isStunning == false)
		{
			// プレイヤーのタイプに合わせて子関数を呼ぶ
			switch (player.type)
			{
		case PlayerType::Glass:		Skill_Glass_Draw(p);	break;
		case PlayerType::Concrete:	//Skill_Concrete_Draw(p);	break;
		case PlayerType::Plant:		//Skill_Plant_Draw(p);		break;
		case PlayerType::Electric:	//Skill_Electric_Draw(p);	break;
		default: break;
			}
		}
	}

	// 3. 後始末
	SetBlendState(BLENDSTATE_ALPHA);
}

SKILL_OBJECT* GetSkill(int playerIndex)
{
	// 範囲チェック 0未満 または 4以上なら nullptr を返す
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX)
	{
		return nullptr;
	}

	return &Skill[playerIndex];
}