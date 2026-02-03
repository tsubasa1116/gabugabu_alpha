#include "Building.h"
#include "field.h"
#include "Camera.h"
#include "keyboard.h"

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
#define FIELD_TEX_MAX 3
static ID3D11ShaderResourceView* g_Texture[FIELD_TEX_MAX];
static const wchar_t* g_TexturePaths[FIELD_TEX_MAX] = {
	L"Asset\\Texture\\gure.jpg",
	L"Asset\\Texture\\textureGlassMain_v1.png",	// ★とりあえず今はこれを張ってる
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
	"torii_ki"
};

// 電気建物
static const char* g_ElectricModels[] = {
	"singou",
	"tawa-",
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
	: Type(type),
	position(pos),
	Phase(BuildingPhase::New),
	m_Model(nullptr),
	isActive(true),
	m_ModelIndex(modelIndex)
{
	// 初期トランスフォーム
	scaling = { 1.0f, 1.0f, 1.0f };
	rotation = { 0.0f, 0.0f, 0.0f };

	// モデル番号の範囲チェック
	switch (Type)
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
	switch (Type)
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
	// 今は未使用
}

//=========================================
// 描画
//=========================================
void Building::Draw(bool)
{
	if (!m_Model) return;

	Shader_Begin();

	// View × Projection
	XMMATRIX VP = GetViewMatrix() * GetProjectionMatrix();

	// World行列
	XMMATRIX World =
		XMMatrixScaling(scaling.x, scaling.y, scaling.z) *
		XMMatrixRotationRollPitchYaw(
			rotation.x + XMConvertToRadians(-90.0f),
			rotation.y,
			rotation.z) *
		XMMatrixTranslation(position.x, position.y + 1.0f, position.z);

	Shader_SetWorldMatrix(World);
	Shader_SetMatrix(World * VP);

	// ★テクスチャセット
	g_pContext->PSSetShaderResources(0, 1, &g_Texture[1]);

	ModelDraw(m_Model);
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
