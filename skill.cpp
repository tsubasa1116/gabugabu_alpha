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
static ID3D11ShaderResourceView* g_Skill_Texture[PLAYER_MAX];

// オブジェクト
static SKILL_OBJECT Skill[PLAYER_MAX];

// 攻撃タイマー
float PlayerAttackTimer[PLAYER_MAX];

// プレイヤー 攻撃フラグ
bool PlayerIsAttacking[PLAYER_MAX] = { false, false/*, false, false*/ };

// マクロ定義
#define NUM_VERTEX (36)

static Vertex Skill_vdata[NUM_VERTEX] =
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
static UINT Skill_idxdata[6 * 6]
{
	 0,  1,  2,  2,  1,  3, // -Z面
	 4,  5,  6,  6,  5,  7, // +X面
	 8,  9, 10, 10,  9, 11, // +Z面
	12, 13, 14, 14, 13, 15, // -X面
	16, 17, 18, 18, 17, 19, // +Y面
	20, 21, 22, 22, 21, 23, // -Y面
};

void Skill_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	Skill[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Skill[0].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Skill[0].scaling = XMFLOAT3(0.2f, 0.2f, 0.2f);
	Skill[0].velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	Skill[1].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Skill[1].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Skill[1].scaling = XMFLOAT3(0.3f, 0.3f, 0.3f);
	Skill[1].velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// 頂点バッファ作成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * NUM_VERTEX; // 格納できる頂点数 * 頂点サイズ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	g_pDevice = pDevice;
	g_pContext = pContext;

	// テクスチャ読み込み
	TexMetadata metadata1;
	ScratchImage image1;

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata1, image1);
	CreateShaderResourceView(pDevice, image1.GetImages(),
		image1.GetImageCount(), metadata1, &g_Skill_Texture[0]);
	assert(g_Skill_Texture[0]);

	// テクスチャ読み込み
	TexMetadata metadata2;
	ScratchImage image2;
	LoadFromWICFile(L"Asset\\Texture\\SkyBlue.jpg", WIC_FLAGS_NONE, &metadata2, image2);
	CreateShaderResourceView(pDevice, image2.GetImages(),
		image2.GetImageCount(), metadata2, &g_Skill_Texture[1]);
	assert(g_Skill_Texture[1]);

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

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (g_Skill_Texture[i])
		{
			g_Skill_Texture[i]->Release();
			g_Skill_Texture[i] = NULL;
		}
	}
}

void Player1_Skill_Update()
{
	PLAYEROBJECT* Player1Object = GetPlayer(1);

	if (Player1Object->isAttacking == true)
	{
		Skill[0].position.x = Player1Object->position.x;
		Skill[0].position.y = Player1Object->position.y;
		Skill[0].position.z = Player1Object->position.z;

		float Player1_RotationY = Player1Object->rotation.y;
		float rad = XMConvertToRadians(Player1_RotationY);

		// 進行方向を計算
		XMFLOAT3 dir =
		{
			sinf(rad),  // X方向
			0.0f,       // Y方向（水平）
			cosf(rad)   // Z方向
		};

		// スキルの速度を設定（前方向に飛ばす）
		//float speed = 0.15f;
		//Skill[0].Velocity.x = dir.x * speed;
		//Skill[0].Velocity.y = dir.y * speed;
		//Skill[0].Velocity.z = dir.z * speed;

		Skill[0].position.x = dir.x * Player1Object->scaling.x + Player1Object->position.x;
		Skill[0].position.y = Player1Object->position.y;
		Skill[0].position.z = dir.z * Player1Object->scaling.z + Player1Object->position.z;

		Player1Object->attackTimer += 1.0f / 60.0f;

		if (Player1Object->attackTimer > Player1Object->attackDuration)
		{
			Player1Object->isAttacking = false;
			Player1Object->attackTimer = 0.0f;
		}
	}

	// -------------------------------------------------------------
	// 当たり判定
	// -------------------------------------------------------------
	// AABBの更新
	CalculateAABB(Skill[0].boundingBox, Skill[0].position, XMFLOAT3(1.0f, 1.0f, 1.0f));

	// フィールドオブジェクトのリストを取得
	MAPDATA* fieldObjects = GetFieldObjects();
	int fieldCount = GetFieldObjectCount();

	// 全てのフィールドオブジェクトと衝突判定を行う
	for (int i = 0; i < fieldCount; ++i)
	{
		// i番目のフィールドオブジェクトのAABBを取得
		// field.cppのInitializeで計算済みのため、そのまま参照
		AABB pStaticObjectAABB = fieldObjects[i].boundingBox;

		// プレイヤーのAABBとフィールドオブジェクトのAABBでMTVを計算
		MTV collision = CalculateAABBMTV(Skill[0].boundingBox, pStaticObjectAABB);

		if (collision.isColliding)
		{
			// ガラスに衝突していて、かつスペースキーが押されていたら
			if (fieldObjects[i].no == FIELD_Glass && Keyboard_IsKeyDown(KK_SPACE))
			{
				fieldObjects[i].isActive = false;
				Player1Object->form = (Form)((int)Player1Object->form + 1);
				// 必要ならここで効果音やエフェクトを再生
				// スキルはヒット時に消す（任意）
				Player1Object->isAttacking = false;
				Player1Object->attackTimer = 0.0f;
				// 更新済みAABB
				CalculateAABB(Skill[0].boundingBox, Skill[0].position, Skill[0].scaling);
				continue;
			}

			// 衝突していたら、MTVの分だけ位置を戻す
			Skill[0].position.x += collision.translation.x;
			Skill[0].position.y += collision.translation.y;
			Skill[0].position.z += collision.translation.z;

			// 押し戻し後の新しいAABBを再計算
			// これにより、同じフレーム内で次のフィールドオブジェクトとの判定に備えます。
			CalculateAABB(Skill[0].boundingBox, Skill[0].position, Skill[0].scaling);

			// デバッグ出力
			hal::dout << "衝突！押し戻し量: " << collision.overlap << " @ " << (collision.translation.x != 0 ? "X軸" : (collision.translation.y != 0 ? "Y軸" : "Z軸")) << std::endl;

			// ↑↑↑　#include "debug_ostream.h"　のインクルードでデバッグ確認
		}
	}
}

void Player2_Skill_Update()
{
	PLAYEROBJECT* Player2Object = GetPlayer(2);

	if (Player2Object->isAttacking == true)
	{
		// プレイヤーの座標からスキルを発射
		Skill[1].position.x = Player2Object->position.x;
		Skill[1].position.y = Player2Object->position.y;
		Skill[1].position.z = Player2Object->position.z;

		float Player2_RotationY = Player2Object->rotation.y;
		float rad = XMConvertToRadians(Player2_RotationY);

		// 進行方向を計算
		XMFLOAT3 dir = {
			sinf(rad),  // X方向
			0.0f,       // Y方向（水平）
			cosf(rad)   // Z方向
		};

		// スキルの速度を設定（前方向に飛ばす）
		//float speed = 0.1f;
		//Skill[1].Velocity.x = dir.x * speed;
		//Skill[1].Velocity.y = dir.y * speed;
		//Skill[1].Velocity.z = dir.z * speed;

		Skill[1].position.x = dir.x * Player2Object->scaling.x + Player2Object->position.x;
		Skill[1].position.y = Player2Object->position.y;
		Skill[1].position.z = dir.z * Player2Object->scaling.z + Player2Object->position.z;

		Player2Object->attackTimer += 1.0f / 60.0f;

		if (Player2Object->attackTimer > Player2Object->attackDuration)
		{
			Player2Object->isAttacking = false;
			Player2Object->attackTimer = 0.0f;
		}
	}

	// -------------------------------------------------------------
	// 当たり判定
	// -------------------------------------------------------------
	// AABBの更新
	CalculateAABB(Skill[1].boundingBox, Skill[1].position, XMFLOAT3(1.0f, 1.0f, 1.0f));

	// フィールドオブジェクトのリストを取得
	MAPDATA* fieldObjects = GetFieldObjects();
	int fieldCount = GetFieldObjectCount();

	// 全てのフィールドオブジェクトと衝突判定を行う
	for (int i = 0; i < fieldCount; ++i)
	{
		// i番目のフィールドオブジェクトのAABBを取得
		// field.cppのInitializeで計算済みのため、そのまま参照
		AABB pStaticObjectAABB = fieldObjects[i].boundingBox;

		// プレイヤーのAABBとフィールドオブジェクトのAABBでMTVを計算
		MTV collision = CalculateAABBMTV(Skill[1].boundingBox, pStaticObjectAABB);

		if (collision.isColliding)
		{
			// 建物（FIELD_BUILDING）に衝突していて、かつスペースキーが押されていたら
			if (fieldObjects[i].no == FIELD_Glass && Keyboard_IsKeyDown(KK_ENTER))
			{
				fieldObjects[i].isActive = false;
				Player2Object->form = (Form)((int)Player2Object->form + 1);
				// 必要ならここで効果音やエフェクトを再生
				// スキルはヒット時に消す（任意）
				Player2Object->isAttacking = false;
				Player2Object->attackTimer = 0.0f;
				// 更新済みAABB
				CalculateAABB(Skill[1].boundingBox, Skill[1].position, Skill[1].scaling);
				continue;
			}

			// 衝突していたら、MTVの分だけ位置を戻す
			Skill[1].position.x += collision.translation.x;
			Skill[1].position.y += collision.translation.y;
			Skill[1].position.z += collision.translation.z;

			// 押し戻し後の新しいAABBを再計算
			// これにより、同じフレーム内で次のフィールドオブジェクトとの判定に備えます。
			CalculateAABB(Skill[1].boundingBox, Skill[1].position, Skill[1].scaling);

			// デバッグ出力
			hal::dout << "衝突！押し戻し量: " << collision.overlap << " @ " << (collision.translation.x != 0 ? "X軸" : (collision.translation.y != 0 ? "Y軸" : "Z軸")) << std::endl;

			// ↑↑↑　#include "debug_ostream.h"　のインクルードでデバッグ確認
		}
	}
}

void Player1_Skill_Draw()
{
	// =====================
	// ワールド行列の作成
	// =====================

	// スケーリング行列の作成
	XMMATRIX ScalingMatrix = XMMatrixScaling
	(
		Skill[0].scaling.x,
		Skill[0].scaling.y,
		Skill[0].scaling.z
	);

	// 回転行列の作成
	XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw
	(
		XMConvertToRadians(Skill[0].rotation.x),
		XMConvertToRadians(Skill[0].rotation.y),
		XMConvertToRadians(Skill[0].rotation.z)
	);

	// 平行移動行列の作成
	XMMATRIX TranslationMatrix = XMMatrixTranslation
	(
		Skill[0].position.x,
		Skill[0].position.y,
		Skill[0].position.z
	);

	XMMATRIX WorldMatrix = ScalingMatrix * RotationMatrix * TranslationMatrix;

	// プロジェクション行列作成
	XMMATRIX Projection = GetProjectionMatrix();

	// ビュー行列作成
	XMMATRIX View = GetViewMatrix();

	// 最終的な変換行列を作成
	XMMATRIX WVP = WorldMatrix * View * Projection;

	// 変換行列を頂点シェーダーへセット
	Shader_SetMatrix(WVP);

	// シェーダーを描画パイプラインへ設定
	Shader_Begin();

	// 頂点シェーダーを描画パイプラインへ設定
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex* vertex = (Vertex*)msr.pData;

	// 頂点データを頂点バッファへコピーする
	CopyMemory(&vertex[0], &Skill_vdata[0], sizeof(Vertex) * NUM_VERTEX);

	// コピー完了
	g_pContext->Unmap(g_VertexBuffer, 0);

	// テクスチャをセット
	g_pContext->PSSetShaderResources(0, 1, &g_Skill_Texture[0]);

	// 頂点バッファをセット
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// インデックスバッファをセット
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 描画するポリゴンの種類をセット 3頂点でポリゴン1枚として表示
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//// 描画リクエスト
	//g_pContext->Draw(NUM_VERTEX, 0);

	g_pContext->DrawIndexed(6 * 6, 0, 0);
}

void Player2_Skill_Draw()
{
	// =====================
	// ワールド行列の作成
	// =====================

	// スケーリング行列の作成
	XMMATRIX ScalingMatrix = XMMatrixScaling
	(
		Skill[1].scaling.x,
		Skill[1].scaling.y,
		Skill[1].scaling.z
	);

	// 回転行列の作成
	XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw
	(
		XMConvertToRadians(Skill[1].rotation.x),
		XMConvertToRadians(Skill[1].rotation.y),
		XMConvertToRadians(Skill[1].rotation.z)
	);

	// 平行移動行列の作成
	XMMATRIX TranslationMatrix = XMMatrixTranslation
	(
		Skill[1].position.x,
		Skill[1].position.y,
		Skill[1].position.z
	);

	XMMATRIX WorldMatrix = ScalingMatrix * RotationMatrix * TranslationMatrix;

	// プロジェクション行列作成
	XMMATRIX Projection = GetProjectionMatrix();

	// ビュー行列作成
	XMMATRIX View = GetViewMatrix();

	// 最終的な変換行列を作成
	XMMATRIX WVP = WorldMatrix * View * Projection;

	// 変換行列を頂点シェーダーへセット
	Shader_SetMatrix(WVP);

	// シェーダーを描画パイプラインへ設定
	Shader_Begin();

	// 頂点シェーダーを描画パイプラインへ設定
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex* vertex = (Vertex*)msr.pData;

	// 頂点データを頂点バッファへコピーする
	CopyMemory(&vertex[0], &Skill_vdata[0], sizeof(Vertex) * NUM_VERTEX);

	// コピー完了
	g_pContext->Unmap(g_VertexBuffer, 0);

	// テクスチャをセット
	g_pContext->PSSetShaderResources(0, 1, &g_Skill_Texture[1]);

	// 頂点バッファをセット
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// インデックスバッファをセット
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 描画するポリゴンの種類をセット 3頂点でポリゴン1枚として表示
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//// 描画リクエスト
	//g_pContext->Draw(NUM_VERTEX, 0);

	g_pContext->DrawIndexed(6 * 6, 0, 0);

}

SKILL_OBJECT* GetSkill(int index)
{
	if (index > PLAYER_MAX || index <= 0)
	{
		return nullptr;
	}

	return &Skill[index - 1];
}

int GetSkill1ObjectCount()
{
	int count = 1;
	//int count = 0;
	//// map配列はFIELD_MAXを終了マーカーとしている
	//while (map[count].no != FIELD_MAX)
	//{
	//	count++;
	//}
	return count;
}

int GetSkill2ObjectCount()
{
	int count = 1;
	//int count = 0;
	//// map配列はFIELD_MAXを終了マーカーとしている
	//while (map[count].no != FIELD_MAX)
	//{
	//	count++;
	//}
	return count;
}
