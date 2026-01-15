//======================================================
//	result.cpp[]
// 
//	制作者：田中佑奈			日付：2026//
//======================================================

//Result.cpp
#include	"Manager.h"
#include	"sprite.h"
#include	"keyboard.h"

#include	"Result.h"

#include "fade.h"
#include "swipe.h"
#include "shader.h"

#include "model.h"

#include <cfloat> // FLT_MAX
#include <array>

static	ID3D11ShaderResourceView* g_Texture = NULL;	//背景
static ID3D11ShaderResourceView* g_ResultTex = nullptr; // 既存モデル用テクスチャ
static TexMetadata		g_ResultTexMeta{};
static MODEL* g_ResultModel = nullptr;               // 既存モデル

// 新規：タワーモデルとテクスチャ群（4つ）
static MODEL* g_TowerModel = nullptr;
static ID3D11ShaderResourceView* g_TowerTex[4] = { nullptr, nullptr, nullptr, nullptr };
static TexMetadata g_TowerTexMeta[4]{};
static XMFLOAT3 g_TowerModelCenter = { 0.0f, 0.0f, 0.0f };
static float    g_TowerModelRadius = 1.0f;

static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// モデル中心・半径（Initializeで計算）
static XMFLOAT3 g_ResultModelCenter = { 0.0f, 0.0f, 0.0f };
static float    g_ResultModelRadius = 1.0f;

// ユーティリティ：モデルのバウンディング計算
static void CalculateModelBounds(MODEL* model, XMFLOAT3& outCenter, float& outRadius)
{
	if (model == nullptr || model->AiScene == nullptr)
	{
		outCenter = { 0.0f, 0.0f, 0.0f };
		outRadius = 1.0f;
		return;
	}

	aiVector3D minv(FLT_MAX, FLT_MAX, FLT_MAX);
	aiVector3D maxv(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; ++m)
	{
		aiMesh* mesh = model->AiScene->mMeshes[m];
		if (mesh == nullptr || mesh->mNumVertices == 0) continue;

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			aiVector3D v = mesh->mVertices[i];
			if (v.x < minv.x) minv.x = v.x;
			if (v.y < minv.y) minv.y = v.y;
			if (v.z < minv.z) minv.z = v.z;
			if (v.x > maxv.x) maxv.x = v.x;
			if (v.y > maxv.y) maxv.y = v.y;
			if (v.z > maxv.z) maxv.z = v.z;
		}
	}

	outCenter.x = (minv.x + maxv.x) * 0.5f;
	outCenter.y = (minv.y + maxv.y) * 0.5f;
	outCenter.z = (minv.z + maxv.z) * 0.5f;

	float hx = (maxv.x - minv.x) * 0.5f;
	float hy = (maxv.y - minv.y) * 0.5f;
	float hz = (maxv.z - minv.z) * 0.5f;
	outRadius = sqrtf(hx * hx + hy * hy + hz * hz);
}

//======================================================
//	初期化関数
//======================================================
void Result_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 背景テクスチャ読み込み
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(L"asset\\texture\\uiBack_v1.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
		assert(g_Texture);//読み込み失敗時にダイアログを表示
	}

	// 既存モデル読み込み（FBX）
	g_ResultModel = ModelLoad("asset\\model\\uiResultArch_v3.fbx");
	assert(g_ResultModel);

	// 既存モデルテクスチャ読み込み
	{
		TexMetadata metadata;
		ScratchImage image;
		HRESULT hr = LoadFromWICFile(L"asset\\texture\\uiTextureArch03_v1.png", WIC_FLAGS_NONE, &metadata, image);
		assert(SUCCEEDED(hr));
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_ResultTex);
		assert(g_ResultTex);
		g_ResultTexMeta = metadata;
	}

	// ----- 既存モデルのバウンディングボックスを計算して中心を保存 -----
	CalculateModelBounds(g_ResultModel, g_ResultModelCenter, g_ResultModelRadius);
	// ------------------------------------------------------------

	// --- タワーモデル読み込み ---
	g_TowerModel = ModelLoad("asset\\model\\uiResultTour_v2.fbx");
	assert(g_TowerModel);
	CalculateModelBounds(g_TowerModel, g_TowerModelCenter, g_TowerModelRadius);

	// タワー用 4 テクスチャ読み込み（順に Red, Blue, Green, Yellow）
	const wchar_t* towerTexFiles[4] = {
		L"asset\\texture\\uiTextureTowerRed_v1.png",
		L"asset\\texture\\uiTextureTowerBlue_v1.png",
		L"asset\\texture\\uiTextureTowerGreen_v1.png",
		L"asset\\texture\\uiTextureTowerYellow_v1.png"
	};
	for (int i = 0; i < 4; ++i)
	{
		TexMetadata meta;
		ScratchImage img;
		HRESULT hr = LoadFromWICFile(towerTexFiles[i], WIC_FLAGS_NONE, &meta, img);
		assert(SUCCEEDED(hr));
		CreateShaderResourceView(pDevice, img.GetImages(), img.GetImageCount(), meta, &g_TowerTex[i]);
		assert(g_TowerTex[i]);
		g_TowerTexMeta[i] = meta;
	}

	//フェードインのセット
	XMFLOAT4	color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	SetFade(60.0f, color, FADE_IN, SCENE_GAME);
}

//======================================================
//	終了処理関数
//======================================================
void Result_Finalize()
{
	//テクスチャの解放など
	SAFE_RELEASE(g_Texture);
	SAFE_RELEASE(g_ResultTex);

	for (int i = 0; i < 4; ++i)
	{
		SAFE_RELEASE(g_TowerTex[i]);
	}
	if (g_TowerModel)
	{
		ModelRelease(g_TowerModel);
		g_TowerModel = nullptr;
	}

	if (g_ResultModel)
	{
		ModelRelease(g_ResultModel);
		g_ResultModel = nullptr;
	}
}

//======================================================
//	更新処理
//======================================================
void Result_Update()
{
	//キー入力チェック
	//スタートボタンが押されたらシーンを切り替え
	//フェード処理中はキーを受け付けない
	if (Keyboard_IsKeyDownTrigger(KK_ENTER) && (GetFadeState() == FADE_NONE))
	{
		//フェードアウトさせてシーンを切り替える
		XMFLOAT4	color(0.0f, 0.0f, 0.0f, 1.0f);
		SetFade(40.0f, color, FADE_OUT, SCENE_TITLE);
	}
}

//======================================================
//	描画関数
//======================================================
void Result_Draw()
{
	// シェーダーを描画パイプラインに設定
	Shader_Begin();

	// 画面サイズ取得
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

	// 頂点シェーダーに変換行列を設定（UI用：直交投影）
	Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
		0.0f,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		0.0f,
		0.0f,
		1.0f));
	//---------------------------------------------------

	//背景描画
	if (g_Texture)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture);//g_Textureを使うように設定する
		SetBlendState(BLENDSTATE_NONE);//ブレンド無し
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };	//スプライトの色
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
		XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };
		DrawSprite(pos, size, col);//1枚絵を表示
	}

	// モデル描画（既存モデル：アーチ）
	if (g_ResultModel && g_ResultTex)
	{
		ID3D11DeviceContext* ctx = Direct3D_GetDeviceContext();
		if (ctx == nullptr) return;

		// 3D 用の行列（アーチはそのまま表示）
		XMMATRIX worldArch = XMMatrixIdentity();

		// カメラ（モデル群を中心に）
		XMVECTOR eyePos = XMVectorSet(g_ResultModelCenter.x, g_ResultModelCenter.y + 1.5f, g_ResultModelCenter.z - (g_ResultModelRadius * 2.0f + 2.0f), 0.0f);
		XMVECTOR focus = XMVectorSet(g_ResultModelCenter.x, g_ResultModelCenter.y, g_ResultModelCenter.z, 0.0f);
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMMATRIX view = XMMatrixLookAtLH(eyePos, focus, up);

		float aspect = (SCREEN_HEIGHT != 0.0f) ? (SCREEN_WIDTH / SCREEN_HEIGHT) : 1.0f;
		XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.1f, 1000.0f);

		// 描画：アーチ
		Shader_SetMatrix(worldArch * view * proj);

		// 深度ステンシルの設定
		ID3D11DepthStencilState* oldDepth = nullptr;
		UINT oldRef = 0;
		ctx->OMGetDepthStencilState(&oldDepth, &oldRef);

		ID3D11DepthStencilState* pDepthState = nullptr;
		D3D11_DEPTH_STENCIL_DESC dsDesc{};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		dsDesc.StencilEnable = FALSE;
		if (SUCCEEDED(Direct3D_GetDevice()->CreateDepthStencilState(&dsDesc, &pDepthState)))
		{
			ctx->OMSetDepthStencilState(pDepthState, 0);
		}

		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (unsigned int m = 0; m < g_ResultModel->AiScene->mNumMeshes; m++)
		{
			aiMesh* mesh = g_ResultModel->AiScene->mMeshes[m];

			ctx->PSSetShaderResources(0, 1, &g_ResultTex);

			UINT stride = sizeof(Vertex3D);
			UINT offset = 0;
			ctx->IASetVertexBuffers(0, 1, &g_ResultModel->VertexBuffer[m], &stride, &offset);
			ctx->IASetIndexBuffer(g_ResultModel->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);
			ctx->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
		}

		ctx->OMSetDepthStencilState(oldDepth, oldRef);
		if (oldDepth) oldDepth->Release();
		SAFE_RELEASE(pDepthState);
	}

	// --- タワーを4つ均等配置して描画 ---
	if (g_TowerModel)
	{
		ID3D11DeviceContext* ctx = Direct3D_GetDeviceContext();
		if (ctx == nullptr) return;

		// カメラをタワー群中心に合わせ（既存アーチ中心を基準）
		XMVECTOR eyePos = XMVectorSet(g_ResultModelCenter.x, g_ResultModelCenter.y + 1.5f, g_ResultModelCenter.z - (g_ResultModelRadius * 2.0f + 2.0f), 0.0f);
		XMVECTOR focus = XMVectorSet(g_ResultModelCenter.x, g_ResultModelCenter.y, g_ResultModelCenter.z, 0.0f);
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMMATRIX view = XMMatrixLookAtLH(eyePos, focus, up);

		float aspect = (SCREEN_HEIGHT != 0.0f) ? (SCREEN_WIDTH / SCREEN_HEIGHT) : 1.0f;
		XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.1f, 1000.0f);

		// 配置間隔（モデルサイズに基づく）
		float spacing = g_TowerModelRadius * 2.2f;
		// 中心からのオフセット: -1.5, -0.5, 0.5, 1.5
		float offsets[4] = { -1.5f * spacing, -0.5f * spacing, 0.5f * spacing, 1.5f * spacing };

		// 深度ステンシル（共通）
		ID3D11DepthStencilState* oldDepth = nullptr;
		UINT oldRef = 0;
		ctx->OMGetDepthStencilState(&oldDepth, &oldRef);

		ID3D11DepthStencilState* pDepthState = nullptr;
		D3D11_DEPTH_STENCIL_DESC dsDesc{};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		dsDesc.StencilEnable = FALSE;
		if (SUCCEEDED(Direct3D_GetDevice()->CreateDepthStencilState(&dsDesc, &pDepthState)))
		{
			ctx->OMSetDepthStencilState(pDepthState, 0);
		}

		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (int i = 0; i < 4; ++i)
		{
			// 目標ワールド位置（Result の中心基準）
			XMFLOAT3 targetPos;
			targetPos.x = g_ResultModelCenter.x + offsets[i];
			targetPos.y = g_ResultModelCenter.y;
			targetPos.z = g_ResultModelCenter.z;

			// モデルの重心を targetPos に合わせるワールド行列
			XMMATRIX world = XMMatrixTranslation(
				targetPos.x - g_TowerModelCenter.x,
				targetPos.y - g_TowerModelCenter.y,
				targetPos.z - g_TowerModelCenter.z);

			Shader_SetMatrix(world * view * proj);

			// 各メッシュを描画（各タワーに固有テクスチャを適用）
			for (unsigned int m = 0; m < g_TowerModel->AiScene->mNumMeshes; m++)
			{
				aiMesh* mesh = g_TowerModel->AiScene->mMeshes[m];

				// テクスチャをピクセルシェーダにバインド（i 番目の色）
				ctx->PSSetShaderResources(0, 1, &g_TowerTex[i]);

				UINT stride = sizeof(Vertex3D);
				UINT offset = 0;
				ctx->IASetVertexBuffers(0, 1, &g_TowerModel->VertexBuffer[m], &stride, &offset);
				ctx->IASetIndexBuffer(g_TowerModel->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);
				ctx->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
			}
		}

		ctx->OMSetDepthStencilState(oldDepth, oldRef);
		if (oldDepth) oldDepth->Release();
		SAFE_RELEASE(pDepthState);
	}
}