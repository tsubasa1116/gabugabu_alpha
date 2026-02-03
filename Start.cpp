//======================================================
//	start.cpp
// 
//	制作者：田中佑奈			日付：2026//
//======================================================

#include	"Manager.h"
#include	"sprite.h"
#include	"keyboard.h"

#include	"Start.h"

#include "fade.h"
#include "shader.h"

#include "model.h" // 追加：モデル読み込み用

#include <chrono>
#include <cmath>
#include "LoadingScreen.h"

static	ID3D11ShaderResourceView* g_Texture = NULL;	//従来のフルスクリーンUIテクスチャ（必要なら残す）
static	ID3D11ShaderResourceView* g_Texture3 = NULL;	//従来のフルスクリーンUIテクスチャ（必要なら残す）
static	DirectX::TexMetadata		g_Metadata3{};
static MODEL* g_StartModel = nullptr;                 // 追加：FBXモデル
static ID3D11ShaderResourceView* g_StartTex = nullptr; // 追加：FBXに貼るPNGテクスチャ

static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// --- ロゴふよふよ用（調整可能） ---
static std::chrono::steady_clock::time_point g_LastTime;
static float g_Texture3FloatTime = 0.0f;
static float g_Texture3OffsetY = 0.0f;
static constexpr float g_Texture3Amplitude = 9.0f; // 振幅（ピクセル）
static constexpr float g_Texture3Speed = 2.0f;     // 速度（周期係数）
// ---------------------------------------

void Start_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 時間初期化
	g_LastTime = std::chrono::steady_clock::now();

	// FBXモデル読み込み
	g_StartModel = ModelLoad("asset\\model\\uiStartMap_v1.fbx");
	assert(g_StartModel);

	// fbxに貼るやつ
	{
		DirectX::TexMetadata metadata2;
		DirectX::ScratchImage image2;
		HRESULT hr = LoadFromWICFile(L"asset\\texture\\uiTextureStage_v1.png", WIC_FLAGS_NONE, &metadata2, image2);
		assert(SUCCEEDED(hr));
		CreateShaderResourceView(pDevice, image2.GetImages(), image2.GetImageCount(), metadata2, &g_StartTex);
		assert(g_StartTex);
	}

	// ミールシティ
	{
		DirectX::TexMetadata	metadata3;
		DirectX::ScratchImage	image3;
		HRESULT hr = LoadFromWICFile(L"asset\\texture\\stageBoard.png", WIC_FLAGS_NONE, &metadata3, image3);
		assert(SUCCEEDED(hr));
		CreateShaderResourceView(pDevice, image3.GetImages(), image3.GetImageCount(), metadata3, &g_Texture3);
		assert(g_Texture3);//読み込み失敗時にダイアログを表示

		// 実ピクセルサイズを保持（描画時にアスペクト比を保つため）
		g_Metadata3 = metadata3;
	}
}
void Start_Finalize()
{
	//テクスチャの解放など
	SAFE_RELEASE(g_Texture);

	// Start 用リソース解放
	if (g_StartModel)
	{
		ModelRelease(g_StartModel);
		g_StartModel = nullptr;
	}
	SAFE_RELEASE(g_StartTex);
	SAFE_RELEASE(g_Texture3);
}
void Start_Update()
{
	// 時間差分を計算してふよふよアニメーションを更新
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsed = now - g_LastTime;
	float dt = elapsed.count();
	g_LastTime = now;

	// サイン波で上下移動
	g_Texture3FloatTime += dt * g_Texture3Speed;
	g_Texture3OffsetY = std::sinf(g_Texture3FloatTime) * g_Texture3Amplitude;

	//キー入力チェック
	if (Keyboard_IsKeyDownTrigger(KK_ENTER) && (GetFadeState() == FADE_NONE))
	{
		// �L�[���̓`�F�b�N�i���[�h���͎󂯕t���Ȃ��j
		if (Keyboard_IsKeyDownTrigger(KK_ENTER) && (GetFadeState() == FADE_NONE) && !IsLoading())
		{
			XMFLOAT4 color(0.0f, 0.0f, 0.0f, 1.0f);

			// ���[�h��ʕt���t�F�[�h�ŃQ�[���V�[���֑J��
			SetFadeWithLoading(40, color, FADE_OUT, SCENE_GAME, L"asset\\movie\\uiRored.mp4");
		}
	}
}

void Start_Draw()
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

	// FBXモデルを描画（全メッシュに g_StartTex を適用して描画）
	if (g_StartModel && g_StartTex)
	{
		ID3D11DeviceContext* ctx = Direct3D_GetDeviceContext();

		// ワールド行列（既存の回転を維持）
		XMMATRIX world = XMMatrixRotationZ(XM_PI) * XMMatrixRotationY(XM_PI);

		// カメラ：斜め上からの角度
		const float camX = 0.0f;
		const float camY = -10.0f;
		const float camZ = -30.0f;
		XMVECTOR eyePos = XMVectorSet(camX, camY, camZ, 0.0f);

		// 注視点はモデル中心（必要ならモデルのバウンディングボックス中心に変更）
		XMVECTOR focus = XMVectorSet(0.0f, 2.0f, 0.0f, 0.0f);

		// 上方向ベクトル（標準）
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		// ビュー行列（斜め上からの視点）
		XMMATRIX view = XMMatrixLookAtLH(eyePos, focus, up);

		// 投影行列
		float aspect = (SCREEN_HEIGHT != 0.0f) ? (SCREEN_WIDTH / SCREEN_HEIGHT) : 1.0f;
		XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.1f, 1000.0f);

		// 頂点シェーダに最終行列をセット（ワールド*ビュー*プロジェクション）
		Shader_SetMatrix(world * view * proj);

		// 深度ステートのバックアップ
		ID3D11DepthStencilState* oldDepth = nullptr;
		UINT oldRef = 0;
		ctx->OMGetDepthStencilState(&oldDepth, &oldRef);

		// 一時的に深度テストと深度書き込みを有効にしたステートを作成してセット
		ID3D11DepthStencilState* pDepthState = nullptr;
		D3D11_DEPTH_STENCIL_DESC dsDesc;
		ZeroMemory(&dsDesc, sizeof(dsDesc));
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		dsDesc.StencilEnable = FALSE;
		if (SUCCEEDED(Direct3D_GetDevice()->CreateDepthStencilState(&dsDesc, &pDepthState)))
		{
			ctx->OMSetDepthStencilState(pDepthState, 0);
		}

		// プリミティブ設定は ModelDraw と同じ
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (unsigned int m = 0; m < g_StartModel->AiScene->mNumMeshes; m++)
		{
			aiMesh* mesh = g_StartModel->AiScene->mMeshes[m];

			// 常に g_StartTex をピクセルシェーダにバインド
			ctx->PSSetShaderResources(0, 1, &g_StartTex);

			// 頂点バッファセット
			UINT stride = sizeof(Vertex3D);
			UINT offset = 0;
			ctx->IASetVertexBuffers(0, 1, &g_StartModel->VertexBuffer[m], &stride, &offset);

			// インデックスバッファセット
			ctx->IASetIndexBuffer(g_StartModel->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

			// 描画
			ctx->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
		}

		// 深度ステートを復元
		ctx->OMSetDepthStencilState(oldDepth, oldRef);
		if (oldDepth) oldDepth->Release();

		SAFE_RELEASE(pDepthState);
	}

	// ミールシティの描画
	if (g_Texture3)
	{
		// UI 描画用に直交投影に戻す（重要）
		Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
			0.0f,
			SCREEN_WIDTH,
			SCREEN_HEIGHT,
			0.0f,
			0.0f,
			1.0f));

		g_pContext->PSSetShaderResources(0, 1, &g_Texture3);
		SetBlendState(BLENDSTATE_ALPHA);

		// 幅を画面幅の30%に合わせ、高さはテクスチャのアスペクト比で計算する
		float texW = (g_Metadata3.width > 0) ? (float)g_Metadata3.width : 100.0f;
		float texH = (g_Metadata3.height > 0) ? (float)g_Metadata3.height : 50.0f;

		float desiredWidth = SCREEN_WIDTH * 0.30f; // 幅を画面の30%にする例
		float scale = desiredWidth / texW;
		XMFLOAT2 size = { texW * scale, texH * scale };

		// ベース位置にふよふよオフセットを加える
		XMFLOAT2 pos = { SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT * 0.37f + g_Texture3OffsetY };
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };

		DrawSprite(pos, size, col);

		SetBlendState(BLENDSTATE_NONE);
	}
}