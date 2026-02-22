#include "Building.h"
#include "field.h"
#include "Camera.h"
#include "keyboard.h"
#include "Effect.h"

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

// ★テクスチャのパス用意
#define FIELD_TEX_MAX 17
static ID3D11ShaderResourceView* g_Texture[FIELD_TEX_MAX];
static const wchar_t* g_TexturePaths[FIELD_TEX_MAX] = {
	L"Asset\\Texture\\gure.jpg",
	L"Asset\\Texture\\とんがり木.png",   // ← togeki専用
	L"Asset\\Texture\\ライブ.png",
	L"Asset\\Texture\\美術館.png",
	L"Asset\\Texture\\こんくり三段.png",
	L"Asset\\Texture\\３個のコンクリ.png",
	L"Asset\\Texture\\４つのガラス.png",
	L"Asset\\Texture\\信号.png",
	L"Asset\\Texture\\２この丸ガラス.png",
	L"Asset\\Texture\\木と遊具.png",
	L"Asset\\Texture\\木といえ.png",
	L"Asset\\Texture\\togegarasu.png",
	L"Asset\\Texture\\3kabe.png",
	L"Asset\\Texture\\1kabe.png",
	L"Asset\\Texture\\textureTreeMain_v3.png",
	L"Asset\\Texture\\textureTowerMain_v2.png",
	L"Asset\\Texture\\fade.bmp"
};


//=========================================
// モデル定義（複数対応）
//=========================================

// ガラス建物
static const char* g_GlassModels[] = {
	"3birugarsu",
	"2marugarasu",
	"anaaiterugarasu",
	"togegarasu"

};

// コンクリート建物
static const char* g_ConcreteModels[] = {
	"bizyutukan",
	"biru3dannkonkuri",
	"3biltateconkuri",

};

// 植物建物
static const char* g_PlantModels[] = {
	"propsTreeSub_v2",
	"kitoyugu",
	"togeki",
	"kitoie",
	"propsTreeMain_v10",
	"propsTreeMain_v12"
};

// 電気建物
static const char* g_ElectricModels[] = {
	"singou",
	"taw-",
	"raibu",
	"denki1kaba-",
	"denki3kaba-"

};

// 配列数取得マクロ
#define COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

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
	m_Model(nullptr),
	isActive(true),
	isDestroyed(false),
	m_ModelIndex(modelIndex)
{
	// 初期トランスフォーム
	scaling = { 1.0f, 1.0f, 1.0f };
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
	LoadModelForPhase();
}

//=========================================
// デストラクタ
//=========================================
Building::~Building()
{
	if (m_Model)
	{
		ModelRelease(m_Model);
		m_Model = nullptr;
	}
}

//=========================================
// 初期化（Field から建物生成）
//=========================================
void Building_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	Building_Finalize();

	// ★デバイス＆コンテキスト保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	MAPDATA* map = GetFieldObjects();
	int count = GetFieldObjectCount();

	// ★複数のテクスチャを読み込み
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

		// 建物生成
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
	for (int i = 0; i < BuildingCount; i++)
	{
		delete Buildings[i];
		Buildings[i] = nullptr;
	}
	BuildingCount = 0;
}

//=========================================
// モデル読み込み処理
//=========================================
void Building::LoadModelForPhase()
{
	// 既存モデル解放
	if (m_Model)
	{
		ModelRelease(m_Model);
		m_Model = nullptr;
	}

	const char* modelName = nullptr;

	// 建物タイプごとにモデル決定
	switch (type)
	{
	case BuildingType::Glass:		modelName = g_GlassModels[m_ModelIndex];	break;
	case BuildingType::Concrete:	modelName = g_ConcreteModels[m_ModelIndex];	break;
	case BuildingType::Plant:		modelName = g_PlantModels[m_ModelIndex];	break;
	case BuildingType::Electricity:	modelName = g_ElectricModels[m_ModelIndex];	break;

	return;
	default:
		//path = "asset/build_default.fbx"; // デフォルトモデル
		break;
	}

	// パス組み立て
	char path[256];
	snprintf(path, sizeof(path), "asset/model/%s.fbx", modelName);

	// モデルロード
	m_Model = ModelLoad(path);
}

//=========================================
// フェーズ変更（将来拡張用）
//=========================================
void Building::SetPhase(BuildingPhase phase)
{
	if (Phase != phase)
	{
		Phase = phase;
		LoadModelForPhase();
	}
}

//=========================================
// 更新
//=========================================
void Building::Update()
{
}

//=========================================
// 描画
//=========================================
void Building::Draw(bool s_IsKonamiCodeEntered)
{
	if (!m_Model) return;

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
	ID3D11ShaderResourceView* tex = g_Texture[0]; // デフォルト

	// Plant 
	if (type == BuildingType::Plant &&
		strcmp(g_PlantModels[m_ModelIndex], "togeki") == 0)
	{
		tex = g_Texture[1]; // とんがり木
	}
	if (type == BuildingType::Plant &&
		strcmp(g_PlantModels[m_ModelIndex], "kitoyugu") == 0)
	{
		tex = g_Texture[9]; 
	}
	if (type == BuildingType::Plant &&
		strcmp(g_PlantModels[m_ModelIndex], "kitoie") == 0)
	{
		tex = g_Texture[10]; // とんがり木
	}
	if (type == BuildingType::Plant &&
		strcmp(g_PlantModels[m_ModelIndex], "propsTreeMain_v12") == 0)
	{
		tex = g_Texture[14]; // とんがり木
	}
	// Electricity 
	else if (type == BuildingType::Electricity &&
		strcmp(g_ElectricModels[m_ModelIndex], "raibu") == 0)
	{
		tex = g_Texture[2]; // ライブ
	}
	else if (type == BuildingType::Electricity &&
		strcmp(g_ElectricModels[m_ModelIndex], "singou") == 0)
	{
		tex = g_Texture[7]; 
	}
	else if (type == BuildingType::Electricity &&
		strcmp(g_ElectricModels[m_ModelIndex], "denki3kaba-") == 0)
	{
		tex = g_Texture[12];
	}
	else if (type == BuildingType::Electricity &&
		strcmp(g_ElectricModels[m_ModelIndex], "denki1kaba-") == 0)
	{
		tex = g_Texture[13];
	}
	else if (type == BuildingType::Electricity &&
		strcmp(g_ElectricModels[m_ModelIndex], "taw-") == 0)
	{
		tex = g_Texture[15];
	}


	// Concrete 
	else if (type == BuildingType::Concrete &&
		strcmp(g_ConcreteModels[m_ModelIndex], "bizyutukan") == 0)
	{
		tex = g_Texture[3]; // 美術館
	}
	else if (type == BuildingType::Concrete &&
		strcmp(g_ConcreteModels[m_ModelIndex], "biru3dannkonkuri") == 0)
	{
		tex = g_Texture[4]; 
	}
	else if (type == BuildingType::Concrete &&
		strcmp(g_ConcreteModels[m_ModelIndex], "3biltateconkuri") == 0)
	{
		tex = g_Texture[5];
	}
	// Glass 
	else if (type == BuildingType::Glass &&
		strcmp(g_GlassModels[m_ModelIndex], "3birugarsu") == 0)
	{
		tex = g_Texture[6];
	}
	else if (type == BuildingType::Glass &&
		strcmp(g_GlassModels[m_ModelIndex], "2marugarasu") == 0)
	{
		tex = g_Texture[8];
	}
	else if (type == BuildingType::Glass &&
		strcmp(g_GlassModels[m_ModelIndex], "togegarasu") == 0)
	{
		tex = g_Texture[11];
	}

	g_pContext->PSSetShaderResources(0, 1, &tex);

	if (!s_IsKonamiCodeEntered) ModelDraw(m_Model);
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
