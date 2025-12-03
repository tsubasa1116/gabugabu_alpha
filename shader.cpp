/*==============================================================================

   シェーダー [shader.cpp]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "debug_ostream.h"
#include <fstream>

//======================================================
//	グローバル変数
//======================================================
static ID3D11VertexShader* g_pVertexShader = nullptr;//頂点シェーダー
static ID3D11InputLayout* g_pInputLayout = nullptr;//頂点レイアウト
static ID3D11Buffer* g_pVSConstantBuffer = nullptr;//定数バッファ1個
static ID3D11PixelShader* g_pPixelShader = nullptr;//ピクセルシェーダー

static ID3D11Buffer* g_pLightConstantBuffer = nullptr;//定数バッファ1個
static ID3D11Buffer* g_pWorldConstantBuffer = nullptr;//定数バッファ1個

static ID3D11PixelShader* g_pGaugeShader = nullptr;
static ID3D11PixelShader* g_pOutGaugeShader = nullptr;

static ID3D11Buffer* g_pGaugeBuffer = nullptr;
static ID3D11Buffer* g_pOutGaugeBuffer = nullptr;
static ID3D11Buffer* g_pColorBuffer = nullptr;

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

struct GAUGEBUFFER
{
	float gaugeValue;
	float pad[3];
	XMFLOAT4 gaugeColor;

	float fire;
	float water;
	float wind;
	float earth;

	XMFLOAT4 fireColor;
	XMFLOAT4 waterColor;
	XMFLOAT4 windColor;
	XMFLOAT4 earthColor;
};

struct OUTGAUGEBUFFER
{
	float gaugeValue;
	float pad[3];
	XMFLOAT4 gaugeColor;
};

struct COLORBUFFER
{
	XMFLOAT4 setColor;
};

//======================================================
//	初期化関数
//======================================================
bool Shader_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	HRESULT hr; // 戻り値格納用

	// デバイスとデバイスコンテキストのチェック
	if (!pDevice || !pContext) {
		hal::dout << "Shader_Initialize() : 与えられたデバイスかコンテキストが不正です" << std::endl;
		return false;
	}

	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	//GaugeBuffer
	{
		D3D11_BUFFER_DESC desc{};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(GAUGEBUFFER);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&desc, nullptr, &g_pGaugeBuffer);
	}

	//OutGaugeBuffer
	{
		D3D11_BUFFER_DESC desc{};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(OUTGAUGEBUFFER);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&desc, nullptr, &g_pOutGaugeBuffer);
	}

	//ColorBuffer
	{
		D3D11_BUFFER_DESC cbd{};
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.ByteWidth = sizeof(COLORBUFFER);
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&cbd, NULL, &g_pColorBuffer);
	}


	// 事前コンパイル済み頂点シェーダーの読み込み
	//csoはhlslファイルの実行形式ファイル
	std::ifstream ifs_vs("shader_vertex_2d.cso", std::ios::binary);

	if (!ifs_vs) {
		MessageBox(nullptr, "頂点シェーダーの読み込みに失敗しました\n\nshader_vertex_2d.cso", "エラー", MB_OK);
		return false;
	}

	// ファイルサイズを取得
	ifs_vs.seekg(0, std::ios::end); // ファイルポインタを末尾に移動
	std::streamsize filesize = ifs_vs.tellg(); // ファイルポインタの位置を取得（つまりファイルサイズ）
	ifs_vs.seekg(0, std::ios::beg); // ファイルポインタを先頭に戻す

	// バイナリデータを格納するためのバッファを確保
	unsigned char* vsbinary_pointer = new unsigned char[filesize];
	
	ifs_vs.read((char*)vsbinary_pointer, filesize); // バイナリデータを読み込む
	ifs_vs.close(); // ファイルを閉じる

	// 頂点シェーダーの作成
	hr = g_pDevice->CreateVertexShader(vsbinary_pointer, filesize, nullptr, &g_pVertexShader);

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点シェーダーの作成に失敗しました" << std::endl;
		delete[] vsbinary_pointer; // メモリリークしないようにバイナリデータのバッファを解放
		return false;
	}


	// 頂点レイアウトの定義<<<<<<<NORMAL追加
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT num_elements = ARRAYSIZE(layout); // 配列の要素数を取得

	// 頂点レイアウトの作成
	hr = g_pDevice->CreateInputLayout(layout, num_elements, vsbinary_pointer, filesize, &g_pInputLayout);

	delete[] vsbinary_pointer; // バイナリデータのバッファを解放

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点レイアウトの作成に失敗しました" << std::endl;
		return false;
	}


	// 頂点シェーダー用定数バッファの作成
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(XMFLOAT4X4); // バッファのサイズ //<<<<<<<<<<XMFLOAT4X4に変更
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // バインドフラグ

	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer);
	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer);

	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pWorldConstantBuffer);
	g_pContext->VSSetConstantBuffers(1, 1, &g_pWorldConstantBuffer);

	buffer_desc.ByteWidth = sizeof(LIGHT);//バッファサイズ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pLightConstantBuffer);
	g_pContext->VSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);




	// 事前コンパイル済みピクセルシェーダーの読み込み
	std::ifstream ifs_ps("shader_pixel_2d.cso", std::ios::binary);
	if (!ifs_ps) {
		MessageBox(nullptr, "ピクセルシェーダーの読み込みに失敗しました\n\nshader_pixel_2d.cso", "エラー", MB_OK);
		return false;
	}

	ifs_ps.seekg(0, std::ios::end);
	filesize = ifs_ps.tellg();
	ifs_ps.seekg(0, std::ios::beg);

	unsigned char* psbinary_pointer = new unsigned char[filesize];
	ifs_ps.read((char*)psbinary_pointer, filesize);
	ifs_ps.close();

	// ピクセルシェーダーの作成
	hr = g_pDevice->CreatePixelShader(psbinary_pointer, filesize, nullptr, &g_pPixelShader);

	delete[] psbinary_pointer; // バイナリデータのバッファを解放

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : ピクセルシェーダーの作成に失敗しました" << std::endl;
		return false;
	}

	//----------------------------------------------------------
   // ピクセルシェーダー読み込み
   //----------------------------------------------------------
	std::ifstream ifs_ps_g("shader_gauge.cso", std::ios::binary);
	if (!ifs_ps_g) return false;

	ifs_ps_g.seekg(0, std::ios::end);
	size_t psSize_g = (size_t)ifs_ps_g.tellg();
	ifs_ps_g.seekg(0, std::ios::beg);

	std::vector<unsigned char> psBin_g(psSize_g);
	ifs_ps_g.read((char*)psBin_g.data(), psSize_g);

	g_pDevice->CreatePixelShader(psBin_g.data(), psSize_g, nullptr, &g_pGaugeShader);


	//----------------------------------------------------------
   // ピクセルシェーダー読み込み
   //----------------------------------------------------------
	std::ifstream ifs_ps_og("shader_outgauge.cso", std::ios::binary);
	if (!ifs_ps_og) return false;

	ifs_ps_og.seekg(0, std::ios::end);
	size_t psSize_og = (size_t)ifs_ps_og.tellg();
	ifs_ps_og.seekg(0, std::ios::beg);

	std::vector<unsigned char> psBin_og(psSize_og);
	ifs_ps_og.read((char*)psBin_og.data(), psSize_og);

	g_pDevice->CreatePixelShader(psBin_og.data(), psSize_og, nullptr, &g_pOutGaugeShader);




	return true;
}


//======================================================
//	終了処理関数
//======================================================
void Shader_Finalize()
{
	SAFE_RELEASE(g_pPixelShader);
	SAFE_RELEASE(g_pVSConstantBuffer);
	SAFE_RELEASE(g_pInputLayout);
	SAFE_RELEASE(g_pVertexShader);

	SAFE_RELEASE(g_pWorldConstantBuffer);
	SAFE_RELEASE(g_pLightConstantBuffer);

}


//======================================================
//	行列関数
//======================================================
void Shader_SetMatrix(const DirectX::XMMATRIX& matrix)
{
	// 定数バッファ格納用行列の構造体を定義
	XMFLOAT4X4 transpose;

	// 行列を転置して定数バッファ格納用行列に変換
	XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));

	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pVSConstantBuffer, 0, nullptr, &transpose, 0, 0);
}


//======================================================
//	ワールド行列関数
//======================================================
void Shader_SetWorldMatrix(const DirectX::XMMATRIX& matrix)
{
	// 定数バッファ格納用行列の構造体を定義
	XMFLOAT4X4 transpose;

	// 行列を転置して定数バッファ格納用行列に変換
	XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));

	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pWorldConstantBuffer, 0, nullptr, &transpose, 0, 0);
}


//======================================================
//	セットライト関数
//======================================================
void Shader_SetLight(LIGHT light)
{
	// 定数バッファにLIGHT構造体をセット
	g_pContext->UpdateSubresource(g_pLightConstantBuffer, 0, nullptr, &light, 0, 0);
}



//======================================================
//	シェーダー設定
//======================================================
void Shader_Begin()
{
	// 頂点シェーダーとピクセルシェーダーを描画パイプラインに設定
	g_pContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pContext->PSSetShader(g_pPixelShader, nullptr, 0);

	// 頂点レイアウトを描画パイプラインに設定
	g_pContext->IASetInputLayout(g_pInputLayout);

	// 定数バッファを描画パイプラインに設定
	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer);
}


//======================================================
//	UI用シェーダー設定
//======================================================
void Shader_BeginUI()
{
	float w = (float)Direct3D_GetBackBufferWidth();
	float h = (float)Direct3D_GetBackBufferHeight();

	// 正射影行列の計算（2D）
	XMMATRIX UI = XMMatrixOrthographicOffCenterLH(0, w, h, 0, 0, 1);

	Shader_SetMatrix(UI);
}


//======================================================
//	内ゲージ用シェーダー設定●
//======================================================
void Shader_BeginGauge()
{
	g_pContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pContext->PSSetShader(g_pGaugeShader, nullptr, 0);

	g_pContext->IASetInputLayout(g_pInputLayout);
	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer);
}


//======================================================
//	外ゲージ用シェーダー設定〇
//======================================================
void Shader_BeginOutGauge()
{
	g_pContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pContext->PSSetShader(g_pOutGaugeShader, nullptr, 0);

	g_pContext->IASetInputLayout(g_pInputLayout);
	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer);
}


//======================================================
//	内ゲージ
//======================================================
void Shader_SetGaugeMulti(float fire, float water, float wind, float earth,
	XMFLOAT4 fireColor, XMFLOAT4 waterColor,
	XMFLOAT4 windColor, XMFLOAT4 earthColor)
{
	D3D11_MAPPED_SUBRESOURCE mapped{};
	g_pContext->Map(g_pGaugeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	struct GAUGEBUFFER
	{
		float fire;
		float water;
		float wind;
		float earth;

		XMFLOAT4 fireColor;
		XMFLOAT4 waterColor;
		XMFLOAT4 windColor;
		XMFLOAT4 earthColor;
	};

	GAUGEBUFFER gb{};
	gb.fire = fire;
	gb.water = water;
	gb.wind = wind;
	gb.earth = earth;

	gb.fireColor = fireColor;
	gb.waterColor = waterColor;
	gb.windColor = windColor;
	gb.earthColor = earthColor;

	memcpy(mapped.pData, &gb, sizeof(gb));
	g_pContext->Unmap(g_pGaugeBuffer, 0);

	g_pContext->PSSetConstantBuffers(3, 1, &g_pGaugeBuffer);
}


//======================================================
//	外ゲージ
//======================================================
void Shader_SetOutGauge(float value, XMFLOAT4 color)
{
	D3D11_MAPPED_SUBRESOURCE mapped{};
	g_pContext->Map(g_pOutGaugeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	OUTGAUGEBUFFER ob{};
	ob.gaugeValue = value;
	ob.pad[0] = ob.pad[1] = ob.pad[2] = 0.0f;
	ob.gaugeColor = color;

	memcpy(mapped.pData, &ob, sizeof(ob));
	g_pContext->Unmap(g_pOutGaugeBuffer, 0);

	g_pContext->PSSetConstantBuffers(4, 1, &g_pOutGaugeBuffer);
}


//======================================================
//	色設定
//======================================================
void Shader_SetColor(const XMFLOAT4& color)
{
	if (!g_pColorBuffer) return;

	D3D11_MAPPED_SUBRESOURCE mapped{};
	g_pContext->Map(g_pColorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	COLORBUFFER cb{};
	cb.setColor = color;
	memcpy(mapped.pData, &cb, sizeof(cb));

	g_pContext->Unmap(g_pColorBuffer, 0);

	// register(b1)に送る
	g_pContext->PSSetConstantBuffers(1, 1, &g_pColorBuffer);
}