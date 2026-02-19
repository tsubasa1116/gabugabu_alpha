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
#include "Player.h"
#include "special.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "color.h"

//======================================================
//	マクロ定義
//======================================================
#define BOX_NUM_VERTEX	(24)
#define FIELD_TEX_MAX	(2)

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

// FIELD::no の値に対応するテクスチャファイル名
static const wchar_t* g_TexturePaths[FIELD_TEX_MAX] = {
	L"Asset\\Texture\\gure.jpg",
	L"Asset\\Texture\\fade.bmp"
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
	{ {},{}, FIELD::FIELD_Plant,2},           // 3kaku
	{ {},{}, FIELD::FIELD_Electricity,0},           // 4kaku
	{ {},{}, FIELD::FIELD_Plant,2}, // 5kaku
	{ {},{}, FIELD::FIELD_Electricity,0},           // 6kaku
	{ {},{}, FIELD::FIELD_Plant,2},           // 7kaku
	{ {},{}, FIELD::FIELD_Concrete,2},           // 8kaku
	{ {},{}, FIELD::FIELD_Concrete,1},           // 9
	{ {},{}, FIELD::FIELD_Glass},           // 10
						  
	// ===== BOX 10 ===== 
	{ {},{}, FIELD::FIELD_Glass,}, // 11
	{ {},{}, FIELD::FIELD_Plant,2}, // 12kaku
	{ {},{}, FIELD::FIELD_Plant,2}, // 13kaku
	{ {},{}, FIELD::FIELD_Concrete,1}, // 14kaku
	{ {},{}, FIELD::FIELD_Glass}, // 15kaku
	{ {},{}, FIELD::FIELD_Electricity,4}, // 16
	{ {},{}, FIELD::FIELD_Electricity,0}, // 17kaku
	{ {},{}, FIELD::FIELD_Concrete,2}, // 18
	{ {},{}, FIELD::FIELD_Electricity,0}, // 19kaku
	{ {},{}, FIELD::FIELD_Concrete,1}, // 20kaku
						  
	// ===== BOX 20 ===== 
	{ {},{}, FIELD::FIELD_Plant,2 }, // 21
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 22
	{ {},{}, FIELD::FIELD_Plant,1 }, // 23
	{ {},{}, FIELD::FIELD_Plant,2}, // 24
	{ {},{}, FIELD::FIELD_Plant,2 }, // 25
	{ {},{}, FIELD::FIELD_Electricity,0 }, // 26
	{ {},{}, FIELD::FIELD_Concrete,2}, // 27
	{ {},{}, FIELD::FIELD_Plant,2 }, // 28
	{ {},{}, FIELD::FIELD_Plant,3 }, // 29
	{ {},{}, FIELD::FIELD_Glass }, // 30
						 
	// ===== BOX 30 =====
	{ {},{}, FIELD::FIELD_Electricity,0 }, // 31
	{ {},{}, FIELD::FIELD_Plant,2 }, // 32
	{ {},{}, FIELD::FIELD_Plant,2 }, // 33
	{ {},{}, FIELD::FIELD_Electricity,4}, // 34
	{ {},{}, FIELD::FIELD_Glass }, // 35
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 36
	{ {},{}, FIELD::FIELD_Plant,2 }, // 37
	{ {},{}, FIELD::FIELD_Glass,3 }, // 38
	{ {},{}, FIELD::FIELD_BOX }, // 39
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 40
						 
	// ===== BOX 40 =====
	{ {},{}, FIELD::FIELD_Plant,2 }, // 41
	{ {},{}, FIELD::FIELD_Plant,2 }, // 42
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 43
	{ {},{}, FIELD::FIELD_Glass }, // 44
	{ {},{}, FIELD::FIELD_Plant,2 }, // 45
	{ {},{}, FIELD::FIELD_Glass, }, // 46
	{ {},{}, FIELD::FIELD_Electricity,4}, // 47
	{ {},{}, FIELD::FIELD_Glass,1 }, // 48  左上デカい建物
	{ {},{}, FIELD::FIELD_Electricity,2 }, // 49   右上デカい建物
	{ {},{}, FIELD::FIELD_Electricity,4}, // 50
						 
	// ===== BOX 50 =====
	{ {},{}, FIELD::FIELD_Electricity }, // 51
	{ {},{}, FIELD::FIELD_Concrete }, // 52 右下デカい建物
	{ {},{}, FIELD::FIELD_Plant,4 }, // 53左下デカい
	{ {},{}, FIELD::FIELD_Electricity}, // 54
	{ {},{}, FIELD::FIELD_Plant,2}, // 55
	{ {},{}, FIELD::FIELD_Glass,3}, // 56
	{ {},{}, FIELD::FIELD_Electricity}, // 57
	{ {},{}, FIELD::FIELD_Plant,2}, // 58
	{ {},{}, FIELD::FIELD_Concrete,1}, // 59
	{ {},{}, FIELD::FIELD_Electricity,}, // 60
						  
	// ===== BOX 60 ===== 
	{ {},{}, FIELD::FIELD_Plant,2 }, // 61
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
	{ {},{}, FIELD::FIELD_Plant,2 }, // 71
	{ {},{}, FIELD::FIELD_Plant,2 }, // 72
	{ {},{}, FIELD::FIELD_Electricity }, // 73
	{ {},{}, FIELD::FIELD_Electricity,3 },// 74
	{ {},{}, FIELD::FIELD_Plant,1 }, // 75
	{ {},{}, FIELD::FIELD_Plant,3 }, // 76
	{ {},{}, FIELD::FIELD_Plant,2 }, // 77
	{ {},{}, FIELD::FIELD_Plant,1 }, // 78
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 79
	{ {},{}, FIELD::FIELD_Plant,2 }, // 80
						  
	// ===== BOX 80 ===== 
	{ {},{}, FIELD::FIELD_Plant,1 }, // 81
	{ {},{}, FIELD::FIELD_Plant,2 }, // 82
	{ {},{}, FIELD::FIELD_Plant,3 }, // 83
	{ {},{}, FIELD::FIELD_Electricity ,4}, // 84
	{ {},{}, FIELD::FIELD_Plant,2 }, // 85
	{ {},{}, FIELD::FIELD_Plant,2 }, // 86
	{ {},{}, FIELD::FIELD_Plant,3 }, // 87
	{ {},{}, FIELD::FIELD_Glass },   // 88
	{ {},{}, FIELD::FIELD_Plant,2 }, // 89
	{ {},{}, FIELD::FIELD_Plant,2 }, // 90
	 					  
	// ===== BOX 90 ===== 
	{ {},{}, FIELD::FIELD_Plant,2 }, // 91
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 92
	{ {},{}, FIELD::FIELD_Plant,2 }, // 93
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 94
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 95
	{ {},{}, FIELD::FIELD_Concrete ,2}, // 96
	{ {},{}, FIELD::FIELD_Plant,3 }, // 97
	{ {},{}, FIELD::FIELD_Plant,2 }, // 98
	{ {},{}, FIELD::FIELD_Plant,2 }, // 99
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 100
						  
	// ===== BOX 100 =====
	{ {},{}, FIELD::FIELD_Plant,2}, // 101
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
	char modelPath[256];
	snprintf(modelPath, sizeof(modelPath), "asset\\model\\%s.fbx", g_ModelName[1]);

	Test = ModelLoad(modelPath);//デバッグ

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
	const float marginFactor =5.0f; // 1.0 = 最低限, 1.25 = 余裕 25%
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
	for (int i = 0; i < FIELD_TEX_MAX; ++i) // 定義したテクスチャの数だけループ
	{
		TexMetadata metadata;
		ScratchImage image;
		// 配列に定義したパスからテクスチャを読み込む
		LoadFromWICFile(g_TexturePaths[i], WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(g_pDevice, image.GetImages(),
			image.GetImageCount(), metadata, &g_Texture[i]);
		assert(g_Texture[i]);
	}
	// --------------------------------------------------------------------

	// 初期ブロックの生成とAABBの計算
	int i = 0;

	while (Map[i].no != FIELD::FIELD_MAX && Map[i].isActive) {
		if (i == 0) {
			//CreateBox();
		}

		// 全てのマップオブジェクトに対してAABBを計算する
		// Player_CalculateAABB(&map[i]); // 古い呼び出し
		//CalculateAABB(Map[i].boundingBox, Map[i].pos, XMFLOAT3{ 1.0f, 1.0f, 1.0f }); // ★新しい呼び出し

		i++;
	}

	Building_Initialize(pDevice, pContext);
}
//======================================================
//	終了処理関数
//======================================================
void Field_Finalize(void)
{
	ModelRelease(Test);

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
	static bool input2 = false;
	// デバッグモード中のみキー入力を受け付ける
	if (s_IsKonamiCodeEntered)
	{
		if (Keyboard_IsKeyDownTrigger(KK_D2))
		{
			input2 = !input2;	// フラグ反転
		}
	}
	//シェーダーを描画パイプラインへ設定
	Shader_Begin();
	Shader_SetColor(color::white);

	//プロジェクション行列作成
	XMMATRIX	projection = GetProjectionMatrix();
	//ビュー行列作成
	XMMATRIX	view = GetViewMatrix();
	//先にVP変換行列を作っておく
	XMMATRIX VP = view * projection;

	//MAPの表示
	int i = 0;
	static float rot = 0.0f;
	rot -= 0.5f;
	while (Map[i].no != FIELD_MAX)
	{
		// もしアクティブじゃなかったら、描画しないで次へ
		if (!Map[i].isActive)
		{
			i++; // i を進めるのを忘れないで！
			continue; // この先の描画処理をスキップ
		}

		///////////////////////////////////////////////debug
		//ImGui::Begin("Player Debug");

		//// 座標調整
		//ImGui::Text("Position");
		//ImGui::DragFloat3("pos", (float*)&Map[i].pos, 0.1f);
		//if (i % 10 == 0)
		//{
		//	ImGui::Text("/");
		//}

		//ImGui::End();
		///////////////////////////////////////////////

		//スケーリング行列の作成
		XMMATRIX	ScalingMatrix = XMMatrixScaling
		(
			1.0f, 1.0f, 1.0f
		);
		//平行移動行列の作成
		XMMATRIX	TranslationMatrix = XMMatrixTranslation
		(
			Map[i].pos.x, Map[i].pos.y, Map[i].pos.z
		);
		//回転行列の作成
		XMMATRIX	RotationMatrix = XMMatrixRotationRollPitchYaw
		(
			//-3.141592 / 2,
			XMConvertToRadians(-90.0f),
			XMConvertToRadians(0.0f),
			XMConvertToRadians(0.0f)
		);
		//ワールド行列の作成
		XMMATRIX	World = ScalingMatrix * RotationMatrix * TranslationMatrix;
		//最終的な変換行列を作成
		XMMATRIX	WVP = World * VP;	//(VP = View * Projection)

		//DirectXへ行列をセット
		Shader_SetWorldMatrix(World);
		Shader_SetMatrix(WVP);

		// --------------------------------------------------------
		// map[i].no の値 (intにキャスト) に対応するテクスチャをセット
		// --------------------------------------------------------
		int texIndex = (int)Map[i].no; // FIELD_BUILDING, FIELD_BOXが 0, 1 に対応していることを利用
		if (texIndex >= 0 && texIndex < FIELD_TEX_MAX)
		{
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[texIndex]);
		}

		////頂点バッファをセット
		//UINT	stride = sizeof(Vertex3D);	//頂点１個のデータサイズ
		//UINT	offset = 0;
		//g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

		////インデックスバッファをセット
		//g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		////描画するポリゴンの種類をセット 3頂点でポリゴン１枚として表示
		//g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//////描画リクエスト
		//g_pContext->DrawIndexed(6 * 6, 0, 0);

		if (!s_IsKonamiCodeEntered || input2)
		{
			ModelDraw(Test);//デバッグ
		}

		//// テクスチャをパイプラインから解除
		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		g_pContext->PSSetShaderResources(0, 1, &g_Texture[1]);
		////------------------------------------------------

		if (s_IsKonamiCodeEntered)
		{
			// ------------------------------------
			// コライダーフレーム（六角柱）の描画
			// ------------------------------------
			{
				//// 1. デバッグ描画が前の描画に引きずられないようテクスチャを強制解除
				//ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
				//g_pContext->PSSetShaderResources(0, 1, nullSRV);

				// デバッグ描画前に、行列をリセットした状態のシェーダー設定を確定させる
				// プレイヤーの描画に使われた行列をクリアする
				XMMATRIX world = XMMatrixIdentity();
				Shader_SetMatrix(world * GetViewMatrix() * GetProjectionMatrix()); // WVP行列をIdentity * View * Projectionに設定
				//Shader_Begin(); // シェーダーを再設定

				int fieldCount = GetFieldObjectCount();
				MAPDATA* fieldObjects = GetFieldObjects();

				for (int j = 0; j < fieldCount; ++j)
				{
					if (!fieldObjects[j].isActive) continue;

					// HexCollider情報を構築
					HexCollider hex;
					hex.center = fieldObjects[j].pos;
					hex.radius = fieldObjects[j].radius;
					hex.height = fieldObjects[j].height;

					// 六角柱を描画
					Debug_DrawHex(hex, XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
				}

				// Shader_End() があればここで呼ぶ (なければ次の描画で上書きされる)
			}
		}
		i++;
	}

	///////////////////////////////////////////////////////
	// 取りあえずのテクスチャ再セット
	// 建物のテクスチャは別で設定する
	//g_pContext->PSSetShaderResources(0, 1, &g_Texture[0]);
	///////////////////////////////////////////////////////
	Building_DrawAll(s_IsKonamiCodeEntered);

	if (s_IsKonamiCodeEntered)
	{
		// 植物・コンクリートのスペシャルが使用されている場合、円のフレームを赤色で表示
		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			PLAYEROBJECT* playerObject = GetPlayer(p);
			PLAYEROBJECT& player = *playerObject;
			if (!player.useSpecial) continue;

			// 植物・コンクリートのスペシャル
			if (player.type == PlayerType::Plant || player.type == PlayerType::Concrete)
			{
				// 円の中心と半径を設定
				XMFLOAT3 center = playerObject->position;
				float radius = 5.0f;

				// 赤色で円を描画
				Debug_DrawCircle(center, radius, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
			}
			// 電気のスペシャル
			if (player.type == PlayerType::Electricity)
			{
				for (int i = 0; i < SPECIAL_ELECTRICITY_QUANTITY; ++i)
				{
					// 電気の円の中心と半径を取得
					XMFLOAT3 center = player.electricityCircles[i].center;
					float radius = player.electricityCircles[i].radius;

					// 赤色で円を描画
					Debug_DrawCircle(center, radius, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
				}
			}
			// ガラスのスペシャル
			if (player.type == PlayerType::Glass)
			{
				for (const auto& box : player.glassBoxes)
				{
					// ガラスの円の中心と半径を設定
					XMFLOAT3 center = box.position;
					float radius = 0.3f; // 半径0.3の円

					// 赤色で円を描画
					Debug_DrawCircle(center, radius, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
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


