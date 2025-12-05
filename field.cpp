//======================================================
//	field.cpp[]
// 
//	制作者：前野翼			日付：2024//
//======================================================
#include "field.h"
#include "Camera.h"
#include "keyboard.h"

///////////////////////////////////////
#include "collider.h"
#include "debug_render.h"
///////////////////////////////////////

#include "model.h"

#include "Building.h"


#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

//======================================================
//	マクロ定義
//======================================================
#define BOX_NUM_VERTEX	(24)

//======================================================
//	グローバル変数
//======================================================
MODEL* Test = NULL;//デバッグ

////グローバル変数
//static	ID3D11Device* g_pDevice = NULL;
static	ID3D11DeviceContext* g_pContext = NULL;
////頂点バッファ
//static	ID3D11Buffer* g_VertexBuffer = NULL;
////インデックスバッファ
//static	ID3D11Buffer* g_IndexBuffer = NULL;
//テクスチャ変数
//static ID3D11ShaderResourceView* g_Texture;

// FIELD enum (FIELD_BUILDING, FIELD_BOX) の数だけテクスチャを管理
#define FIELD_TEX_MAX 2 
static ID3D11ShaderResourceView* g_Texture[FIELD_TEX_MAX];

// FIELD::no の値に対応するテクスチャファイル名
static const wchar_t* g_TexturePaths[FIELD_TEX_MAX] = {
	L"Asset\\Texture\\green.png", 
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
};
//マップデータ配列
MAPDATA		Map[] =
{
	// 地面
	{ {},{}, FIELD::FIELD_Glass },	// 配列番号[0]の確認
	{ {},{}, FIELD::FIELD_Concrete },
	{ {},{}, FIELD::FIELD_Plant },
	{ {},{}, FIELD::FIELD_Electric },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },	// 10
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },	// 20
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },	// 30
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },	// 40
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },	// 50
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },	// 60
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_Glass },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },	// 70
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_Glass },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },	// 80
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },	// 90
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },	// 100
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },	// 110
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },
	{ {},{}, FIELD::FIELD_BOX },	// 118
	//{ {},{}, FIELD::FIELD_BOX },	// 120

	{ XMFLOAT3(2.0f,-1.0f,	5.0f), {}, FIELD::FIELD_MAX }	// MAPデータ終了
};

//======================================================
//	初期化関数
//======================================================
void Field_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	char modelPath[256];
	snprintf(modelPath, sizeof(modelPath), "asset\\model\\%s.fbx", g_ModelName[1]);
	
	Test = ModelLoad(modelPath);//デバッグ



	const int NUM = 10;		// 1行/列あたりのfieldの個数
	//int count = sizeof(Map) / sizeof(Map[0]);	// 配列の要素数
	int count = GetFieldObjectCount();

	float sin60 = sinf(XMConvertToRadians(60.0f)); // 60度のcos値

	// ----------------------------------------------------
	// 中央補正のためのオフセット計算
	// ----------------------------------------------------

	// Z軸方向の列数を確定 (FIELD_MAXを除くため count-1 で考えるのが確実)
	int tiles_count = count - 1; // 描画対象のタイル数
	int col_max = tiles_count / NUM;

	if (tiles_count % NUM != 0) {
		col_max++;
	}

	// X軸の最大座標とZ軸の最大座標を計算
	// X軸の最大位置 (最後のタイル位置)
	float max_x_pos = (NUM - 1) * Map->radius * 3.0f;
	// Z軸の最大位置 (最後のタイル位置)
	float max_z_pos = (col_max - 1) * (sin60 * Map->radius);

	// 中心オフセットを決定 (全体の最大位置の半分を引く)
	float offset_x = max_x_pos / 2.0f;
	float offset_z = max_z_pos / 2.0f;

	for (int i = 0; i < count; i++)
	{
		int row = i % NUM;	// 行番号
		int col = i / NUM;	// 列番号

		// もしアクティブじゃなかったら次へ
		if (Map[i].no == FIELD::FIELD_MAX) continue;

		// 行
		if (col % 2 == 0)	Map[i].pos.x = row * Map->radius * 3.0f;	// 偶数行
		else				Map[i].pos.x = row * Map->radius * 3.0f + Map->radius * 1.5f;	// 奇数行 

		// 列
		Map[i].pos.z = col * (sin60 * Map->radius);

		// y 座標
		Map[i].pos.y = -1.0f;

		// 計算した座標から、中心オフセットを引く
		Map[i].pos.x -= offset_x;
		Map[i].pos.z -= offset_z;



		// 穴デバッグ
		if (i % 7 == 0)
			Map[i].isActive = false;

	}



	//g_pDevice = pDevice;
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
		CreateShaderResourceView(pDevice, image.GetImages(),
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
		// Polygon3D_CalculateAABB(&map[i]); // 古い呼び出し
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
	for (int i = 0; i < FIELD_TEX_MAX; ++i) {
		SAFE_RELEASE(g_Texture[i]);
	}

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
	Shader_SetColor({ 1.0f,1.0f,1.0f,1.0f });

	//プロジェクション行列作成
	XMMATRIX	Projection = GetProjectionMatrix();
	//ビュー行列作成
	XMMATRIX	View = GetViewMatrix();
	//先にVP変換行列を作っておく
	XMMATRIX VP = View * Projection;

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
		//ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		//g_pContext->PSSetShaderResources(0, 1, &g_Texture[1]);
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
	g_pContext->PSSetShaderResources(0, 1, &g_Texture[0]);
	///////////////////////////////////////////////////////
	Building_DrawAll(s_IsKonamiCodeEntered);
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


