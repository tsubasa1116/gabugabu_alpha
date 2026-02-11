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
#include "imgui.h"

#include "color.h"

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
static ID3D11PixelShader* g_pSkillGaugeShader = nullptr;
static ID3D11PixelShader* g_pHpberShader = nullptr;

static ID3D11Buffer* g_pGaugeBuffer = nullptr;
static ID3D11Buffer* g_pOutGaugeBuffer = nullptr;
static ID3D11Buffer* g_pSkillGaugeBuffer = nullptr;
static ID3D11Buffer* g_pHpberBuffer = nullptr;
static ID3D11Buffer* g_pColorBuffer = nullptr;

static ID3D11PixelShader* g_pDebugColorShader = nullptr; // コライダー可視化

static ID3D11ShaderResourceView* g_GaugeTex[4] = {};
static ID3D11SamplerState* g_GaugeSampler = nullptr;

static ID3D11ShaderResourceView* g_OutGaugeTex = nullptr;
static ID3D11SamplerState* g_OutGaugeSampler = nullptr;

static ID3D11ShaderResourceView* g_SkillGaugeTex = nullptr;
static ID3D11SamplerState* g_SkillGaugeSampler = nullptr;


// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

struct GAUGEBUFFER
{
	float glass;
	float concrete;
	float plant;
	float electric;

};

struct OUTGAUGEBUFFER
{
	float gaugeValue;
	float pad[3];
	XMFLOAT4 gaugeColor;
};

struct SINGLEGAUGEBUFFER
{
	float fill;
	float pad[3];
};

struct COLORBUFFER
{
	XMFLOAT4 setColor;   // 乗算用カラー
	XMFLOAT4 lerpColor;  // 線形補間用カラー
	float lerpFactor;    // 補間係数
	float pad[3];
};

struct HPBERBUFFER
{
	XMFLOAT4 palam;
	XMFLOAT4 colorA;
	XMFLOAT4 colorB;
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

	//SkillGaugeBuffer
	{
		D3D11_BUFFER_DESC desc{};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(SINGLEGAUGEBUFFER);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&desc, nullptr, &g_pSkillGaugeBuffer);
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

	//HpberBuffer
	{
		D3D11_BUFFER_DESC cbd{};
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.ByteWidth = sizeof(HPBERBUFFER);
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&cbd, NULL, &g_pHpberBuffer);
	}


	// 事前コンパイル済み頂点シェーダーの読み込み
	//csoはhlslファイルの実行形式ファイル
	std::ifstream ifs_vs("vs_main.cso", std::ios::binary);

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
	std::ifstream ifs_ps("ps_main.cso", std::ios::binary);
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
	// 内ゲージシェーダー読み込み
	//----------------------------------------------------------
	std::ifstream ifs_ps_g("ps_gauge.cso", std::ios::binary);
	if (!ifs_ps_g) return false;

	ifs_ps_g.seekg(0, std::ios::end);
	size_t psSize_g = (size_t)ifs_ps_g.tellg();
	ifs_ps_g.seekg(0, std::ios::beg);

	std::vector<unsigned char> psBin_g(psSize_g);
	ifs_ps_g.read((char*)psBin_g.data(), psSize_g);

	g_pDevice->CreatePixelShader(psBin_g.data(), psSize_g, nullptr, &g_pGaugeShader);


	//----------------------------------------------------------
	// 外ゲージシェーダー読み込み
	//----------------------------------------------------------
	std::ifstream ifs_ps_og("ps_outgauge.cso", std::ios::binary);
	if (!ifs_ps_og) return false;

	ifs_ps_og.seekg(0, std::ios::end);
	size_t psSize_og = (size_t)ifs_ps_og.tellg();
	ifs_ps_og.seekg(0, std::ios::beg);

	std::vector<unsigned char> psBin_og(psSize_og);
	ifs_ps_og.read((char*)psBin_og.data(), psSize_og);

	g_pDevice->CreatePixelShader(psBin_og.data(), psSize_og, nullptr, &g_pOutGaugeShader);

	//----------------------------------------------------------
	// 単ゲージシェーダー読み込み
	//----------------------------------------------------------
	std::ifstream ifs_ps_sg("ps_singlegauge.cso", std::ios::binary);
	if (!ifs_ps_sg) return false;

	ifs_ps_sg.seekg(0, std::ios::end);
	size_t psSize_sg = (size_t)ifs_ps_sg.tellg();
	ifs_ps_sg.seekg(0, std::ios::beg);

	std::vector<unsigned char> psBin_sg(psSize_sg);
	ifs_ps_sg.read((char*)psBin_sg.data(), psSize_sg);

	g_pDevice->CreatePixelShader(psBin_sg.data(), psSize_sg, nullptr, &g_pSkillGaugeShader);



	//----------------------------------------------------------
	// デバッグ用カラーピクセルシェーダー読み込み
	//----------------------------------------------------------
	std::ifstream ifs_ps_dbg("ps_debug_color.cso", std::ios::binary);
	if (!ifs_ps_dbg) {
		hal::dout << "Shader_Initialize() : デバッグ用シェーダーの読み込みに失敗しました\n\nshader_pixel_debug_color.cso" << std::endl;
		return false;
	}

	ifs_ps_dbg.seekg(0, std::ios::end);
	size_t psSize_dbg = (size_t)ifs_ps_dbg.tellg();
	ifs_ps_dbg.seekg(0, std::ios::beg);

	std::vector<unsigned char> psBin_dbg(psSize_dbg);
	ifs_ps_dbg.read((char*)psBin_dbg.data(), psSize_dbg);

	g_pDevice->CreatePixelShader(psBin_dbg.data(), psSize_dbg, nullptr, &g_pDebugColorShader);

	//----------------------------------------------------------
	// ピクセルシェーダー読み込み
	//----------------------------------------------------------
	std::ifstream ifs_ps_hpber("ps_hpber.cso", std::ios::binary);
	if (!ifs_ps_hpber) return false;

	ifs_ps_hpber.seekg(0, std::ios::end);
	size_t psSize_hpber = (size_t)ifs_ps_hpber.tellg();
	ifs_ps_hpber.seekg(0, std::ios::beg);

	std::vector<unsigned char> psBin_hpber(psSize_hpber);
	ifs_ps_hpber.read((char*)psBin_hpber.data(), psSize_hpber);

	g_pDevice->CreatePixelShader(psBin_hpber.data(), psSize_hpber, nullptr, &g_pHpberShader);


	//======================================================
	//	ゲージ用テクスチャ読み込み
	//======================================================
	const wchar_t* files[4] =
	{
		L"asset/texture/uiMaterialGlass_v3.png",
		L"asset/texture/uiMaterialConcrete_v3.png",
		L"asset/texture/uiMaterialTree_v3.png",
		L"asset/texture/uiMaterialElectricity_v3.png"
	};

	TexMetadata metadata;
	ScratchImage image;

	for (int i = 0; i < 4; i++)
	{
		LoadFromWICFile(files[i], WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_GaugeTex[i]);
		assert(g_GaugeTex[i]);
	}

	LoadFromWICFile(L"Asset\\Texture\\uiEvolutionGauge_v3.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_OutGaugeTex);
	assert(g_OutGaugeTex);

	LoadFromWICFile(L"Asset\\Texture\\icon_barrier.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_SkillGaugeTex);
	assert(g_SkillGaugeTex);


	//======================================================
	//	ゲージ用サンプラーステート作成
	//======================================================
	D3D11_SAMPLER_DESC desc{};
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	pDevice->CreateSamplerState(&desc, &g_GaugeSampler);
	pDevice->CreateSamplerState(&desc, &g_SkillGaugeSampler);
	pDevice->CreateSamplerState(&desc, &g_OutGaugeSampler);


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
//	内ゲージ用テクスチャセット関数
//======================================================
void Shader_SetGaugeTextures()
{
	// t0-t3
	g_pContext->PSSetShaderResources(0, 4, g_GaugeTex);  // glass, concrete, plant, electric の順
	
	// s0
	g_pContext->PSSetSamplers(0, 1, &g_GaugeSampler);
}

//======================================================
//	外ゲージ用テクスチャセット関数
//======================================================
void Shader_SetOutGaugeTextures()
{
	// t0
	g_pContext->PSSetShaderResources(0, 1, &g_OutGaugeTex); 

	// s0
	g_pContext->PSSetSamplers(0, 1, &g_OutGaugeSampler);
}

//======================================================
//	スキルゲージ用テクスチャセット関数
//======================================================
void Shader_SetSkillGaugeTextures()
{
	// ✅ デバッグ出力
	if (!g_SkillGaugeTex) {
		hal::dout << "ERROR: g_SkillGaugeTex is nullptr!" << std::endl;
		return;
	}
	if (!g_SkillGaugeSampler) {
		hal::dout << "ERROR: g_SkillGaugeSampler is nullptr!" << std::endl;
		return;
	}

	hal::dout << "Setting SkillGauge texture and sampler" << std::endl;

	g_pContext->PSSetShaderResources(0, 1, &g_SkillGaugeTex);
	g_pContext->PSSetSamplers(0, 1, &g_SkillGaugeSampler);
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
//	単ゲージ用シェーダー設定
//======================================================
void Shader_BeginSkillGauge()
{
	g_pContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pContext->PSSetShader(g_pSkillGaugeShader, nullptr, 0);

	g_pContext->IASetInputLayout(g_pInputLayout);
	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer);
}

//======================================================
//	HPバー用シェーダー設定
//======================================================
void Shader_BeginHpber()
{
	g_pContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pContext->PSSetShader(g_pHpberShader, nullptr, 0);

	g_pContext->IASetInputLayout(g_pInputLayout);
	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer);
}


//======================================================
//	内ゲージ
//======================================================
void Shader_SetGaugeMulti(float glass, float concrete, float plant, float electric)
{
	D3D11_MAPPED_SUBRESOURCE mapped{};
	g_pContext->Map(g_pGaugeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	GAUGEBUFFER gb{};
	gb.glass = glass;
	gb.concrete = concrete;
	gb.plant = plant;
	gb.electric = electric;

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
//  単ゲージ
//======================================================
void Shader_SetSingleGauge(float fill)
{
	if (fill < 0.0f) fill = 0.0f;
	if (fill > 1.0f) fill = 1.0f;

	D3D11_MAPPED_SUBRESOURCE mapped{};
	g_pContext->Map(g_pSkillGaugeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	SINGLEGAUGEBUFFER sb{};
	sb.fill = fill;
	sb.pad[0] = sb.pad[1] = sb.pad[2] = 0.0f;

	memcpy(mapped.pData, &sb, sizeof(sb));
	g_pContext->Unmap(g_pSkillGaugeBuffer, 0);

	g_pContext->PSSetConstantBuffers(5, 1, &g_pSkillGaugeBuffer);
}



//======================================================
//	色設定
//======================================================
void Shader_SetHpber(XMFLOAT4 colA, XMFLOAT4 colB, float al, float speed)
{
	D3D11_MAPPED_SUBRESOURCE mapped{};
	g_pContext->Map(g_pHpberBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	static float g_Time = 0.0f;
	g_Time += 0.02f;

	HPBERBUFFER ob{};
	ob.palam = { g_Time, al, speed, 0 };
	ob.colorA = colA;
	ob.colorB = colB;

	memcpy(mapped.pData, &ob, sizeof(ob));
	g_pContext->Unmap(g_pHpberBuffer, 0);

	g_pContext->PSSetConstantBuffers(6, 1, &g_pHpberBuffer);
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

//======================================================
//	デバッグ用カラーシェーダー設定
//======================================================
void Shader_BeginDebugColor()
{
	// 頂点シェーダーとデバッグ用ピクセルシェーダーを描画パイプラインに設定
	g_pContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pContext->PSSetShader(g_pDebugColorShader, nullptr, 0); // ★デバッグ専用シェーダーをセット！

	// 頂点レイアウトを描画パイプラインに設定
	g_pContext->IASetInputLayout(g_pInputLayout);

	// 定数バッファを描画パイプラインに設定
	// WVP行列 (b0)
	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer);
	// World行列 (b1)
	g_pContext->VSSetConstantBuffers(1, 1, &g_pWorldConstantBuffer);
	// Light (b2)
	g_pContext->VSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

	// カラーバッファ (b1)
	// ※デバッグシェーダーが register(b1) の COLORBUFFER を参照する場合に必要
	g_pContext->PSSetConstantBuffers(1, 1, &g_pColorBuffer);

	Shader_SetColor(color::red);

}

// 線形補間カラー設定
void Shader_SetColorLerp(const XMFLOAT4& mulColor, const XMFLOAT4& lerpColor, float lerpFactor)
{
	if (!g_pColorBuffer) return;

	D3D11_MAPPED_SUBRESOURCE mapped{};
	g_pContext->Map(g_pColorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	COLORBUFFER cb{};
	cb.setColor = mulColor;
	cb.lerpColor = lerpColor;
	cb.lerpFactor = lerpFactor;
	memcpy(mapped.pData, &cb, sizeof(cb));

	g_pContext->Unmap(g_pColorBuffer, 0);
	g_pContext->PSSetConstantBuffers(1, 1, &g_pColorBuffer);
}