// attack.cpp

#include <DirectXMath.h>
#include <d3d11.h>
using namespace DirectX;
#include "attack.h"
#include "sprite.h"
#include "shader.h"
#include "Camera.h"
#include "collider.h"
#include "field.h"
#include "Building.h"
#include "debug_ostream.h"
#include "player.h"
#include "keyboard.h"
#include "DamageText.h"
#include "Effect.h"
#include "input.h"
#include "hp.h"
#include "color.h"
#include "gamepad.h"
#include "Audio.h"
#include "game.h"

// グローバル変数
static ID3D11Device* g_pDevice = NULL;
static ID3D11DeviceContext* g_pContext = NULL;

// 頂点バッファ
static ID3D11Buffer* g_VertexBuffer;

// インデックスバッファ
static ID3D11Buffer* g_IndexBuffer;

// テクスチャ変数
static ID3D11ShaderResourceView* g_Attack_Texture[PLAYER_MAX];

// オブジェクト
static ATTACK_OBJECT Attack[PLAYER_MAX];

static int g_SE_ID[ATTACK_SE_COUNT] = { NULL };

// マクロ定義
#define ATTACK_VERTEX (24)

//////////////////////////////////////////////////////////////////////////////////////////
// TODO:三角形の攻撃用の当たり判定を作り、2重に食らうのをなくし、ヒットストップを作る

static Vertex2 Attack_vdata[ATTACK_VERTEX] =
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
static UINT Attack_idxdata[6 * 6]
{
	 0,  1,  2,  2,  1,  3, // -Z面
	 4,  5,  6,  6,  5,  7, // +X面
	 8,  9, 10, 10,  9, 11, // +Z面
	12, 13, 14, 14, 13, 15, // -X面
	16, 17, 18, 18, 17, 19, // +Y面
	20, 21, 22, 22, 21, 23, // -Y面
};


void Attack_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	for (int p = 0; p < PLAYER_MAX; p++)
	{
		Attack[p].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Attack[p].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Attack[p].scaling = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	// 頂点バッファ作成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex2) * ATTACK_VERTEX; // 格納できる頂点数 * 頂点サイズ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	g_pDevice = pDevice;
	g_pContext = pContext;

	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(),image.GetImageCount(), metadata, &g_Attack_Texture[0]);
	assert(g_Attack_Texture[0]);

	LoadFromWICFile(L"Asset\\Texture\\SkyBlue.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(),image.GetImageCount(), metadata, &g_Attack_Texture[1]);
	assert(g_Attack_Texture[1]);

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
		CopyMemory(&index[0], &Attack_idxdata[0], sizeof(UINT) * 6 * 6);
		pContext->Unmap(g_IndexBuffer, 0);
	}

	// SEの初期化
	g_SE_ID[0] = LoadAudio("asset\\Audio\\BuildingDestroy.wav");	// 建物 崩壊
	g_SE_ID[1] = LoadAudio("asset\\Audio\\gabugabu01.wav");			// プレイヤーをがぶがぶする音
	g_SE_ID[2] = LoadAudio("asset\\Audio\\transform.wav");			// 

}

void Attack_Finalize()
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

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (g_Attack_Texture[i])
		{
			g_Attack_Texture[i]->Release();
			g_Attack_Texture[i] = NULL;
		}
	}

	for (int i = 0; i < ATTACK_SE_COUNT; ++i)	UnloadAudio(g_SE_ID[i]);
}

void Attack_Update(int playerIndex)
{
	// 範囲チェックとプレイヤー取得
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;
	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	ATTACK_OBJECT& attackObject = Attack[playerIndex];

	// =========================================================
	// 1. 攻撃中の処理（座標の更新 ＆ 当たり判定）
	// =========================================================
	if (player.isAttacking)
	{
		// --- 攻撃の位置と向きを更新 ---
		float rad = XMConvertToRadians(player.rotation.y);
		player.dir = { sinf(rad), 0.0f, cosf(rad) }; // プレイヤーの向きを保存

		attackObject.position.x = player.dir.x * player.scaling.x + player.position.x;
		attackObject.position.y = player.position.y;
		attackObject.position.z = player.dir.z * player.scaling.z + player.position.z;

		// --- ★扇形（Sector）を1回だけ作る ---
		Sector attackSector;
		attackSector.center = player.position;
		attackSector.forward = player.dir;
		//attackSector.radius = 30.0f;        // 攻撃距離
		//attackSector.angleDegree = 1200.0f; // 攻撃範囲

		Keyboard_Keys_tag confirmKey[PLAYER_MAX] = { KK_SPACE , KK_ENTER, KK_V, KK_SPACE };
		bool isAttackKeyPressed = (g_Input[playerIndex].A || Keyboard_IsKeyDown(confirmKey[playerIndex]));


		// ---------------------------------------------------------
		// [A] 建物との当たり判定
		// ---------------------------------------------------------
		int buildingCount = GetBuildingCount();
		Building** buildingObjects = GetBuildings();

		for (int i = 0; i < buildingCount; ++i)
		{
			if (!buildingObjects[i]->isActive) continue;

			if (CheckAABBSectorCollision(buildingObjects[i]->boundingBox, attackSector))
			{
				if (isAttackKeyPressed) {
					// 攻撃キーが押されていれば破壊！
					if (isAttackKeyPressed)
					{
						BuildingType type = buildingObjects[i]->type;

						// 壊した種類をカウント
						switch (type) {
						case BuildingType::Glass:		player.breakCount_Glass += 1;		break;
						case BuildingType::Concrete:	player.breakCount_Concrete += 1;	break;
						case BuildingType::Plant:		player.breakCount_Plant += 1;		break;
						case BuildingType::Electricity:	player.breakCount_Electricity += 1;	break;
						}

						buildingObjects[i]->isActive = false;
						buildingObjects[i]->isDestroyed = true;
						player.brokenHistory.push_back(type);
						player.evolutionGauge += player.evolutionGaugeRate;

						// HPと満腹度を回復（上限を超えないように min を使うと1行で書けるよ！）
						player.hp = min(player.hp + 10.0f, PLAYER_MAX_HP);
						player.satiety = min(player.satiety + 1.0f, PLAYER_MAX_SATIETY);
						player.isHealing = true;

						//// ヒットしたので攻撃終了
						//player.isAttacking = false;
						//player.attackTimer = 0.0f;

						CalculateAABB(attackObject.boundingBox, attackObject.position, attackObject.scaling);
						//break; // ★1つ壊したらループを抜ける（複数同時破壊したい場合は消してね）

						// 建物壊した時に 0.05秒くらい止める
						StartHitStop(0.05f);
					}
				}
			}
		}

		// ---------------------------------------------------------
		// [B] 他プレイヤーとの当たり判定
		// ---------------------------------------------------------
		// ★ atk のループは削除！「自分(player)」が「相手(defender)」に当たるかだけ調べるよ。
		const float RENDER_SCALE = 2.0f;
		const float HITBOX_HEIGHT_SCALE = 1.0f;
		const float HITBOX_SHORT = 0.35f;
		const float HITBOX_LONG = 0.65f;

		// 自分がまだ攻撃中なら他プレイヤーへの判定を行う
		if (player.isAttacking)
		{
			for (int def = 0; def < PLAYER_MAX; ++def)
			{
				if (def == playerIndex) continue; // 自分には当たらない

				PLAYEROBJECT* defenderObject = GetPlayer(def);
				if (defenderObject == nullptr || !defenderObject->active || defenderObject->isInvincible) continue;

				PLAYEROBJECT& defender = *defenderObject;

				// defenderの向きからAABBのサイズを決める
				float radDef = XMConvertToRadians(defender.rotation.y);
				bool defFacingZDominant = fabsf(cosf(radDef)) >= fabsf(sinf(radDef));

				float widthScale = defFacingZDominant ? HITBOX_SHORT : HITBOX_LONG;
				float depthScale = defFacingZDominant ? HITBOX_LONG : HITBOX_SHORT;

				if (defender.form == Form::Second || defender.form == Form::Third) {
					widthScale = 0.3f; depthScale = 0.3f;
				}

				XMFLOAT3 defenderHitboxScaling = {
					defender.scaling.x * RENDER_SCALE * widthScale,
					defender.scaling.y * RENDER_SCALE * HITBOX_HEIGHT_SCALE,
					defender.scaling.z * RENDER_SCALE * depthScale
				};
				CalculateAABB(defender.boundingBox, defender.position, defenderHitboxScaling);

				// ★扇形とプレイヤーの判定
				if (CheckAABBSectorCollision(defender.boundingBox, attackSector))
				{
					// ノックバックとダメージ処理
					defender.velocity.x = player.dir.x * player.power * defender.weight;
					defender.velocity.y = 2.0f; // 少し浮かす
					defender.velocity.z = player.dir.z * player.power * defender.weight;

					float rawDamage = player.attack * defender.defense;
					defender.hp = max(defender.hp - rawDamage, 0.0f); // 0未満にならないように

					TriggerbyHPShake(def, 8.0f, 20.0f, 1.5f);
					defender.stunGauge += 0.5f;

					// ダメージ文字
					XMFLOAT3 hitPos = defender.position;
					hitPos.y += defender.scaling.y + 0.3f;
					SetDamageText(hitPos, static_cast<int>(rawDamage + 0.5f), TextColor::Blue);

					defender.isAttacked = true;
					defender.attackedTimer = 0.0f;
					CalculateAABB(defender.boundingBox, defender.position, defenderHitboxScaling);

					//// ヒットしたので攻撃終了
					//player.isAttacking = false;
					//player.attackTimer = 0.0f;
					//break; // ★1人に当てたら終わり（複数人巻き込みたい場合は消してね）

					// プレイヤーに当てた時はちょっと長めに 0.1秒くらい止める
					StartHitStop(0.1f);
				}
			}
		}

		// --- 攻撃タイマーの更新（攻撃中だった場合） ---
		if (player.isAttacking)
		{
			player.attackTimer += DELTA_TIME;
			if (player.attackTimer >= ATTACKING_TIME)
			{
				player.isAttacking = false;
				player.attackTimer = 0.0f;
			}
		}
	}

	// =========================================================
	// 2. 進化処理 (攻撃していなくてもゲージが溜まっていれば進化)
	// =========================================================
	if (player.evolutionGauge >= EVOLUTIONGAUGE_MAX)
	{
		player.isInvincible = true;
		player.invincibleTimer = 0.0f;
		player.isEvolving = true;
		player.evolvingTimer += DELTA_TIME;

		Form currentForm = player.form;
		player.form = static_cast<Form>(static_cast<int>(player.form) + 1);
		if (player.form >= Form::Third) player.form = Form::Third;

		// --- タイプ決定ロジック ---
		if (currentForm == Form::First)
		{
			const int counts[4] = { player.breakCount_Glass, player.breakCount_Concrete, player.breakCount_Plant, player.breakCount_Electricity };
			const BuildingType types[4] = { BuildingType::Glass, BuildingType::Concrete, BuildingType::Plant, BuildingType::Electricity };

			int maxCount = 0;
			for (int i = 0; i < 4; i++) if (counts[i] > maxCount) maxCount = counts[i];

			int maxIdx = 0;
			for (int i = player.brokenHistory.size() - 1; i >= 0; i--)
			{
				BuildingType historyType = player.brokenHistory[i];
				int typeIdx = -1;
				for (int j = 0; j < 4; j++) if (historyType == types[j]) { typeIdx = j; break; }

				if (typeIdx != -1 && counts[typeIdx] == maxCount) { maxIdx = typeIdx; break; }
			}

			switch (maxIdx) {
			case 0: player.type = PlayerType::Glass;		break;
			case 1: player.type = PlayerType::Concrete;		break;
			case 2: player.type = PlayerType::Plant;		break;
			case 3: player.type = PlayerType::Electricity;	break;
			}
			player.isTypeFixed = true;
		}

		// --- 第3形態のエフェクト ---
		if (player.form == Form::Third && currentForm != Form::Third)
		{
			float screenX = SCREEN_ADJUST_X;
			float screenY = 620.0f * SCREEN_ADJUST_Y;
			const XMFLOAT2 playerEffectPos[PLAYER_MAX] = {
				{ 170.0f * screenX, screenY }, { 490.0f * screenX, screenY },
				{ 810.0f * screenX, screenY }, { 1130.0f * screenX, screenY }
			};

			int effectTexNo = 0;
			switch (player.type) {
			case PlayerType::Glass:			effectTexNo = 0; break;
			case PlayerType::Concrete:		effectTexNo = 1; break;
			case PlayerType::Plant:			effectTexNo = 2; break;
			case PlayerType::Electricity:	effectTexNo = 3; break;
			}
			Effect_SetUI(effectTexNo, playerEffectPos[playerIndex], { 350.0f, 350.0f });
		}

		// --- リセット処理 ---
		player.brokenHistory.clear();
		player.evolutionGauge = 0;
		player.breakCount_Glass = 0;
		player.breakCount_Concrete = 0;
		player.breakCount_Plant = 0;
		player.breakCount_Electricity = 0;
	}
}

void Attack_Draw(int playerIndex)
{
	// 範囲チェック 0 1 2 3 以外なら return
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	// 参照を取る
	ATTACK_OBJECT& attackObject = Attack[playerIndex];
	ID3D11ShaderResourceView* tex = g_Attack_Texture[playerIndex];


	// 1. プレイヤー情報を取得（向きや座標を使うため）
	PLAYEROBJECT* playerPtr = GetPlayer(playerIndex);
	if (!playerPtr) return;
	PLAYEROBJECT& player = *playerPtr;

	// =====================
	// ワールド行列の作成
	// =====================

	// スケーリング行列の作成
	XMMATRIX ScalingMatrix = XMMatrixScaling
	(
		attackObject.scaling.x,
		attackObject.scaling.y,
		attackObject.scaling.z
	);

	// 回転行列の作成
	XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw
	(
		XMConvertToRadians(attackObject.rotation.x),
		XMConvertToRadians(attackObject.rotation.y),
		XMConvertToRadians(attackObject.rotation.z)
	);

	// 平行移動行列の作成
	XMMATRIX TranslationMatrix = XMMatrixTranslation
	(
		attackObject.position.x,
		attackObject.position.y,
		attackObject.position.z
	);

	XMMATRIX WorldMatrix = ScalingMatrix * RotationMatrix * TranslationMatrix;

	// プロジェクション行列作成
	XMMATRIX projection = GetProjectionMatrix();

	// ビュー行列作成
	XMMATRIX view = GetViewMatrix();

	// 最終的な変換行列を作成
	XMMATRIX WVP = WorldMatrix * view * projection;

	// 変換行列を頂点シェーダーへセット
	Shader_SetMatrix(WVP);

	LIGHT light{};
	light.Enable = TRUE;
	// 光の向き（ワールド空間）シェーダー側で単位化して使っている想定
	light.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
	// 拡散光と環境光
	light.Diffuse = XMFLOAT4(1.5f, 1.5f, 1.5f, 1.0f);
	light.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	Shader_SetLight(light);

	// シェーダーを描画パイプラインへ設定
	Shader_Begin();

	// 不透明で描画するためブレンドを無効化し、描画カラーのアルファを1に固定する
	SetBlendState(BLENDSTATE_NONE);
	Shader_SetColor(color::white);

	// 頂点シェーダーを描画パイプラインへ設定
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex2* vertex = (Vertex2*)msr.pData;

	// 頂点データを頂点バッファへコピーする
	CopyMemory(&vertex[0], &Attack_vdata[0], sizeof(Vertex2) * ATTACK_VERTEX);

	// コピー完了
	g_pContext->Unmap(g_VertexBuffer, 0);

	// テクスチャをセット
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// 頂点バッファをセット
	UINT stride = sizeof(Vertex2);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// インデックスバッファをセット
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 描画するポリゴンの種類をセット 3頂点でポリゴン1枚として表示
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_pContext->DrawIndexed(6 * 6, 0, 0);

	SetBlendState(BLENDSTATE_ALPHA);


	// 2. デバッグ描画：攻撃中だけ扇形を表示する
	if (player.isAttacking)
	{
		// 判定で使っているのと「全く同じ」設定で扇を作る
		Sector debugSector;
		debugSector.center = player.position;

		// プレイヤーの向きから前方ベクトルを計算
		float rad = XMConvertToRadians(player.rotation.y);
		debugSector.forward = { sinf(rad), 0.0f, cosf(rad) };

		// 扇形を描画！
		DrawDebugSector(debugSector);
	}

	// 最後に念のためブレンド状態などを戻す
	SetBlendState(BLENDSTATE_ALPHA);
}

void AttackPlayerCollisions()
{
	// 各プレイヤーの攻撃オブジェクトをループして、他プレイヤー全員に当たり判定を行う
	for (int atk = 0; atk < PLAYER_MAX; ++atk)
	{
		if (atk < 0 || atk >= PLAYER_MAX) continue; // 追加の安全チェック
		ATTACK_OBJECT& attackObject = Attack[atk];

		// player は既存の GetPlayer を使ってヌルチェック
		PLAYEROBJECT* attackerPtr = GetPlayer(atk);
		if (attackerPtr == nullptr) continue;
		PLAYEROBJECT& attacker = *attackerPtr;

		if (!attacker.isAttacking) continue;	// 攻撃中のみ判定

		// 攻撃オブジェクトと攻撃者の AABB を更新
		CalculateAABB(attackObject.boundingBox, attackObject.position, attackObject.scaling);
		CalculateAABB(attacker.boundingBox, attacker.position, attacker.scaling);

		// 攻撃者の向きベクトルを更新（rotation.y から算出）
		{
			float rad = XMConvertToRadians(attacker.rotation.y);
			attacker.dir.x = sinf(rad);
			attacker.dir.z = cosf(rad);
		}

		// --- プレイヤー側で使っている描画スケール・ヒットボックス比率と合わせる ---
		const float RENDER_SCALE = 2.0f;
		const float HITBOX_HEIGHT_SCALE = 1.0f;
		// Player と同じ短辺/長辺定義を使う
		const float HITBOX_SHORT = 0.35f;
		const float HITBOX_LONG = 0.65f;

		// 攻撃が当たる対象として他プレイヤー全員をチェック
		for (int def = 0; def < PLAYER_MAX; ++def)
		{
			if (def == atk) continue; // 自分には当たらない

			PLAYEROBJECT* defenderObject = GetPlayer(def);
			if (defenderObject == nullptr) continue;
			PLAYEROBJECT& defender = *defenderObject;

			if (!defender.active) continue;

			// リスポーン中や卵割れ中はダメージを受けないよう無視する
			if (defender.duringRespawn || defender.isEggBreaking) continue;

			// 被弾中や無敵ならスキップ
			if (defender.isInvincible) continue;

			// defender の向きから短辺/長辺を決める
			float radDef = XMConvertToRadians(defender.rotation.y);
			float defFacingX = sinf(radDef);
			float defFacingZ = cosf(radDef);
			bool defFacingZDominant = fabsf(defFacingZ) >= fabsf(defFacingX);

			float widthScale = defFacingZDominant ? HITBOX_SHORT : HITBOX_LONG;		// X方向スケール
			float depthScale = defFacingZDominant ? HITBOX_LONG : HITBOX_SHORT;	// Z方向スケール

			// 第2形態 第3形態はXとZ同じにする
			if (defender.form == Form::Second || defender.form == Form::Third)
			{
				widthScale = 0.3f;
				depthScale = 0.3f;
			}

			// defender 用のヒットボックススケールを計算して AABB を作る
			XMFLOAT3 defenderHitboxScaling =
			{
				defender.scaling.x * RENDER_SCALE * widthScale,
				defender.scaling.y * RENDER_SCALE * HITBOX_HEIGHT_SCALE,
				defender.scaling.z * RENDER_SCALE * depthScale
			};
			CalculateAABB(defender.boundingBox, defender.position, defenderHitboxScaling);

			// 判定（defender AABB と 攻撃オブジェクト AABB）
			MTV col = CalculateAABBMTV(defender.boundingBox, attackObject.boundingBox);

			if (col.isColliding)
			{
				//// ノックバック（攻撃者の向きと攻撃力を使用）
				//defender.position.x += attacker.dir.x/* * attacker.power*/;
				//defender.position.y += attacker.power;
				//defender.position.z += attacker.dir.z/* * attacker.power*/;

				// 吹っ飛ばす強さ（ここを大きくするとめっちゃ飛ぶ！）
				float liftUpPower = 2.0f;    // 少し上に浮かせると吹っ飛ばされた感が出るよ

				// 座標を直接いじるのではなく、速度（velocity）に力を溜める
				defender.velocity.x = attacker.dir.x * attacker.power * defender.weight;
				defender.velocity.y = liftUpPower;
				defender.velocity.z = attacker.dir.z * attacker.power * defender.weight;

				// ダメージ用変数
				float rawDamage = attacker.attack * defender.defense;

				// ダメージ（防御で軽減）
				defender.hp -= rawDamage;
				if (defender.hp < 0.0f) defender.hp = 0.0f;

				TriggerbyHPShake(def, 8.0f, 20.0f, 1.5f);
				TriggerVibration(def, 0.1f, 0.1f, 50);

				// スタンゲージ増加
				defender.stunGauge += 0.5f;

				// ダメージ数字を表示（頭上にオフセット）
				int dmgInt = static_cast<int>(rawDamage + 0.5f);
				XMFLOAT3 hitPos = defender.position;
				hitPos.y += defender.scaling.y + 0.3f;

				/*TextColor dmgColor = TextColor::White;

				if (def == 0)      dmgColor = TextColor::P1color;
				else if (def == 1) dmgColor = TextColor::P2color;
				else if (def == 2) dmgColor = TextColor::P3color;
				else if (def == 3) dmgColor = TextColor::P4color;*/

				SetDamageText(hitPos, dmgInt, TextColor::Red);

				// ダメージフラグ・タイマー（アニメ/UI 用）
				defender.isAttacked = true;
				defender.attackedTimer = 0.0f;

				// がぶがぶ音(ループなし)
				if (defender.attackedTimer == 0.0f)	PlayAudio(g_SE_ID[1], false);

				// 再計算
				CalculateAABB(defender.boundingBox, defender.position, defenderHitboxScaling);
				CalculateAABB(attackObject.boundingBox, attackObject.position, attackObject.scaling);

				attacker.isAttacking = false;	// 攻撃終了
			}
		}
	}
}

void DrawDebugSector(const Sector& sector)
{
	const int CIRCLE_SEGMENTS = 16;
	std::vector<Vertex2> vdata;

	// --- 1. 頂点作成（扇の骨組み） ---
	Vertex2 centerV;
	centerV.position = sector.center;
	centerV.position.y += 0.2f; // 地面より少し浮かせる
	centerV.normal = { 0, 1, 0 };
	centerV.color = { 1.0f, 0.0f, 0.0f, 1.0f }; // 赤色
	centerV.tex = { 0, 0 };

	vdata.push_back(centerV); // 中心点

	float baseAngle = atan2f(sector.forward.x, sector.forward.z);
	float halfAngle = XMConvertToRadians(sector.angleDegree * 0.5f);

	for (int i = 0; i <= CIRCLE_SEGMENTS; i++) {
		float currentAngle = (baseAngle - halfAngle) + (halfAngle * 2.0f * (float)i / CIRCLE_SEGMENTS);
		Vertex2 v = centerV;
		v.position.x = sector.center.x + sinf(currentAngle) * sector.radius;
		v.position.z = sector.center.z + cosf(currentAngle) * sector.radius;
		vdata.push_back(v);
	}
	vdata.push_back(centerV); // 最後に中心に戻って閉じる

	// --- 2. GPUへ転送 ---
	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX WVP = world * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(WVP);

	LIGHT light{};
	light.Enable = FALSE; // 線なのでライティング無効
	Shader_SetLight(light);

	Shader_Begin();
	Shader_SetColor(color::white);

	D3D11_MAPPED_SUBRESOURCE msr;
	if (SUCCEEDED(g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr))) {
		// vdata.size() が ATTACK_VERTEX (24) を超えないかチェック
		size_t count = min(vdata.size(), (size_t)ATTACK_VERTEX);
		CopyMemory(msr.pData, vdata.data(), sizeof(Vertex2) * count);
		g_pContext->Unmap(g_VertexBuffer, 0);

		// --- 3. 描画 ---
		UINT stride = sizeof(Vertex2);
		UINT offset = 0;
		g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP); // 線でつなぐ
		g_pContext->Draw((UINT)count, 0);
	}

	// トポロジーを元に戻す（重要！）
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

ATTACK_OBJECT* GetAttack(int playerIndex)
{
	// 範囲チェック 0 1 2 3 以外なら nullptr を返す
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX)	return nullptr;

	return &Attack[playerIndex];
}