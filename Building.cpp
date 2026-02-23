#include "Building.h"
#include "field.h"
#include "Camera.h"
#include "keyboard.h"
#include "Effect.h"
#include "player.h"
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

// ★テクスチャのパス用意
static const wchar_t* g_TexturePaths[] =
{ L"Asset\\Texture\\gure.jpg",
	L"Asset\\Texture\\とんがり木.png",   // ← togeki専用
	L"Asset\\Texture\\とんがり木エフェクト.png",
	L"Asset\\Texture\\ライブ.png",
	L"Asset\\Texture\\ライブエフェクト.png",
	L"Asset\\Texture\\美術館.png",
	L"Asset\\Texture\\美術館エフェクト.png",
	L"Asset\\Texture\\こんくり三段.png",
	L"Asset\\Texture\\こんくり三段エフェクト.png",
	L"Asset\\Texture\\３個のコンクリ.png",
	L"Asset\\Texture\\３個のコンクリエフェクト.png",
	L"Asset\\Texture\\４つのガラス.png",
	L"Asset\\Texture\\４つのガラスエフェクト.png",
	L"Asset\\Texture\\信号.png",
	L"Asset\\Texture\\信号エフェクト.png",
	L"Asset\\Texture\\２この丸ガラス.png",
	L"Asset\\Texture\\２この丸ガラスエフェクト.png",
	L"Asset\\Texture\\木と遊具.png",
	L"Asset\\Texture\\木と遊具エフェクト.png",
	L"Asset\\Texture\\木といえ.png",
	L"Asset\\Texture\\木といえエフェクト.png",
	L"Asset\\Texture\\togegarasu.png",
	L"Asset\\Texture\\togegarasuエフェクト.png",
	L"Asset\\Texture\\3kabe.png",
	L"Asset\\Texture\\3kabeエフェクト.png",
	L"Asset\\Texture\\1kabe.png",
	L"Asset\\Texture\\1kabeエフェクト.png",
	L"Asset\\Texture\\textureTreeMain_v3.png",
	L"Asset\\Texture\\textureTreeMainHighlight_v2.png",
	L"Asset\\Texture\\textureTowerMain_v2.png",
	L"Asset\\Texture\\東京タワーエフェクト.png",
	L"Asset\\Texture\\fade.bmp"
};
// 配列要素数から定数を作成（定義と実データの不一致を防ぐ）
static const int FIELD_TEX_MAX = static_cast<int>(sizeof(g_TexturePaths) / sizeof(g_TexturePaths[0]));

// テクスチャ配列（要素数は FIELD_TEX_MAX に合わせる）
static ID3D11ShaderResourceView* g_Texture[FIELD_TEX_MAX] = { nullptr };

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
	"denki1kaba-",
	"denki3kaba-"
};

// 配列数取得マクロ
#define COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

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
	m_Model(nullptr),
	isActive(true),
	isDestroyed(false),
	m_ModelIndex(modelIndex),
	m_TexOffset(0),
	m_IsPlayerNear(false)
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

	// テクスチャ解放（安全のためここに追加）
	for (int i = 0; i < FIELD_TEX_MAX; ++i)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = nullptr;
		}
	}
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
	default:
		//path = "asset/build_default.fbx"; // デフォルトモデル
		break;
	}

	if (modelName == nullptr)
	{
		hal::dout << "Building::LoadModelForPhase: modelName==nullptr for type=" << (int)type << " index=" << m_ModelIndex << std::endl;
		m_Model = nullptr;
		isActive = false; // モデル無ければ描画しない
		return;
	}

	// パス組み立て
	char path[256];
	snprintf(path, sizeof(path), "asset/model/%s.fbx", modelName);

	// モデルロード（例外発生の可能性のある実装ならここでキャッチ）
	try
	{
		m_Model = ModelLoad(path);
	}
	catch (const std::exception& ex)
	{
		hal::dout << "Building::LoadModelForPhase: ModelLoad threw exception for " << path << " : " << ex.what() << std::endl;
		m_Model = nullptr;
		isActive = false;
		return;
	}
	catch (...)
	{
		hal::dout << "Building::LoadModelForPhase: ModelLoad threw unknown exception for " << path << std::endl;
		m_Model = nullptr;
		isActive = false;
		return;
	}

	if (!m_Model)
	{
		hal::dout << "Building::LoadModelForPhase: ModelLoad returned nullptr for " << path << std::endl;
		isActive = false;
	}
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
		strcmp(g_ElectricModels[m_ModelIndex], "denki3kaba-") == 0)
	{
		baseTexIndex = 23;//ok
	}
	else if (type == BuildingType::Electricity &&
		strcmp(g_ElectricModels[m_ModelIndex], "denki1kaba-") == 0)
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
		strcmp(g_GlassModels[m_ModelIndex], "togegarasu") == 0)
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