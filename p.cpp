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

static HP g_HPBar;

static ID3D11ShaderResourceView* g_Texture = NULL;

static constexpr int NUM_VERTEX = 24; // 使用できる最大頂点数

static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
static ID3D11Buffer* g_pIndexBuffer = nullptr; // 頂点バッファ
static ID3D11Buffer* g_pCubeVertexBuffer = nullptr;

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
static ID3D11DeviceContext* g_pContext2 = nullptr;

P p;


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

	p.HP = 10.0f;
	p.MaxHP = 10.0f;
	p.pos = { 0.0f, 1.0f, 0.0f };
	p.scale = { 1.0f, 1.0f, 1.0f };
	p.color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白

	p.isIllum = false;
	p.illumTimer = 0.0f;
	p.illumTotal = 0.0f;
	p.illumInterval = 0.0f;
	p.illumInTimer = 0.0f;
	p.illumColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白

	p.isStar = false;
	p.starTimer = 0.0f;

	p.active = true;

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

	InitializeHP(pDevice, pContext, &g_HPBar, { 250.0f, 50.0f }, { 400.0f, 20.0f },color::red, color::green);

	

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
// 色制御
//====================================================================================
void P_Color(const XMFLOAT4& color, bool illum)
{
	if (!p.active) return;

	if (illum)
	{
		p.isIllum = true;
		p.illumTimer = 0.0f;
	}
	else
	{
		p.color = color;
		p.isIllum = false;
	}
}



//====================================================================================
// 点滅呼び出し関数
//====================================================================================
void P_StartIllum(const XMFLOAT4& color, float total, float interval)
{
	p.isIllum = true;
	p.illumTimer = 0.0f;
	p.illumInTimer = 0.0f;

	p.illumTotal = total;
	p.illumInterval = interval;

	p.illumColor = color;
}



//====================================================================================
// 点滅関数
//====================================================================================
void P_UpdateIllum()
{
	if (!p.isIllum) return;

	// 全体時間
	p.illumTimer += 1.0f / 60.0f;

	// 点滅間隔
	p.illumInTimer += 1.0f / 60.0f;

	// 一定間隔で色を反転
	if (p.illumInTimer >= p.illumInterval)
	{
		p.illumInTimer = 0.0f;

		// 白-点滅色 の切り替え
		if (p.color.x == p.illumColor.x &&
			p.color.y == p.illumColor.y &&
			p.color.z == p.illumColor.z)
		{
			p.color = color::white;
		}
		else
		{
			p.color = p.illumColor;
		}
	}

	// 点滅終了
	if (p.illumTimer >= p.illumTotal)
	{
		p.isIllum = false;
		p.color = color::white;
	}
}




//====================================================================================
// 無敵呼び出し関数
//====================================================================================
void P_StartStar(float time)
{
	p.isStar = true;
	p.starTimer = time;

	// 無敵中は赤点滅など入れるなら
	P_StartIllum(color::red, time, 0.2f);
}



//====================================================================================
// 描画
//====================================================================================
void P_Update()
{

	const float moveSpeed = 0.1f;

	// プレイヤーの移動
	if (Keyboard_IsKeyDown(KK_W)) p.pos.z -= moveSpeed;
	if (Keyboard_IsKeyDown(KK_S)) p.pos.z += moveSpeed;
	if (Keyboard_IsKeyDown(KK_A)) p.pos.x += moveSpeed;
	if (Keyboard_IsKeyDown(KK_D)) p.pos.x -= moveSpeed;

	// カメラをプレイヤーの位置に追従
	g_Eye.x = p.pos.x + g_CamOffset.x;
	g_Eye.y = p.pos.y + g_CamOffset.y;
	g_Eye.z = p.pos.z + g_CamOffset.z;

	// カメラが見る注視点はプレイヤー自身
	g_At = p.pos;

	// 当たり判定取得
	Player_UpdateAABB();

	// 点滅関数取得
	P_UpdateIllum();


	// HPバー
	SetHPValue(&g_HPBar, (int)p.HP, (int)p.MaxHP);
	UpdateHP(&g_HPBar);

	





	// 無敵判定
	if (p.isStar)
	{
		p.starTimer -= 1.0f / 60.0f;  // 1フレーム経過

		if (p.starTimer <= 0.0f)
		{
			p.isStar = false;
			p.starTimer = 0.0f;
		}
	}




	//===========================================================//

	ImGui::Begin("Debug Switch");
	ImGui::Checkbox("Debug Logic", &debug);
	ImGui::End();

	//==================== ImGui デバッグ表示 ====================//
	{
		ImGui::Begin("P Debug");

		// HP
		ImGui::SliderInt("HP", &p.HP, 0.0f, p.MaxHP);

		// 無敵情報
		ImGui::Separator();
		ImGui::Text("Invincible (Star Mode)");
		ImGui::Checkbox("isStar", &p.isStar);
		ImGui::SliderFloat("Star Timer", &p.starTimer, 0.0f, 5.0f);

		// 点滅情報
		ImGui::Separator();
		ImGui::Text("Illumination");
		ImGui::Checkbox("isIllum", &p.isIllum);
		ImGui::SliderFloat("illumTimer", &p.illumTimer, 0.0f, p.illumTotal);
		ImGui::SliderFloat("illumTotal", &p.illumTotal, 0.0f, 3.0f);
		ImGui::SliderFloat("illumInterval", &p.illumInterval, 0.0f, 1.0f);

		// 座標
		ImGui::Separator();
		ImGui::Text("Position");
		ImGui::DragFloat3("pos", (float*)&p.pos, 0.1f);

		// カラー
		ImGui::ColorEdit4("Color", (float*)&p.color);

		// 速度調整
		static float moveSpeedDebug = 0.1f;
		ImGui::SliderFloat("Move Speed", &moveSpeedDebug, 0.01f, 1.0f);

		// HP回復
		if (ImGui::Button("Heal +1"))
		{
			p.HP += 1.0f;
			if (p.HP > p.MaxHP) p.HP = p.MaxHP;
		}

		ImGui::End();
	}

	//==================== Enemy Debug ====================//

}


//====================================================================================
// 描画
//====================================================================================
void P_Draw(void)
{
	// シェーダーを描画パイプラインに設定
	Shader_Begin();

	DrawHP(&g_HPBar);

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
			XMMatrixTranslation(p.pos.x, p.pos.y, p.pos.z);

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
   
	g_pContext->PSSetShaderResources(0, 1, &g_Texture);

}

//====================================================================================
// プレイヤー判定
//====================================================================================
void Player_UpdateAABB()
{
	float sx = p.scale.x;
	float sy = p.scale.y;
	float sz = p.scale.z;
	float px = p.pos.x;
	float py = p.pos.y;
	float pz = p.pos.z;

	p.box.Min = { px - sx * COLL, py - sy * COLL, pz - sz * COLL };
	p.box.Max = { px + sx * COLL, py + sy * COLL, pz + sz * COLL };
}


P* GetP() { return &p; }