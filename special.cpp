// special.cpp

#include <DirectXMath.h>
#include <d3d11.h>
using namespace DirectX;
#include "special.h"
#include "sprite.h"
#include "shader.h"
#include "Camera.h"
#include "collider.h"
#include "field.h"
#include "building.h"
#include "debug_ostream.h"
#include "Player.h"
#include "keyboard.h"
#include "DamageText.h"
#include "Effect.h"

// グローバル変数
static ID3D11Device* g_pDevice = NULL;
static ID3D11DeviceContext* g_pContext = NULL;

// 頂点バッファ
static ID3D11Buffer* g_VertexBuffer;

// インデックスバッファ
static ID3D11Buffer* g_IndexBuffer;

// テクスチャ変数
static ID3D11ShaderResourceView* g_Special_Texture[10];

// オブジェクト
static SPECIAL_OBJECT Special[PLAYER_MAX];

static bool g_plantInitialized[PLAYER_MAX] = { false };
static PLANT_CIRCLE g_PlantCircle[PLAYER_MAX];

// マクロ定義
#define SPECIAL_VERTEX (24)

// スペシャル アニメーション用変数
static int   g_animFrame[PLAYER_MAX];
static float g_animTimer[PLAYER_MAX];
static const float ANIM_FRAME_TIME = 0.15f;	// 1フレームあたりの秒数
static const int   SHEET_COLS = 8;
static const int   SHEET_ROWS = 8;

static int g_lightningFrame[PLAYER_MAX] = { 0 };
static float g_lightningTimer[PLAYER_MAX] = { 0.0f };
static const int LIGHTNING_SHEET_COLS = 8;
static const int LIGHTNING_SHEET_ROWS = 8;
static const int LIGHTNING_FRAME_MAX = 59; // 0～58
static const float LIGHTNING_ANIM_FRAME_TIME = 0.15f;

static bool g_concreteRangeFinished[PLAYER_MAX] = { false };
static int  g_concreteFrame[PLAYER_MAX] = { 0 };
static float g_concreteTimer[PLAYER_MAX] = { 0.0f };
static const int   CONCRETE_FRAME_MAX = 72;	// 0～72
static const float CONCRETE_ANIM_FRAME_TIME = 0.15f;

static Vertex2 Special_vdata[SPECIAL_VERTEX] =
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
static UINT Special_idxdata[6 * 6]
{
	 0,  1,  2,  2,  1,  3, // -Z面
	 4,  5,  6,  6,  5,  7, // +X面
	 8,  9, 10, 10,  9, 11, // +Z面
	12, 13, 14, 14, 13, 15, // -X面
	16, 17, 18, 18, 17, 19, // +Y面
	20, 21, 22, 22, 21, 23, // -Y面
};

void Special_Glass_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
}

void Special_Concrete_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
}

void Special_Plant_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
}

void Special_Electricity_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
}

void Special_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点バッファ作成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex2) * SPECIAL_VERTEX; // 格納できる頂点数 * 頂点サイズ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	// スペシャル範囲表示
	LoadFromWICFile(L"Asset\\Texture\\uiSpecialRed_v2.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[0]);
	assert(g_Special_Texture[0]);
	LoadFromWICFile(L"Asset\\Texture\\uiSpecialBlue_v2.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[1]);
	assert(g_Special_Texture[1]);
	LoadFromWICFile(L"Asset\\Texture\\uiSpecialYellow_v3.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[2]);
	assert(g_Special_Texture[2]);
	LoadFromWICFile(L"Asset\\Texture\\uiSpecialGreen_v2.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[3]);
	assert(g_Special_Texture[3]);
	// ガラスミサイル
	LoadFromWICFile(L"Asset\\Texture\\ice.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[4]);
	assert(g_Special_Texture[4]);
	// コンクリート 衝撃波 エフェクト1
	LoadFromWICFile(L"Asset\\Texture\\effectSPConcrete01_v2.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[5]);
	assert(g_Special_Texture[5]);
	// コンクリート 衝撃波 エフェクト2
	LoadFromWICFile(L"Asset\\Texture\\effectSPConcrete02_v2.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[6]);
	assert(g_Special_Texture[6]);
	// 電気 雷 エフェクト
	LoadFromWICFile(L"Asset\\Texture\\effectlighting.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[7]);
	assert(g_Special_Texture[7]);

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
		CopyMemory(&index[0], &Special_idxdata[0], sizeof(UINT) * 6 * 6);
		pContext->Unmap(g_IndexBuffer, 0);
	}

	Special_Glass_Initialize(pDevice, pContext);
	Special_Concrete_Initialize(pDevice, pContext);
	Special_Plant_Initialize(pDevice, pContext);
	Special_Electricity_Initialize(pDevice, pContext);
}

void Special_Finalize()
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

	for (int i = 0; i < 10; i++)
	{
		if (g_Special_Texture[i])
		{
			g_Special_Texture[i]->Release();
			g_Special_Texture[i] = NULL;
		}
	}
}

void Special_Glass_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// スペシャルタイマー更新
	player.specialTimer += DELTA_TIME;

	// スペシャルの初期化処理
	static bool initialized[PLAYER_MAX] = { false }; // 各プレイヤーごとに初期化フラグを持つ
	static bool missileRain[PLAYER_MAX] = { false }; // 各プレイヤーごとのミサイル雨フラグ

	if (!initialized[playerIndex])
	{
		player.glassBoxes.clear();
		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			if (p == playerIndex) continue; // 自分自身は無視

			PLAYEROBJECT* otherPlayerObject = GetPlayer(p);
			if (otherPlayerObject == nullptr || !otherPlayerObject->active) continue;
			PLAYEROBJECT& otherPlayer = *otherPlayerObject;

			// リスポーン中のプレイヤーには箱を飛ばさない
			if (otherPlayer.duringRespawn) continue;

			// 他のプレイヤーの周りに3つの箱を生成
			XMFLOAT3 offsets[SPECIAL_GLASSBOX_QUANTITY];
			for (int i = 0; i < SPECIAL_GLASSBOX_QUANTITY; ++i)
			{
				bool validPosition = false;
				int maxAttempts = 100; // 無限ループ防止
				while (!validPosition && maxAttempts > 0)
				{
					float randomX = -3.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (5.0f)));
					float randomZ = -3.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (5.0f)));
					offsets[i] = { randomX, 0.0f, randomZ };

					// 既に生成済みのオフセットとの距離をチェック
					validPosition = true;
					for (int j = 0; j < i; ++j)
					{
						float dx = offsets[i].x - offsets[j].x;
						float dz = offsets[i].z - offsets[j].z;
						float distSq = dx * dx + dz * dz;
						if (distSq < 2.5f * 2.5f) // 2.5f未満なら再生成
						{
							validPosition = false;
							break;
						}
					}
					maxAttempts--;
				}
			}
			for (int i = 0; i < SPECIAL_GLASSBOX_QUANTITY; ++i)
			{
				GLASS_BOX box;
				box.position = player.position; // 箱をプレイヤーの位置に出現させる
				box.rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
				box.scaling = XMFLOAT3(0.25f, 0.25f, 0.25f); // 箱のサイズ
				box.targetPosition =
				{
					otherPlayer.position.x + offsets[i].x,
					0.5f,
					otherPlayer.position.z + offsets[i].z
				};
				box.dir = XMFLOAT3(0.0f, 1.0f, 0.0f);	// 初期は上昇方向
				box.active = true;	// 初期状態で有効
				player.glassBoxes.push_back(box);
			}
		}
		initialized[playerIndex] = true;
	}

	// 箱の移動処理
	for (auto& box : player.glassBoxes)
	{
		if (!box.active) continue; // 非アクティブな箱はスキップ

		if (!missileRain[playerIndex])
		{
			if (box.position.y < player.position.y + 6.0f)
			{
				// ① プレイヤーの位置からY座標+6まで上昇
				box.position.y += DELTA_TIME * 5.0f; // 上昇速度
				if (box.position.y >= player.position.y + 6.0f)	box.position.y = player.position.y + 6.0f;
			}
			else missileRain[playerIndex] = true; // 上昇完了後にフラグを立てる
		}
		else
		{
			if (box.position.x != box.targetPosition.x || box.position.z != box.targetPosition.z)
			{
				// ② targetPositionの真上まで平行移動
				XMVECTOR currentPos = XMLoadFloat3(&box.position);
				XMVECTOR targetPos = XMVectorSet(box.targetPosition.x, box.position.y, box.targetPosition.z, 0.0f);
				XMVECTOR direction = XMVector3Normalize(targetPos - currentPos);

				// 移動速度を設定
				const float speed = DELTA_TIME * 5.0f;
				XMVECTOR movement = direction * speed;

				// 新しい位置を計算
				currentPos += movement;

				// 目標位置に到達したか確認
				XMVECTOR distanceVec = targetPos - currentPos;
				float distance = XMVectorGetX(XMVector3Length(distanceVec));
				if (distance <= speed)	XMStoreFloat3(&box.position, targetPos);	// 真上にスナップ
				else					XMStoreFloat3(&box.position, currentPos);	// 移動
			}
			else if (box.position.y > box.targetPosition.y)
			{
				// ③ targetPositionまで降下
				box.position.y -= DELTA_TIME * 5.0f;	// 降下速度
				if (box.position.y <= box.targetPosition.y)
				{
					box.position.y = box.targetPosition.y;	// 降下完了
					box.active = false;	// 地面に着いたら非アクティブ化
					Camera_StartShake(0.2f, 0.2f);

					// 衝突判定
					Circle boxCollider = { box.position, 0.3f };	// 半径0.3の円
					for (int p = 0; p < PLAYER_MAX; ++p)
					{
						if (p == playerIndex) continue; // 自分自身は無視

						PLAYEROBJECT* otherPlayerObject = GetPlayer(p);
						if (otherPlayerObject == nullptr || !otherPlayerObject->active) continue;
						PLAYEROBJECT& otherPlayer = *otherPlayerObject;

						if (otherPlayer.isInvincible) continue; // 無敵中は無視

						// 箱とプレイヤーの衝突判定
						if (CheckCircleAABBCollision(boxCollider, otherPlayer.boundingBox))
						{
							float rawDamage = SPECIAL_GLASS_DAMAGE * otherPlayer.defense;;
							// 衝突している場合、ダメージを与える
							otherPlayer.hp -= rawDamage;

							// ダメージ数字を表示（頭上にオフセット）
							int dmgInt = static_cast<int>(rawDamage + 0.5f);
							XMFLOAT3 hitPos = otherPlayer.position;
							hitPos.y += otherPlayer.scaling.y + 0.3f;
							SetDamageText(hitPos, dmgInt, TextColor::Red);

							// HPが0以下にならないように
							if (otherPlayer.hp < 0.0f) otherPlayer.hp = 0.0f;

							otherPlayer.isAttacked = true; // 攻撃を受けたフラグを立てる

							// 衝突した箱を非アクティブ化
							box.active = false;
						}
					}
				}
			}
		}
	}

	// すべての箱が非アクティブになった場合、スペシャル終了
	bool allInactive = std::all_of(player.glassBoxes.begin(), player.glassBoxes.end(), [](const GLASS_BOX& box) { return !box.active; });

	if (allInactive)
	{
		player.useSpecial = false;
		player.specialTimer = 0.0f;
		g_animFrame[playerIndex] = 0;		// アニメーションリセット
		g_animTimer[playerIndex] = 0.0f;
		initialized[playerIndex] = false;	// 次回のスペシャル使用時に再初期化するため
		missileRain[playerIndex] = false;	// フラグをリセット
		player.form = Form::First;			// 変身形態を第1形態に戻す
		player.type = PlayerType::None;		// タイプをリセット
		player.useSkill = false;			// スキル解除
		player.useSpecial = false;			// スペシャル解除
		Effect_ClearUI(playerIndex);		// エフェクトクリア
		player.isTypeFixed = false;			// タイプ固定解除
	}
}

void Special_Concrete_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// スペシャルの初期位置をプレイヤーの位置に設定
	if (player.specialTimer == 0.0f)
	{
		player.oldPosition = player.position;
		g_concreteRangeFinished[playerIndex] = false;
	}

	// スペシャルタイマー更新
	player.specialTimer += DELTA_TIME;

	// ジャンプ処理
	if (player.specialTimer <= 0.75f)
	{
		player.position.y = player.oldPosition.y + 3.0f * player.specialTimer / 0.75f; // 線形補間でY座標を上げる
	}
	else if (player.specialTimer > 0.75f && player.specialTimer <= 1.5f)
	{
		// 着地処理
		player.position.y = player.oldPosition.y + (3.0f * (1.0f - (player.specialTimer - 0.75f) / 0.15f)); // 線形補間でY座標を上げる
		if (player.position.y <= player.oldPosition.y)	player.position.y = player.oldPosition.y;

		// ダメージ処理（1回だけ実行）
		if (player.specialTimer - DELTA_TIME < 0.75f) // 0.75秒を超えた瞬間に実行
		{
			g_concreteRangeFinished[playerIndex] = true;

			const float radius = 5.0f;
			Circle circle = { player.position, radius }; // 円の中心と半径を設定

			// 画面を揺らす
			Camera_StartShake(0.8f, 0.6f);

			for (int p = 0; p < PLAYER_MAX; ++p)
			{
				if (p == playerIndex) continue; // 自分自身は無視

				PLAYEROBJECT* otherPlayerObject = GetPlayer(p);
				if (otherPlayerObject == nullptr || !otherPlayerObject->active) continue;
				PLAYEROBJECT& otherPlayer = *otherPlayerObject;

				if (otherPlayer.isInvincible) continue; // 無敵中は無視

				// 円とAABBの衝突判定
				if (CheckCircleAABBCollision(circle, otherPlayer.boundingBox))
				{
					float rawDamage = SPECIAL_CONCRETE_DAMAGE * otherPlayer.defense;
					// 衝突している場合、ダメージを与える
					otherPlayer.hp -= rawDamage;

					// ダメージ数字を表示（頭上にオフセット）
					int dmgInt = static_cast<int>(rawDamage + 0.5f);
					XMFLOAT3 hitPos = otherPlayer.position;
					hitPos.y += otherPlayer.scaling.y + 0.3f;
					SetDamageText(hitPos, dmgInt, TextColor::Red);

					// HPが0以下にならないように
					if (otherPlayer.hp < 0.0f) otherPlayer.hp = 0.0f;

					otherPlayer.isAttacked = true; // 攻撃を受けたフラグを立てる
				}
			}
		}
	}

	// スペシャルの効果時間が経過したらスペシャル終了
	if (player.specialTimer >= SPECIAL_CONCRETE_TIME)
	{
		player.useSpecial = false;
		player.specialTimer = 0.0f;
		g_animFrame[playerIndex] = 0;
		g_animTimer[playerIndex] = 0.0f;
		player.form = Form::First;
		player.type = PlayerType::None;
		player.defense = 1.0f;
		player.useSkill = false;
		player.useSpecial = false;
		Effect_Clear(playerIndex);
		player.isTypeFixed = false;

		// 範囲表示終了フラグを立てる
		g_concreteRangeFinished[playerIndex] = true;
		g_concreteFrame[playerIndex] = 0;
		g_concreteTimer[playerIndex] = 0.0f;
	}
}

void Special_Plant_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// スペシャルタイマー更新
	player.specialTimer += DELTA_TIME;

	// 半径2.5fの円形当たり判定を作成
	g_PlantCircle[playerIndex].radius = 2.5f;
	Circle circle = { player.position, g_PlantCircle[playerIndex].radius }; // 円の中心と半径を設定

	if (!g_plantInitialized[playerIndex])
	{
		Effect_Set(22, { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 }, { SCREEN_WIDTH, SCREEN_HEIGHT }, playerIndex);
		g_plantInitialized[playerIndex] = true;
	}

	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (p == playerIndex) continue; // 自分自身は無視

		PLAYEROBJECT* otherPlayerObject = GetPlayer(p);
		if (otherPlayerObject == nullptr || !otherPlayerObject->active) continue;
		PLAYEROBJECT& otherPlayer = *otherPlayerObject;

		if (otherPlayer.isInvincible) continue; // 無敵中は無視

		// 円とAABBの衝突判定
		if (CheckCircleAABBCollision(circle, otherPlayer.boundingBox))
		{
			otherPlayer.isPoisoned = true;	// 毒状態にする
			otherPlayer.poisonTimer = POISON_TIME;

			// ダメージ 防御率でダメージ軽減（ノックバックは与えない）
			otherPlayer.hp -= SPECIAL_PLANT_DAMAGE * otherPlayer.defense;

			// HPが0以下にならないように
			if (otherPlayer.hp < 0.0f) otherPlayer.hp = 0.0f;
		}
	}

	// スペシャルの効果時間が経過したらスペシャル終了
	if (player.specialTimer >= SPECIAL_PLANT_TIME)
	{
		player.useSpecial = false;
		player.specialTimer = 0.0f;
		g_animFrame[playerIndex] = 0;
		g_animTimer[playerIndex] = 0.0f;
		player.form = Form::First;									// 変身形態を第1形態に戻す
		player.type = PlayerType::None;								// タイプをリセット
		player.evolutionGaugeRate = PLAYER_EVOLUTION_GAUGE_RATE;	// スキルの進化ゲージバフもリセット
		player.useSkill = false;									// スキル解除
		Effect_Clear(playerIndex);									// エフェクトクリア
		Effect_ClearUI(playerIndex);
		player.isTypeFixed = false;
		g_plantInitialized[playerIndex] = false;					// ここでリセット
	}
}

void Special_Electricity_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// スペシャルタイマー更新
	player.specialTimer += DELTA_TIME;

	// スペシャルの初期化処理
	static bool initialized[PLAYER_MAX] = { false };
	static bool circleCollided[PLAYER_MAX][PLAYER_MAX][SPECIAL_ELECTRICITY_QUANTITY] = { { { false } } }; // 各プレイヤーごとのサークル判定フラグ

	if (!initialized[playerIndex])
	{
		for (int i = 0; i < SPECIAL_ELECTRICITY_QUANTITY; ++i)
		{
			// ランダムな位置に円を生成 (-5から5の範囲)
			float randomX = -5.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (10.0f)));
			float randomZ = -5.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (10.0f)));
			player.electricityCircles[i] = { XMFLOAT3(randomX, 0.0f, randomZ), 0.3f };

			// 各プレイヤーごとの判定フラグを初期化
			for (int p = 0; p < PLAYER_MAX; ++p) circleCollided[playerIndex][p][i] = false;
		}
		initialized[playerIndex] = true;
	}

	// 他のプレイヤーとの衝突判定
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (p == playerIndex) continue; // 自分自身は無視

		PLAYEROBJECT* otherPlayerObject = GetPlayer(p);
		if (otherPlayerObject == nullptr || !otherPlayerObject->active) continue;
		PLAYEROBJECT& otherPlayer = *otherPlayerObject;

		if (otherPlayer.isInvincible) continue; // 無敵中は無視

		for (int i = 0; i < SPECIAL_ELECTRICITY_QUANTITY; ++i)
		{
			// すでにこのプレイヤーがこのサークルに対して判定済みの場合はスキップ
			if (circleCollided[playerIndex][p][i]) continue;

			// 円とAABBの衝突判定
			if (CheckCircleAABBCollision(player.electricityCircles[i], otherPlayer.boundingBox))
			{
				float rawDamage = SPECIAL_ELECTRICITY_DAMAGE * otherPlayer.defense;

				// ダメージ 防御率でダメージ軽減（ノックバックは与えない）
				otherPlayer.hp -= rawDamage;

				// ダメージ数字を表示（頭上にオフセット）
				int dmgInt = static_cast<int>(rawDamage + 0.5f);
				XMFLOAT3 hitPos = otherPlayer.position;
				hitPos.y += otherPlayer.scaling.y + 0.3f;
				SetDamageText(hitPos, dmgInt, TextColor::Red);

				// スペシャルを使っていなければスタン
				if (!otherPlayer.useSpecial) otherPlayer.stunGauge = 10.0f;

				// HPが0以下にならないように
				if (otherPlayer.hp < 0.0f) otherPlayer.hp = 0.0f;

				// このプレイヤーに対するこのサークルでの判定を終了
				circleCollided[playerIndex][p][i] = true;
			}
		}
	}

	// スペシャルの効果時間が経過したらスペシャル終了
	if (player.specialTimer >= SPECIAL_ELECTRICITY_TIME)
	{
		player.useSpecial = false;
		player.specialTimer = 0.0f;
		g_animFrame[playerIndex] = 0;		// アニメーションリセット
		g_animTimer[playerIndex] = 0.0f;
		initialized[playerIndex] = false;	// 次回のスペシャル使用時に再初期化するため
		player.form = Form::First;			// 変身形態を第1形態に戻す
		player.type = PlayerType::None;		// タイプをリセット
		player.speed = 0.06f;				// スキルのスピードバフもリセット
		player.useSkill = false;			// スキル解除
		player.useSpecial = false;			// スペシャル解除
		Effect_ClearUI(playerIndex);		// エフェクトクリア
		player.isTypeFixed = false;			// タイプ固定解除
	}
}

void Special_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// スペシャル使用中かつスタン中でない場合に更新処理
	if (player.useSpecial && !player.isStunning)
	{
		// スペシャル範囲アニメーション
		g_animTimer[playerIndex] += DELTA_TIME;
		if (g_animTimer[playerIndex] >= ANIM_FRAME_TIME)
		{
			g_animTimer[playerIndex] -= ANIM_FRAME_TIME;
			g_animFrame[playerIndex] = (g_animFrame[playerIndex] + 1) % 30;
		}

		// 雷エフェクト専用アニメーション
		if (player.type == PlayerType::Electricity)
		{
			g_lightningTimer[playerIndex] += DELTA_TIME;
			if (g_lightningTimer[playerIndex] >= LIGHTNING_ANIM_FRAME_TIME)
			{
				g_lightningTimer[playerIndex] -= LIGHTNING_ANIM_FRAME_TIME;
				g_lightningFrame[playerIndex]++;
				if (g_lightningFrame[playerIndex] >= LIGHTNING_FRAME_MAX)	g_lightningFrame[playerIndex] = 0;
			}
		}

		// コンクリートエフェクト専用アニメーション
		if (g_concreteRangeFinished[playerIndex])
		{
			if (player.type == PlayerType::Concrete)
			{
				g_concreteTimer[playerIndex] += DELTA_TIME;
				if (g_concreteTimer[playerIndex] >= CONCRETE_ANIM_FRAME_TIME)
				{
					g_concreteTimer[playerIndex] -= CONCRETE_ANIM_FRAME_TIME;
					g_concreteFrame[playerIndex]++;
					if (g_concreteFrame[playerIndex] >= CONCRETE_FRAME_MAX) g_concreteFrame[playerIndex] = CONCRETE_FRAME_MAX - 1;
				}
			}
		}
		else	g_concreteFrame[playerIndex] = 0;

		switch (player.type)
		{
		case PlayerType::Glass:			Special_Glass_Update(playerIndex);			break;
		case PlayerType::Concrete:		Special_Concrete_Update(playerIndex);		break;
		case PlayerType::Plant:			Special_Plant_Update(playerIndex);			break;
		case PlayerType::Electricity:	Special_Electricity_Update(playerIndex);	break;
		default: break;
		}
	}
}

// Glass専用描画
void Special_Glass_Draw(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// 箱の描画
	for (const auto& box : player.glassBoxes)
	{
		if (!box.active) continue; // 非アクティブな箱は描画しない

		XMMATRIX WorldMatrix =
			XMMatrixScaling(box.scaling.x, box.scaling.y, box.scaling.z) *
			XMMatrixRotationRollPitchYaw(XMConvertToRadians(box.rotation.x), XMConvertToRadians(box.rotation.y), XMConvertToRadians(box.rotation.z)) *
			XMMatrixTranslation(box.position.x, box.position.y, box.position.z);

		XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
		Shader_SetMatrix(WVP);

		// 箱のテクスチャを設定
		g_pContext->PSSetShaderResources(0, 1, &g_Special_Texture[4]);

		// 明るさを強調するためにカラーを設定 (RGB値を2倍に設定)
		Shader_SetColor(XMFLOAT4(2.0f, 2.0f, 2.0f, 1.0f)); // 明るさを強調

		// 描画実行
		g_pContext->DrawIndexed(6 * 6, 0, 0);

		// 描画後にカラーをリセット
		Shader_SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)); // デフォルトの明るさに戻す
	}

	// 攻撃範囲の描画
	for (const auto& box : player.glassBoxes)
	{
		if (!box.active) continue; // 非アクティブな箱は描画しない

		// targetPositionに基づいて描画
		XMMATRIX rangeWorldMatrix =
			XMMatrixScaling(1.0f, 1.0f, 1.0f) *
			XMMatrixRotationX(XMConvertToRadians(0.0f)) *
			XMMatrixTranslation(box.targetPosition.x, box.targetPosition.y + 0.1f, box.targetPosition.z - 0.5f); // Y座標を少し上げて地面と重ならないようにする カメラの向きに考慮してZ座標をずらす

		XMMATRIX rangeWVP = rangeWorldMatrix * GetViewMatrix() * GetProjectionMatrix();
		Shader_SetMatrix(rangeWVP);

		// 範囲のテクスチャを設定
		g_pContext->PSSetShaderResources(0, 1, &g_Special_Texture[playerIndex]);

		// アルファブレンディングを有効化
		SetBlendState(BLENDSTATE_ALPHA);

		// 描画実行 (+Y面の一枚だけ描画)
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); // トポロジーを三角形ストリップに設定
		g_pContext->Draw(4, 16); // +Y面の4頂点 (16, 17, 18, 19) を描画
	}
}

// Concrete専用描画
void Special_Concrete_Draw(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// 攻撃範囲のワールド行列
	XMMATRIX WorldMatrix =
		XMMatrixScaling(10.0f, 1.0f, 10.0f) *
		XMMatrixRotationX(XMConvertToRadians(0.0f)) *
		XMMatrixTranslation(player.position.x, 0.1f, player.position.z);

	XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(WVP);

	SetBlendState(BLENDSTATE_ALPHA);

	// 範囲表示フェーズ
	if (!g_concreteRangeFinished[playerIndex])
	{
		// テクスチャ番号0～3をplayerIndexで表示
		g_pContext->PSSetShaderResources(0, 1, &g_Special_Texture[playerIndex]);
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		g_pContext->Draw(4, 16); // +Y面
	}
	// 衝撃波アニメーションフェーズ
	else if (g_concreteFrame[playerIndex] < 32 + 40) // 32+40=72フレーム分
	{
		int frame = g_concreteFrame[playerIndex];
		int texIndex;
		int uvFrame;
		int uvMaxCols = 8, uvMaxRows = 8;

		if (frame < 32)
		{
			texIndex = 5;		// effectSPConcrete01_v2
			uvFrame = frame;	// 0～31
			// 8x8分割
		}
		else
		{
			texIndex = 6;			// effectSPConcrete02_v2
			uvFrame = frame - 32;	// 0～39
			// 8x8分割
		}

		XMMATRIX WorldMatrix =
			XMMatrixScaling(20.0f, 1.0f, 20.0f) *
			XMMatrixRotationX(XMConvertToRadians(0.0f)) *
			XMMatrixTranslation(player.position.x, 0.1f, player.position.z);

		XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
		Shader_SetMatrix(WVP);

		g_pContext->PSSetShaderResources(0, 1, &g_Special_Texture[texIndex]);

		// UV計算（8x8分割）
		int col = uvFrame % uvMaxCols;
		int row = uvFrame / uvMaxCols;
		float u0 = (float)col / (float)uvMaxCols;
		float v0 = (float)row / (float)uvMaxRows;
		float u1 = u0 + 1.0f / (float)uvMaxCols;
		float v1 = v0 + 1.0f / (float)uvMaxRows;

		// +Y面のUVだけ書き換え
		D3D11_MAPPED_SUBRESOURCE msr;
		g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		Vertex2* vertex = (Vertex2*)msr.pData;
		CopyMemory(vertex, &Special_vdata[0], sizeof(Vertex2) * SPECIAL_VERTEX);
		vertex[16].tex = XMFLOAT2(u0, v0);	// LEFT-TOP
		vertex[17].tex = XMFLOAT2(u1, v0);	// RIGHT-TOP
		vertex[18].tex = XMFLOAT2(u0, v1);	// LEFT-BOTTOM
		vertex[19].tex = XMFLOAT2(u1, v1);	// RIGHT-BOTTOM
		g_pContext->Unmap(g_VertexBuffer, 0);

		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		g_pContext->Draw(4, 16); // +Y面
	}
}

// Plant専用描画
void Special_Plant_Draw(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// Plant用の座標計算
	SPECIAL_OBJECT& specialPlant = Special[playerIndex];

	XMMATRIX WorldMatrix =
		XMMatrixScaling(specialPlant.scaling.x, specialPlant.scaling.y, specialPlant.scaling.z) *
		XMMatrixRotationRollPitchYaw(XMConvertToRadians(specialPlant.rotation.x), XMConvertToRadians(specialPlant.rotation.y), XMConvertToRadians(specialPlant.rotation.z)) *
		XMMatrixTranslation(specialPlant.position.x, specialPlant.position.y, specialPlant.position.z);

	XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(WVP);

	// 描画実行
	g_pContext->DrawIndexed(6 * 6, 0, 0);

	// 攻撃範囲の描画
	XMMATRIX rangeWorldMatrix =
		XMMatrixScaling(5.0f, 1.0f, 5.0f) * // 半径2.5の円を表現するためにスケールを5倍に設定
		XMMatrixRotationX(XMConvertToRadians(0.0f)) *
		XMMatrixTranslation(player.position.x + 0.2f, 0.1f, player.position.z - 0.5f); // Y座標を少し上げて地面と重ならないようにする

	XMMATRIX rangeWVP = rangeWorldMatrix * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(rangeWVP);

	// アルファブレンディングを有効化
	SetBlendState(BLENDSTATE_ALPHA);

	// 描画実行 (+Y面の一枚だけ描画)
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); // トポロジーを三角形ストリップに設定
	g_pContext->Draw(4, 16); // +Y面の4頂点 (16, 17, 18, 19) を描画
}

// Electricity専用描画
void Special_Electricity_Draw(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	for (int i = 0; i < SPECIAL_ELECTRICITY_QUANTITY; ++i)
	{
		XMFLOAT3 target = player.electricityCircles[i].center;

		// --- 範囲円（+Y面）のUVを6x6分割で書き換え ---
		{
			int frame = g_animFrame[playerIndex];
			int col = frame % SHEET_COLS;
			int row = frame / SHEET_COLS;
			float u0 = (float)col / (float)SHEET_COLS;
			float v0 = (float)row / (float)SHEET_ROWS;
			float u1 = u0 + 1.0f / (float)SHEET_COLS;
			float v1 = v0 + 1.0f / (float)SHEET_ROWS;

			D3D11_MAPPED_SUBRESOURCE msr;
			g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
			Vertex2* vertex = (Vertex2*)msr.pData;
			CopyMemory(vertex, &Special_vdata[0], sizeof(Vertex2) * SPECIAL_VERTEX);
			vertex[16].tex = XMFLOAT2(u0, v0);	// LEFT-TOP
			vertex[17].tex = XMFLOAT2(u1, v0);	// RIGHT-TOP
			vertex[18].tex = XMFLOAT2(u0, v1);	// LEFT-BOTTOM
			vertex[19].tex = XMFLOAT2(u1, v1);	// RIGHT-BOTTOM
			g_pContext->Unmap(g_VertexBuffer, 0);
		}

		// --- 範囲円描画前にライトを親関数と同じ明るさに設定 ---
		LIGHT rangeLight{};
		rangeLight.Enable = TRUE;
		rangeLight.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
		rangeLight.Diffuse = XMFLOAT4(2.5f, 2.5f, 2.5f, 1.0f);
		rangeLight.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		Shader_SetLight(rangeLight);

		// 範囲円（+Y面）を描画
		XMMATRIX circleWorldMatrix =
			XMMatrixScaling(1.0f, 1.0f, 1.0f) *
			XMMatrixRotationX(XMConvertToRadians(0.0f)) *
			XMMatrixTranslation(target.x, target.y + 0.1f, target.z);

		XMMATRIX circleWVP = circleWorldMatrix * GetViewMatrix() * GetProjectionMatrix();
		Shader_SetMatrix(circleWVP);

		SetBlendState(BLENDSTATE_ALPHA);
		g_pContext->PSSetShaderResources(0, 1, &g_Special_Texture[playerIndex]);
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		g_pContext->Draw(4, 16);	// +Y面の4頂点 (16, 17, 18, 19)

		// --- 雷エフェクト（-Z面）描画前にライトを強くする ---
		LIGHT lightningLight{};
		lightningLight.Enable = TRUE;
		lightningLight.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
		lightningLight.Diffuse = XMFLOAT4(4.0f, 4.0f, 4.0f, 1.0f);
		lightningLight.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		Shader_SetLight(lightningLight);

		// --- 雷エフェクト（-Z面）のUVを8x8分割で書き換え ---
		{
			int frame = g_lightningFrame[playerIndex];
			int col = frame % LIGHTNING_SHEET_COLS;
			int row = frame / LIGHTNING_SHEET_COLS;
			float u0 = (float)col / (float)LIGHTNING_SHEET_COLS;
			float v0 = (float)row / (float)LIGHTNING_SHEET_ROWS;
			float u1 = u0 + 1.0f / (float)LIGHTNING_SHEET_COLS;
			float v1 = v0 + 1.0f / (float)LIGHTNING_SHEET_ROWS;

			D3D11_MAPPED_SUBRESOURCE msr;
			g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
			Vertex2* vertex = (Vertex2*)msr.pData;
			CopyMemory(vertex, &Special_vdata[0], sizeof(Vertex2) * SPECIAL_VERTEX);
			vertex[0].tex = XMFLOAT2(u0, v0);	// LEFT-TOP
			vertex[1].tex = XMFLOAT2(u1, v0);	// RIGHT-TOP
			vertex[2].tex = XMFLOAT2(u0, v1);	// LEFT-BOTTOM
			vertex[3].tex = XMFLOAT2(u1, v1);	// RIGHT-BOTTOM
			g_pContext->Unmap(g_VertexBuffer, 0);
		}

		// --- 雷エフェクト（-Z面）を描画 ---
		float lightningTopY = 10.0f;
		float lightningBottomY = target.y;
		float length = fabsf(lightningTopY - lightningBottomY);

		XMMATRIX lightningWorldMatrix =
			XMMatrixScaling(2.0f, length * 1.0f, 2.0f) *
			XMMatrixRotationX(XMConvertToRadians(0.0f)) *
			XMMatrixTranslation(target.x + 0.5f, 4.0f, target.z + 0.1f);
		//XMMatrixTranslation(target.x, (lightningTopY + lightningBottomY) / 2.0f, target.z);

		XMMATRIX lightningWVP = lightningWorldMatrix * GetViewMatrix() * GetProjectionMatrix();
		Shader_SetMatrix(lightningWVP);

		g_pContext->PSSetShaderResources(0, 1, &g_Special_Texture[7]);
		SetBlendState(BLENDSTATE_ALPHA);

		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		g_pContext->Draw(4, 0);	// -Z面の4頂点 (0, 1, 2, 3)
	}

	// --- ループ終了後にライトを元に戻す ---
	LIGHT normalLight{};
	normalLight.Enable = TRUE;
	normalLight.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
	normalLight.Diffuse = XMFLOAT4(1.5f, 1.5f, 1.5f, 1.0f);
	normalLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	Shader_SetLight(normalLight);
}

void Special_Draw(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// 1. 共通設定 (パイプラインステートの設定)
	//    これを親で一度だけやることで処理落ちを防ぐ

	// シェーダー開始
	// スペシャル描画前にレンダリングステートをリセット
	SetBlendState(BLENDSTATE_ALPHA);
	Shader_Begin();

	// ライト設定をリセット
	LIGHT light{};
	light.Enable = TRUE;
	light.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
	light.Diffuse = XMFLOAT4(2.5f, 2.5f, 2.5f, 1.0f);
	light.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	Shader_SetLight(light);

	SetDepthTest(false); // 深度テストを無効化
	SetBlendState(BLENDSTATE_ALPHA); // アルファブレンディングを有効化

	// 頂点バッファ・インデックスバッファのセット
	UINT stride = sizeof(Vertex2);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ※ プレイヤー本体の描画と同様に、ここで一度だけ頂点データをGPUに送ります
	D3D11_MAPPED_SUBRESOURCE msr;
	// (注意: g_VertexBuffer が D3D11_USAGE_DYNAMIC である必要があります)
	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex2* vertex = (Vertex2*)msr.pData;
	// vdata[] はキューブの頂点データを格納した配列を想定
	CopyMemory(&vertex[0], &Special_vdata[0], sizeof(Vertex2) * SPECIAL_VERTEX);
	g_pContext->Unmap(g_VertexBuffer, 0);

	// プレイヤーの番号に対応するテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Special_Texture[playerIndex];
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// スプライトシートのUV座標を計算
	int frame = g_animFrame[playerIndex];
	int col = frame % SHEET_COLS;
	int row = frame / SHEET_COLS;
	float u0 = (float)col / (float)SHEET_COLS;
	float v0 = (float)row / (float)SHEET_ROWS;
	float u1 = u0 + 1.0f / (float)SHEET_COLS;
	float v1 = v0 + 1.0f / (float)SHEET_ROWS;

	// 頂点データをバッファに直接書き込み、UVを調整 (+Y面のみ)
	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	vertex = (Vertex2*)msr.pData;

	// 全体のデータをコピー
	CopyMemory(vertex, &Special_vdata[0], sizeof(Vertex2) * SPECIAL_VERTEX);

	// +Y面のUV座標を調整
	vertex[16].tex = XMFLOAT2(u0, v0);	// LEFT-TOP
	vertex[17].tex = XMFLOAT2(u1, v0);	// RIGHT-TOP
	vertex[18].tex = XMFLOAT2(u0, v1);	// LEFT-BOTTOM
	vertex[19].tex = XMFLOAT2(u1, v1);	// RIGHT-BOTTOM

	g_pContext->Unmap(g_VertexBuffer, 0);

	// プレイヤーのタイプに応じた描画処理
	switch (player.type)
	{
	case PlayerType::Glass:			Special_Glass_Draw(playerIndex);		break;
	case PlayerType::Concrete:		Special_Concrete_Draw(playerIndex);		break;
	case PlayerType::Plant:			Special_Plant_Draw(playerIndex);		break;
	case PlayerType::Electricity:	Special_Electricity_Draw(playerIndex);	break;
	default: break;
	}
}

SPECIAL_OBJECT* GetSpecial(int playerIndex)
{
	// 範囲チェック 0 1 2 3 以外なら nullptr を返す
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX)	return nullptr;

	return &Special[playerIndex];
}