//======================================================
//	debug_render.cpp[]
// 
//	制作者：前野翼			日付：2025//
//======================================================
#include "debug_render.h"
#include "direct3d.h"
#include "shader.h"


#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

using namespace DirectX;
//======================================================
//	グローバル変数
//======================================================
static ID3D11Device* g_pDevice = NULL;
static ID3D11DeviceContext* g_pContext = NULL;
static ID3D11Buffer* g_pVertexBuffer = NULL;

//======================================================
//	関数
//======================================================
// 初期化
void Debug_Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	g_pDevice = device;
	g_pContext = context;

	// ライン描画用の動的頂点バッファを作成（少し多めに確保）
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(DebugVertex) * 100; // 一度に描画する最大頂点数
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	g_pDevice->CreateBuffer(&bd, NULL, &g_pVertexBuffer);
}

// 終了処理
void Debug_Finalize()
{
	if (g_pVertexBuffer) {
		g_pVertexBuffer->Release();
		g_pVertexBuffer = NULL;
	}
}

// 汎用ライン描画関数（内部使用）
void DrawLines(DebugVertex* vertices, int vertexCount)
{
	if (!g_pVertexBuffer) return;

	// バッファにデータを書き込む
	D3D11_MAPPED_SUBRESOURCE msr;
	if (SUCCEEDED(g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr)))
	{
		CopyMemory(msr.pData, vertices, sizeof(DebugVertex) * vertexCount);
		g_pContext->Unmap(g_pVertexBuffer, 0);
	}

	// テクスチャを使わないデバッグ専用シェーダーをセットする
	Shader_BeginDebugColor();

	// パイプライン設定
	UINT stride = sizeof(DebugVertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// トポロジーを「ラインリスト」に変更
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// シェーダーの設定（既存のテクスチャなしシェーダーがあればそれを使うのがベスト）
	// ここではすでにShader_Begin()されている前提で、白テクスチャなどをバインドしておくと良いです

	// 描画
	g_pContext->Draw(vertexCount, 0);

	// トポロジーを「トライアングルリスト」に戻しておく（他の描画への影響を防ぐ）
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Shader_Begin(); // メインシェーダーに戻す
}

// AABB描画
void Debug_DrawAABB(const AABB& box, XMFLOAT4 color)
{
	// 直方体のワイヤーフレームは12本の線、24頂点必要
	DebugVertex v[24];

	XMFLOAT3 corners[8] = {
		{ box.Min.x, box.Min.y, box.Min.z }, // 0: 手前・左・下
		{ box.Max.x, box.Min.y, box.Min.z }, // 1: 手前・右・下
		{ box.Min.x, box.Max.y, box.Min.z }, // 2: 手前・左・上
		{ box.Max.x, box.Max.y, box.Min.z }, // 3: 手前・右・上
		{ box.Min.x, box.Min.y, box.Max.z }, // 4: 奥・左・下
		{ box.Max.x, box.Min.y, box.Max.z }, // 5: 奥・右・下
		{ box.Min.x, box.Max.y, box.Max.z }, // 6: 奥・左・上
		{ box.Max.x, box.Max.y, box.Max.z }  // 7: 奥・右・上
	};

	// インデックスを使わず愚直に線を定義
	int idx = 0;
	auto AddLine = [&](int a, int b) {
		v[idx].pos = corners[a]; v[idx].color = color; v[idx].uv = { 0,0 }; idx++;
		v[idx].pos = corners[b]; v[idx].color = color; v[idx].uv = { 0,0 }; idx++;
		};

	// 手前の四角
	AddLine(0, 1); AddLine(1, 3); AddLine(3, 2); AddLine(2, 0);
	// 奥の四角
	AddLine(4, 5); AddLine(5, 7); AddLine(7, 6); AddLine(6, 4);
	// 繋ぐ線
	AddLine(0, 4); AddLine(1, 5); AddLine(2, 6); AddLine(3, 7);

	DrawLines(v, 24);
}

// 六角柱描画
void Debug_DrawHex(const HexCollider& hex, XMFLOAT4 color)
{
	// 上面と底面の頂点計算
	// Flat Topped (頂点がX軸方向) か Pointy Topped (頂点がZ軸方向) かに合わせて調整が必要
	// field.cppのロジックに合わせて Flat Topped で計算します

	const int segments = 6;
	DebugVertex v[36]; // 上面6本 + 底面6本 + 柱6本 = 18本 * 2頂点 = 36

	float halfH = hex.height / 2.0f;
	float topY = hex.center.y + halfH;
	float bottomY = hex.center.y - halfH;

	int idx = 0;

	for (int i = 0; i < segments; i++)
	{
		//  360 を $6 セグメントで割った 60度 ごとに角度を計算
		float radd1 = XMConvertToRadians(i * 60.0f);
		float radd2 = XMConvertToRadians((i + 1) * 60.0f);

		// 頂点座標計算 (Flat Toppedなら30度ずらす等の調整が必要だが、一旦標準的な回転で)
		// field.cppの生成ロジックとズレがある場合は +30.0f などして

		// radd1 の座標
		float x1 = cos(radd1) * hex.radius;
		float z1 = sin(radd1) * hex.radius;

		// radd2 の座標
		float x2 = cos(radd2) * hex.radius;
		float z2 = sin(radd2) * hex.radius;

		// 上面の線
		v[idx].pos = { hex.center.x + x1, topY, hex.center.z + z1 };	v[idx].color = color; v[idx].uv = { 0,0 }; idx++;
		v[idx].pos = { hex.center.x + x2, topY, hex.center.z + z2 };	v[idx].color = color; v[idx].uv = { 0,0 }; idx++;

		// 底面の線
		v[idx].pos = { hex.center.x + x1, bottomY, hex.center.z + z1 };	v[idx].color = color; v[idx].uv = { 0,0 }; idx++;
		v[idx].pos = { hex.center.x + x2, bottomY, hex.center.z + z2 };	v[idx].color = color; v[idx].uv = { 0,0 }; idx++;

		// 縦の柱
		v[idx].pos = { hex.center.x + x1, topY, hex.center.z + z1 };	v[idx].color = color; v[idx].uv = { 0,0 }; idx++;
		v[idx].pos = { hex.center.x + x1, bottomY, hex.center.z + z1 };	v[idx].color = color; v[idx].uv = { 0,0 }; idx++;
	}

	DrawLines(v, 36);
}