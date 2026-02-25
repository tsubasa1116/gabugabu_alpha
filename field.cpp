//======================================================
//	field.cpp[]
//======================================================
#include "field.h"
#include "Camera.h"
#include "keyboard.h"
#include "collider.h"
#include "debug_render.h"
#include "model.h"
#include "Building.h"
#include "player.h"
#include "special.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "color.h"
#include "loadThread.h"

//======================================================
//	マクロ定義
//======================================================
#define BOX_NUM_VERTEX	(24)
#define FIELD_TEX_MAX	(4)

//======================================================
//	グローバル変数
//======================================================
MODEL* Test = NULL;//デバッグ

////グローバル変数
static	ID3D11Device* g_pDevice = NULL;
static	ID3D11DeviceContext* g_pContext = NULL;
////頂点バッファ
//static	ID3D11Buffer* g_VertexBuffer = NULL;
////インデックスバッファ
//static	ID3D11Buffer* g_IndexBuffer = NULL;
//テクスチャ変数
//static ID3D11ShaderResourceView* g_Texture;


// FIELD enum (FIELD_BUILDING, FIELD_BOX) の数だけテクスチャを管理
static ID3D11ShaderResourceView* g_Texture[FIELD_TEX_MAX];
#define FIELD_TEX_MAX (4)
// FIELD::no の値に対応するテクスチャファイル名
static const wchar_t* g_TexturePaths[FIELD_TEX_MAX] =
{
	L"Asset\\Texture\\灰色.png",  // 0
	L"Asset\\Texture\\texturefieldTree02_v1.png",  // 1
	L"Asset\\Texture\\texturefieldTree01_v1.png",  
	L"Asset\\Texture\\灰色.png",
	//L"Asset\\Texture\\texturefieldConcrete01_v1.png",// 1
};

static const char* g_ModelName[] = {
	"field",
	"field_v2",
	"field_v3",
	"propsConcreteMain_v2",		// 3マス大建物
	"propsConcreteSub_v2",		// マンション
	"propsElectricitySub_v2",	// 車と信号
	"propsGlassSub_v2",			// ビル
	"propsTreeSub_v2",			// 広葉樹
	"build_glass_new"			// 変な建物
	"propsTowerMain_v3"			//東京タワ-
};
static const char* g_ModelName1[] = {
	"raibu",
	"kitosaku"
};
 
//マップデータ配列
MAPDATA Map[] =
{
	// ===== 地面・特殊 =====			 
	{ {},{}, FIELD::FIELD_Electricity,1}, // 1kaku
	{ {},{}, FIELD::FIELD_Electricity,0}, // 2kaku
	{ {},{}, FIELD::FIELD_Plant,1},           // 3kaku
	{ {},{}, FIELD::FIELD_Electricity,0},           // 4kaku
	{ {},{}, FIELD::FIELD_Plant,1}, // 5kaku
	{ {},{}, FIELD::FIELD_Electricity,0},           // 6kaku
	{ {},{}, FIELD::FIELD_Plant,1},           // 7kaku
	{ {},{}, FIELD::FIELD_Concrete,2},           // 8kaku
	{ {},{}, FIELD::FIELD_Concrete,1},           // 9
	{ {},{}, FIELD::FIELD_Glass},           // 10

	// ===== BOX 10 ===== 
	{ {},{}, FIELD::FIELD_Glass,}, // 11
	{ {},{}, FIELD::FIELD_Plant,1}, // 12kaku
	{ {},{}, FIELD::FIELD_Plant,1}, // 13kaku
	{ {},{}, FIELD::FIELD_Concrete,1}, // 14kaku
	{ {},{}, FIELD::FIELD_Glass}, // 15kaku
	{ {},{}, FIELD::FIELD_Electricity,4}, // 16
	{ {},{}, FIELD::FIELD_Electricity,0}, // 17kaku
	{ {},{}, FIELD::FIELD_Concrete,2}, // 18
	{ {},{}, FIELD::FIELD_Electricity,0}, // 19kaku
	{ {},{}, FIELD::FIELD_Concrete,1}, // 20kaku

	// ===== BOX 20 ===== 
	{ {},{}, FIELD::FIELD_Plant,1 }, // 21
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 22
	{ {},{}, FIELD::FIELD_Plant,0 }, // 23
	{ {},{}, FIELD::FIELD_Plant,1}, // 24
	{ {},{}, FIELD::FIELD_Plant,1 }, // 25
	{ {},{}, FIELD::FIELD_Electricity,0 }, // 26
	{ {},{}, FIELD::FIELD_Concrete,2}, // 27
	{ {},{}, FIELD::FIELD_Plant,1 }, // 28
	{ {},{}, FIELD::FIELD_Plant,2 }, // 29
	{ {},{}, FIELD::FIELD_Glass }, // 30

	// ===== BOX 30 =====
	{ {},{}, FIELD::FIELD_Electricity,0 }, // 31
	{ {},{}, FIELD::FIELD_Plant,1 }, // 32
	{ {},{}, FIELD::FIELD_Plant,1 }, // 33
	{ {},{}, FIELD::FIELD_Electricity,4}, // 34
	{ {},{}, FIELD::FIELD_Glass }, // 35
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 36
	{ {},{}, FIELD::FIELD_Plant,1 }, // 37
	{ {},{}, FIELD::FIELD_Glass,2 }, // 38
	{ {},{}, FIELD::FIELD_BOX }, // 39
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 40

	// ===== BOX 40 =====
	{ {},{}, FIELD::FIELD_Plant,1 }, // 41
	{ {},{}, FIELD::FIELD_Plant,1}, // 42
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 43
	{ {},{}, FIELD::FIELD_Glass }, // 44
	{ {},{}, FIELD::FIELD_Plant,1 }, // 45
	{ {},{}, FIELD::FIELD_Glass, }, // 46
	{ {},{}, FIELD::FIELD_Electricity,4}, // 47
	{ {},{}, FIELD::FIELD_Glass,1 }, // 48  左上デカい建物
	{ {},{}, FIELD::FIELD_Electricity,2 }, // 49   右上デカい建物
	{ {},{}, FIELD::FIELD_Electricity,4}, // 50

	// ===== BOX 50 =====
	{ {},{}, FIELD::FIELD_Electricity }, // 51
	{ {},{}, FIELD::FIELD_Concrete }, // 52 右下デカい建物
	{ {},{}, FIELD::FIELD_Plant,3}, // 53左下デカい
	{ {},{}, FIELD::FIELD_Electricity}, // 54
	{ {},{}, FIELD::FIELD_Plant,1}, // 55
	{ {},{}, FIELD::FIELD_Glass,2}, // 56
	{ {},{}, FIELD::FIELD_Electricity}, // 57
	{ {},{}, FIELD::FIELD_Plant,1}, // 58
	{ {},{}, FIELD::FIELD_Concrete,1}, // 59
	{ {},{}, FIELD::FIELD_Electricity,}, // 60

	// ===== BOX 60 ===== 
	{ {},{}, FIELD::FIELD_Plant,1 }, // 61
	{ {},{}, FIELD::FIELD_Glass }, // 62
	{ {},{}, FIELD::FIELD_Electricity }, // 63
	{ {},{}, FIELD::FIELD_Electricity }, // 64
	{ {},{}, FIELD::FIELD_Electricity,3 }, // 65
	{ {},{}, FIELD::FIELD_Electricity }, // 66
	{ {},{}, FIELD::FIELD_Glass }, // 67
	{ {},{}, FIELD::FIELD_BOX }, // 68
	{ {},{}, FIELD::FIELD_Electricity }, // 69
	{ {},{}, FIELD::FIELD_Electricity }, // 70

	// ===== BOX 70 =====
	{ {},{}, FIELD::FIELD_Plant,1 }, // 71
	{ {},{}, FIELD::FIELD_Plant,1 }, // 72
	{ {},{}, FIELD::FIELD_Electricity }, // 73
	{ {},{}, FIELD::FIELD_Electricity,3 },// 74
	{ {},{}, FIELD::FIELD_Plant,0 }, // 75
	{ {},{}, FIELD::FIELD_Plant,2 }, // 76
	{ {},{}, FIELD::FIELD_Plant,1 }, // 77
	{ {},{}, FIELD::FIELD_Plant,0 }, // 78
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 79
	{ {},{}, FIELD::FIELD_Plant,1 }, // 80

	// ===== BOX 80 ===== 
	{ {},{}, FIELD::FIELD_Plant,0 }, // 81
	{ {},{}, FIELD::FIELD_Plant,1 }, // 82
	{ {},{}, FIELD::FIELD_Plant,2 }, // 83
	{ {},{}, FIELD::FIELD_Electricity ,4}, // 84
	{ {},{}, FIELD::FIELD_Plant,1 }, // 85
	{ {},{}, FIELD::FIELD_Plant,1 }, // 86
	{ {},{}, FIELD::FIELD_Plant,2 }, // 87
	{ {},{}, FIELD::FIELD_Glass },   // 88
	{ {},{}, FIELD::FIELD_Plant,1 }, // 89
	{ {},{}, FIELD::FIELD_Plant,1 }, // 90
	 					  
	// ===== BOX 90 ===== 
	{ {},{}, FIELD::FIELD_Plant,1 }, // 91
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 92
	{ {},{}, FIELD::FIELD_Plant,1 }, // 93
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 94
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 95
	{ {},{}, FIELD::FIELD_Concrete ,2}, // 96
	{ {},{}, FIELD::FIELD_Plant,2 }, // 97
	{ {},{}, FIELD::FIELD_Plant,1 }, // 98
	{ {},{}, FIELD::FIELD_Plant,1 }, // 99
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 100

	// ===== BOX 100 =====
	{ {},{}, FIELD::FIELD_Plant,1}, // 101
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 102  いまのままだとここまでしかモデルが置けない
	{ {},{}, FIELD::FIELD_BOX }, // 103
	{ {},{}, FIELD::FIELD_BOX }, // 104
	{ {},{}, FIELD::FIELD_BOX }, // 105
	{ {},{}, FIELD::FIELD_BOX }, // 106
	{ {},{}, FIELD::FIELD_BOX }, // 107
	{ {},{}, FIELD::FIELD_BOX }, // 108
	{ {},{}, FIELD::FIELD_BOX }, // 109
	{ {},{}, FIELD::FIELD_BOX }, // 110

	// ===== BOX 110 ==== BOX
	{ {},{}, FIELD::FIELD_BOX }, // 111
	{ {},{}, FIELD::FIELD_BOX }, // 112
	{ {},{}, FIELD::FIELD_BOX }, // 113
	{ {},{}, FIELD::FIELD_BOX }, // 114
	{ {},{}, FIELD::FIELD_BOX }, // 115
	{ {},{}, FIELD::FIELD_BOX }, // 116
	{ {},{}, FIELD::FIELD_BOX }, // 117
	{ {},{}, FIELD::FIELD_BOX }, // 118
	{ {},{}, FIELD::FIELD_BOX }, // 119
	{ {},{}, FIELD::FIELD_BOX }, // 120

	// ===== 終了マーカー（カウントしない）=====
	{ XMFLOAT3(2.0f,-1.0f,5.0f), {}, FIELD::FIELD_MAX }
};


//======================================================
//	初期化関数
//======================================================
void Field_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// 配列要素数（終了マーカー FIELD_MAX を含まない）
	int count = GetFieldObjectCount();
	if (count <= 1)
	{
		g_pDevice = pDevice;
		g_pContext = pContext;
		Building_Initialize(pDevice, pContext);
		return;
	}

	// ====== 六角格子候補を多数生成し、中心に近いものから N 個選んで「より円形」に配置 ======
	// MAPDATA::radius を hex size（center->corner）と見なす（flat-top）
	const float size = Map->radius;
	const float sqrt3 = sqrtf(3.0f);

	// 横方向スケール（必要なら調整）
	const float horizontalScale = 1.0f;

	// 候補を生成するためのリング数（余裕を持たせる）
	// count 個を丸く選ぶため、候補は多少多めに生成する（marginFactor）
	const float marginFactor = 5.0f; // 1.0 = 最低限, 1.25 = 余裕 25%
	int rings = 1;
	while (1 + 3 * rings * (rings + 1) < static_cast<int>(count * marginFactor))
		++rings;

	// 軸座標(q,r)を同心リングで生成（totalCandidates >= count）
	int totalCandidates = 1 + 3 * rings * (rings + 1);

	// ヘルパー構造体（ローカル）
	struct Candidate { int q; int r; float wx; float wz; float dist; };

	// 動的確保（ローカルに vector を使わない形にしてインクルード不要に）
	Candidate* candidates = new Candidate[totalCandidates];

	// 中心
	int idx = 0;
	candidates[idx].q = 0;
	candidates[idx].r = 0;
	candidates[idx].wx = 0.0f;
	candidates[idx].wz = 0.0f;
	candidates[idx].dist = 0.0f;
	++idx;

	// 6方向ベクトル（axial coords）
	const int dirQ[6] = { 1, 1, 0, -1, -1, 0 };
	const int dirR[6] = { 0, -1, -1, 0, 1, 1 };

	for (int k = 1; idx < totalCandidates; ++k)
	{
		int q = -k;
		int r = k;
		for (int side = 0; side < 6 && idx < totalCandidates; ++side)
		{
			for (int step = 0; step < k && idx < totalCandidates; ++step)
			{
				float wx = size * 1.5f * static_cast<float>(q) * horizontalScale;
				float wz = size * sqrt3 * (static_cast<float>(r) + static_cast<float>(q) * 0.5f);
				float d = sqrtf(wx * wx + wz * wz);

				candidates[idx].q = q;
				candidates[idx].r = r;
				candidates[idx].wx = wx;
				candidates[idx].wz = wz;
				candidates[idx].dist = d;
				++idx;

				q += dirQ[side];
				r += dirR[side];
			}
		}
	}

	// 中心に近い順に count 個を選ぶ（簡易選択ソートライク）
	// 選択数 N = count（Map 配列の要素数）
	int N = count;
	// 安全策: N が候補数を超えないように
	if (N > totalCandidates) N = totalCandidates;

	// 部分選択：先頭 N 件を初期選択し，残りを走査してより近ければ入れ替える（O(M*N)だが候補はそこまで大きくない）
	// まず先頭 N を selected とする（配列内操作）
	Candidate* selected = new Candidate[N];
	for (int i = 0; i < N; ++i) selected[i] = candidates[i];

	// 現在の最遠インデックスを求める関数
	auto findWorstIndex = [&](int limit) -> int {
		int worst = 0;
		float maxd = selected[0].dist;
		for (int j = 1; j < limit; ++j)
		{
			if (selected[j].dist > maxd)
			{

				maxd = selected[j].dist;
				worst = j;
			}
		}
		return worst;
		};

	int worstIdx = findWorstIndex(N);

	// 残り候補を検査
	for (int i = N; i < totalCandidates; ++i)
	{
		if (candidates[i].dist < selected[worstIdx].dist)
		{
			// 置換
			selected[worstIdx] = candidates[i];
			// worstIndex を再計算
			worstIdx = findWorstIndex(N);
		}
	}

	// ここで selected[] は中心に近い N 個の候補（ただし順序は任意）なので、中心に近い順に並べ替えることで見た目がより自然に
	// 簡易的にバブルソート（N が小さいので十分）
	for (int a = 0; a < N - 1; ++a)
	{
		for (int b = 0; b < N - 1 - a; ++b)
		{
			if (selected[b].dist > selected[b + 1].dist)
			{
				Candidate tmp = selected[b];
				selected[b] = selected[b + 1];
				selected[b + 1] = tmp;
			}
		}
	}

	// Map 配列へ割り当て：中心に近い順に配置していく
	int assign = 0;
	for (int i = 0; i < count; ++i)
	{
		if (Map[i].no == FIELD::FIELD_MAX) break;

		if (assign < N)
		{
			Map[i].pos.x = selected[assign].wx;
			Map[i].pos.z = selected[assign].wz;
			Map[i].pos.y = -1.0f;
			Map[i].isActive = true;
			++assign;
		}
		else
		{
			// 割り当てられなかった残りは非表示
			Map[i].isActive = false;
		}
	}

	// 解放
	delete[] candidates;
	delete[] selected;

	// ====================================================

	g_pDevice = pDevice;
	g_pContext = pContext;

	// --------------------------------------------------------------------
	// 複数のテクスチャを読み込み
	// --------------------------------------------------------------------
	Loader::AddTask([pDevice]()
		{
			for (int i = 0; i < FIELD_TEX_MAX; ++i)
			{
				TexMetadata metadata;
				ScratchImage image;
				HRESULT hr = LoadFromWICFile(g_TexturePaths[i], WIC_FLAGS_NONE, &metadata, image);
				if (FAILED(hr))
				{
					g_Texture[i] = nullptr;
					continue;
				}

				hr = CreateShaderResourceView(g_pDevice, image.GetImages(),
					image.GetImageCount(), metadata, &g_Texture[i]);
				if (FAILED(hr))
				{
					g_Texture[i] = nullptr;
					continue;
				}
			}

			char modelPath[256];
			snprintf(modelPath, sizeof(modelPath), "asset\\model\\%s.fbx", g_ModelName[1]);
			Test = ModelLoad(modelPath);
		}); 
	//------------------------------------------------------------------

	Building_Initialize(pDevice, pContext);
}


//======================================================
//	終了処理関数
//======================================================
void Field_Finalize(void)
{
	ModelRelease(Test);
	Test = nullptr;

	//SAFE_RELEASE(g_VertexBuffer);
	//SAFE_RELEASE(g_IndexBuffer);
	for (int i = 0; i < FIELD_TEX_MAX; ++i) SAFE_RELEASE(g_Texture[i]);

	Building_Finalize();
}


//======================================================
//	描画関数
//======================================================
void Field_Draw(bool s_IsKonamiCodeEntered)
{
	if (!Loader::IsFinished) return;

	static bool input2 = false;

	// デバッグキー
	if (s_IsKonamiCodeEntered)
	{
		if (Keyboard_IsKeyDownTrigger(KK_D2))
		{
			input2 = !input2;
		}
	}

	// シェーダー開始
	Shader_Begin();
	Shader_SetColor(color::white);

	// 行列取得
	XMMATRIX projection = GetProjectionMatrix();
	XMMATRIX view = GetViewMatrix();
	XMMATRIX VP = view * projection;

	int i = 0;

	// ======================================================
	// フィールド描画
	// ======================================================
	while (Map[i].no != FIELD_MAX)
	{
		if (!Map[i].isActive)
		{
			i++;
			continue;
		}

		// ------------------------------
		// ワールド行列作成
		// ------------------------------
		XMMATRIX ScalingMatrix = XMMatrixScaling(1.1f, 1.1f, 1.1f);

		XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(-90.0f),
			0.0f,
			0.0f
		);

		XMMATRIX TranslationMatrix = XMMatrixTranslation(
			Map[i].pos.x,
			Map[i].pos.y,
			Map[i].pos.z
		);

		XMMATRIX World = ScalingMatrix * RotationMatrix * TranslationMatrix;
		XMMATRIX WVP = World * VP;

		Shader_SetWorldMatrix(World);
		Shader_SetMatrix(World * VP);

		// ------------------------------
		// 種類ごとにテクスチャ切り替え
		// ------------------------------
		int texIndex = 0; // デフォルト

		switch (Map[i].no)
		{
		case FIELD::FIELD_Plant:
			texIndex = 2;
			break;

		case FIELD::FIELD_Concrete:
			texIndex = 0;
			break;

		case FIELD::FIELD_Glass:
			texIndex = 0;
			break;

		case FIELD::FIELD_Electricity:
			texIndex = 0;
			break;

		case FIELD::FIELD_BOX:
			texIndex = 3;
			break;

		default:
			texIndex = 0;
			break;
		}

		g_pContext->PSSetShaderResources(0, 1, &g_Texture[texIndex]);

		// ------------------------------
		// 地面モデル描画
		// ------------------------------
		if (!s_IsKonamiCodeEntered || input2)
		{
			ModelDraw(Test);
		}

		//// テクスチャをパイプラインから解除
		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		g_pContext->PSSetShaderResources(0, 1, &g_Texture[1]);
		//------------------------------------------------
		i++;
	}

	///////////////////////////////////////
	// TODO:boundingBoxを参照したい
	// ======================================================
	// 建物描画
	// ======================================================
	Building_DrawAll(s_IsKonamiCodeEntered);

	// --- 3. デバッグ描画は全部のマップを描き終わった後に「1回だけ」やる ---
	if (s_IsKonamiCodeEntered)
	{
		SetBlendState(BLENDSTATE_NONE);
		SetDepthTest(false); // 重なりを無視して見えるように
		Shader_SetMatrix(VP); // ワールド行列はIdentityにするのでVPだけでOK

		// フィールドオブジェクトの六角柱
		int fieldCount = GetFieldObjectCount();
		MAPDATA* fieldObjects = GetFieldObjects();
		for (int j = 0; j < fieldCount; ++j)
		{
			if (!fieldObjects[j].isActive) continue;
			Debug_DrawHex(Map[j].boundingBox, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}

		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			PLAYEROBJECT* playerObject = GetPlayer(p);
			if (!playerObject) continue;
			if (!playerObject->useSpecial) continue;

			// Plant / Concrete
			if (playerObject->type == PlayerType::Plant ||
				playerObject->type == PlayerType::Concrete)
			{
				Debug_DrawCircle(
					playerObject->position,
					5.0f,
					XMFLOAT4(1, 0, 0, 1)
				);
			}

			// Electricity
			if (playerObject->type == PlayerType::Electricity)
			{
				for (int e = 0; e < SPECIAL_ELECTRICITY_QUANTITY; ++e)
				{
					Debug_DrawCircle(
						playerObject->electricityCircles[e].center,
						playerObject->electricityCircles[e].radius,
						XMFLOAT4(1, 0, 0, 1)
					);
				}
			}

			// Glass
			if (playerObject->type == PlayerType::Glass)
			{
				for (const auto& box : playerObject->glassBoxes)
				{
					Debug_DrawCircle(
						box.position,
						0.3f,
						XMFLOAT4(1, 0, 0, 1)
					);
				}
			}
		}
	}
}

//======================================================
//	更新処理
//======================================================
void Field_Update(void)
{
	int i = 0;
	while (Map[i].no != FIELD_MAX)
	{
		if (!Map[i].isActive)
		{
			i++;
			continue;
		}

		Map[i].boundingBox.center = Map[i].pos;			// -1
		Map[i].boundingBox.radius = Map[i].radius;		// 1
		Map[i].boundingBox.height = Map[i].height;		// 3.0



		i++;
	}
}

// ======================================================
//	ゲッター
// ------------------------------------------------------
//	フィールドの配列の先頭ポインタを返す
// ======================================================
MAPDATA* GetFieldObjects()
{
	return Map;
}

// フィールドオブジェクトの総数を返す
int GetFieldObjectCount()
{
	int count = 0;
	// map配列はFIELD_MAXを終了マーカーとしている
	while (Map[count].no != FIELD_MAX)
	{
		count++;
	}
	return count;
}


