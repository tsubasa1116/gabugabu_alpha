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
static ID3D11ShaderResourceView* g_Special_Texture[10];

// オブジェクト
static SPECIAL_OBJECT Special[PLAYER_MAX];

// スペシャル 電気 オブジェクト
Circle electricCircles[SPECIAL_ELECTRIC_QUANTITY];

// ガラススペシャル ミサイル リスト
std::vector<GLASS_BOX> glassBoxes;

// マクロ定義
#define NUM_VERTEX (24)

static Vertex2 Special_vdata[NUM_VERTEX] =
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
	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	// 攻撃範囲表示用テクスチャ
	LoadFromWICFile(L"Asset\\Texture\\uiAim.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[0]);
	assert(g_Special_Texture[0]);
	// とげとげミサイル用テクスチャ
	LoadFromWICFile(L"Asset\\Texture\\TileA3.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[1]);
	assert(g_Special_Texture[1]);
}

void Special_Concrete_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[2]);
	assert(g_Special_Texture[2]);
}

void Special_Plant_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[3]);
	assert(g_Special_Texture[3]);
}

void Special_Electric_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\uiAim.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[4]);
	assert(g_Special_Texture[4]);
}

void Special_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
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
		CopyMemory(&index[0], &Special_idxdata[0], sizeof(UINT) * 6 * 6);
		pContext->Unmap(g_IndexBuffer, 0);
	}

	Special_Glass_Initialize(pDevice, pContext);
	Special_Concrete_Initialize(pDevice, pContext);
	Special_Plant_Initialize(pDevice, pContext);
	Special_Electric_Initialize(pDevice, pContext);
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
	static bool missileRain = false;
	static bool initialized = false;
	if (!initialized)
	{
		glassBoxes.clear();
		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			if (p == playerIndex) continue; // 自分自身は無視

			PLAYEROBJECT* otherPlayerObject = GetPlayer(p);
			if (otherPlayerObject == nullptr || !otherPlayerObject->active) continue;
			PLAYEROBJECT& otherPlayer = *otherPlayerObject;

			// 他のプレイヤーの周りに3つの箱を生成
			XMFLOAT3 offsets[SPECIAL_GLASSBOX_QUANTITY];
			for (int i = 0; i < SPECIAL_GLASSBOX_QUANTITY; ++i)
			{
				float randomX = -2.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (2.0f - (-2.0f))));
				float randomZ = -2.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (2.0f - (-2.0f))));
				offsets[i] = { randomX, 0.0f, randomZ };
			}
			for (int i = 0; i < SPECIAL_GLASSBOX_QUANTITY; ++i)
			{
				GLASS_BOX box;
				box.position = player.position; // 箱をプレイヤーの位置に出現させる
				box.rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
				box.scaling = XMFLOAT3(0.25f, 0.25f, 0.25f); // 箱のサイズ
				box.targetPosition = {
					otherPlayer.position.x + offsets[i].x,
					otherPlayer.position.y + offsets[i].y,
					otherPlayer.position.z + offsets[i].z
				};
				box.dir = XMFLOAT3(0.0f, 1.0f, 0.0f);	// 初期は上昇方向
				box.active = true;	// 初期状態で有効
				glassBoxes.push_back(box);
			}
		}
		initialized = true;
	}

	// 箱の移動処理
	for (auto& box : glassBoxes)
	{
		if (!box.active) continue; // 非アクティブな箱はスキップ

		if (!missileRain)
		{
			if (box.position.y < player.position.y + 6.0f)
			{
				// ① プレイヤーの位置からY座標+6まで上昇
				box.position.y += DELTA_TIME * 5.0f; // 上昇速度
				if (box.position.y >= player.position.y + 6.0f)
				{
					box.position.y = player.position.y + 6.0f; // 上昇完了
				}
			}
			else	missileRain = true; // 上昇完了後にフラグを立てる
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
							// 衝突している場合、ダメージを与える
							otherPlayer.hp -= SPECIAL_GLASS_DAMAGE * otherPlayer.defense;

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

	// 9個の箱すべてが非アクティブになった場合、スペシャル終了
	bool allInactive = std::all_of(glassBoxes.begin(), glassBoxes.end(), [](const GLASS_BOX& box){ return !box.active; });

	if (allInactive)
	{
		player.useSpecial = false;
		player.specialTimer = 0.0f;
		initialized = false;			// 次回のスペシャル使用時に再初期化するため
		missileRain = false;			// フラグをリセット
		player.form = Form::Normal;		// 変身形態を通常に戻す
		player.type = PlayerType::None;	// タイプをリセット
		player.useSkill = false;		// スキル解除
		player.useSpecial = false;		// スペシャル解除
	}
}

void Special_Concrete_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	static XMFLOAT3 playerPastPosition = { 0.0f, 0.0f, 0.0f }; // 初期化

	// スペシャルの初期位置をプレイヤーの位置に設定
	if (player.specialTimer == 0.0f) playerPastPosition = player.position;

	// スペシャルタイマー更新
	player.specialTimer += DELTA_TIME;

	// ジャンプ処理
	if (player.specialTimer <= 0.75f)
	{
		player.position.y = playerPastPosition.y + 3.0f * player.specialTimer / 0.75f; // 線形補間でY座標を上げる
	}
	else if (player.specialTimer > 0.75f && player.specialTimer <= 1.5f)
	{
		// 着地処理
		player.position = playerPastPosition;

		// ダメージ処理（1回だけ実行）
		if (player.specialTimer - DELTA_TIME < 0.75f) // 0.75秒を超えた瞬間に実行
		{
			const float radius = 5.0f;
			Circle circle = { player.position, radius }; // 円の中心と半径を設定

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
					// 衝突している場合、ダメージを与える
					otherPlayer.hp -= SPECIAL_CONCRETE_DAMAGE * otherPlayer.defense;

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
		player.form = Form::Normal;		// 変身形態を通常に戻す
		player.type = PlayerType::None;	// タイプをリセット
		player.useSkill = false;		// スキル解除
		player.useSpecial = false;		// スペシャル解除
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

	// 半径5.0fの円形当たり判定を作成
	const float radius = 5.0f;
	Circle circle = { player.position, radius }; // 円の中心と半径を設定

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
			// 衝突している場合、防御率でダメージ軽減（ノックバックは与えない）
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
		player.form = Form::Normal;		// 変身形態を通常に戻す
		player.type = PlayerType::None;	// タイプをリセット
		player.useSkill = false;		// スキル解除
		player.useSpecial = false;		// スペシャル解除
	}
}

void Special_Electric_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// スペシャルタイマー更新
	player.specialTimer += DELTA_TIME;

	// スペシャルの初期化処理
	static bool initialized = false;
	if (!initialized)
	{
		for (int i = 0; i < SPECIAL_ELECTRIC_QUANTITY; ++i)
		{
			// ランダムな位置に円を生成 (-5から5の範囲)
			float randomX = static_cast<float>(rand() % 10 - 5);
			float randomZ = static_cast<float>(rand() % 10 - 5);
			electricCircles[i] = { XMFLOAT3(randomX, 0.0f, randomZ), 0.3f };	// 半径0.3の円
		}
		initialized = true;
	}

	// 他のプレイヤーとの衝突判定
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (p == playerIndex) continue;	// 自分自身は無視

		PLAYEROBJECT* otherPlayerObject = GetPlayer(p);
		if (otherPlayerObject == nullptr || !otherPlayerObject->active) continue;
		PLAYEROBJECT& otherPlayer = *otherPlayerObject;

		if (otherPlayer.isInvincible) continue;	// 無敵中は無視

		for (int i = 0; i < SPECIAL_ELECTRIC_QUANTITY; ++i)
		{
			// 円とAABBの衝突判定 スタンさせる
			if (CheckCircleAABBCollision(electricCircles[i], otherPlayer.boundingBox))	otherPlayer.stunGauge = 10.0f;
		}
	}

	// スペシャルの効果時間が経過したらスペシャル終了
	if (player.specialTimer >= SPECIAL_ELECTRIC_TIME)
	{
		player.useSpecial = false;
		player.specialTimer = 0.0f;
		initialized = false;			// 次回のスペシャル使用時に再初期化するため
		player.form = Form::Normal;		// 変身形態を通常に戻す
		player.type = PlayerType::None;	// タイプをリセット
		player.useSkill = false;		// スキル解除
		player.useSpecial = false;		// スペシャル解除
	}
}

void Special_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	// スペシャル使用中かつスタン中でない場合に更新処理を行う
	if (player.useSpecial && !player.isStunning)
	{
		switch (player.type)
		{
		case PlayerType::Glass:		Special_Glass_Update(playerIndex);		break;
		case PlayerType::Concrete:	Special_Concrete_Update(playerIndex);	break;
		case PlayerType::Plant:		Special_Plant_Update(playerIndex);		break;
		case PlayerType::Electric:	Special_Electric_Update(playerIndex);	break;
		default: break;
		}
	}
}

// Glass専用描画
void Special_Glass_Draw(int playerIndex)
{
	// Glass専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Special_Texture[0]; // uiAim.png のテクスチャ
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// 箱の描画
	for (const auto& box : glassBoxes)
	{
		if (!box.active) continue; // 非アクティブな箱は描画しない

		XMMATRIX WorldMatrix =
			XMMatrixScaling(box.scaling.x, box.scaling.y, box.scaling.z) *
			XMMatrixRotationRollPitchYaw(XMConvertToRadians(box.rotation.x), XMConvertToRadians(box.rotation.y), XMConvertToRadians(box.rotation.z)) *
			XMMatrixTranslation(box.position.x, box.position.y, box.position.z);

		XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
		Shader_SetMatrix(WVP);

		// 描画実行
		g_pContext->DrawIndexed(6 * 6, 0, 0);
	}

	// 攻撃範囲の描画 (uiAimをtargetPositionに表示)
	for (const auto& box : glassBoxes)
	{
		if (!box.active) continue; // 非アクティブな箱は描画しない

		// targetPositionに基づいて描画
		XMMATRIX debugWorldMatrix =
			XMMatrixScaling(1.0f, 1.0f, 1.0f) *
			XMMatrixRotationX(XMConvertToRadians(0.0f)) *
			XMMatrixTranslation(box.targetPosition.x, box.targetPosition.y + 0.1f, box.targetPosition.z - 0.5f); // Y座標を少し上げて地面と重ならないようにする カメラの向きに考慮してZ座標をずらす

		XMMATRIX debugWVP = debugWorldMatrix * GetViewMatrix() * GetProjectionMatrix();
		Shader_SetMatrix(debugWVP);

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
	//// Concrete専用のテクスチャをセット
	//ID3D11ShaderResourceView* tex = g_Special_Texture[1];
	//g_pContext->PSSetShaderResources(0, 1, &tex);

	// Concrete用の座標計算
	SPECIAL_OBJECT& specialConcrete = Special[playerIndex];

	XMMATRIX WorldMatrix =
		XMMatrixScaling(specialConcrete.scaling.x, specialConcrete.scaling.y, specialConcrete.scaling.z) *
		XMMatrixRotationRollPitchYaw(XMConvertToRadians(specialConcrete.rotation.x), XMConvertToRadians(specialConcrete.rotation.y), XMConvertToRadians(specialConcrete.rotation.z)) *
		XMMatrixTranslation(specialConcrete.position.x, specialConcrete.position.y, specialConcrete.position.z);

	XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(WVP);

	// 描画実行
	g_pContext->DrawIndexed(6 * 6, 0, 0);
}

void Special_Plant_Draw(int playerIndex)
{
	//// Plant専用のテクスチャをセット
	//ID3D11ShaderResourceView* tex = g_Special_Texture[1];
	//g_pContext->PSSetShaderResources(0, 1, &tex);

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
}

void Special_Electric_Draw(int playerIndex)
{
	// Electric専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Special_Texture[4]; // uiAim.png のテクスチャ
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// デバッグ用に円の中心に uiAim.png を地面に表示
	for (int i = 0; i < SPECIAL_ELECTRIC_QUANTITY; ++i)
	{
		// 各円の中心にテクスチャを描画
		XMMATRIX debugWorldMatrix =
			XMMatrixScaling(1.0f, 1.0f, 1.0f) *
			XMMatrixRotationX(XMConvertToRadians(0.0f)) *
			XMMatrixTranslation(electricCircles[i].center.x, electricCircles[i].center.y + 0.1f, electricCircles[i].center.z); // Y座標を少し上げて地面と重ならないようにする

		XMMATRIX debugWVP = debugWorldMatrix * GetViewMatrix() * GetProjectionMatrix();
		Shader_SetMatrix(debugWVP);

		// アルファブレンディングを有効化
		SetBlendState(BLENDSTATE_ALPHA);

		// 描画実行 (+Y面の一枚だけ描画)
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); // トポロジーを三角形ストリップに設定
		g_pContext->Draw(4, 16); // +Y面の4頂点 (16, 17, 18, 19) を描画
	}
}

void Special_Draw()
{
	// 1. 共通設定 (パイプラインステートの設定)
	//    これを親で一度だけやることで処理落ちを防ぐ

	// シェーダー開始
	Shader_Begin();

	// ブレンドステート
	SetBlendState(BLENDSTATE_NONE);
	Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	// 頂点バッファ・インデックスバッファのセット
	UINT stride = sizeof(Vertex2);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 頂点データ書き込み
	// ※ プレイヤー本体の描画と同様に、ここで一度だけ頂点データをGPUに送ります
	D3D11_MAPPED_SUBRESOURCE msr;
	// (注意: g_VertexBuffer が D3D11_USAGE_DYNAMIC である必要があります)
	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex2* vertex = (Vertex2*)msr.pData;
	// vdata[] はキューブの頂点データを格納した配列を想定
	CopyMemory(&vertex[0], &Special_vdata[0], sizeof(Vertex2) * NUM_VERTEX);
	g_pContext->Unmap(g_VertexBuffer, 0);

	// 2. プレイヤーごとの振り分け処理
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		PLAYEROBJECT* playerObject = GetPlayer(p);
		if (playerObject == nullptr) continue;
		PLAYEROBJECT& player = *playerObject;

		// そのプレイヤーがスペシャルを使っているかチェック
		if (!player.useSpecial) continue;

		// プレイヤーがスタンしていない場合のみ描画
		if (player.isStunning == false)
		{
			// プレイヤーのタイプに合わせて子関数を呼ぶ
			switch (player.type)
			{
			case PlayerType::Glass:		Special_Glass_Draw(p);		break;
			case PlayerType::Concrete:	Special_Concrete_Draw(p);	break;
			case PlayerType::Plant:		Special_Plant_Draw(p);		break;
			case PlayerType::Electric:	Special_Electric_Draw(p);	break;
			default: break;
			}
		}
	}

	// 3. 後始末
	SetBlendState(BLENDSTATE_ALPHA);
}

SPECIAL_OBJECT* GetSpecial(int playerIndex)
{
	// 範囲チェック 0 1 2 3 以外なら nullptr を返す
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX)	return nullptr;

	return &Special[playerIndex];
}