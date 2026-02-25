#include "Building.h"
#include "field.h"
#include "Camera.h"
#include "keyboard.h"
#include "Effect.h"
#include "player.h"
#include "Audio.h"
#include "debug_ostream.h"     // ← 追加: hal::dout を使うため
#include <codecvt>            // ← 追加: ワイド→UTF-8 変換用
#include <locale>


//=========================================
// グローバル管理
//=========================================
// ★Direct3D デバイス＆コンテキスト
static	ID3D11Device* g_pDevice = NULL;				// テクスチャの場所をGPU上に確保するために使う
static	ID3D11DeviceContext* g_pContext = NULL;		// テクスチャを描画するために使う

// 建物配列（最大100個）
static Building* Buildings[300];

// 現在の建物数
static int BuildingCount = 0;

//static int g_SE_ID[10] = { NULL };

// ★テクスチャのパス用意
static const wchar_t* g_TexturePaths[] =
{ L"Asset\\Texture\\gure.jpg",//1
	L"Asset\\Texture\\とんがり木.png",   			  //1
	L"Asset\\Texture\\とんがり木エフェクト.png",		 //2
	L"Asset\\Texture\\ライブ.png",						//3
	L"Asset\\Texture\\ライブエフェクト.png",		   //4
	L"Asset\\Texture\\美術館.png",						//5
	L"Asset\\Texture\\美術館エフェクト.png",		   //6
	L"Asset\\Texture\\こんくり三段.png",				 //7
	L"Asset\\Texture\\こんくり三段エフェクト.png",		//8
	L"Asset\\Texture\\３個のコンクリ.png",				//9
	L"Asset\\Texture\\３個のコンクリエフェクト.png",   //10
	L"Asset\\Texture\\４つのガラス.png",				 //11
	L"Asset\\Texture\\４つのガラスエフェクト.png",		//12
	L"Asset\\Texture\\信号.png",						 //13
	L"Asset\\Texture\\信号エフェクト.png",				//14
	L"Asset\\Texture\\２この丸ガラス.png",				//15
	L"Asset\\Texture\\２この丸ガラスエフェクト.png",   //16
	L"Asset\\Texture\\木と遊具.png",				   //17
	L"Asset\\Texture\\木と遊具エフェクト.png",		  //18
	L"Asset\\Texture\\木といえ.png",				   //19
	L"Asset\\Texture\\木といえエフェクト.png",		  //20
	L"Asset\\Texture\\togegarasu2.png",				   //21
	L"Asset\\Texture\\togegarasuエフェクト.png",		  //22
	L"Asset\\Texture\\3kabe.png",					   //23
	L"Asset\\Texture\\3kabeエフェクト.png",			  //24
	L"Asset\\Texture\\1kabe.png",					   //25
	L"Asset\\Texture\\1kabeエフェクト.png",			  //26
	L"Asset\\Texture\\textureTreeMain_v3.png",		   //27
	L"Asset\\Texture\\textureTreeMainHighlight_v2.png",//28
	L"Asset\\Texture\\textureTowerMain_v2.png",		   //29
	L"Asset\\Texture\\東京タワーエフェクト.png",		 //30
	L"Asset\\Texture\\fade.bmp"						   //31
};
// 配列要素数から定数を作成（定義と実データの不一致を防ぐ）
static const int FIELD_TEX_MAX = static_cast<int>(sizeof(g_TexturePaths) / sizeof(g_TexturePaths[0]));

// テクスチャ配列（要素数は FIELD_TEX_MAX に合わせる）
static ID3D11ShaderResourceView* g_Texture[FIELD_TEX_MAX] = { nullptr };

//=========================================
// モデル定義（複数対応）
//=========================================

// ガラス建物
static const char* g_GlassModels[] = {
	"3birugarsu",
	"2marugarasu",
	"togegarasu2"

};

// コンクリート建物
static const char* g_ConcreteModels[] = {
	"bizyutukan",
	"biru3dannkonkuri",
	"3biltateconkuri",
};

// 植物建物
static const char* g_PlantModels[] = {
	"kitoyugu",
	"togeki",
	"kitoie",
	"propsTreeMain_v12"
};

// 電気建物
static const char* g_ElectricModels[] = {
	"singou",
	"taw-",
	"raibu",
	"propsElectricitySub03_v9",
	"propsElectricitySub02_v9"
};

// 配列数取得マクロ
#define COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

// --- モデルカタログ --------------------------------------------------------------
static MODEL* g_pGlassModels		[COUNT(g_GlassModels)]		= { nullptr };
static MODEL* g_pConcreteModels		[COUNT(g_ConcreteModels)]	= { nullptr };
static MODEL* g_pPlantModels		[COUNT(g_PlantModels)]		= { nullptr };
static MODEL* g_pElectricModels		[COUNT(g_ElectricModels)]	= { nullptr };


struct ModelBaseSize { float x, y, z; };

static ModelBaseSize g_GlassModelSizes[COUNT(g_GlassModels)] = {
	{1.0f, 2.0f, 1.0f},		// 3birugarsuのサイズ
	{0.8f, 1.5f, 0.8f},		// 2marugarasu
	{0.8f, 1.5f, 0.8f}		// togegarasu2
};

static ModelBaseSize g_ConcreteModelSizes[COUNT(g_ConcreteModels)] = {
	{1.5f, 3.0f, 1.5f},		// bizyutukan
	{1.2f, 2.5f, 1.2f},		// biru3dannkonkuri
	{1.5f, 3.5f, 1.5f}		// 3biltateconkuri
};

static ModelBaseSize g_PlantModelSizes[COUNT(g_PlantModels)] = {
	{1.0f, 2.5f, 1.0f},		// kitoyugu
	{1.5f, 3.0f, 1.5f},		// togoki
	{1.2f, 2.8f, 1.2f},		// kitoie
	{1.5f, 3.5f, 1.5f}		// propsTreeMain_v12
};

static ModelBaseSize g_ElectricModelSizes[COUNT(g_ElectricModels)] = {
	{1.0f, 2.0f, 1.0f},		// singou
	{1.5f, 3.0f, 1.5f},		// taw-
	{1.2f, 2.5f, 1.2f},		// raibu
	{1.5f, 3.5f, 1.5f},		// propsElectricitySub03_v9
	{1.2f, 2.8f, 1.2f}		// propsElectricitySub02_v9
};

//=========================================
// 全建物更新
//=========================================
void Building_UpdateAll()
{
	for (int i = 0; i < BuildingCount; i++)
	{
		if (Buildings[i] && Buildings[i]->isActive)
		{
			Buildings[i]->Update();
		}
	}
}
//=========================================
// 全建物描画
//=========================================
void Building_DrawAll(bool s_IsKonamiCodeEntered)
{
	for (int i = 0; i < BuildingCount; i++)
	{
		if (Buildings[i] && Buildings[i]->isActive)
		{
			Buildings[i]->Draw(s_IsKonamiCodeEntered);
		}
	}
}

//=========================================
// コンストラクタ
//=========================================
Building::Building(BuildingType type, XMFLOAT3 pos, int modelIndex)
	: type(type),
	position(pos),
	Phase(BuildingPhase::New),
	boundingBox({}),
	//m_Model(nullptr),
	isActive(true),
	isDestroyed(false),
	m_ModelIndex(modelIndex),
	m_TexOffset(0),
	m_IsPlayerNear(false)
{
	// 初期トランスフォーム
	scaling = { 1.5f, 1.5f, 1.5f };
	rotation = { 0.0f, 0.0f, 0.0f };

	// モデル番号の範囲チェック
	switch (type)
	{
	case BuildingType::Glass:
		if (m_ModelIndex >= COUNT(g_GlassModels)) m_ModelIndex = 0;
		break;

	case BuildingType::Concrete:
		if (m_ModelIndex >= COUNT(g_ConcreteModels)) m_ModelIndex = 0;
		break;

	case BuildingType::Plant:
		if (m_ModelIndex >= COUNT(g_PlantModels)) m_ModelIndex = 0;
		break;

	case BuildingType::Electricity:
		if (m_ModelIndex >= COUNT(g_ElectricModels)) m_ModelIndex = 0;
		break;
	}

	// モデル読み込み
	//LoadModelForPhase();
}


//=========================================
// デストラクタ
//=========================================
Building::~Building()
{
}

//=========================================
// 初期化（Field から建物生成）
// モデル、テクスチャのロード
// fieldのマップデータから建物を生成
//=========================================
void Building_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// ★デバイス＆コンテキスト保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	// --- 1.fbxのロード ---
	char path[256];

	// ガラス
	for (int i = 0; i < COUNT(g_GlassModels); i++) {
		snprintf(path, sizeof(path), "asset/model/%s.fbx", g_GlassModels[i]);
		g_pGlassModels[i] = ModelLoad(path);
	}
	// コンクリート
	for (int i = 0; i < COUNT(g_ConcreteModels); i++) {
		snprintf(path, sizeof(path), "asset/model/%s.fbx", g_ConcreteModels[i]);
		g_pConcreteModels[i] = ModelLoad(path);
	}
	// 植物
	for (int i = 0; i < COUNT(g_PlantModels); i++) {
		snprintf(path, sizeof(path), "asset/model/%s.fbx", g_PlantModels[i]);
		g_pPlantModels[i] = ModelLoad(path);
	}
	// 電気
	for (int i = 0; i < COUNT(g_ElectricModels); i++) {
		snprintf(path, sizeof(path), "asset/model/%s.fbx", g_ElectricModels[i]);
		g_pElectricModels[i] = ModelLoad(path);
	}

	// --- 2.テクスチャのロード ---

	MAPDATA* map = GetFieldObjects();
	int count = GetFieldObjectCount();

	// ★複数のテクスチャを読み込み
	int texToLoad = FIELD_TEX_MAX;
	// 変換ユーティリティを用意
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	for (int i = 0; i < texToLoad; ++i) // 定義したテクスチャの数だけループ
	{
		TexMetadata metadata;
		ScratchImage image;
		HRESULT hr = LoadFromWICFile(g_TexturePaths[i], WIC_FLAGS_NONE, &metadata, image);
		if (FAILED(hr))
		{
			// 読み込み失敗 → nullptr をセットしてログ（パスは UTF-8 に変換して出力）
			hal::dout << "Building_Initialize: LoadFromWICFile failed for " << conv.to_bytes(g_TexturePaths[i]) << " index=" << i << " hr=0x" << std::hex << hr << std::endl;
			g_Texture[i] = nullptr;
			continue;
		}

		HRESULT hr2 = CreateShaderResourceView(g_pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[i]);
		if (FAILED(hr2) || g_Texture[i] == nullptr)
		{
			// SRV 作成失敗 → nullptr をセットしてログ
			hal::dout << "Building_Initialize: CreateShaderResourceView failed for index=" << i << " hr=0x" << std::hex << hr2 << std::endl;
			g_Texture[i] = nullptr;
			continue;
		}
	}

	// --- 3.Field のマップデータから建物生成 ---
	for (int i = 0; i < count; i++)
	{
		BuildingType type = BuildingType::None;

		// FIELD → BuildingType 変換
		switch (map[i].no)
		{
		case FIELD::FIELD_Glass:		type = BuildingType::Glass;			break;
		case FIELD::FIELD_Concrete:		type = BuildingType::Concrete;		break;
		case FIELD::FIELD_Plant:		type = BuildingType::Plant;			break;
		case FIELD::FIELD_Electricity:	type = BuildingType::Electricity;	break;

		default: continue;
		}

		// Field側で指定したモデル番号を使用
		int modelIndex = map[i].variant;

		// 建物生成（buildingsに追加していく）
		Buildings[BuildingCount++] =
			new Building(type, map[i].pos, modelIndex);

		if (BuildingCount >= 100) break;
	}
}

//=========================================
// 終了処理
//=========================================
void Building_Finalize()
{
	// 建物インスタンスの削除
	for (int i = 0; i < BuildingCount; i++) {
		delete Buildings[i];	// インスタンスの削除
		Buildings[i] = nullptr;
	}
	BuildingCount = 0;	// 安全のためカウンタリセット

	// --- モデルカタログの解放 ---
	for (int i = 0; i < COUNT(g_GlassModels); i++)    if (g_pGlassModels[i])    ModelRelease(g_pGlassModels[i]);
	for (int i = 0; i < COUNT(g_ConcreteModels); i++) if (g_pConcreteModels[i]) ModelRelease(g_pConcreteModels[i]);
	for (int i = 0; i < COUNT(g_PlantModels); i++)    if (g_pPlantModels[i])    ModelRelease(g_pPlantModels[i]);
	for (int i = 0; i < COUNT(g_ElectricModels); i++) if (g_pElectricModels[i]) ModelRelease(g_pElectricModels[i]);

	// --- テクスチャの解放 ---
	for (int i = 0; i < FIELD_TEX_MAX; ++i)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = nullptr;
		}
	}
}

////=========================================
//// モデル読み込み処理
////=========================================
//void Building::LoadModelForPhase()
//{
//	// 既存モデル解放
//	if (m_Model)
//	{
//		ModelRelease(m_Model);
//		m_Model = nullptr;
//	}
//
//	const char* modelName = nullptr;
//
//	// 建物タイプごとにモデル決定
//	switch (type)
//	{
//	case BuildingType::Glass:		modelName = g_GlassModels[m_ModelIndex];	break;
//	case BuildingType::Concrete:	modelName = g_ConcreteModels[m_ModelIndex];	break;
//	case BuildingType::Plant:		modelName = g_PlantModels[m_ModelIndex];	break;
//	case BuildingType::Electricity:	modelName = g_ElectricModels[m_ModelIndex];	break;
//	default:
//		//path = "asset/build_default.fbx"; // デフォルトモデル
//		break;
//	}
//
//	if (modelName == nullptr)
//	{
//		hal::dout << "Building::LoadModelForPhase: modelName==nullptr for type=" << (int)type << " index=" << m_ModelIndex << std::endl;
//		m_Model = nullptr;
//		isActive = false; // モデル無ければ描画しない
//		return;
//	}
//
//	// パス組み立て
//	char path[256];
//	snprintf(path, sizeof(path), "asset/model/%s.fbx", modelName);
//
//	// モデルロード（例外発生の可能性のある実装ならここでキャッチ）
//	try
//	{
//		m_Model = ModelLoad(path);
//	}
//	catch (const std::exception& ex)
//	{
//		hal::dout << "Building::LoadModelForPhase: ModelLoad threw exception for " << path << " : " << ex.what() << std::endl;
//		m_Model = nullptr;
//		isActive = false;
//		return;
//	}
//	catch (...)
//	{
//		hal::dout << "Building::LoadModelForPhase: ModelLoad threw unknown exception for " << path << std::endl;
//		m_Model = nullptr;
//		isActive = false;
//		return;
//	}
//
//	if (!m_Model)
//	{
//		hal::dout << "Building::LoadModelForPhase: ModelLoad returned nullptr for " << path << std::endl;
//		isActive = false;
//	}
//}
//
////=========================================
//// フェーズ変更（将来拡張用）
////=========================================
//void Building::SetPhase(BuildingPhase phase)
//{
//	if (Phase != phase)
//	{
//		Phase = phase;
//		LoadModelForPhase();
//	}
//}

//=========================================
// 更新
//=========================================
void Building::Update()
{
	// プレイヤーとの距離判定
	bool anyPlayerNear = false;

	for (int p = 0; p < PLAYER_MAX; p++)
	{
		PLAYEROBJECT* player = GetPlayer(p);
		if (!player || !player->active) continue;

		float dx = player->position.x - position.x;
		float dy = player->position.y - position.y;
		float dz = player->position.z - position.z;
		float distSq = dx * dx + dy * dy + dz * dz;

		if (distSq <= 3.0f * 3.0f)
		{
			anyPlayerNear = true;
			break;
		}
	}

	// 近づいた瞬間にテクスチャオフセットを+1
	if (anyPlayerNear && !m_IsPlayerNear)
	{
		m_TexOffset = (m_TexOffset + 1) % FIELD_TEX_MAX;
	}
	// 離れた瞬間にテクスチャオフセットをリセット
	else if (!anyPlayerNear && m_IsPlayerNear)
	{
		m_TexOffset = 0;
	}

	m_IsPlayerNear = anyPlayerNear;

	// ======================================================================

	// 1.カタログから自分のモデルサイズを特定
	ModelBaseSize base;
		 if(type == BuildingType::Glass)		base = g_GlassModelSizes[m_ModelIndex];
	else if(type == BuildingType::Concrete)		base = g_ConcreteModelSizes[m_ModelIndex];
	else if(type == BuildingType::Plant)		base = g_PlantModelSizes[m_ModelIndex];
	else if(type == BuildingType::Electricity)	base = g_ElectricModelSizes[m_ModelIndex];
	else return; // タイプ不明なら終わり

	// --- 2. 実際の当たり判定サイズを計算 ---
	// 「モデルの素のサイズ」×「今のスケール設定」
	XMFLOAT3 actualSize;
	actualSize.x = base.x * scaling.x;
	actualSize.y = base.y * scaling.y;
	actualSize.z = base.z * scaling.z;

	// 2. 今の位置とスケールを反映して世界座標のAABBを作る
	// 描画と同じように Y + 1.0f する
	XMFLOAT3 currentPos = position;
	currentPos.y += 1.0f;

	// CalculateAABBをここで呼ぶ（インスタンスごとの計算）
	CalculateAABB(this->boundingBox, currentPos, this->scaling);
}

//=========================================
// 描画
//=========================================
void Building::Draw(bool s_IsKonamiCodeEntered)
{
	// カタログから対象のモデルを取得
	MODEL* pTarget = nullptr;
	switch (type)
	{
	case BuildingType::Glass:       pTarget = g_pGlassModels[m_ModelIndex];     break;
	case BuildingType::Concrete:    pTarget = g_pConcreteModels[m_ModelIndex];  break;
	case BuildingType::Plant:       pTarget = g_pPlantModels[m_ModelIndex];     break;
	case BuildingType::Electricity: pTarget = g_pElectricModels[m_ModelIndex];  break;
	}

	if (!pTarget) return; // モデルがなければ終わり



	Shader_Begin();

	XMMATRIX VP = GetViewMatrix() * GetProjectionMatrix();

	XMMATRIX World =
		XMMatrixScaling(scaling.x, scaling.y, scaling.z) *
		XMMatrixRotationRollPitchYaw(
			rotation.x + XMConvertToRadians(-90.0f),
			rotation.y,
			rotation.z) *
		XMMatrixTranslation(position.x, position.y + 1.0f, position.z);

	Shader_SetWorldMatrix(World);
	Shader_SetMatrix(World * VP);


	//===========================
	// ★ テクスチャ選択
	//===========================
	int baseTexIndex = 0; // デフォルト

	// Plant
	if (type == BuildingType::Plant &&
		strcmp(g_PlantModels[m_ModelIndex], "togeki") == 0)
	{
		baseTexIndex = 1; // とんがり木
	}
	if (type == BuildingType::Plant &&
		strcmp(g_PlantModels[m_ModelIndex], "kitoyugu") == 0)
	{
		baseTexIndex = 17;//ok
	}
	if (type == BuildingType::Plant &&
		strcmp(g_PlantModels[m_ModelIndex], "kitoie") == 0)
	{
		baseTexIndex =19; //ok
	}
	if (type == BuildingType::Plant &&
		strcmp(g_PlantModels[m_ModelIndex], "propsTreeMain_v12") == 0)
	{
		baseTexIndex = 27;//ok 
	}
	// Electricity 
	else if (type == BuildingType::Electricity &&
		strcmp(g_ElectricModels[m_ModelIndex], "raibu") == 0)
	{
		baseTexIndex = 3;//ok
	}
	else if (type == BuildingType::Electricity &&
		strcmp(g_ElectricModels[m_ModelIndex], "singou") == 0)
	{
		baseTexIndex = 13;//ok
	}
	else if (type == BuildingType::Electricity &&
		strcmp(g_ElectricModels[m_ModelIndex], "propsElectricitySub02_v9") == 0)
	{
		baseTexIndex = 23;//ok
	}
	else if (type == BuildingType::Electricity &&
		strcmp(g_ElectricModels[m_ModelIndex], "propsElectricitySub03_v9") == 0)
	{
		baseTexIndex = 25;//ok
	}
	else if (type == BuildingType::Electricity &&
		strcmp(g_ElectricModels[m_ModelIndex], "taw-") == 0)
	{
		baseTexIndex =29;//ok
	}

	// Concrete 
	else if (type == BuildingType::Concrete &&
		strcmp(g_ConcreteModels[m_ModelIndex], "bizyutukan") == 0)
	{
		baseTexIndex = 5; //ok
	}
	else if (type == BuildingType::Concrete &&
		strcmp(g_ConcreteModels[m_ModelIndex], "biru3dannkonkuri") == 0)
	{
		baseTexIndex = 7;//ok
	}
	else if (type == BuildingType::Concrete &&
		strcmp(g_ConcreteModels[m_ModelIndex], "3biltateconkuri") == 0)
	{
		baseTexIndex = 9;//ok
	}
	// Glass 
	else if (type == BuildingType::Glass &&
		strcmp(g_GlassModels[m_ModelIndex], "3birugarsu") == 0)
	{
		baseTexIndex = 11;//ok
	}
	else if (type == BuildingType::Glass &&
		strcmp(g_GlassModels[m_ModelIndex], "2marugarasu") == 0)
	{
		baseTexIndex = 15;//ok
	}
	else if (type == BuildingType::Glass &&
		strcmp(g_GlassModels[m_ModelIndex], "togegarasu2") == 0)
	{
		baseTexIndex = 21;//ok
	}

	// プレイヤー接近時にテクスチャオフセットを加算
	int finalTexIndex = (baseTexIndex + m_TexOffset) % FIELD_TEX_MAX;

	// SRV が nullptr の場合は null をセット（前フレームの SRV が残らないようにする）
	if (finalTexIndex >= 0 && finalTexIndex < FIELD_TEX_MAX && g_Texture[finalTexIndex])
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture[finalTexIndex]);
	}
	else
	{
		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		g_pContext->PSSetShaderResources(0, 1, nullSRV);
	}

	if (!s_IsKonamiCodeEntered) ModelDraw(pTarget);
}

//=========================================
// ゲッター
//=========================================
int GetBuildingCount()
{
	return BuildingCount;
}

Building** GetBuildings()
{
	return Buildings;
}