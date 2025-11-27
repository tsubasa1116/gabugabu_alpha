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
static ID3D11Buffer* g_VertexBuffer = NULL;

// インデックスバッファ
static ID3D11Buffer* g_IndexBuffer = NULL;

// テクスチャ変数
static ID3D11ShaderResourceView* g_Skill1_Texture;
static ID3D11ShaderResourceView* g_Skill2_Texture;

// マクロ定義
#define NUM_VERTEX (100)

// オブジェクト
static SKILL1_OBJECT Skill1;
static SKILL2_OBJECT Skill2;

static bool Skill1_Use = true;
static bool Skill2_Use = true;

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

void Player1_Skill_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	Skill1.Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Skill1.Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Skill1.Scaling = XMFLOAT3(0.3f, 0.3f, 0.3f);
	Skill1.Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

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
	LoadFromWICFile(L"Asset\\Texture\\Red.jfif", WIC_FLAGS_NONE, &metadata1, image1);
	CreateShaderResourceView(pDevice, image1.GetImages(),
		image1.GetImageCount(), metadata1, &g_Skill1_Texture);
	assert(g_Skill1_Texture);

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

void Player2_Skill_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	PLAYER2_OBJECT* Player2Object = GetPlayer2();

	Skill2.Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Skill2.Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Skill2.Scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
	Skill2.Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

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
	TexMetadata metadata2;
	ScratchImage image2;
	LoadFromWICFile(L"Asset\\Texture\\TileA3.png", WIC_FLAGS_NONE, &metadata2, image2);
	CreateShaderResourceView(pDevice, image2.GetImages(),
		image2.GetImageCount(), metadata2, &g_Skill2_Texture);
	assert(g_Skill2_Texture);

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

	if (g_Skill1_Texture)
	{
		g_Skill1_Texture->Release();
		g_Skill1_Texture = NULL;
	}
	if (g_Skill2_Texture)
	{
		g_Skill2_Texture->Release();
		g_Skill2_Texture = NULL;
	}
}

void Player1_Skill_Update()
{
	if (Skill1_Use)
	{
		PLAYER1_OBJECT* Player1Object = GetPlayer1();
		Skill1.Position.x = Player1Object->Position.x;
		Skill1.Position.y = Player1Object->Position.y;
		Skill1.Position.z = Player1Object->Position.z;

		float Player1_RotationY = Player1Object->Rotation.y;
		float rad = XMConvertToRadians(Player1_RotationY);

		// 進行方向を計算
		XMFLOAT3 dir = {
			sinf(rad),  // X方向
			0.0f,       // Y方向（水平）
			cosf(rad)   // Z方向
		};

		// スキルの速度を設定（前方向に飛ばす）
		float speed = 0.2f;
		Skill1.Velocity.x = dir.x * speed;
		Skill1.Velocity.y = dir.y * speed;
		Skill1.Velocity.z = dir.z * speed;
		Skill1_Use = false;
	}

	Skill1.Position.x += Skill1.Velocity.x;
	Skill1.Position.y += Skill1.Velocity.y;
	Skill1.Position.z += Skill1.Velocity.z;

	// -------------------------------------------------------------
	// 当たり判定
	// -------------------------------------------------------------
	// AABBの更新
	CalculateAABB(Skill1.boundingBox, Skill1.Position, XMFLOAT3(1.0f, 1.0f, 1.0f));

	// フィールドオブジェクトのリストを取得
	Map* fieldObjects = GetFieldObjects();
	int fieldCount = GetFieldObjectCount();

	// 全てのフィールドオブジェクトと衝突判定を行う
	for (int i = 0; i < fieldCount; ++i)
	{
		// i番目のフィールドオブジェクトのAABBを取得
		// field.cppのInitializeで計算済みのため、そのまま参照
		AABB pStaticObjectAABB = fieldObjects[i].boundingBox;

		// プレイヤーのAABBとフィールドオブジェクトのAABBでMTVを計算
		MTV collision = CalculateAABBMTV(Skill1.boundingBox, pStaticObjectAABB);

		if (collision.isColliding)
		{
			// 衝突していたら、MTVの分だけ位置を戻す
			Skill1.Position.x += collision.translation.x;
			Skill1.Position.y += collision.translation.y;
			Skill1.Position.z += collision.translation.z;

			// 押し戻し後の新しいAABBを再計算
			// これにより、同じフレーム内で次のフィールドオブジェクトとの判定に備えます。
			CalculateAABB(Skill1.boundingBox, Skill1.Position, Skill1.Scaling);

			// デバッグ出力
			hal::dout << "衝突！押し戻し量: " << collision.overlap << " @ " << (collision.translation.x != 0 ? "X軸" : (collision.translation.y != 0 ? "Y軸" : "Z軸")) << std::endl;

			// ↑↑↑　#include "debug_ostream.h"　のインクルードでデバッグ確認
		}
	}
}

void Player2_Skill_Update()
{
	if (Skill2_Use)
	{
		PLAYER2_OBJECT* Player2Object = GetPlayer2();
		// プレイヤーの座標からスキルを発射
		Skill2.Position.x = Player2Object->Position.x;
		Skill2.Position.y = Player2Object->Position.y;
		Skill2.Position.z = Player2Object->Position.z;

		float Player2_RotationY = Player2Object->Rotation.y;
		float rad = XMConvertToRadians(Player2_RotationY);

		// 進行方向を計算
		XMFLOAT3 dir = {
			sinf(rad),  // X方向
			0.0f,       // Y方向（水平）
			cosf(rad)   // Z方向
		};

		// スキルの速度を設定（前方向に飛ばす）
		float speed = 0.1f;
		Skill2.Velocity.x = dir.x * speed;
		Skill2.Velocity.y = dir.y * speed;
		Skill2.Velocity.z = dir.z * speed;
		Skill2_Use = false;
	}

	Skill2.Position.x += Skill2.Velocity.x;
	Skill2.Position.y += Skill2.Velocity.y;
	Skill2.Position.z += Skill2.Velocity.z;

	// -------------------------------------------------------------
	// 当たり判定
	// -------------------------------------------------------------
	// AABBの更新
	CalculateAABB(Skill2.boundingBox, Skill2.Position, XMFLOAT3(1.0f, 1.0f, 1.0f));

	// フィールドオブジェクトのリストを取得
	Map* fieldObjects = GetFieldObjects();
	int fieldCount = GetFieldObjectCount();

	// 全てのフィールドオブジェクトと衝突判定を行う
	for (int i = 0; i < fieldCount; ++i)
	{
		// i番目のフィールドオブジェクトのAABBを取得
		// field.cppのInitializeで計算済みのため、そのまま参照
		AABB pStaticObjectAABB = fieldObjects[i].boundingBox;

		// プレイヤーのAABBとフィールドオブジェクトのAABBでMTVを計算
		MTV collision = CalculateAABBMTV(Skill2.boundingBox, pStaticObjectAABB);

		if (collision.isColliding)
		{
			// 衝突していたら、MTVの分だけ位置を戻す
			Skill2.Position.x += collision.translation.x;
			Skill2.Position.y += collision.translation.y;
			Skill2.Position.z += collision.translation.z;

			// 押し戻し後の新しいAABBを再計算
			// これにより、同じフレーム内で次のフィールドオブジェクトとの判定に備えます。
			CalculateAABB(Skill2.boundingBox, Skill2.Position, Skill2.Scaling);

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
		Skill1.Scaling.x,
		Skill1.Scaling.y,
		Skill1.Scaling.z
	);

	// 回転行列の作成
	XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw
	(
		XMConvertToRadians(Skill1.Rotation.x),
		XMConvertToRadians(Skill1.Rotation.y),
		XMConvertToRadians(Skill1.Rotation.z)
	);

	// 平行移動行列の作成
	XMMATRIX TranslationMatrix = XMMatrixTranslation
	(
		Skill1.Position.x,
		Skill1.Position.y,
		Skill1.Position.z
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
	g_pContext->PSSetShaderResources(0, 1, &g_Skill1_Texture);

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
		Skill2.Scaling.x,
		Skill2.Scaling.y,
		Skill2.Scaling.z
	);

	// 回転行列の作成
	XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw
	(
		XMConvertToRadians(Skill2.Rotation.x),
		XMConvertToRadians(Skill2.Rotation.y),
		XMConvertToRadians(Skill2.Rotation.z)
	);

	// 平行移動行列の作成
	XMMATRIX TranslationMatrix = XMMatrixTranslation
	(
		Skill2.Position.x,
		Skill2.Position.y,
		Skill2.Position.z
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
	g_pContext->PSSetShaderResources(0, 1, &g_Skill2_Texture);

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

SKILL1_OBJECT* GetSkill1Objects()
{
	return &Skill1;
}

SKILL2_OBJECT* GetSkill2Objects()
{
	return &Skill2;
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
