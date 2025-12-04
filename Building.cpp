//1. 10×10マップに4種類の建物を置く
//MapGrid を使って 10×10 の二次元配列に Building を配置
//初期化時にランダムで Glass / Concrete / Plant / Electric を割り当て可能
//
//2. 新たに建物クラスを作って管理
//Building クラスで種類（BuildingType）と状態（BuildingPhase）を管理
//
//3. 種類はスキルに合わせた 4 種類（列挙体で管理）
//enum class BuildingType { Glass, Concrete, Plant, Electric };
//列挙体で管理しているので後でスイッチや配列で扱いやすい
//
//4. 新品→壊れかけ→壊れた（フェーズ）
//enum class BuildingPhase { New, Damaged, Broken };
//SetPhase() で状態変更可能
//
//5. フェーズごとに仮テクスチャを変える
//UpdateTexture() でフェーズ変更時に仮テクスチャ文字列を自動更新
//GetTexture() で現在のフェーズのテクスチャ名を取得可能

//Building.cpp
//#include "field.h"
#include "Camera.h"

#include "Building.h"
#include "field.h"
#include "model.h"   // ModelLoad / ModelRelease / ModelDraw

///////////////////////////////////////
#include "collider.h"
#include "debug_render.h"
///////////////////////////////////////

//======================================================
//	マクロ定義
//======================================================
#define BOX_NUM_VERTEX	(24)

//======================================================
//	グローバル変数
//======================================================
//MODEL* Test = NULL;//デバッグ


////グローバル変数
//static	ID3D11Device* g_pDevice = NULL;
//static	ID3D11DeviceContext* g_pContext = NULL;
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
	L"Asset\\Texture\\TileA3.png",
	L"Asset\\Texture\\TileA3.png"
	L"Asset\\Texture\\TileA3.png"
	L"Asset\\Texture\\TileA3.png"
};

static Building* g_Buildings[100];
static int g_BuildingCount = 0; // 現在の建物数

//==============================================
// 全ての建物を描画
//==============================================
void Building_DrawAll(bool s_IsKonamiCodeEntered)
{
	for (int i = 0; i < g_BuildingCount; ++i)
	{
		if (g_Buildings[i] != nullptr) {
			g_Buildings[i]->Draw();
		}
	}
}

//======================================================
//	コンストラクタ
//======================================================
Building::Building(BuildingType type, XMFLOAT3 pos)
	: m_Type(type), m_Pos(pos), m_Phase(BuildingPhase::New), m_Model(nullptr)
{
	// 生成時に現在のTypeとPhase(New)に合わせてモデルをロード
	LoadModelForPhase();
}

//======================================================
//	デストラクタ
//======================================================
Building::~Building()
{
	if (m_Model)
	{
		ModelRelease(m_Model);
		m_Model = nullptr;
	}
}

//======================================================
//	初期化
//======================================================
void Building_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// 既存の建物をクリア
	Building_Finalize();

	// Fieldの情報を取得
	MAPDATA* fieldMap = GetFieldObjects();
	int fieldCount = GetFieldObjectCount();	// マップの配列数を取得

	// マップを走査して建物を配置
	for (int i = 0; i < fieldCount; i++)
	{
		// もしマップが無効ならスキップ
		if (fieldMap[i].no == FIELD::FIELD_MAX) continue;

		// ----------------------------------------------------
		// FIELD の種類を BuildingType に変換する
		// ----------------------------------------------------
		BuildingType type = BuildingType::None;

		switch (fieldMap[i].no)
		{
		case FIELD::FIELD_Glass:	type = BuildingType::Glass;		break;
		case FIELD::FIELD_Concrete:	type = BuildingType::Concrete;	break;
		case FIELD::FIELD_Plant:	type = BuildingType::Plant;		break;
		case FIELD::FIELD_Electric:	type = BuildingType::Electric;	break;

		// FIELD_BOX（ただの地面）の場合は建物を作らない
		case FIELD::FIELD_BOX:		type = BuildingType::None;		break;
		}

		// ----------------------------------------------------
		// 建物タイプが決まったら生成する
		// ----------------------------------------------------
		if (type != BuildingType::None)
		{
			if (g_BuildingCount >= 100) break; // 上限チェック

			// 生成（座標はMapのものを使う）
			g_Buildings[g_BuildingCount] = new Building(type, fieldMap[i].pos);
			g_BuildingCount++;
		}
	}
}

//======================================================
//	終了処理
//======================================================
void Building_Finalize(void)
{
	// 生成した建物をすべて削除
	for (int i = 0; i < g_BuildingCount; ++i)
	{
		if (g_Buildings[i])
		{
			delete g_Buildings[i];
			g_Buildings[i] = nullptr;
		}
	}
	g_BuildingCount = 0;
}

static const char* g_ModelName[] = {
	"propsConcreteMain_v2",		// 3マス大建物
	"propsConcreteSub_v2",		// マンション
	"propsElectricitySub_v2",	// 車と信号
	"propsGlassSub_v2",			// ビル
	"propsTreeSub_v2",			// 広葉樹
	"build_glass_new"			// 変な建物
};

//==============================================
// フェーズや種類に応じてモデルを読み込む
//==============================================
void Building::LoadModelForPhase()
{
	// すでにモデルがある場合は解放（フェーズ切り替え時など）
	if (m_Model)
	{
		ModelRelease(m_Model);
		m_Model = nullptr;
	}

	//const char* path = nullptr;		// モデルファイルパス

	const char* path = nullptr;

	// 種類とフェーズで使用するFBXを決定
	switch (m_Type)
	{
	case BuildingType::Glass:
		if		(m_Phase == BuildingPhase::New)		path = g_ModelName[3];
		else if (m_Phase == BuildingPhase::Damaged)	path = g_ModelName[3];
		else										path = g_ModelName[3];
		break;

	case BuildingType::Concrete:
		if		(m_Phase == BuildingPhase::New)		path = g_ModelName[1];
		else if (m_Phase == BuildingPhase::Damaged)	path = g_ModelName[1];
		else										path = g_ModelName[1];
		break;

	case BuildingType::Plant:
		if		(m_Phase == BuildingPhase::New)		path = g_ModelName[4];
		else if (m_Phase == BuildingPhase::Damaged)	path = g_ModelName[4];
		else										path = g_ModelName[4];
		break;

	case BuildingType::Electric:
		if		(m_Phase == BuildingPhase::New)		path = g_ModelName[2];
		else if (m_Phase == BuildingPhase::Damaged)	path = g_ModelName[2];
		else										path = g_ModelName[2];
		break;

	default:
		//path = "asset/build_default.fbx"; // デフォルトモデル
		break;
	}

	if (path)
	{
		char modelPath[256];
		// asset/model/[モデルファイル名].fbx の形式に組み立てる
		snprintf(modelPath, sizeof(modelPath), "asset/model/%s.fbx", path);

		// モデル読み込み
		m_Model = ModelLoad(modelPath);
	}
}
	
//==============================================
// フェーズ変更（New / Damaged / Broken）
//==============================================
void Building::SetPhase(BuildingPhase phase)
{
	if (m_Phase != phase) // 変更がある場合のみ
	{
		m_Phase = phase;
		LoadModelForPhase(); // モデルをリロードして見た目を変える
	}
}

//==============================================
// 毎フレーム更新
//==============================================
void Building::Update()
{
	// 現状は何も行わない
}

//==============================================
// 描画
//==============================================
void Building::Draw()
{
	// 自分のモデルがロードされているかチェック
	if (!m_Model) return;

	//シェーダーを描画パイプラインへ設定
	Shader_Begin();

	//プロジェクション行列作成
	XMMATRIX	Projection = GetProjectionMatrix();
	//ビュー行列作成
	XMMATRIX	View = GetViewMatrix();
	//先にVP変換行列を作っておく
	XMMATRIX VP = View * Projection;


	// スケーリング・回転・平行移動
	XMMATRIX ScalingMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);


	// 要修正　///////////////////////////////////////////////////////////////////////////////////////
	// 自分の座標へ移動（取りあえずy座標にoffsetを足す）
	XMMATRIX TranslationMatrix = XMMatrixTranslation(m_Pos.x, m_Pos.y + 1.0f, m_Pos.z);
	////////////////////////////////////////////////////////////////////////////////////////////

	// モデルが寝ている場合は起こす（-90度回転など）
	XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(-90.0f), 0.0f, 0.0f);

	//ワールド行列の作成
	XMMATRIX	World = ScalingMatrix * RotationMatrix * TranslationMatrix;
	//最終的な変換行列を作成
	XMMATRIX	WVP = World * VP;	//(VP = View * Projection)

	//DirectXへ行列をセット
	Shader_SetWorldMatrix(World);
	Shader_SetMatrix(WVP);

	// 描画実行
	ModelDraw(m_Model);
}
