// special.cpp

#include "DirectXMath.h"
#include "d3d11.h"
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
static ID3D11ShaderResourceView* g_Special_Texture[4];

// オブジェクト
static SPECIAL_OBJECT Special[PLAYER_MAX];

static GlassSpecial g_GlassSpecial[PLAYER_MAX];

// マクロ定義
#define NUM_VERTEX (36)

static Vertex Special_vdata[NUM_VERTEX] =
{
	// -Z面
	{
		// 頂点0 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点1 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点2 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),    // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 1.0f),             // テクスチャ座標
	},
	{
		// 頂点3 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 1.0f),             // テクスチャ座標
	},

	// +X面
	{
		// 頂点4 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点5 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),       // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点6 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 1.0f),             // テクスチャ座標
	},
	{
		// 頂点7 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 1.0f),             // テクスチャ座標
	},

	// +Z面
	{
		// 頂点8 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),       // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点9 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点10 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 1.0f),             // テクスチャ座標
	},
	{
		// 頂点11 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 1.0f),             // テクスチャ座標
	},

	// -X面
	{
		// 頂点12 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点13 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点14 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 1.0f),             // テクスチャ座標
	},
	{
		// 頂点15 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),    // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 1.0f),             // テクスチャ座標
	},

	// +Y面
	{
		// 頂点16 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点17 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),       // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点18 LEFT-BOTTOM
		XMFLOAT3(-0.5f, 0.5f, -0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 1.0f),             // テクスチャ座標
	},
	{
		// 頂点19 RIGHT-BOTTOM
		XMFLOAT3(0.5f, 0.5f, -0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 1.0f),             // テクスチャ座標
	},

	// -Y面
	{
		// 頂点20 LEFT-TOP
		XMFLOAT3(-0.5f, -0.5f, -0.5f),    // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点21 RIGHT-TOP
		XMFLOAT3(0.5f, -0.5f, -0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点22 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 1.0f),             // テクスチャ座標
	},
	{
		// 頂点23 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 1.0f),             // テクスチャ座標
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
	// 構造体のインスタンス（グローバル変数 g_GlassSpecial を想定）
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		for (int i = 0; i < 5; ++i)
		{
			// 各箱の初期座標を設定
			// 例えば、プレイヤーの前にオフセットを持たせるなど
			g_GlassSpecial[p].boxes[i].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_GlassSpecial[p].boxes[i].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_GlassSpecial[p].boxes[i].scaling = XMFLOAT3(0.2f, 0.2f, 0.2f);
			// BoundingBoxの初期化などもここで行う
		}
		// その他の初期状態を設定
		g_GlassSpecial[p].isActive = false;
	}

	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\SkyBlue.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[0]);
	assert(g_Special_Texture[0]);

}

void Special_Concrete_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	Special[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Special[0].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Special[0].scaling = XMFLOAT3(0.2f, 0.2f, 0.2f);

	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[1]);
	assert(g_Special_Texture[1]);

}

void Special_Plant_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[2]);
	assert(g_Special_Texture[2]);

}

void Special_Electric_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Special_Texture[3]);
	assert(g_Special_Texture[3]);

}

void Special_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点バッファ作成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * NUM_VERTEX; // 格納できる頂点数 * 頂点サイズ
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

	for (int i = 0; i < PLAYER_MAX; i++)
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

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex + 1);

	// ここで Radius の値を動的に計算する
	float dynamicRadius = playerObject->scaling.x; // scalingは等しいのでy,zでも可

	// 5つの箱に対応する相対角度 (度)
	const float RelativeAngles[5] = { 20.0f, 130.0f, 180.0f, 220.0f, 290.0f };

	// 5つの箱のプレイヤーからの高さオフセット
	const float High[5] = { 0.0f, 0.5f, -0.1f, 0.2f, 0.3f };

	// 5つの箱の回転角度 (度)
	const float Rot[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	// 5つの箱のスケーリング値
	const float Scal[5] = { 0.0f, 0.15f, 0.075f, 0.2f, 0.25f };

	// プレイヤーの現在の回転角度 (ラジアン)
	float playerYaw = XMConvertToRadians(playerObject->rotation.y);

	GlassSpecial& glassObject = g_GlassSpecial[playerIndex]; 

	for (int i = 0; i < 5; ++i)
	{
		float relativeRad = XMConvertToRadians(RelativeAngles[i]);
		float finalAngle = playerYaw + relativeRad;

		// 座標オフセットの計算
		float offsetX = dynamicRadius * cosf(finalAngle);
		float offsetZ = dynamicRadius * sinf(finalAngle);

		// 箱の座標を設定
		glassObject.boxes[i].position.x = playerObject->position.x + offsetX;
		glassObject.boxes[i].position.y = playerObject->position.y + High[i];
		glassObject.boxes[i].position.z = playerObject->position.z + offsetZ;
		glassObject.boxes[i].rotation = XMFLOAT3(Rot[i], Rot[i], Rot[i]);
		glassObject.boxes[i].scaling = XMFLOAT3(Scal[i], Scal[i], Scal[i]);
	}

	// スキル効果：

	// スキルタイマーを進める
	playerObject->specialTimer += 1.0f / 60.0f;

	// スキルの効果時間が経過したらスキル終了
	if (playerObject->specialTimer >= 30)
	{
		playerObject->useSpecial = false;
		playerObject->specialTimer = 0.0f;
	}
}

void Special_Concrete_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex + 1);
	SPECIAL_OBJECT& sk = Special[playerIndex];

	// スキル効果： ダメージ軽減20% (デフォルトは1.0f)
	playerObject->defense = 0.8f;

	// スキルの初期位置をプレイヤーの位置に設定
	sk.position.x = playerObject->position.x;
	sk.position.y = playerObject->position.y;
	sk.position.z = playerObject->position.z;

	// スキルタイマーを進める
	playerObject->specialTimer += 1.0f / 60.0f;

	// スキルの効果時間が経過したらスキル終了
	if (playerObject->specialTimer >= SPECIAL_CONCRETE_TIME)
	{
		playerObject->useSpecial = false;
		playerObject->specialTimer = 0.0f;
	}
}

void Special_Plant_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex + 1);

	// スキル効果：進化ゲージ2倍
	playerObject->evolutionGaugeRate *= 2;

	// スキルタイマーを進める
	playerObject->specialTimer += 1.0f / 60.0f;

	// スキルの効果時間が経過したらスキル終了
	if (playerObject->specialTimer >= SPECIAL_PLANT_TIME)
	{
		playerObject->useSpecial = false;
		playerObject->specialTimer = 0.0f;
	}
}

void Special_Electric_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex + 1);

	// スキル効果：スピード1.5倍
	playerObject->speed *= 1.5f;

	// スキルタイマーを進める
	playerObject->specialTimer += 1.0f / 60.0f;

	// スキルの効果時間が経過したらスキル終了
	if (playerObject->specialTimer >= SPECIAL_ELECTRIC_TIME)
	{
		playerObject->useSpecial = false;
		playerObject->specialTimer = 0.0f;
	}
}

//void Special_Update(int playerIndex)
//{
//	// 範囲チェック
//	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;
//
//	PLAYEROBJECT* playerObject = GetPlayer(playerIndex + 1);
//	SPECIAL_OBJECT& sk = Special[playerIndex];
//
//	if (playerObject->isAttacking == true)
//	{
//		// スキルの初期位置をプレイヤーの位置に設定
//		sk.position.x = playerObject->position.x;
//		sk.position.y = playerObject->position.y;
//		sk.position.z = playerObject->position.z;
//
//		float Player_RotationY = playerObject->rotation.y;
//		float rad = XMConvertToRadians(Player_RotationY);
//
//		// 進行方向を計算
//		XMFLOAT3 dir =
//		{
//			sinf(rad),  // X方向
//			0.0f,       // Y方向（水平）
//			cosf(rad)   // Z方向
//		};
//
//		// スキルの速度を設定（前方向に飛ばす）
//		//float speed = 0.15f;
//		//Special[0].Velocity.x = dir.x * speed;
//		//Special[0].Velocity.y = dir.y * speed;
//		//Special[0].Velocity.z = dir.z * speed;
//
//		// プレイヤーの前方にスキルを配置
//		sk.position.x = dir.x * playerObject->scaling.x + playerObject->position.x;
//		sk.position.y = playerObject->position.y;
//		sk.position.z = dir.z * playerObject->scaling.z + playerObject->position.z;
//
//		// 攻撃タイマーを進める
//		playerObject->attackTimer += 1.0f / 60.0f;
//
//		// プレイヤー毎の攻撃時間が経過したら攻撃終了
//		if (playerObject->attackTimer > playerObject->attackDuration)
//		{
//			playerObject->isAttacking = false;
//			playerObject->attackTimer = 0.0f;
//		}
//	}
//
//	// -------------------------------------------------------------
//	// 当たり判定
//	// -------------------------------------------------------------
//	// AABBの更新
//	CalculateAABB(sk.boundingBox, sk.position, XMFLOAT3(1.0f, 1.0f, 1.0f));
//
//	int buildingCount = GetBuildingCount();			// 数を取得
//	Building** buildingObjects = GetBuildings();	// リストを取得
//
//	// 全てのフィールドオブジェクトと衝突判定を行う
//	for (int i = 0; i < buildingCount; ++i)
//	{
//		// i番目のフィールドオブジェクトのAABBを取得
//		// field.cppのInitializeで計算済みのため、そのまま参照
//		AABB pStaticObjectAABB = buildingObjects[i]->boundingBox;
//
//		// プレイヤーのAABBとフィールドオブジェクトのAABBでMTVを計算
//		MTV collision = CalculateAABBMTV(sk.boundingBox, pStaticObjectAABB);
//
//		if (collision.isColliding)
//		{
//			// プレイヤーごとに使うキーを決定（Player1 -> SPACE, Player2 -> ENTER）
//			Keyboard_Keys_tag confirmKey = (playerIndex == 0) ? KK_SPACE : KK_ENTER;
//
//			// 建物（FIELD_BUILDING）に衝突していて、かつスペースキーが押されていたら
//			if (/*buildingObjects[i]->Type == BuildingType::Glass && */Keyboard_IsKeyDown(confirmKey))
//			{
//				buildingObjects[i]->isActive = false;
//				playerObject->form = (Form)((int)playerObject->form + 1);
//				// 必要ならここで効果音やエフェクトを再生
//				// スキルはヒット時に消す（任意）
//				playerObject->isAttacking = false;
//				playerObject->attackTimer = 0.0f;
//				// 更新済みAABB
//				CalculateAABB(sk.boundingBox, sk.position, sk.scaling);
//				continue;
//			}
//
//			// 衝突していたら、MTVの分だけ位置を戻す
//			sk.position.x += collision.translation.x;
//			sk.position.y += collision.translation.y;
//			sk.position.z += collision.translation.z;
//
//			// 押し戻し後の新しいAABBを再計算
//			// これにより、同じフレーム内で次のフィールドオブジェクトとの判定に備えます。
//			CalculateAABB(sk.boundingBox, sk.position, sk.scaling);
//
//			// デバッグ出力
//			hal::dout << "衝突！押し戻し量: " << collision.overlap << " @ " << (collision.translation.x != 0 ? "X軸" : (collision.translation.y != 0 ? "Y軸" : "Z軸")) << std::endl;
//
//			// ↑↑↑　#include "debug_ostream.h"　のインクルードでデバッグ確認
//		}
//	}
//}

// Glass専用描画 (5つの箱をループで描画)
void Special_Glass_Draw(int playerIndex)
{
	// Glass専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Special_Texture[0];
	g_pContext->PSSetShaderResources(0, 1, &tex);

	GlassSpecial& glassObject = g_GlassSpecial[playerIndex];

	// GlassSpecial構造体（5つの箱）を使ってループ描画
	for (int i = 0; i < 5; ++i)
	{
		SPECIAL_OBJECT& box = glassObject.boxes[i];

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

// Concrete専用描画 (例: 1つの大きな塊を描画)
void Special_Concrete_Draw(int playerIndex)
{
	// Concrete専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Special_Texture[1];
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// Concrete用の座標計算 (ここでは例として Special[playerIndex] を使用)
	// ※Concrete専用構造体があるならそちらを使う
	SPECIAL_OBJECT& sk = Special[playerIndex];

	XMMATRIX WorldMatrix =
		XMMatrixScaling(sk.scaling.x, sk.scaling.y, sk.scaling.z) *
		XMMatrixRotationRollPitchYaw(XMConvertToRadians(sk.rotation.x), XMConvertToRadians(sk.rotation.y), XMConvertToRadians(sk.rotation.z)) *
		XMMatrixTranslation(sk.position.x, sk.position.y, sk.position.z);

	XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(WVP);

	// 描画実行
	g_pContext->DrawIndexed(6 * 6, 0, 0);
}

void Special_Plant_Draw(int playerIndex)
{
	// Concrete専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Special_Texture[1];
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// Concrete用の座標計算 (ここでは例として Special[playerIndex] を使用)
	// ※Concrete専用構造体があるならそちらを使う
	SPECIAL_OBJECT& sk = Special[playerIndex];

	XMMATRIX WorldMatrix =
		XMMatrixScaling(sk.scaling.x, sk.scaling.y, sk.scaling.z) *
		XMMatrixRotationRollPitchYaw(XMConvertToRadians(sk.rotation.x), XMConvertToRadians(sk.rotation.y), XMConvertToRadians(sk.rotation.z)) *
		XMMatrixTranslation(sk.position.x, sk.position.y, sk.position.z);

	XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(WVP);

	// 描画実行
	g_pContext->DrawIndexed(6 * 6, 0, 0);

}

void Special_Electric_Draw(int playerIndex)
{
	// Concrete専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Special_Texture[1];
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// Concrete用の座標計算 (ここでは例として Special[playerIndex] を使用)
	// ※Concrete専用構造体があるならそちらを使う
	SPECIAL_OBJECT& sk = Special[playerIndex];

	XMMATRIX WorldMatrix =
		XMMatrixScaling(sk.scaling.x, sk.scaling.y, sk.scaling.z) *
		XMMatrixRotationRollPitchYaw(XMConvertToRadians(sk.rotation.x), XMConvertToRadians(sk.rotation.y), XMConvertToRadians(sk.rotation.z)) *
		XMMatrixTranslation(sk.position.x, sk.position.y, sk.position.z);

	XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(WVP);

	// 描画実行
	g_pContext->DrawIndexed(6 * 6, 0, 0);

}

void Special_Draw()
{
	// ==========================================
	// 1. 共通設定 (パイプラインステートの設定)
	//    これを親で一度だけやることで処理落ちを防ぐ
	// ==========================================

	// シェーダー開始
	Shader_Begin();

	// ブレンドステート (必要に応じて)
	SetBlendState(BLENDSTATE_NONE); // または BLENDSTATE_ALPHA
	Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	// 頂点バッファ・インデックスバッファのセット
	// (全てのスキルで同じモデル/キューブを使う場合のみここでOK)
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ★ 頂点データ書き込み (静的なキューブモデルと仮定) ★
	// ※ プレイヤー本体の描画と同様に、ここで一度だけ頂点データをGPUに送ります
	D3D11_MAPPED_SUBRESOURCE msr;
	// (注意: g_VertexBuffer が D3D11_USAGE_DYNAMIC である必要があります)
	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex* vertex = (Vertex*)msr.pData;
	// vdata[] はキューブの頂点データを格納した配列を想定
	CopyMemory(&vertex[0], &Special_vdata[0], sizeof(Vertex) * NUM_VERTEX);
	g_pContext->Unmap(g_VertexBuffer, 0);

	// ==========================================
	// 2. プレイヤーごとの振り分け処理
	// ==========================================
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		PLAYEROBJECT* playerObject = GetPlayer(i + 1);

		// そのプレイヤーがスキルを使っているかチェック (フラグ確認など)
		if (!playerObject->useSpecial) continue;

		// プレイヤーのタイプに合わせて子関数を呼ぶ
		switch (playerObject->type)
		{
		case PlayerType::Glass:
			Special_Glass_Draw(i);
			break;

		case PlayerType::Concrete:
			Special_Concrete_Draw(i);
			break;

		case PlayerType::Plant:
			Special_Plant_Draw(i);
			break;

		case PlayerType::Electric:
			Special_Electric_Draw(i);
			break;

		default:
			break;
		}
	}

	// ==========================================
	// 3. 後始末
	// ==========================================
	SetBlendState(BLENDSTATE_ALPHA);
}

SPECIAL_OBJECT* GetSpecial(int playerIndex)
{
	if (playerIndex > PLAYER_MAX || playerIndex <= 0)
	{
		return nullptr;
	}

	return &Special[playerIndex - 1];
}