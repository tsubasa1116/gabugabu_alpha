/*==============================================================================

   ポリゴン描画 [p.cpp]
--------------------------------------------------------------------------------

==============================================================================*/
#include "p.h"
#include "keyboard.h"
#include "polygon.h"
#include "shader.h"
#include "color.h"
#include "sprite.h"
#include "hp.h"
#include "imgui.h"
#include "gauge.h"

#define PLAYER_MAX 4
#define HPBER_SIZE_X 180.0f
#define HPBER_SIZE_Y 30.0f

static HP g_HPBar[PLAYER_MAX];

static ID3D11ShaderResourceView* g_Texture = NULL;

static constexpr int NUM_VERTEX = 24; // 使用できる最大頂点数

static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
static ID3D11Buffer* g_pIndexBuffer = nullptr; // 頂点バッファ
static ID3D11Buffer* g_pCubeVertexBuffer = nullptr;

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
static ID3D11DeviceContext* g_pContext2 = nullptr;

P p[PLAYER_MAX];

XMFLOAT3 g_Eye = { 0.0f, 10.0f, 15.0f }; // カメラ位置
XMFLOAT3 g_At = { 0.0f, 0.0f, 0.0f }; // 注視点
XMFLOAT3 g_Up = { 0.0f, 1.0f, 0.0f }; // 上方向
// カメラとの距離
XMFLOAT3 g_CamOffset = { 0.0f, 10.0f, 15.0f };

// 8頂点
Vertex cubeVertices[] =
{
	{{-1,-1,-1}, {1,1,1,1}, {0,1}}, // 0: 左下手前
	{{-1, 1,-1}, {1,1,1,1}, {0,0}}, // 1: 左上手前
	{{ 1, 1,-1}, {1,1,1,1}, {1,0}}, // 2: 右上手前
	{{ 1,-1,-1}, {1,1,1,1}, {1,1}}, // 3: 右下手前
	{{-1,-1, 1}, {1,1,1,1}, {0,1}}, // 4: 左下奥
	{{-1, 1, 1}, {1,1,1,1}, {0,0}}, // 5: 左上奥
	{{ 1, 1, 1}, {1,1,1,1}, {1,0}}, // 6: 右上奥
	{{ 1,-1, 1}, {1,1,1,1}, {1,1}}, // 7: 右下奥
};

// 36インデックス（12三角形）
unsigned short cubeIndices[] =
{
	0,1,2, 0,2,3,   // 前
	4,6,5, 4,7,6,   // 後
	4,5,1, 4,1,0,   // 左
	3,2,6, 3,6,7,   // 右
	1,5,6, 1,6,2,   // 上
	4,0,3, 4,3,7    // 下
};

static bool debug = false;

//====================================================================================
// 初期化
//====================================================================================
void P_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// デバイスとデバイスコンテキストのチェック
	if (!pDevice || !pContext)
	{
		hal::dout << "P_Initialize() : 与えられたデバイスかコンテキストが不正です" << std::endl;
		return;
	}

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		p[i].HP = 10.0f;
		p[i].MaxHP = 10.0f;
		p[i].pos = { 0.0f, 1.0f, 0.0f };
		p[i].scale = { 1.0f, 1.0f, 1.0f };
		p[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };

		p[i].isIllum = false;
		p[i].illumTimer = 0.0f;
		p[i].illumTotal = 0.0f;
		p[i].illumInterval = 0.0f;
		p[i].illumInTimer = 0.0f;
		p[i].illumColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		p[i].isStar = false;
		p[i].starTimer = 0.0f;

		p[i].fi = 1.0f;
		p[i].wa = 1.0f;
		p[i].wi = 1.0f;
		p[i].ea = 1.0f;
		p[i].gaugeOuter = 1.0f;

		p[i].active = true;
	}


	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * NUM_VERTEX;// バッファのサイズ　構造体×頂点数
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	g_pDevice->CreateBuffer(&bd, NULL, &g_pVertexBuffer);

	// 頂点バッファ(3D)
	D3D11_BUFFER_DESC vbd = {};
	vbd.ByteWidth = sizeof(cubeVertices);      // 8頂点ぶん
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA initV = { cubeVertices };
	g_pDevice->CreateBuffer(&vbd, &initV, &g_pCubeVertexBuffer);

	// インデックスバッファ
	D3D11_BUFFER_DESC ibd = {};
	ibd.ByteWidth = sizeof(cubeIndices);
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA initI = { cubeIndices };
	g_pDevice->CreateBuffer(&ibd, &initI, &g_pIndexBuffer);

	// テクスチャ画像読み込み
	{
		TexMetadata  metadata;
		ScratchImage image;
		LoadFromWICFile(L"asset\\texture\\runningman003.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
		assert(g_Texture);// 読み込み失敗時にダイアログを表示
	}

	// HPバー初期化
	InitializeHP(pDevice, pContext, &g_HPBar[0], { 200.0f, 650.0f }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::red, color::green);
	InitializeHP(pDevice, pContext, &g_HPBar[1], { 500.0f, 650.0f }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::red, color::green);
	InitializeHP(pDevice, pContext, &g_HPBar[2], { 800.0f, 650.0f }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::red, color::green);
	InitializeHP(pDevice, pContext, &g_HPBar[3], { 1100.0f, 650.0f },{ HPBER_SIZE_X, HPBER_SIZE_Y }, color::red, color::green);

}



//====================================================================================
// 終了
//====================================================================================
void P_Finalize(void)
{
	//FinalizeHP(&g_HPBar);

	SAFE_RELEASE(g_pVertexBuffer);

	//g_Texture->Release();
}


//====================================================================================
// 描画
//====================================================================================
void P_Update()
{
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		const float moveSpeed = 0.1f;

		// プレイヤーの移動
		if (Keyboard_IsKeyDown(KK_W)) p[i].pos.z -= moveSpeed;
		if (Keyboard_IsKeyDown(KK_S)) p[i].pos.z += moveSpeed;
		if (Keyboard_IsKeyDown(KK_A)) p[i].pos.x += moveSpeed;
		if (Keyboard_IsKeyDown(KK_D)) p[i].pos.x -= moveSpeed;

		// カメラをプレイヤーの位置に追従
		g_Eye.x = p[i].pos.x + g_CamOffset.x;
		g_Eye.y = p[i].pos.y + g_CamOffset.y;
		g_Eye.z = p[i].pos.z + g_CamOffset.z;

		// カメラが見る注視点はプレイヤー自身
		g_At = p[i].pos;

		// 当たり判定取得
		Player_UpdateAABB();

		SetHPValue(&g_HPBar[i], (int)p[i].HP, (int)p[i].MaxHP);
		UpdateHP(&g_HPBar[i]);

	}

	//==================== ImGui デバッグ表示 ====================//
	{
		ImGui::Begin("P Debug");

		// HP
		ImGui::SliderInt("1p:HP", &p[0].HP, 0.0f, p[0].MaxHP);
		ImGui::SliderInt("2p:HP", &p[1].HP, 0.0f, p[1].MaxHP);
		ImGui::SliderInt("3p:HP", &p[2].HP, 0.0f, p[2].MaxHP);
		ImGui::SliderInt("4p:HP", &p[3].HP, 0.0f, p[3].MaxHP);

		if (ImGui::Button("fi +1"))
		{
			p[0].fi += 0.1f;
		}
		else if (ImGui::Button("wa +1"))
		{
			p[0].fi += 0.1f;
		}
		else if (ImGui::Button("wi +1"))
		{
			p[0].wi += 0.1f;
		}
		else if (ImGui::Button("ea +1"))
		{
			p[0].ea += 0.1f;
		}

		ImGui::SliderFloat("Outer", &p[0].gaugeOuter, 0.0f, 1.0f);

		ImGui::End();
	}
	//============================================================//

}


//====================================================================================
// 描画
//====================================================================================
void P_Draw(void)
{
	// シェーダーを描画パイプラインに設定
	Shader_Begin();

	// 個別UIステータス描画
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		// HPバー描画
		DrawHP(&g_HPBar[i]);
		XMFLOAT2 hp = g_HPBar[i].pos;

		// ゲージ描画用設定
		Gauge_Set(i, p[i].fi, p[i].wa, p[i].wi, p[i].ea,
			      p[i].gaugeOuter, { hp.x - 100.0f , hp.y});

		// ゲージ描画
		Gauge_Draw(i);

		// シェーダーリセット
		Shader_Begin();
	}

	float w = (float)Direct3D_GetBackBufferWidth();
	float h = (float)Direct3D_GetBackBufferHeight();

	XMVECTOR Eye = XMVectorSet(g_Eye.x, g_Eye.y, g_Eye.z, 1.0f);
	XMVECTOR At = XMVectorSet(g_At.x, g_At.y, g_At.z, 1.0f);
	XMVECTOR Up = XMVectorSet(g_Up.x, g_Up.y, g_Up.z, 0.0f);

	// 頂点バッファをロックする
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// 頂点バッファへの仮想ポインタを取得
	Vertex* v = (Vertex*)msr.pData;

	// 頂点バッファのロックを解除
	g_pContext->Unmap(g_pVertexBuffer, 0);

	{// プレイヤー
		for (int i = 0; i < PLAYER_MAX; i++)
		{
			// 頂点バッファを描画パイプラインに設定
			UINT stride = sizeof(Vertex);
			UINT offset = 0;
			g_pContext->IASetVertexBuffers(0, 1, &g_pCubeVertexBuffer, &stride, &offset);
			g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

			XMMATRIX view = XMMatrixLookAtLH(Eye, At, Up);

			// 透視投影
			XMMATRIX proj = XMMatrixPerspectiveFovLH(
				XMConvertToRadians(45.0f), w / h, 0.1f, 100.0f);

			// オブジェクトのワールド変換
			static float angle = 0.0f;
			angle += 0.01f;
			XMMATRIX world =
				/*XMMatrixRotationY(angle) **/
				XMMatrixTranslation(p[i].pos.x, p[i].pos.y, p[i].pos.z);

			// 最終的な行列
			XMMATRIX wvp = world * view * proj;

			// 定数バッファに送信
			Shader_SetMatrix(wvp);

			//Shader_SetColor(p.color);

			// 三角形リストで描画
			g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			g_pContext->PSSetShaderResources(0, 1, &g_Texture);

			// 描画命令
			g_pContext->DrawIndexed(36, 0, 0);
			//g_pContext->Draw(NUM_VERTEX, 0);

			// 頂点シェーダーに変換行列を設定
			//Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));
		}

	}
   
	g_pContext->PSSetShaderResources(0, 1, &g_Texture);

}

//====================================================================================
// プレイヤー判定
//====================================================================================
void Player_UpdateAABB()
{
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		float sx = p[i].scale.x;
		float sy = p[i].scale.y;
		float sz = p[i].scale.z;
		float px = p[i].pos.x;
		float py = p[i].pos.y;
		float pz = p[i].pos.z;


		p[i].box.Min = { px - sx * COLL, py - sy * COLL, pz - sz * COLL };
		p[i].box.Max = { px + sx * COLL, py + sy * COLL, pz + sz * COLL };
	}
}


P* GetP() 
{
	for (int i = 0; i < PLAYER_MAX; i++) return &p[i]; 
}