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
	// 構造体のインスタンス
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
	}

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
	Special[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Special[0].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Special[0].scaling = XMFLOAT3(0.2f, 0.2f, 0.2f);

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

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata, image);
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

	// スペシャルの効果時間が経過したらリセット
	if (player.specialTimer >= SPECIAL_GLASS_TIME)
	{
		player.useSpecial = false;
		player.specialTimer = 0.0f;
	}
}

void Special_Concrete_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	SPECIAL_OBJECT& specialConcrete = Special[playerIndex];

	// スペシャルの初期位置をプレイヤーの位置に設定
	specialConcrete.position = player.position;

	// スペシャルタイマー更新
	player.specialTimer += DELTA_TIME;

	// スペシャルの効果時間が経過したらスペシャル終了
	if (player.specialTimer >= SPECIAL_CONCRETE_TIME)
	{
		player.useSpecial = false;
		player.specialTimer = 0.0f;
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

	// スペシャルの効果時間が経過したらスペシャル終了
	if (player.specialTimer >= SPECIAL_PLANT_TIME)
	{
		player.useSpecial = false;
		player.specialTimer = 0.0f;
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

	// スペシャルの効果時間が経過したらスペシャル終了
	if (player.specialTimer >= SPECIAL_ELECTRIC_TIME)
	{
		player.useSpecial = false;
		player.specialTimer = 0.0f;
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
		//SetCameraAtPosition(player.position);

		switch (player.type)
		{
		case PlayerType::Glass:		Special_Glass_Update(playerIndex);		break;
		case PlayerType::Concrete:	Special_Concrete_Update(playerIndex);	break;
		case PlayerType::Plant:		Special_Plant_Update(playerIndex);		break;
		case PlayerType::Electric:	Special_Electric_Update(playerIndex);	break;
		default: break;
		}

		player.form = Form::Normal;		// 変身形態を通常に戻す
		player.type = PlayerType::None;	// スペシャル使用後にタイプをリセット
		player.useSkill = false;		// スキル解除
		player.useSpecial = false;		// スペシャル解除
	}
}

// Glass専用描画
void Special_Glass_Draw(int playerIndex)
{
	// Glass専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Special_Texture[0];
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// Glass用の座標計算
	SPECIAL_OBJECT& specialGlass = Special[playerIndex];

	XMMATRIX WorldMatrix =
		XMMatrixScaling(specialGlass.scaling.x, specialGlass.scaling.y, specialGlass.scaling.z) *
		XMMatrixRotationRollPitchYaw(XMConvertToRadians(specialGlass.rotation.x), XMConvertToRadians(specialGlass.rotation.y), XMConvertToRadians(specialGlass.rotation.z)) *
		XMMatrixTranslation(specialGlass.position.x, specialGlass.position.y, specialGlass.position.z);

	XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(WVP);

	// 描画実行
	g_pContext->DrawIndexed(6 * 6, 0, 0);
}

// Concrete専用描画
void Special_Concrete_Draw(int playerIndex)
{
	// Concrete専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Special_Texture[1];
	g_pContext->PSSetShaderResources(0, 1, &tex);

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
	// Plant専用のテクスチャをセット
	ID3D11ShaderResourceView* tex = g_Special_Texture[1];
	g_pContext->PSSetShaderResources(0, 1, &tex);

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
	ID3D11ShaderResourceView* tex = g_Special_Texture[1];
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// Electric用の座標計算
	SPECIAL_OBJECT& specialElectric = Special[playerIndex];

	XMMATRIX WorldMatrix =
		XMMatrixScaling(specialElectric.scaling.x, specialElectric.scaling.y, specialElectric.scaling.z) *
		XMMatrixRotationRollPitchYaw(XMConvertToRadians(specialElectric.rotation.x), XMConvertToRadians(specialElectric.rotation.y), XMConvertToRadians(specialElectric.rotation.z)) *
		XMMatrixTranslation(specialElectric.position.x, specialElectric.position.y, specialElectric.position.z);

	XMMATRIX WVP = WorldMatrix * GetViewMatrix() * GetProjectionMatrix();
	Shader_SetMatrix(WVP);

	// 描画実行
	g_pContext->DrawIndexed(6 * 6, 0, 0);

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