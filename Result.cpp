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
#include <chrono> // 追加：時間計測
#include <algorithm> // 追加：std::min（残しても問題なし）

static	ID3D11ShaderResourceView* g_Texture = NULL;		// 背景
static	ID3D11ShaderResourceView* g_Texture2 = NULL;	// 次へ
static	ID3D11ShaderResourceView* g_Texture3 = NULL;	// 選択
static ID3D11ShaderResourceView* g_ResultTex = nullptr; // 既存モデル用テクスチャ
static TexMetadata		g_ResultTexMeta{};
static MODEL* g_ResultModel = nullptr;					// 既存モデル

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

// ---------- ポップイン用タイマー / アニメーション状態 ----------
static std::chrono::steady_clock::time_point g_StartTime;
static bool g_ButtonsAnimStarted = false;
static std::chrono::steady_clock::time_point g_AnimStartTime;
static constexpr double g_ButtonDelaySeconds = 5.0;     // 5秒遅延
static constexpr double g_ButtonAnimDuration = 0.6;     // アニメーション長（秒）
// -----------------------------------------------------------------

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

// イージング：EaseOutBack（ポップ感、オーバーシュートあり）
static float EaseOutBack(float t)
{
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;
	t = t - 1.0f;
	return 1.0f + (c3 * t * t * t + c1 * t * t);
}

// リニア補間
static float Lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

// 値を [0,1] にクランプ（std::min マクロ衝突回避のため使用）
static double Clamp01(double v)
{
	if (v <= 0.0) return 0.0;
	if (v >= 1.0) return 1.0;
	return v;
}

// 現在時刻（秒）取得（開始時刻からの経過）
static double GetElapsedSeconds()
{
	using namespace std::chrono;
	auto now = steady_clock::now();
	auto elapsed = duration_cast<duration<double>>(now - g_StartTime);
	return elapsed.count();
}

//======================================================
//	初期化関数
//======================================================
void Result_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// タイマー開始時刻を記録
	g_StartTime = std::chrono::steady_clock::now();
	g_ButtonsAnimStarted = false;

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

	// 次へテクスチャ読み込み
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(L"asset\\texture\\nextButton.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture2);
		assert(g_Texture2);//読み込み失敗時にダイアログを表示
	}

	// 選択テクスチャ読み込み
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(L"asset\\texture\\selectButton.png", WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture3);
		assert(g_Texture3);//読み込み失敗時にダイアログを表示
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
	SAFE_RELEASE(g_Texture2);
	SAFE_RELEASE(g_Texture3);
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

	// 遅延経過チェック：アニメーション開始タイミングを Result_Update でも保持
	double elapsed = GetElapsedSeconds();
	if (!g_ButtonsAnimStarted && elapsed >= g_ButtonDelaySeconds)
	{
		g_ButtonsAnimStarted = true;
		g_AnimStartTime = std::chrono::steady_clock::now();
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

		// 回転を追加（FBX向けの回転調整）
		// rotation は必要に応じて変更してください（ラジアン）
		XMFLOAT3 rotationArch = { 0.0f, 0.0f, 0.0f };
		XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw(
			rotationArch.x + XMConvertToRadians(-90.0f),
			rotationArch.y,
			rotationArch.z);
		worldArch = RotationMatrix * worldArch;

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

			// 回転を追加（FBX向けの回転調整）
			// rotation は必要に応じて変更してください（ラジアン）
			XMFLOAT3 rotationTower = { 0.0f, 0.0f, 0.0f };
			XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw(
				rotationTower.x + XMConvertToRadians(-90.0f),
				rotationTower.y,
				rotationTower.z);
			world = RotationMatrix * world;

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

	// ここで UI 用の直交投影行列に戻す（重要）
	Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
		0.0f,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		0.0f,
		0.0f,
		1.0f));
	// スプライトの描画は通常ブレンド無しかアルファにする（既存スタイルに合わせる）
	SetBlendState(BLENDSTATE_NONE);

	// --- ボタン：5秒後に右からポップアウト ---
	// ボタン基本情報
	const XMFLOAT2 btnSize = { 200.0f, 200.0f };
	const float targetX = SCREEN_WIDTH - 100.0f;
	const float startX = SCREEN_WIDTH + btnSize.x * 0.5f + 50.0f; // 右画面外から出てくる
	double nowElapsed = GetElapsedSeconds();

	// 次へボタン
	if (g_Texture2)
	{
		// 遅延中はまだ表示しない
		if (nowElapsed >= g_ButtonDelaySeconds)
		{
			// アニメーション進行度
			double animElapsed = 0.0;
			if (g_ButtonsAnimStarted)
			{
				auto now = std::chrono::steady_clock::now();
				animElapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - g_AnimStartTime).count();
			}

			double rawT = animElapsed / g_ButtonAnimDuration;
			float t = static_cast<float>(Clamp01(rawT));
			float eased = EaseOutBack(t);

			// 位置・スケール・アルファ補間
			float curX = Lerp(startX, targetX, eased);
			float curScale = Lerp(0.6f, 1.05f, eased); // 少しオーバーして戻る
			float alpha = Lerp(0.0f, 1.0f, t);

			// ポップ青寄せ
			XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };

			// DrawSprite は中心座標で描画する想定
			XMFLOAT2 pos = { curX, SCREEN_HEIGHT / 2 + 230 };
			XMFLOAT2 size = { btnSize.x * curScale, btnSize.y * curScale };

			g_pContext->PSSetShaderResources(0, 1, &g_Texture2);//g_Textureを使うように設定する
			SetBlendState(BLENDSTATE_ALPHA);
			DrawSprite(pos, size, col);
		}
	}

	// 選択ボタン
	if (g_Texture3)
	{
		// 遅延中はまだ表示しない
		if (nowElapsed >= g_ButtonDelaySeconds)
		{
			double animElapsed = 0.0;
			if (g_ButtonsAnimStarted)
			{
				auto now = std::chrono::steady_clock::now();
				animElapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - g_AnimStartTime).count();
			}

			// 少し遅らせて2つめのボタンは同じアニメだが開始を少し遅らせたい場合は offset を追加可能
			double rawT = animElapsed / g_ButtonAnimDuration;
			float t = static_cast<float>(Clamp01(rawT));
			float eased = EaseOutBack(t);

			float curX = Lerp(startX, targetX, eased);
			float curScale = Lerp(0.6f, 1.05f, eased);
			float alpha = Lerp(0.0f, 1.0f, t);

			XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };

			XMFLOAT2 pos = { curX, SCREEN_HEIGHT / 2 + 300 };
			XMFLOAT2 size = { btnSize.x * curScale, btnSize.y * curScale };

			g_pContext->PSSetShaderResources(0, 1, &g_Texture3);//g_Textureを使うように設定する
			SetBlendState(BLENDSTATE_ALPHA);
			DrawSprite(pos, size, col);
		}
	}
	// ------------------------------------------------------------------

	// スプライトの描画は通常ブレンド無しかアルファにする（既存スタイルに合わせる）
	SetBlendState(BLENDSTATE_NONE);
}