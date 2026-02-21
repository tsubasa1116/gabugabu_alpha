// =====================================================
//	player.cpp
// 
//	制作者：平岡颯馬			日付：2026/01/27
//======================================================
#include <d3d11.h>
#include <iostream>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "shader.h"
#include "keyboard.h"
#include "sprite.h"
#include "color.h"
#include "hp.h"
#include "gauge.h"
#include "Effect.h"
#include "player.h"
#include "Camera.h"
#include "input.h"
#include "skill.h"
#include "special.h"
#include "field.h"
#include "collider.h"
#include "debug_render.h"
#include "debug_ostream.h"
#include "attack.h" 
#include "DamageText.h"
#include "makeText.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <chrono>
#include <codecvt>
#include <vector>
#include <algorithm>

//======================================================
//	マクロ定義
//======================================================
#define GAUGE_POS_X	(69.0f * (SCREEN_WIDTH / 1280.0f))	
#define GAUGE_POS_Y	(8.0f *  (SCREEN_HEIGHT / 720.0f))	
#define	HPBER_SIZE_X (270.0f * (SCREEN_WIDTH / 1280.0f))
#define	HPBER_SIZE_Y (270.0f * (SCREEN_HEIGHT / 720.0f))

//======================================================
//	グローバル変数
//======================================================
// オブジェクト
PLAYEROBJECT player[PLAYER_MAX];

static ID3D11Device* g_pDevice = NULL;
static ID3D11DeviceContext* g_pContext = NULL;
static hp HPBar[PLAYER_MAX];

// 頂点バッファ
static ID3D11Buffer* g_VertexBuffer = NULL;

// インデックスバッファ
static ID3D11Buffer* g_IndexBuffer = NULL;

// テクスチャ変数
static ID3D11ShaderResourceView* g_Texture[17];

// プレイヤー アニメーション用変数
static int   g_animFrame[PLAYER_MAX] = { 0 };
static float g_animTimer[PLAYER_MAX] = { 0.0f };
static const float ANIM_FRAME_TIME = 0.15f;	// 1フレームあたりの秒数
static const int   SHEET_COLS = 16;
static const int   SHEET_ROWS = 16;

static int g_victoryState[PLAYER_MAX] = { 0 };			// 0 = なし, 1 = 初回 再生中, 2 = ループ
static float g_downHoldTimer[PLAYER_MAX] = { 0.0f };	// 最終フレームホールド用タイマー（プレイヤー毎）
static bool g_specialAnimStarted[PLAYER_MAX] = { false, false, false, false };

// 順位・死亡順の管理
static std::vector<int> g_deathOrder;	// 死亡したプレイヤーのインデックス（先に死んだ者が先頭）

// 頂点配列
static Vertex2 vdata[PLAYER_VERTEX] =
{
	{// 頂点0 LEFT-TOP
		XMFLOAT3(-COORDINATE, COORDINATE, 0.0f),	// 座標
		XMFLOAT3(0.0f, 0.0f, -1.0f),				// 法線ベクトル
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),			// カラー
		XMFLOAT2(0.0f, 0.0f)						// テクスチャ座標
	},
	{// 頂点1 RIGHT-TOP
		XMFLOAT3(COORDINATE, COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(TEXCOORD, 0.0f)
	},
	{// 頂点2 LEFT-BOTTOM
		XMFLOAT3(-COORDINATE, -COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f, TEXCOORD)
	},
	{// 頂点3 RIGHT-BOTTOM
		XMFLOAT3(COORDINATE, -COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(TEXCOORD, TEXCOORD)
	},
};

// インデックス配列
static UINT idxdata[6]
{
	 0, 1, 2, 2, 1, 3, // -Z面
};

static float top_y = 0;	// 六角形のtop-y座票のデバッグ表示

//======================================================
//	初期化関数
//======================================================
void Player_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// プレイヤー表示の初期化
	player[0].position = XMFLOAT3(-3.0f, 4.0f, 0.0f);
	player[1].position = XMFLOAT3(1.5f, 4.0f, 2.0f);
	player[2].position = XMFLOAT3(-4.0f, 4.0f, -3.0f);
	player[3].position = XMFLOAT3(4.0f, 4.0f, 1.0f);

	player[0].form = Form::First;
	player[1].form = Form::First;
	player[2].form = Form::First;
	player[3].form = Form::First;
	player[0].type = PlayerType::None;
	player[1].type = PlayerType::None;
	player[2].type = PlayerType::None;
	player[3].type = PlayerType::None;
	//player[0].form = Form::Third;
	//player[1].form = Form::Third;
	//player[2].form = Form::Third;
	//player[3].form = Form::Third;
	//player[0].type = PlayerType::Glass;
	//player[1].type = PlayerType::Concrete;
	//player[2].type = PlayerType::Plant;
	//player[3].type = PlayerType::Plant;

	for (int p = 0; p < PLAYER_MAX; p++)
	{
		player[p].oldPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[p].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[p].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
		player[p].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[p].hp = PLAYER_MAX_HP;
		player[p].attack = 0.0f;
		player[p].power = 0.0f;
		player[p].speed = 0.0f;
		player[p].defense = 1.0f;
		player[p].stock = 3;
		player[p].rank = 0;
		player[p].active = true;
		player[p].satiety = 0.0f;
		player[p].isAttacking = false;
		player[p].attackTimer = 0.0f;
		player[p].isAttacked = false;
		player[p].attackedTimer = 0.0f;
		player[p].isHealing = false;
		player[p].healingTimer = 0.0f;
		player[p].isEvolving = false;
		player[p].evolvingTimer = 0.0f;
		player[p].useSkill = false;
		player[p].skillTimer = 0.0f;
		player[p].skillCoolTimer = 0.0f;
		player[p].useSpecial = false;
		player[p].specialTimer = 0.0f;
		player[p].isInvincible = false;
		player[p].invincibleTimer = 0.0f;
		player[p].stunGauge = 0.0f;
		player[p].isStunning = false;
		player[p].stunTimer = 0.0f;
		player[p].isDown = false;
		player[p].downTimer = 0.0f;
		player[p].isPoisoned = false;
		player[p].poisonTimer = 0.0f;
		player[p].duringRespawn = false;
		player[p].respawnTimer = 0.0f;
		player[p].isEggBreaking = false;
		player[p].eggBreakingTimer = 0.0f;
		player[p].lastDir = PlayerDir::Down; // 正面
		player[p].isMoving = false;
		player[p].isShadowEnabled = false;
		player[p].evolutionGauge = 0.0f;
		player[p].evolutionGaugeRate = 0.3f;
		player[p].breakCount_Glass = 0;
		player[p].breakCount_Concrete = 0;
		player[p].breakCount_Plant = 0;
		player[p].breakCount_Electricity = 0;
	}

	// 頂点バッファ作成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));	// 0でクリア
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * PLAYER_VERTEX;	// 格納できる頂点数*頂点サイズ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	g_pDevice = pDevice;
	g_pContext = pContext;

#ifdef _DEBUG_
	// テクスチャロード時間計測
	auto tex_start = std::chrono::high_resolution_clock::now();
	LoadTextureList(pDevice);
	auto tex_end = std::chrono::high_resolution_clock::now();
	auto tex_ms = std::chrono::duration_cast<std::chrono::milliseconds>(tex_end - tex_start).count();
	hal::dout << "テクスチャロード時間: " << tex_ms << " ms" << std::endl;
#else
	// テクスチャ読み込み
	LoadTextureList(pDevice);
#endif

	// インデックスバッファ作成
	{
		D3D11_BUFFER_DESC	bd;
		ZeroMemory(&bd, sizeof(bd));	// 0でクリア
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(UINT) * 6 * 6;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&bd, NULL, &g_IndexBuffer);

		// インデックスバッファへ書き込み
		D3D11_MAPPED_SUBRESOURCE msr;
		pContext->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		UINT* index = (UINT*)msr.pData;

		// インデックスデータをバッファへコピー
		CopyMemory(&index[0], &idxdata[0], sizeof(UINT) * 6 * 6);
		pContext->Unmap(g_IndexBuffer, 0);
	}
	// デバッグレンダラー初期化
	Debug_Initialize(pDevice, pContext);

	float screenX = SCREEN_ADJUST_X;
	float screenY = 650.0f * SCREEN_ADJUST_Y;

	InitializeHP(pDevice, pContext, &HPBar[0], {  160.0f * screenX, screenY }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);
	InitializeHP(pDevice, pContext, &HPBar[1], {  480.0f * screenX, screenY }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);
	InitializeHP(pDevice, pContext, &HPBar[2], {  800.0f * screenX, screenY }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);
	InitializeHP(pDevice, pContext, &HPBar[3], { 1120.0f * screenX, screenY }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);

	HPBar[0].gaugeIndex = 0;
	HPBar[1].gaugeIndex = 1;
	HPBar[2].gaugeIndex = 2;
	HPBar[3].gaugeIndex = 3;

	// アニメーションの初期化
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		g_animFrame[i] = 0;
		g_animTimer[i] = 0.0f;
	}

	// 順位情報を初期化
	g_deathOrder.clear();
}

static void LoadTextureList(ID3D11Device* pDevice)
{
	TexMetadata metadata;
	ScratchImage image;

	struct TexEntry { int idx; const wchar_t* path;};

	const TexEntry texList[] = 
	{
		{  0, L"asset\\texture\\characterMiniRed_v2.png"},			// 第1形態 P1 赤
		{  1, L"asset\\texture\\characterMiniBlue_v1.png"},			// 第1形態 P2 青
		{  2, L"asset\\texture\\characterMiniYellow_v1.png"},		// 第1形態 P3 黄
		{  3, L"asset\\texture\\characterMiniGreen_v1.png"},		// 第1形態 P4 緑
		{  4, L"asset\\texture\\characterMidGlass_v1.png"},			// 第2形態 ガラス
		{  5, L"asset\\texture\\characterMidConcrete_v1.png" },		// 第2形態 コンクリート
		{  6, L"asset\\texture\\characterMidTree_v1.png" },			// 第2形態 植物
		{  7, L"asset\\texture\\characterMidElectricity_v1.png" },	// 第2形態 電気
		{  8, L"asset\\texture\\characterBigGlass_v2.png" },		// 第3形態 ガラス
		{  9, L"asset\\texture\\characterBigConcrete_v2.png" },		// 第3形態 コンクリート
		{ 10, L"asset\\texture\\characterBigTree_v2.png" },			// 第3形態 植物
		{ 11, L"asset\\texture\\characterBigElectricity_v2.png" },	// 第3形態 電気
		{ 12, L"asset\\texture\\characterBigSP_v3.png" },			// 第3形態 スペシャル
		{ 13, L"asset\\texture\\uiStockRed_v4.png"},				// UI ストック 赤
		{ 14, L"asset\\texture\\uiStockBlue_v4.png"},				// UI ストック 青
		{ 15, L"asset\\texture\\uiStockYellow_v4.png" },			// UI ストック 黄
		{ 16, L"asset\\texture\\uiStockGreen_v4.png" },				// UI ストック 緑
	};

	for (const auto& e : texList)
	{
		auto start = std::chrono::high_resolution_clock::now();

		// コメント化している要素は配列エントリ自体をコメントアウトしているためここには来ない。
		HRESULT hr = LoadFromWICFile(e.path, WIC_FLAGS_NONE, &metadata, image);
		if (SUCCEEDED(hr))
		{
			if (FAILED(CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[e.idx])))
			{
				// 作成失敗時は nullptr を代入して続行
				g_Texture[e.idx] = nullptr;
			}
		}
		// 読み込み失敗は nullptr を代入して続行
		else	g_Texture[e.idx] = nullptr;

		auto end = std::chrono::high_resolution_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		// std::wstring を std::string に変換して出力
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		hal::dout << "テクスチャロード: " << conv.to_bytes(e.path) << " " << ms << " ms" << std::endl;
	}
}

//======================================================
//	終了処理関数
//======================================================
void Player_Finalize()
{
	// シェーダーにバインドされている SRV をアンバインド（安全のため全要素分）
	const size_t TEX_COUNT = sizeof(g_Texture) / sizeof(g_Texture[0]);
	if (g_pContext)
	{
		// 固定長配列を使って確実に nullptr を渡す（API は生配列を要求）
		ID3D11ShaderResourceView* nullSRV[25] = {};
		g_pContext->PSSetShaderResources(0, static_cast<UINT>(TEX_COUNT), nullSRV);
	}

	// インデックス／頂点バッファの解放（NULL チェック後に nullptr に設定）
	if (g_IndexBuffer != nullptr)
	{
		g_IndexBuffer->Release();
		g_IndexBuffer = nullptr;
	}

	if (g_VertexBuffer != nullptr)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = nullptr;
	}

	// テクスチャ配列全要素を安全に解放（コメント化して未ロードの要素も nullptr チェックで安全）
	for (size_t i = 0; i < TEX_COUNT; ++i)
	{
		if (g_Texture[i] != nullptr)
		{
			g_Texture[i]->Release();
			g_Texture[i] = nullptr;
		}
	}

	// デバイス／コンテキストは外部管理のため解放しないが、参照はクリアしておく
	g_pContext = nullptr;
	g_pDevice = nullptr;

	// デバッグレンダラーの終了処理
	Debug_Finalize();
}

// ======================================================
// 移動関数（要変更）
// ------------------------------------------------------
// 移動ベクトルと向いている方向ベクトルは別で持った方がいい
// ======================================================

// 入力(ローカル)をカメラ基準でワールドXZへ変換する（平面移動用）
static inline XMFLOAT3 ToWorldMoveDirByCamera(const XMFLOAT2& input)
{
	// input.x: 右(+), input.y: 上(+)
	XMMATRIX view = GetViewMatrix();
	XMMATRIX invView = XMMatrixInverse(nullptr, view);

	// invView の行からカメラ軸を取得（world）
	XMFLOAT3 right = XMFLOAT3(invView.r[0].m128_f32[0], invView.r[0].m128_f32[1], invView.r[0].m128_f32[2]);
	XMFLOAT3 forward = XMFLOAT3(invView.r[2].m128_f32[0], invView.r[2].m128_f32[1], invView.r[2].m128_f32[2]);

	// XZ平面へ射影（Y成分を捨てる）
	right.y = 0.0f;
	forward.y = 0.0f;

	// 正規化（カメラが真上に近い等でゼロ割りを避ける）
	{
		float rl = sqrtf(right.x * right.x + right.z * right.z);
		if (rl > 0.0001f) { right.x /= rl; right.z /= rl; }
	}
	{
		float fl = sqrtf(forward.x * forward.x + forward.z * forward.z);
		if (fl > 0.0001f) { forward.x /= fl; forward.z /= fl; }
	}

	// ローカル入力をワールドへ合成
	XMFLOAT3 worldDir;
	worldDir.x = right.x * input.x + forward.x * input.y;
	worldDir.y = 0.0f;
	worldDir.z = right.z * input.x + forward.z * input.y;
	return worldDir;
}

void Move(PLAYEROBJECT& player, XMFLOAT3 moveDir)
{
	// 進みたい方向（3平方）
	float length = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);

	if (length > 0.0f)
	{
		// ベクトルの正規化
		moveDir.x /= length;
		moveDir.z /= length;

		// 目標角度を求める
		float targetAngle = atan2f(moveDir.x, moveDir.z);	// ベクトルの角度
		targetAngle = XMConvertToDegrees(targetAngle);		// ラジアン -> 度

		// 差分を調整（180度超えないように）
		float diff = targetAngle - player.moveAngle;	// 角度差
		if (diff > 180.0f) diff -= 360.0f;
		if (diff < -180.0f) diff += 360.0f;

		static float angSpeed = 0.5f;

		// スムーズに補間（0.1fが補間スピード）
		player.moveAngle += diff * angSpeed;

		player.rotation.y = player.moveAngle;	// 角度の反映

		// 前進
		float rad = XMConvertToRadians(player.moveAngle);
		player.position.x += sinf(rad) * player.speed;
		player.position.z += cosf(rad) * player.speed;
	}
}

//======================================================
// 更新関数
//======================================================
void Player_Update()
{
	// デバッグ用 ImGui ウィンドウ
	ImGui::Begin("Player Debug");

	// 各プレイヤーに対応する発動キー
	const Keyboard_Keys_tag attackKeys[PLAYER_MAX] = { KK_SPACE, KK_ENTER, KK_V, KK_SPACE };

	const Keyboard_Keys_tag specialKeys[PLAYER_MAX] = { KK_D7, KK_D8, KK_D9, KK_D0 };

	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		// プレイヤーごとに ID を分ける（同一ラベル衝突回避）
		ImGui::PushID(p);

		ImGui::Text("Player %d", p + 1);
		ImGui::Indent();

		ImGui::SliderFloat("poisonTimer", &player[p].poisonTimer, 0.0f, 5.0f);
		ImGui::SliderFloat("specialTimer", &player[p].specialTimer, 0.0f, 10.0f);
		ImGui::SliderFloat("stunGauge", &player[p].stunGauge, 0.0f, 10.0f);
		ImGui::SliderFloat("satiety", &player[p].satiety, 0.0f, 6.0f);
		ImGui::BulletText("isEggBreaking     : %d", player[p].isEggBreaking);
		ImGui::BulletText("isShadowEnabled   : %d", player[p].isShadowEnabled);
		ImGui::BulletText("isHealing         : %d", player[p].isHealing);
		ImGui::BulletText("isPoisoned        : %d", player[p].isPoisoned);
		ImGui::BulletText("isInvincible      : %d", player[p].isInvincible);
		ImGui::BulletText("useSkill          : %d", player[p].useSkill);
		ImGui::BulletText("EvolutionGauge    : %.1f", player[p].evolutionGauge);
		ImGui::BulletText("EvolutionGaugeRate: %.1f", player[p].evolutionGaugeRate);

		if (ImGui::Button("hp -1"))			player[p].hp -= 0.1f;
		if (ImGui::Button("gl +1"))			player[p].breakCount_Glass += 1;
		else if (ImGui::Button("pl +1"))	player[p].breakCount_Plant += 1;
		else if (ImGui::Button("co +1"))	player[p].breakCount_Concrete += 1;
		else if (ImGui::Button("el +1"))	player[p].breakCount_Electricity += 1;

		ImGui::SliderFloat("HP", &player[p].hp, 0.0f, 500.0f);
		ImGui::SliderFloat("Outer", &player[p].evolutionGauge, 0.0f, 1.0f);
		ImGui::BulletText("2 Concrete breaks : %d", player[p].breakCount_Concrete);
		ImGui::BulletText("3 Plant breaks    : %d", player[p].breakCount_Plant);
		ImGui::BulletText("4 Electricity breaks : %d", player[p].breakCount_Electricity);

		// 履歴リストのサイズを表示
		size_t historySize = player[p].brokenHistory.size();
		ImGui::BulletText("brokenHistory Size : %zu", historySize);

		if (historySize > 0)
		{
			ImGui::Indent(); // 履歴をさらに一段インデント
			ImGui::Text("History (Latest -> Oldest):");

			// 履歴を最新（末尾）から古い方へループして表示
			for (int i = (int)historySize - 1; i >= 0; --i)
			{
				// BuildingType は enum型（整数値）なので、そのまま %d で表示可能
				// または、ImGui::Textで整形して表示する

				// 例1: 履歴のインデックスと値を直接表示
				// ImGui::BulletText("[%d]: %d", p, (int)object[p].brokenHistory[p]);

				// 例2: 履歴の値を横に並べて表示
				ImGui::SameLine(); // 同じ行に表示
				// 履歴の値（整数）を文字列に変換してから表示
				ImGui::Text("%d", (int)player[p].brokenHistory[i]);
			}

			// 履歴が横に並びすぎないよう改行
			ImGui::NewLine();
			ImGui::Unindent();
		}

		ImGui::Unindent();
		ImGui::Separator();
		ImGui::PopID();

		if (!player[p].active) continue;

		// ワールド座標をスクリーン座標に変換
		XMFLOAT3 worldPos = player[p].position;
		worldPos.y += 2.0f; // プレイヤーの上方に表示

		XMVECTOR posVec = XMLoadFloat3(&worldPos);
		XMMATRIX view = GetViewMatrix();
		XMMATRIX proj = GetProjectionMatrix();
		XMMATRIX viewProj = view * proj;

		// ビューポート変換
		XMVECTOR screenPos = XMVector3Project
		(
			posVec,
			0.0f, 0.0f,
			SCREEN_WIDTH, SCREEN_HEIGHT,
			0.0f, 1.0f,
			proj, view,
			XMMatrixIdentity()
		);

		// Z値チェック（カメラの後ろなら描画しない）
		float screenZ = XMVectorGetZ(screenPos);
		if (screenZ > 0.0f && screenZ < 1.0f)
		{
			float screenX = XMVectorGetX(screenPos);
			float screenY = XMVectorGetY(screenPos);

			// テキスト描画（Update内では呼び出さない、Draw内で描画する）
			// ここでは座標を保存しておく
			player[p].screenPos = XMFLOAT2(screenX, screenY);
			player[p].isOnScreen = true;
		}
		else	player[p].isOnScreen = false;

		// -------------------------------------------------------------
		// 変身
		// -------------------------------------------------------------
		switch (player[p].form)
		{
		case Form::First: // 第1形態
			player[p].scaling.x = 0.5f;
			player[p].scaling.y = 0.5f;
			player[p].scaling.z = 0.5f;
			player[p].attack = 10.0f;
			player[p].power = 1.0f;
			player[p].speed = 0.06f;
			break;

		case Form::Second: // 第2形態
			player[p].scaling.x = 0.8f;
			player[p].scaling.y = 0.8f;
			player[p].scaling.z = 0.8f;
			player[p].attack = 15.0f;
			player[p].power = 1.5f;
			player[p].speed = 0.05f;
			break;

		case Form::Third: // 第3形態
			player[p].scaling.x = 1.2f;
			player[p].scaling.y = 1.2f;
			player[p].scaling.z = 1.2f;
			player[p].attack = 20.0f;
			player[p].power = 2.0f;
			player[p].speed = 0.04f;
			break;
		default:
			break;
		}

		// 回復フラグの更新
		if (player[p].isHealing)
		{
			player[p].healingTimer += DELTA_TIME;	// 回復タイマーを更新

			if (player[p].healingTimer >= HEALING_TIME)
			{
				player[p].isHealing = false;	// 回復終了
				player[p].healingTimer = 0.0f;	// タイマーリセット
			}
		}

		// 進化フラグの更新
		if (player[p].isEvolving)
		{
			player[p].evolvingTimer += DELTA_TIME;	// 進化タイマーを更新

			if (player[p].evolvingTimer >= EVOLVING_TIME)
			{
				player[p].isEvolving = false;	// 進化終了
				player[p].evolvingTimer = 0.0f;	// タイマーリセット
			}
		}

		// 満腹度の減少
		player[p].satiety -= DELTA_TIME;
		if (player[p].satiety < 0.0f)	player[p].satiety = 0.0f;
		//// 満腹度が1未満ならHPを減少させる
		//if (player[p].satiety < 1.0f)	player[p].hp -= 0.05f;

		// リスポーン処理
		if (player[p].duringRespawn)
		{
			player[p].respawnTimer += DELTA_TIME;

			// Y座標を4に固定
			player[p].position.y = 4.0f;

			// 攻撃ボタン押下または5秒経過で落下開始
			if (g_Input[p].A || Keyboard_IsKeyDownTrigger(attackKeys[p]) || player[p].respawnTimer >= 5.0f)
			{
				player[p].duringRespawn = false;
				player[p].respawnTimer = 0.0f;
				player[p].isInvincible = true;
				player[p].invincibleTimer = 0.0f;
				player[p].isEggBreaking = true;
				player[p].eggBreakingTimer = 0.0f;
			}
		}
		else
		{
			// y軸の移動量 (重力 + ジャンプ)
			// 重力加速度のない簡易的な重力
			player[p].position.y += -0.1f;
		}

		// 卵エフェクトが割れる時間
		if (player[p].isEggBreaking)
		{
			player[p].eggBreakingTimer += DELTA_TIME;

			if (player[p].eggBreakingTimer >= EGG_BREAKING_TIME)
			{
				player[p].isEggBreaking = false;
				player[p].eggBreakingTimer = 0.0f;
			}
		}

		// 毒の処理
		if (player[p].poisonTimer > 0.0f)
		{
			// 毒状態の間、ダメージを与える
			player[p].hp -= SPECIAL_PLANT_DAMAGE * player[p].defense;

			// 毒タイマーを進める
			player[p].poisonTimer -= DELTA_TIME;

			// 毒タイマーが0になったら毒状態を解除
			if (player[p].poisonTimer <= 0.0f)
			{
				player[p].isPoisoned = false;
				player[p].poisonTimer = 0.0f;
			}
		}

		// スタンゲージが最大でスタンフラグを立てる
		if (player[p].stunGauge >= STUNGAUGE_MAX)
		{
			player[p].isStunning = true;
			player[p].stunGauge = STUNGAUGE_MAX;
		}
		// スタン中の処理
		if (player[p].isStunning)
		{
			// スタンタイマーを進める
			player[p].stunTimer += DELTA_TIME;

			// 時間経過でスタン解除
			if (player[p].stunTimer >= STUN_TIME)
			{
				player[p].isStunning = false;	// スタン解除
				player[p].stunTimer = 0.0f;		// スタンタイマーリセット
				player[p].stunGauge = 0.0f;		// スタンゲージリセット
			}

			// スタン中は移動ベクトルを完全にゼロにする
			player[p].moveDir = { 0.0f, 0.0f, 0.0f };

			player[p].isMoving = false;

			player[p].isAttacking = false;
			player[p].useSkill = false;
			player[p].useSpecial = false;
		}
		else // スタンしていない場合の処理
		{
			// スタンしていない間はスタンゲージを減少させる
			player[p].stunGauge -= DELTA_TIME;

			// スタンゲージが0未満にならないようにクランプ
			if (player[p].stunGauge < 0.0f)	player[p].stunGauge = 0.0f;
		}

		// スタン中・ダウン中でなければ第1形態行動 1位確定後はアニメーションのみ
		if(!player[p].isStunning && !player[p].isDown && player[p].rank != 1 && player[p].active)
		{
			// 発動トリガー入力をチェックして攻撃フラグを立てる
			if (Keyboard_IsKeyDownTrigger(attackKeys[p]))
			{
				player[p].isAttacking = true;

				// 第2・第3形態の場合、スキル使用フラグも立てる
				if (player[p].type != PlayerType::None)	player[p].useSkill = true;
			}
			if (g_Input[p].A)	player[p].isAttacking = true;

			// 第2・第3形態の場合スキル使用フラグ立てる
			if (g_Input[p].X)	if (player[p].type != PlayerType::None)	player[p].useSkill = true;

			// 発動トリガー入力をチェックしてスペシャル使用フラグを立てる
			if (player[p].form == Form::Third && Keyboard_IsKeyDownTrigger(specialKeys[p]))	player[p].useSpecial = true;
			
			// ボタン入力をチェックしてスペシャル使用フラグを立てる
			if (player[p].form == Form::Third && g_Input[p].ZR)	player[p].useSpecial = true;

			// フラグが立ったら更新処理を呼び出す
			if (player[p].isAttacking)	Attack_Update(p);	AttackPlayerCollisions();	// 攻撃
			if (player[p].useSkill)		Skill_Update(p);								// スキル
			if (player[p].useSpecial)	Special_Update(p);								// スペシャル

			// 現在のプレイヤー p の移動ベクトルだけをリセット
			player[p].moveDir = { 0.0f, 0.0f, 0.0f };
			
			XMFLOAT2 moveInput = { 0.0f, 0.0f };

			// スペシャル コンクリート使用中は移動不可
			if (player[p].useSpecial && player[p].type == PlayerType::Concrete)
			{
				player[p].moveDir = { 0.0f, 0.0f, 0.0f };
				player[p].isMoving = false;
			}
			// スペシャル コンクリート使用中でなければ移動処理
			else
			{
				player[p].moveInput2D = { 0.0f, 0.0f };

				if (p == 0) // プレイヤー0 (WASD) 攻撃 Space
				{
					if (g_Input[0].LStickX > 0.0f) { moveInput.x += 1.0f; player[0].isMoving = true; }
					if (g_Input[0].LStickX < 0.0f) { moveInput.x -= 1.0f; player[0].isMoving = true; }
					if (g_Input[0].LStickY < 0.0f) { moveInput.y += 1.0f; player[0].isMoving = true; }
					if (g_Input[0].LStickY > 0.0f) { moveInput.y -= 1.0f; player[0].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_W))  { moveInput.y += 1.0f; player[0].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_S))  { moveInput.y -= 1.0f; player[0].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_A))  { moveInput.x -= 1.0f; player[0].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_D))  { moveInput.x += 1.0f; player[0].isMoving = true; }
					if (moveInput.x == 0.0f && moveInput.y == 0.0f)	player[0].isMoving = false;
				}
				else if (p == 1) // プレイヤー1 (矢印キー) 攻撃 Enter
				{
					if (g_Input[1].LStickX > 0.0f)	  { moveInput.x += 1.0f; player[1].isMoving = true; }
					if (g_Input[1].LStickX < 0.0f)	  { moveInput.x -= 1.0f; player[1].isMoving = true; }
					if (g_Input[1].LStickY < 0.0f)	  { moveInput.y += 1.0f; player[1].isMoving = true; }
					if (g_Input[1].LStickY > 0.0f)	  { moveInput.y -= 1.0f; player[1].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_UP))	  { moveInput.y += 1.0f; player[1].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_DOWN))  { moveInput.y -= 1.0f; player[1].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_LEFT))  { moveInput.x -= 1.0f; player[1].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_RIGHT)) { moveInput.x += 1.0f; player[1].isMoving = true; }
					if (moveInput.x == 0.0f && moveInput.y == 0.0f)	player[1].isMoving = false;
				}
				else if (p == 2) // プレイヤー2 (TFGH) 攻撃 V
				{
					if (g_Input[2].LStickX > 0.0f) { moveInput.x += 1.0f; player[2].isMoving = true; }
					if (g_Input[2].LStickX < 0.0f) { moveInput.x -= 1.0f; player[2].isMoving = true; }
					if (g_Input[2].LStickY < 0.0f) { moveInput.y += 1.0f; player[2].isMoving = true; }
					if (g_Input[2].LStickY > 0.0f) { moveInput.y -= 1.0f; player[2].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_T))  { moveInput.y += 1.0f; player[2].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_G))  { moveInput.y -= 1.0f; player[2].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_F))  { moveInput.x -= 1.0f; player[2].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_H))  { moveInput.x += 1.0f; player[2].isMoving = true; }
					if (moveInput.x == 0.0f && moveInput.y == 0.0f)	player[2].isMoving = false;
				}
				if (p == 3) // プレイヤー3 (WASD) 攻撃 Space
				{
					if (g_Input[3].LStickX > 0.0f) { moveInput.x += 1.0f; player[3].isMoving = true; }
					if (g_Input[3].LStickX < 0.0f) { moveInput.x -= 1.0f; player[3].isMoving = true; }
					if (g_Input[3].LStickY > 0.0f) { moveInput.y += 1.0f; player[3].isMoving = true; }
					if (g_Input[3].LStickY < 0.0f) { moveInput.y -= 1.0f; player[3].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_W))  { moveInput.y += 1.0f; player[3].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_S))  { moveInput.y -= 1.0f; player[3].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_A))  { moveInput.x -= 1.0f; player[3].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_D))  { moveInput.x += 1.0f; player[3].isMoving = true; }
					if (moveInput.x == 0.0f && moveInput.y == 0.0f)	player[3].isMoving = false;
				}
				// 入力を保存（プレイヤーの向き用）
				player[p].moveInput2D = moveInput;

				// 移動はカメラ基準をワールドにする
				player[p].moveDir = ToWorldMoveDirByCamera(moveInput);
			}

			// 現在のプレイヤーpだけを動かす
			Move(player[p], player[p].moveDir);

			// 移動中ならlastDirを更新
			if (player[p].isMoving)
			{
				float dx = player[p].moveInput2D.x;
				float dz = player[p].moveInput2D.y;

				     if (dx < 0.0f && dz < 0.0f) player[p].lastDir = PlayerDir::Down_Left;
				else if (dx < 0.0f && dz > 0.0f) player[p].lastDir = PlayerDir::Up_Left;
				else if (dx > 0.0f && dz > 0.0f) player[p].lastDir = PlayerDir::Up_Right;
				else if (dx > 0.0f && dz < 0.0f) player[p].lastDir = PlayerDir::Down_Right;
				else if (dz < 0.0f)              player[p].lastDir = PlayerDir::Down;
				else if (dx < 0.0f)              player[p].lastDir = PlayerDir::Left;
				else if (dz > 0.0f)              player[p].lastDir = PlayerDir::Up;
				else if (dx > 0.0f)              player[p].lastDir = PlayerDir::Right;
			}
		}

		// プレイヤーごとのスキルクールタイムを毎フレーム減算
		if (player[p].skillCoolTimer > 0.0f)
		{
			player[p].skillCoolTimer -= DELTA_TIME;
			if (player[p].skillCoolTimer < 0.0f) player[p].skillCoolTimer = 0.0f;
		}

		// HPが0以下の処理
		if (player[p].hp <= 0.0f && player[p].active && !player[p].isDown)
		{
			// ダウン状態に移行してタイマーをリセット
			player[p].isDown = true;
			player[p].downTimer = 0.0f;
			Effect_ClearUI(p);
		}

		// ダウン状態のタイマー更新とリスポーン判定
		if (player[p].isDown)
		{
			// 行動停止
			player[p].moveDir = { 0.0f, 0.0f, 0.0f };
			player[p].isAttacking = false;
			player[p].useSkill = false;
			player[p].useSpecial = false;

			// ダウンタイマー更新
			player[p].downTimer += DELTA_TIME;

			// プレイヤー毎のダウン時間が経過したらリスポーン処理
			if (player[p].downTimer >= DOWN_TIME)
			{
				// 残機を1つ減らす
				player[p].stock -= 1;

				if (player[p].stock > 0)	Player_Respawn(p);
				else
				{
					// 残機無しで復活なし
					player[p].active = false;
					player[p].isDown = false;
					player[p].downTimer = 0.0f;

					// 順位登録（内部で重複登録を防止）
					Ranking(p);
				}
			}
		}

		// 落下処理 影エフェクト非表示
		if (player[p].position.y < -1.0f)
		{
			player[p].isShadowEnabled = false;
		}

		if (player[p].active && player[p].position.y <= -10.0f)
		{
			Effect_ClearUI(p);
			// 残機を一つ減らす
			player[p].stock -= 1;

			// リスポーン（位置・ステートリセット）
			if (player[p].stock > 0)	Player_Respawn(p);
			else
			{
				// 残機無しで完全に非アクティブ化
				player[p].active = false;
				// 順位登録
				Ranking(p);
			}
		}

		// ダメージを受けた時の処理
		if (player[p].isAttacked)
		{
			// ダメージタイマー更新
			player[p].attackedTimer += DELTA_TIME;

			// プレイヤー毎のダメージ時間が経過したらダメージ終了
			if (player[p].attackedTimer >= ATTACKED_TIME)
			{
				player[p].isAttacked = false;
				player[p].attackedTimer = 0.0f;
			}
		}

		// 進化時の無敵処理
		if (player[p].isInvincible)
		{
			// 無敵タイマー更新
			player[p].invincibleTimer += DELTA_TIME;

			// プレイヤー毎の無敵時間が経過したら無敵終了
			if (player[p].invincibleTimer >= EVOLVING_TIME)
			{
				player[p].isInvincible = false;
				player[p].invincibleTimer = 0.0f;
			}
		}

		// スペシャル開始時のフレーム初期化（アニメーション更新タイミングに依存しない）
		if (player[p].useSpecial && !g_specialAnimStarted[p])
		{
			int type = -1;
				 if(player[p].type == PlayerType::Concrete)		type = 0;
			else if(player[p].type == PlayerType::Electricity)	type = 1;
			else if(player[p].type == PlayerType::Glass)		type = 2;
			else if(player[p].type == PlayerType::Plant)		type = 3;

			int start = type * 64;
				 if (player[p].lastDir == PlayerDir::Down)		start += 0;
			else if (player[p].lastDir == PlayerDir::Down_Left)	start += 8;
			else if (player[p].lastDir == PlayerDir::Left)		start += 16;
			else if (player[p].lastDir == PlayerDir::Up_Left)	start += 24;
			else if (player[p].lastDir == PlayerDir::Up)		start += 32;
			else if (player[p].lastDir == PlayerDir::Up_Right)	start += 40;
			else if (player[p].lastDir == PlayerDir::Right)		start += 48;
			else if (player[p].lastDir == PlayerDir::Down_Right)start += 56;

			g_animFrame[p] = start;
			g_specialAnimStarted[p] = true;
		}
		// スペシャル終了時のフレームリセット
		if (!player[p].useSpecial && g_specialAnimStarted[p])
		{
			g_specialAnimStarted[p] = false;

			// 通常テクスチャの待機アニメーション開始フレームにリセット
			int start = 0;
				 if (player[p].lastDir == PlayerDir::Down)		start = 0;
			else if (player[p].lastDir == PlayerDir::Down_Left)	start = 26;
			else if (player[p].lastDir == PlayerDir::Left)		start = 52;
			else if (player[p].lastDir == PlayerDir::Up_Left)	start = 78;
			else if (player[p].lastDir == PlayerDir::Up)		start = 104;
			else if (player[p].lastDir == PlayerDir::Up_Right)	start = 130;
			else if (player[p].lastDir == PlayerDir::Right)		start = 156;
			else if (player[p].lastDir == PlayerDir::Down_Right)start = 182;

			g_animFrame[p] = start;
		}

		// プレイヤー アニメーション更新
		g_animTimer[p] += DELTA_TIME;

		// エフェクト アニメーション
		Effect_UpdateForPlayer(p);

		if (g_animTimer[p] >= ANIM_FRAME_TIME)
		{
			int advance = (int)(g_animTimer[p] / ANIM_FRAME_TIME);
			g_animTimer[p] -= advance * ANIM_FRAME_TIME;

			// 勝利 第1形態 13コマ(ラスト5コマ ループ) 第2形態 20コマ(ラスト9コマ ループ) 第3形態 21コマ(ラストコマ ループ)
			//if (Keyboard_IsKeyDown(KK_TAB) || g_victoryState[p] != 0)
			if (player[p].rank == 1 || g_victoryState[p] != 0)
			{
				//if (Keyboard_IsKeyDown(KK_TAB) && g_victoryState[p] == 0)
				if (player[p].rank == 1 && g_victoryState[p] == 0)
				{
					g_victoryState[p] = 1;
					g_animFrame[p] = 208;	// 初回再生開始フレーム
				}

				if (g_victoryState[p] == 1)
				{
					// 初回再生 フレームを単純増加
					g_animFrame[p] += advance;

					// 第1形態 220 を表示した後にループ領域へ移行する
					if (g_animFrame[p] > 220 && player[p].form == Form::First)
					{
						g_victoryState[p] = 2;
						g_animFrame[p] = 216;	// ループ開始フレーム
					}
					// 第2形態 227 を表示した後にループ領域へ移行する
					if (g_animFrame[p] > 227 && player[p].form == Form::Second)
					{
						g_victoryState[p] = 2;
						g_animFrame[p] = 219;	// ループ開始フレーム
					}
					// 第3形態 228 を表示した後にループ領域へ移行する 229コマ目は使用しない
					if (g_animFrame[p] > 228 && player[p].form == Form::Third)
					{
						g_victoryState[p] = 2;
						g_animFrame[p] = 221;	// ループ開始フレーム
					}
				}
				else if (g_victoryState[p] == 2)
				{
					switch (player[p].form)
					{
					case Form::First:	LoopRange(g_animFrame[p], 216, 5, advance);	// 第1形態 216～220をループ
						break;
					case Form::Second:	LoopRange(g_animFrame[p], 219, 9, advance);	// 第2形態 219～227をループ
						break;
					case Form::Third:	LoopRange(g_animFrame[p], 221, 8, advance);	// 第3形態 221～228をループ 229コマ目は使用しない
						break;
					}
				}
			}
			// ダウン 5コマ (ダメージ 2コマ + ダウン 3コマ) 最終コマで停止
			else if (player[p].isDown == true)
			{
				// 向きに応じた開始フレームを決定
				int start = 15; // デフォルト（Down）
					 if (player[p].lastDir == PlayerDir::Down)		 start = 15;
				else if (player[p].lastDir == PlayerDir::Down_Left)	 start = 41;
				else if (player[p].lastDir == PlayerDir::Left)		 start = 67;
				else if (player[p].lastDir == PlayerDir::Up_Left)	 start = 93;
				else if (player[p].lastDir == PlayerDir::Up)		 start = 119;
				else if (player[p].lastDir == PlayerDir::Up_Right)	 start = 145;
				else if (player[p].lastDir == PlayerDir::Right)		 start = 171;
				else if (player[p].lastDir == PlayerDir::Down_Right) start = 197;

				const int count = 5;
				const int lastFrame = start + count - 1;

				// advance に対応する経過秒（g_animTimerでまとめて進めた分）
				float elapsedSec = (float)advance * ANIM_FRAME_TIME;

				// フレームが範囲外なら開始フレームに補正しタイマーリセット
				if (g_animFrame[p] < start || g_animFrame[p] > lastFrame)
				{
					g_animFrame[p] = start;
					g_downHoldTimer[p] = 0.0f;
				}

				// 最終フレーム以外なら第1形態進行（ループ）
				if (g_animFrame[p] != lastFrame)
				{
					LoopRange(g_animFrame[p], start, count, advance);
					g_downHoldTimer[p] = 0.0f; // 到達前はホールドタイマーをリセット
				}
				else
				{
					// 最終フレームに到達 ホールドを進める
					g_downHoldTimer[p] += elapsedSec;

					// ホールドが満了したら次に進める（ここでは1フレーム分だけ進める）
					if (g_downHoldTimer[p] >= DOWN_TIME)
					{
						g_downHoldTimer[p] = 0.0f;
						// 1フレーム分進める（ループにより start に戻る）
						LoopRange(g_animFrame[p], start, count, 1);
					}
				}
			}
			// スペシャル 8コマ
			else if (player[p].useSpecial)
			{
				int type = -1;
					 if(player[p].type == PlayerType::Concrete)		type = 0;
				else if(player[p].type == PlayerType::Electricity)	type = 1;
				else if(player[p].type == PlayerType::Glass)		type = 2;
				else if(player[p].type == PlayerType::Plant)		type = 3;

				// 向きに応じた開始フレームを決定
				int start = type * 64;
					 if (player[p].lastDir == PlayerDir::Down)		start += 0;
				else if (player[p].lastDir == PlayerDir::Down_Left)	start += 8;
				else if (player[p].lastDir == PlayerDir::Left)		start += 16;
				else if (player[p].lastDir == PlayerDir::Up_Left)	start += 24;
				else if (player[p].lastDir == PlayerDir::Up)		start += 32;
				else if (player[p].lastDir == PlayerDir::Up_Right)	start += 40;
				else if (player[p].lastDir == PlayerDir::Right)		start += 48;
				else if (player[p].lastDir == PlayerDir::Down_Right)start += 56;

				const int count = 8;

				LoopRange(g_animFrame[p], start, count, advance);
			}
			// ダメージ 3コマ
			else if (player[p].isAttacked == true || player[p].isStunning)
			{
					 if (player[p].lastDir == PlayerDir::Down)		LoopRange(g_animFrame[p],  14, 3, advance);	//  下   14～16 
				else if (player[p].lastDir == PlayerDir::Down_Left)	LoopRange(g_animFrame[p],  40, 3, advance);	// 左下  40～42
				else if (player[p].lastDir == PlayerDir::Left)		LoopRange(g_animFrame[p],  66, 3, advance);	//  左   66～68
				else if (player[p].lastDir == PlayerDir::Up_Left)	LoopRange(g_animFrame[p],  92, 3, advance);	// 左上  92～94
				else if (player[p].lastDir == PlayerDir::Up)		LoopRange(g_animFrame[p], 118, 3, advance);	//  上  118～120
				else if (player[p].lastDir == PlayerDir::Up_Right)	LoopRange(g_animFrame[p], 144, 3, advance);	// 右上 144～146
				else if (player[p].lastDir == PlayerDir::Right)		LoopRange(g_animFrame[p], 170, 3, advance);	//  右  170～172
				else if (player[p].lastDir == PlayerDir::Down_Right)LoopRange(g_animFrame[p], 196, 3, advance);	// 右下 196～198
			}
			// 攻撃 6コマ
			else if (player[p].isAttacking == true)
			{
					 if (player[p].lastDir == PlayerDir::Down)		LoopRange(g_animFrame[p],  20, 6, advance);	//  下   20～25
				else if (player[p].lastDir == PlayerDir::Down_Left)	LoopRange(g_animFrame[p],  46, 6, advance);	// 左下  46～51
				else if (player[p].lastDir == PlayerDir::Left)		LoopRange(g_animFrame[p],  72, 6, advance);	//  左   72～77
				else if (player[p].lastDir == PlayerDir::Up_Left)	LoopRange(g_animFrame[p],  98, 6, advance);	// 左上  98～103
				else if (player[p].lastDir == PlayerDir::Up)		LoopRange(g_animFrame[p], 124, 6, advance);	//  上  124～129
				else if (player[p].lastDir == PlayerDir::Up_Right)	LoopRange(g_animFrame[p], 150, 6, advance);	// 右上 150～155
				else if (player[p].lastDir == PlayerDir::Right)		LoopRange(g_animFrame[p], 176, 6, advance);	//  右  176～181
				else if (player[p].lastDir == PlayerDir::Down_Right)LoopRange(g_animFrame[p], 202, 6, advance);	// 右下 202～207
			}
			// 移動 8コマ （リスポーン中を除く）
			else if (!player[p].duringRespawn && player[p].isMoving == true)
			{
				float dx = player[p].moveInput2D.x;
				float dz = player[p].moveInput2D.y;

					 if (dx < 0.0f && dz < 0.0f) LoopRange(g_animFrame[p], 32, 8, advance);
				else if (dx < 0.0f && dz > 0.0f) LoopRange(g_animFrame[p], 84, 8, advance);
				else if (dx > 0.0f && dz > 0.0f) LoopRange(g_animFrame[p], 136, 8, advance);
				else if (dx > 0.0f && dz < 0.0f) LoopRange(g_animFrame[p], 188, 8, advance);
				else if (dz < 0.0f)              LoopRange(g_animFrame[p], 6, 8, advance);
				else if (dx < 0.0f)              LoopRange(g_animFrame[p], 58, 8, advance);
				else if (dz > 0.0f)              LoopRange(g_animFrame[p], 110, 8, advance);
				else if (dx > 0.0f)              LoopRange(g_animFrame[p], 162, 8, advance);
			}
			// 待機 6コマ
			else if (player[p].isMoving == false)
			{
					 if (player[p].lastDir == PlayerDir::Down)		LoopRange(g_animFrame[p],   0, 6, advance);	//  下    0～5
				else if (player[p].lastDir == PlayerDir::Down_Left)	LoopRange(g_animFrame[p],  26, 6, advance);	// 左下  26～31
				else if (player[p].lastDir == PlayerDir::Left)		LoopRange(g_animFrame[p],  52, 6, advance);	//  左   52～57
				else if (player[p].lastDir == PlayerDir::Up_Left)	LoopRange(g_animFrame[p],  78, 6, advance);	// 左上  78～83 
				else if (player[p].lastDir == PlayerDir::Up)		LoopRange(g_animFrame[p], 104, 6, advance);	//  上  104～109
				else if (player[p].lastDir == PlayerDir::Up_Right)	LoopRange(g_animFrame[p], 130, 6, advance);	// 右上 130～135
				else if (player[p].lastDir == PlayerDir::Right)		LoopRange(g_animFrame[p], 156, 6, advance);	//  右  156～161
				else if (player[p].lastDir == PlayerDir::Down_Right)LoopRange(g_animFrame[p], 182, 6, advance);	// 右下 182～187		
			}
		}



	
		static XMFLOAT3 posBuff = player[p].position;	// デバッグ表示座標

		// 描画で使っているスプライト倍率と同じ値を物理にも使う
		const float renderScale = 2.0f;	// Draw 側の spriteScale に合わせる
		// 描画スケールを反映したスケール（表示用）
		XMFLOAT3 physicsScaling = XMFLOAT3(player[p].scaling.x * renderScale, player[p].scaling.y * renderScale, player[p].scaling.z * renderScale);

		// --- プレイヤー用ヒットボックス比率（向きで長短を切り替える） ---
		// 高さは固定、水平面は向きに応じて長短を切り替える
		const float HITBOX_HEIGHT_SCALE = 1.0f;
		const float HITBOX_SHORT = 0.35f;	// 向きと直交する短辺
		const float HITBOX_LONG  = 0.65f;	// 向きに沿った長辺

		// 回転から前方ベクトルを算出して、どちらの軸が優勢か判定する
		float radFacing = XMConvertToRadians(player[p].rotation.y);
		float facingX = sinf(radFacing);
		float facingZ = cosf(radFacing);
		bool facingZDominant = fabsf(facingZ) >= fabsf(facingX);

		float widthScale  = facingZDominant ? HITBOX_SHORT : HITBOX_LONG;	// X方向スケール
		float depthScale  = facingZDominant ? HITBOX_LONG  : HITBOX_SHORT;	// Z方向スケール

		// 第2形態 第3形態はXとZ同じにする
		if (player[p].form == Form::Second || player[p].form == Form::Third)
		{
			widthScale = 0.25f;
			depthScale = 0.25f;
		}

		XMFLOAT3 hitboxScaling = XMFLOAT3
		(
			player[p].scaling.x * renderScale * widthScale,
			player[p].scaling.y * renderScale * HITBOX_HEIGHT_SCALE,
			player[p].scaling.z * renderScale * depthScale
		);

		// AABB を現在の位置・スケール（ヒットボックス）で更新しておく（衝突判定で使用）
		CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);

		posBuff = player[p].position;

		// 地面の高さ（最低ライン）
		//float groundHeight = -10.0f;	// 奈落の底
		//bool isShadowEnabled = false;		// 地面に足がついているかフラグ

		// マップデータ（地面）との当たり判定
		int fieldCount = GetFieldObjectCount();
		MAPDATA* fieldObjects = GetFieldObjects();

		for (int j = 0; j < fieldCount; ++j)
		{
			// アクティブじゃない、または no が MAX ならスキップ
			if (!fieldObjects[j].isActive || fieldObjects[j].no == FIELD::FIELD_MAX)
			{
				continue;
			}

			// --- 六角柱コライダーの準備 ---
			HexCollider hex;
			hex.center = fieldObjects[j].pos;		// -1
			hex.radius = fieldObjects[j].radius;	// 1
			hex.height = fieldObjects[j].height;	// 3.0

			// プレイヤーのAABB（体の一部）が六角柱に乗っているか
			if (CheckAABBHexCollision(player[p].boundingBox, hex))
			{
				// タイルの上面のY座標を計算
				float tileTopY = fieldObjects[j].pos.y + (hex.height / 2.0f);	// -1 + 1.5 = 0.5

				// プレイヤーの底面がタイルの上面以下か
				if (player[p].boundingBox.Min.y <= tileTopY)
				{
					const float baseHalfHeight = COORDINATE;
					// 着地では見た目の高さ（描画スケール）を基準に計算しているため physicsScaling を使用
					float halfHeight = baseHalfHeight * player[p].scaling.y * renderScale;

					// 着地させる（めり込みが起きないよう最低値として補正）
					float targetY = tileTopY + halfHeight;
					if (player[p].position.y < targetY)
					{
						player[p].position.y = targetY;
						player[p].isShadowEnabled = true; // 影エフェクト非表示
					}

					// AABB を再計算して整合性を保つ（描画スケールを考慮）
					// ヒットボックス（向きに応じた長方形）で再計算する
					CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);

					top_y = tileTopY;

					break;
				}
			}
		}

		// -------------------------------------------------------------------------------------
		// 建物との当たり判定
		// -------------------------------------------------------------------------------------
		int buildingCount = GetBuildingCount();			// 数を取得
		Building** buildingObjects = GetBuildings();	// リストを取得

		for (int j = 0; j < buildingCount; ++j)
		{
			// アクティブでないなら無視
			if (!buildingObjects[j]->isActive)	continue;

			// y座標の調整
			// Building::Draw() で position.y + 1.0f しているので、判定用の座標も合わせる
			XMFLOAT3 colliderPos = buildingObjects[j]->position;
			colliderPos.y += 1.0f;

			// コライダーの作成と更新（補正した座標 colliderPos を使う）
			CalculateAABB(buildingObjects[j]->boundingBox, colliderPos, buildingObjects[j]->scaling);

			// プレイヤー と 建物の当たり判定
			MTV collision = CalculateAABBMTV(player[p].boundingBox, buildingObjects[j]->boundingBox);

			if (collision.isColliding)
			{
				// 衝突していたら、MTVの分だけ位置を戻す
				player[p].position.x += collision.translation.x;
				player[p].position.y += collision.translation.y;
				player[p].position.z += collision.translation.z;

				// 押し戻し後の新しいAABBを再計算（描画スケールを反映）
				// ヒットボックス（向きに応じた長方形）で再計算する
				CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);
			}
		}

		// プレイヤーに対応する攻撃オブジェクトを PLAYER_MAX 分ループしてスケーリング同期
		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			ATTACK_OBJECT* attackObject = GetAttack(p); // GetAttack は 1-based
			if (attackObject == nullptr) continue;

			// プレイヤー側のスケールに合わせる（攻撃オブジェクトは半分）
			attackObject->scaling.x = player[p].scaling.x * 0.5f;
			attackObject->scaling.y = player[p].scaling.y * 0.5f;
			attackObject->scaling.z = player[p].scaling.z * 0.5f;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////

		// -------------------------------------------------------------
		// プレイヤーオブジェクト同士の当たり判定（PLAYER_MAX分対応）
		// -------------------------------------------------------------
		for (int otherIndex = p + 1; otherIndex < PLAYER_MAX; ++otherIndex)
		{
			// 非アクティブは無視
			if (!player[otherIndex].active) continue;

			// 他プレイヤーのヒットボックススケーリング（向きで長短を切り替える）
			const float HITBOX_HEIGHT_SCALE = 1.0f;
			const float HITBOX_SHORT = 0.35f;
			const float HITBOX_LONG  = 0.65f;

			// 宣言をループスコープの先頭に置く（後で再利用するため）
			XMFLOAT3 hitboxScalingOther;

			{
				float radOther = XMConvertToRadians(player[otherIndex].rotation.y);
				float otherFacingX = sinf(radOther);
				float otherFacingZ = cosf(radOther);
				bool otherFacingZDominant = fabsf(otherFacingZ) >= fabsf(otherFacingX);

				float otherWidthScale = otherFacingZDominant ? HITBOX_SHORT : HITBOX_LONG;
				float otherDepthScale = otherFacingZDominant ? HITBOX_LONG  : HITBOX_SHORT;

				// 第2形態 第3形態はXとZ同じにする
				if (player[otherIndex].form == Form::Second || player[otherIndex].form == Form::Third)
				{
					widthScale = 0.25f;
					depthScale = 0.25f;
				}

				hitboxScalingOther = XMFLOAT3(
					player[otherIndex].scaling.x * renderScale * otherWidthScale,
					player[otherIndex].scaling.y * renderScale * HITBOX_HEIGHT_SCALE,
					player[otherIndex].scaling.z * renderScale * otherDepthScale
				);
			}

			// 他プレイヤーの AABB を更新（ここで定義済みの hitboxScalingOther を使用）
			CalculateAABB(player[otherIndex].boundingBox, player[otherIndex].position, hitboxScalingOther);

			// 衝突チェック（ペア p <-> otherIndex を一度だけ判定）
			MTV collision_player = CalculateAABBMTV(player[p].boundingBox, player[otherIndex].boundingBox);

			if (collision_player.isColliding)
			{
				// 向きベクトルを更新（rotation.y から算出）
				{
					float rad_p = XMConvertToRadians(player[p].rotation.y);
					player[p].dir.x = sinf(rad_p);
					player[p].dir.z = cosf(rad_p);
				}
				{
					float rad_o = XMConvertToRadians(player[otherIndex].rotation.y);
					player[otherIndex].dir.x = sinf(rad_o);
					player[otherIndex].dir.z = cosf(rad_o);
				}

				// 押し戻し量 (MTV) を半分にして双方に適用
				XMFLOAT3 half_translation =
				{
					collision_player.translation.x * 0.5f,
					collision_player.translation.y * 0.5f,
					collision_player.translation.z * 0.5f
				};

				// object[p] を MTV の半分だけ押す
				player[p].position.x += half_translation.x;
				player[p].position.y += half_translation.y;
				player[p].position.z += half_translation.z;

				// object[otherIndex] を逆方向に半分だけ押す
				player[otherIndex].position.x -= half_translation.x;
				player[otherIndex].position.y -= half_translation.y;
				player[otherIndex].position.z -= half_translation.z;

				// 押し戻し後の新しいAABBを再計算 (ヒットボックスで)
				CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);
				CalculateAABB(player[otherIndex].boundingBox, player[otherIndex].position, hitboxScalingOther);
			}
		}

		SetHPValue(&HPBar[p], (int)player[p].hp, (int)PLAYER_MAX_HP);
		UpdateHP(&HPBar[p]);
	}
	ImGui::End();
}

//======================================================
//	描画関数
//======================================================
void Player_Draw(bool s_IsKonamiCodeEntered)
{
	// 攻撃・スキル・スペシャル描画
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (player[p].active && player[p].isAttacking)	Attack_Draw(p);
		//if (player[p].active && player[p].useSkill)		Skill_Draw(p);
		if (player[p].active && player[p].useSpecial)	Special_Draw(p);
	}

	LIGHT light{};
	light.Enable = TRUE;
	// 光の向き（ワールド空間）シェーダー側で単位化して使っている想定
	light.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
	// 拡散光と環境光
	light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	Shader_SetLight(light);

	static bool input1 = false;
	// デバッグモード中のみキー入力を受け付ける
	if (s_IsKonamiCodeEntered)
	{
		if (Keyboard_IsKeyDownTrigger(KK_D1)) input1 = !input1;	// フラグ反転
	}
	
	Shader_Begin(); 

	// ========================================================
	// 奥のプレイヤーが手前のプレイヤーに隠れないように描画
	// ========================================================

	// プロジェクション・ビュー行列を先に取得
	XMMATRIX projection = GetProjectionMatrix();
	XMMATRIX view = GetViewMatrix();

	// カメラ位置を算出（View の逆行列の r[3] がワールド空間のカメラ位置）
	XMMATRIX invView = XMMatrixInverse(nullptr, view);
	XMFLOAT3 camPos;
	camPos.x = invView.r[3].m128_f32[0];
	camPos.y = invView.r[3].m128_f32[1];
	camPos.z = invView.r[3].m128_f32[2];

	// プレイヤーを描画するラムダ（Projection, View をキャプチャ）
	auto DrawPlayerInternal = [&](int idx)
	{
		if (!player[idx].active) return;

		// プレイヤーの影エフェクト描画
		EffectShadow_DrawForPlayer(idx);

		const float spriteScale = 3.5f;	// 表示倍率

		// ワールド行列（ビルボード風の既存ロジックを踏襲）
		XMMATRIX ScalingMatrix = XMMatrixScaling(
			player[idx].scaling.x * spriteScale,
			player[idx].scaling.y * spriteScale,
			player[idx].scaling.z * spriteScale
		);

		XMMATRIX vm = GetViewMatrix();	// カメラの行列
		vm.r[3].m128_f32[0] = 0.0f;
		vm.r[3].m128_f32[1] = 0.0f;
		vm.r[3].m128_f32[2] = 0.0f;
		vm.r[3].m128_f32[3] = 1.0f;
		vm = XMMatrixTranspose(vm);
		vm.r[3].m128_f32[0] = player[idx].position.x;
		vm.r[3].m128_f32[1] = player[idx].position.y;
		vm.r[3].m128_f32[2] = player[idx].position.z;
		vm.r[3].m128_f32[3] = 1.0f;

		// World 行列（ビルボード用）をシェーダーに渡す
		XMMATRIX WorldMatrix = ScalingMatrix * vm;
		Shader_SetWorldMatrix(WorldMatrix);

		XMMATRIX WVP = ScalingMatrix * vm * view * projection;

		Shader_SetMatrix(WVP);
		Shader_Begin();
		SetBlendState(BLENDSTATE_ALPHA);

		// 頂点バッファにデータコピー（フレームに応じてUVを書き換える）
		D3D11_MAPPED_SUBRESOURCE msr;

		// コピー元のvdata をローカル配列にコピーして UV を調整
		Vertex2 localV[PLAYER_VERTEX];
		CopyMemory(&localV[0], &vdata[0], sizeof(Vertex2) * PLAYER_VERTEX);

		// 現在のフレームから UV を計算
		int frame = g_animFrame[idx];
		int col = frame % SHEET_COLS;
		int row = frame / SHEET_COLS;
		float u0 = (float)col / (float)SHEET_COLS;
		float v0 = (float)row / (float)SHEET_ROWS;
		float u1 = u0 + 1.0f / (float)SHEET_COLS;
		float v1 = v0 + 1.0f / (float)SHEET_ROWS;

		// 頂点のテクスチャ座標を上書き
		localV[0].tex = XMFLOAT2(u0, v0);	// LEFT-TOP
		localV[1].tex = XMFLOAT2(u1, v0);	// RIGHT-TOP
		localV[2].tex = XMFLOAT2(u0, v1);	// LEFT-BOTTOM
		localV[3].tex = XMFLOAT2(u1, v1);	// RIGHT-BOTTOM
		
		// バッファへ書き込み
		g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		Vertex2* vertex = (Vertex2*)msr.pData;
		CopyMemory(vertex, &localV[0], sizeof(Vertex2) * PLAYER_VERTEX);
		g_pContext->Unmap(g_VertexBuffer, 0);

		ID3D11ShaderResourceView* srv = nullptr;

		// 形態とタイプに応じたテクスチャを設定
		switch (player[idx].form)
		{
		// 第1形態
		case Form::First:
				 if(idx == 0)	{ srv = g_Texture[0];	break; }
			else if(idx == 1)	{ srv = g_Texture[1];	break; }
			else if(idx == 2)	{ srv = g_Texture[2];	break; }
			else if(idx == 3)	{ srv = g_Texture[3];	break; }
			// 第2形態
		case Form::Second:
			switch (player[idx].type)
			{
			case PlayerType::Glass:			srv = g_Texture[4];	break;				
			case PlayerType::Concrete:		srv = g_Texture[5];	break;
			case PlayerType::Plant:			srv = g_Texture[6];	break;
			case PlayerType::Electricity:	srv = g_Texture[7];	break;
			default: break;
			}
			break;
		// 第3形態
		case Form::Third:
			switch (player[idx].type)
			{
			case PlayerType::Glass:			srv = g_Texture[8];		break;
			case PlayerType::Concrete:		srv = g_Texture[9];		break;
			case PlayerType::Plant:			srv = g_Texture[10];	break;
			case PlayerType::Electricity:	srv = g_Texture[11];	break;
			default: break;
			}
			break;
		default: break;
		}

		// スペシャル使用中は専用テクスチャ
		if (player[idx].useSpecial)			srv = g_Texture[12];

		g_pContext->PSSetShaderResources(0, 1, &srv);

		// プレイヤーごとに異なる色を設定
		if (player[idx].isPoisoned)
		{
			switch (idx)
			{
			// Lerp = 1.乗算色 2.補間する色 3.補間の度合い
			case 0:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			case 1:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			case 2:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			case 3:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			default:	Shader_SetColor(color::white); break;
			}
		}
		else			Shader_SetColor(color::white); // 通常色

		// バッファセット & 描画
		UINT stride = sizeof(Vertex2);
		UINT offset = 0;
		g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
		g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		g_pContext->DrawIndexed(6, 0, 0);

		// エフェクト描画 （プレイヤーの手前）
		EffectFront_DrawForPlayer(idx);
	};

	// -----------------------------------
	// 透明描画のためのソート（遠い順）
	// -----------------------------------
	std::vector<std::pair<float, int>> list;	// (距離二乗, index)
	list.reserve(PLAYER_MAX);

	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (!player[p].active) continue;

		float dx = player[p].position.x - camPos.x;
		float dy = player[p].position.y - camPos.y;
		float dz = player[p].position.z - camPos.z;
		float dist2 = dx * dx + dy * dy + dz * dz;
		list.emplace_back(dist2, p);
	}

	// 遠い順（大きい順）にソート
	std::sort(list.begin(), list.end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
		{
		return a.first > b.first;
		});

	// 透過レンダリング：深度テストは有効、深度書き込みは無効（SetDepthReadOnly を使用）
	SetDepthTest(true);
	SetDepthReadOnly();	// 深度テストはするが深度バッファへの書き込みはしない

	// ソート順（遠いものから描画）
	for (auto& p : list)	DrawPlayerInternal(p.second);

	// 3Dオブジェクトは深度テストを無効にして描画
	SetDepthTest(false);

	if (s_IsKonamiCodeEntered)
	{
		// ------------------------------------
		// コライダーフレーム（AABB）の描画
		// ------------------------------------
		{
			// プレイヤーの描画に使われた行列をクリアする
			XMMATRIX world = XMMatrixIdentity();
			 Shader_SetMatrix(world * GetViewMatrix() * GetProjectionMatrix()); // WVP行列をIdentity * View * Projectionに設定
			//Shader_Begin(); // シェーダーを再設定

			for (int i = 0; i < PLAYER_MAX; i++)
			{
				if (!player[i].active) continue;

				// AABBを描画
				// AABBのMin/Maxは既にワールド座標なので、行列はリセットしたまま描写すればOK
				Debug_DrawAABB(player[i].boundingBox, XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));
			}
		}
		//s_IsKonamiCodeEntered = false;
	}
}


void Player_DrawHP()
{
	Shader_Begin();

	// 個別UIステータス描画
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		SetBlendState(BLENDSTATE_ALPHA);

		DrawHP(&HPBar[i], i + 2);
		XMFLOAT2 hp = HPBar[i].pos;

		// スキルゲージ表示用の値を計算する
		float skillFill = 1.0f;

		// スキル未所持なら0
		if (player[i].type == PlayerType::None)
		{
			skillFill = 0.0f;
		}
		else
		{
			// クールタイマーが0なら利用可能
			if (player[i].skillCoolTimer <= 0.0f)
			{
				skillFill = 1.0f;
			}
			else
			{
				// typeに応じたクールタイムを取得
				float coolTime = 0.0f;
				switch (player[i].type)
				{
				case PlayerType::Glass:			coolTime = SKILL_GLASS_COOLTIME; break;
				case PlayerType::Concrete:		coolTime = SKILL_CONCRETE_COOLTIME; break;
				case PlayerType::Plant:			coolTime = SKILL_PLANT_COOLTIME; break;
				case PlayerType::Electricity:	coolTime = SKILL_ELECTRICITY_COOLTIME; break;
				default: coolTime = 0.0f; break;
				}

				// クールタイムが0の時は1.0fを返す
				if (coolTime <= 0.0f)
				{
					skillFill = 1.0f;
				}
				else
				{
					// 使用直後　skillCoolTimer == coolTime => fill = 0.0
					// クール終了　skillCoolTimer == 0 => fill = 1.0
					skillFill = 1.0f - (player[i].skillCoolTimer / coolTime);
					if (skillFill < 0.0f) skillFill = 0.0f;
					if (skillFill > 1.0f) skillFill = 1.0f;
				}
			}
		}

		// 進化が固定されたら、タイプのゲージを最大値で表示する
		if (player[i].isTypeFixed)
		{
			float glass = 0.0f;
			float concrete = 0.0f;
			float plant = 0.0f;
			float electricity = 0.0f;

			switch (player[i].type)
			{
			case PlayerType::Glass:			glass = 1.0f;		break;
			case PlayerType::Concrete:		concrete = 1.0f;	break;
			case PlayerType::Plant:			plant = 1.0f;		break;
			case PlayerType::Electricity:	electricity = 1.0f;	break;
			default: break;
			}

			Gauge_Set(i, glass, concrete, plant, electricity,
				player[i].evolutionGauge, skillFill, { hp.x - GAUGE_POS_X , hp.y + GAUGE_POS_Y }, player[i].type);
		}
		else
		{
			// 固定前はカウント数をそのまま表示する
			Gauge_Set(i, player[i].breakCount_Glass, player[i].breakCount_Concrete, player[i].breakCount_Plant, player[i].breakCount_Electricity,
				player[i].evolutionGauge, skillFill, { hp.x - GAUGE_POS_X , hp.y + GAUGE_POS_Y }, player[i].type);
		}


		if (Player_CanUseSpecial(i))
		{
			Effect_Set(26, { (hp.x + 12.0f * SCREEN_ADJUST_X), hp.y - (100.0f * SCREEN_ADJUST_Y) }, { (162.0f * SCREEN_ADJUST_X), (60.0f * SCREEN_ADJUST_Y) }, i);
		}
		if (!Player_CanUseSpecial(i))
		{
			Effect_Clear(i);
		}


		// 通常ゲージ（内＋外）は常に描画
		// スキルゲージは属性確定のときのみ描画
		Gauge_DrawBasic(i);

		if (player[i].isTypeFixed)
		{
			Gauge_DrawSkill(i);
		}

		Shader_Begin();

		Player_DrawStock(i);
	}
}

void Player_Respawn(int playerIndex)
{
	// 範囲チェック 0 1 2 3 以外なら return
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	// 残機が1つ以上ある場合
	if (player[playerIndex].active == true)
	{
		player[playerIndex].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[playerIndex].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
		player[playerIndex].hp = PLAYER_MAX_HP;
		player[playerIndex].attack = 0.0f;
		player[playerIndex].power = 0.0f;
		player[playerIndex].speed = 0.0f;
		player[playerIndex].defense = 1.0f;
		player[playerIndex].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[playerIndex].active = true;
		player[playerIndex].satiety = 0.0f;
		player[playerIndex].isAttacking = false;
		player[playerIndex].attackTimer = 0.0f;
		player[playerIndex].isAttacked = false;
		player[playerIndex].attackedTimer = 0.0f;
		player[playerIndex].isHealing = false;
		player[playerIndex].healingTimer = 0.0f;
		player[playerIndex].isEvolving = false;
		player[playerIndex].evolvingTimer = 0.0f;
		player[playerIndex].useSkill = false;
		player[playerIndex].skillTimer = 0.0f;
		player[playerIndex].skillCoolTimer = 0.0f;
		player[playerIndex].useSpecial = false;
		player[playerIndex].specialTimer = 0.0f;
		player[playerIndex].isInvincible = false;
		player[playerIndex].invincibleTimer = 0.0f;
		player[playerIndex].stunGauge = 0.0f;
		player[playerIndex].isStunning = false;
		player[playerIndex].stunTimer = 0.0f;
		player[playerIndex].isDown = false;
		player[playerIndex].downTimer = 0.0f;
		player[playerIndex].isPoisoned = false;
		player[playerIndex].poisonTimer = 0.0f;
		player[playerIndex].duringRespawn = true;
		player[playerIndex].respawnTimer = 0.0f;
		player[playerIndex].isEggBreaking = false;
		player[playerIndex].eggBreakingTimer = 0.0f;
		player[playerIndex].lastDir = PlayerDir::Down; // 正面
		player[playerIndex].isMoving = false;
		player[playerIndex].isShadowEnabled = true;
		player[playerIndex].form = Form::First;
		player[playerIndex].type = PlayerType::None;
		player[playerIndex].evolutionGauge = 0;
		player[playerIndex].evolutionGaugeRate = 0.5f;
		player[playerIndex].breakCount_Glass = 0;
		player[playerIndex].breakCount_Concrete = 0;
		player[playerIndex].breakCount_Plant = 0;
		player[playerIndex].breakCount_Electricity = 0;
		player[playerIndex].brokenHistory.clear();
		player[playerIndex].knockback_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[playerIndex].is_knocked_back = false;
		player[playerIndex].knockback_duration = 0.0f;
		player[playerIndex].isTypeFixed = false;
	}

	if (playerIndex == 0) player[0].position = XMFLOAT3(-4.0f, 4.0f, 0.0f);
	if (playerIndex == 1) player[1].position = XMFLOAT3(1.5f, 4.0f, 2.0f);
	if (playerIndex == 2) player[2].position = XMFLOAT3(-4.0f, 4.0f, -3.0f);
	if (playerIndex == 3) player[3].position = XMFLOAT3(4.0f, 4.0f, 1.0f);
}

inline void LoopRange(int& animFrame, int start, int count, int advance)
{
	int relative = (animFrame - start + advance) % count;
	if (relative < 0) relative += count;
	animFrame = start + relative;
}

//==================================
// 残機描画
//==================================
void Player_DrawStock(int i)
{
	Shader_Begin();
	Shader_BeginUI();

	// HPバー位置取得・ゲージ座標設定
	float bx = HPBar[i].pos.x - (60.0f * SCREEN_ADJUST_X);
	float by = HPBar[i].pos.y + (60.0f * SCREEN_ADJUST_Y);


	// プレイヤーごとのストック描画
	for (int j = 0; j < player[i].stock; j++)
	{
		// ストック描画変数
		XMFLOAT2 pos = { bx + (j * 30.0f * SCREEN_ADJUST_X), by };	// 横並び
		XMFLOAT2 size = { (260.0f * SCREEN_ADJUST_X), (260.0f * SCREEN_ADJUST_Y) };

		g_pContext->PSSetShaderResources(0, 1, &g_Texture[i + 13]);
	
		SetBlendState(BLENDSTATE_ALPHA);
		DrawSprite(pos, size, color::white);
	}
}

void Player_DrawText()
{
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (!player[p].active || !player[p].isOnScreen) continue;

		wchar_t playerLabel[8];
		swprintf_s(playerLabel, L"P%d", p + 1);

		// プレイヤーごとに色設定
		TextColor textColor;
		switch (p)
		{
		case 0:
			textColor = TextColor::P1color;
			break;
		case 1:
			textColor = TextColor::P2color;
			break;
		case 2:
			textColor = TextColor::P3color;
			break;
		case 3:
			textColor = TextColor::P4color;
			break;
		default:
			textColor = TextColor::White;
			break;
		}

		// フォントサイズの半分程度左にずらす
		float offsetX = 15.0f;

		DrawTextEx(
			playerLabel,
			player[p].screenPos.x - offsetX,
			player[p].screenPos.y - 10.0f,	// テキストの高さ分上に表示
			40.0f,							// フォントサイズ
			L"Impact",
			textColor
		);
	}
}

static void Ranking(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;
	// 二重登録防止
	if (player[playerIndex].rank != 0) return;

	// 死亡順に追加
	g_deathOrder.push_back(playerIndex);
	size_t pos = g_deathOrder.size();

	// 先に死んだプレイヤーが低順位になる（pos=1 -> 4位）
	player[playerIndex].rank = PLAYER_MAX - (int)(pos - 1);

	// 最後の一人が確定したら残りを1位にする
	if (g_deathOrder.size() == (size_t)(PLAYER_MAX - 1))
	{
		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			if (player[p].rank == 0)
			{
				player[p].rank = 1;
				break;
			}
		}
	}
}

PLAYEROBJECT* GetPlayer(int playerIndex)
{
	// 範囲チェック 0 1 2 3 以外なら nullptr を返す
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX)	return nullptr;

	return &player[playerIndex];
}

void TriggerbyHPShake(int playerIndex, float amplitude, float duration, float speed)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	
		SetHPShake(&HPBar[playerIndex], amplitude, duration, speed, playerIndex + 6);
	
}


bool Player_CanUseSpecial(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return false;

	PLAYEROBJECT& pl = player[playerIndex];

	if (!pl.active) return false;
	if (pl.isStunning) return false;
	if (pl.isDown) return false;
	if (pl.rank == 1) return false;

	// 形態が第3形態であること
	if (pl.form != Form::Third) return false;

	// タイプが未設定だとスペシャルがないからタイプもチェック
	if (pl.type == PlayerType::None) return false;

	// すべて通ったらtrue
	return true;
}