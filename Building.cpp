//1. 10×10マップに4種類の建物を置く
//MapGrid を使って 10×10 の二次元配列に Building を配置
//初期化時にランダムで Glass / Concrete / Plant / Electric を割り当て可能

//2. 新たに建物クラスを作って管理
//Building クラスで種類（BuildingType）と状態（BuildingPhase）を管理

//3. 種類はスキルに合わせた 4 種類（列挙体で管理）
//enum class BuildingType { Glass, Concrete, Plant, Electric };
//列挙体で管理しているので後でスイッチや配列で扱いやすい

//4. 新品→壊れかけ→壊れた（フェーズ）
//enum class BuildingPhase { New, Damaged, Broken };
//SetPhase() で状態変更可能

//5. フェーズごとに仮テクスチャを変える
//UpdateTexture() でフェーズ変更時に仮テクスチャ文字列を自動更新
//GetTexture() で現在のフェーズのテクスチャ名を取得可能


//nya------

//Building.cpp
#include "Building.h"
#include "model.h"   // ModelLoad / ModelRelease / ModelDraw

// コンストラクタ
Building::Building()
{
    Type = BuildingType::None;     // 種類未設定
    Phase = BuildingPhase::New;    // デフォルトフェーズは新品
    Model = nullptr;               // モデルはまだ読み込まない
}

// デストラクタ
Building::~Building()
{
    if (Model)                     // モデルがある場合は解放
    {
        ModelRelease(Model);
        Model = nullptr;
    }
}

// 初期化（種類を決めてモデルを読み込む）
void Building::Initialize(BuildingType type)
{
    Type = type;                   // 建物種類セット
    Phase = BuildingPhase::New;    // フェーズは新品
    LoadModelForPhase();           // モデル読み込み
}

//==============================================
// フェーズや種類に応じてモデルを読み込む
//==============================================
void Building::LoadModelForPhase()
{
    // すでにモデルがある場合は破棄
    if (Model)
    {
        ModelRelease(Model);
        Model = nullptr;
    }

    const char* path = nullptr;    // モデルファイルパス

    // 種類とフェーズで使用するFBXを決定
    switch (Type)
    {
    case BuildingType::Glass:
        if (Phase == BuildingPhase::New)       path = "asset/build_glass_new.fbx";//仮
        else if (Phase == BuildingPhase::Damaged) path = "asset/build_glass_damage.fbx";//仮
        else                                     path = "asset/build_glass_break.fbx";//仮
        break;

    case BuildingType::Concrete:
        if (Phase == BuildingPhase::New)       path = "asset/build_con_new.fbx";//仮
        else if (Phase == BuildingPhase::Damaged) path = "asset/build_con_damage.fbx";//仮
        else                                     path = "asset/build_con_break.fbx";//仮
        break;

    case BuildingType::Plant:
        if (Phase == BuildingPhase::New)       path = "asset/build_plant_new.fbx";//仮
        else if (Phase == BuildingPhase::Damaged) path = "asset/build_plant_damage.fbx";//仮
        else                                     path = "asset/build_plant_break.fbx";//仮
        break;

    case BuildingType::Electric:
        if (Phase == BuildingPhase::New)       path = "asset/build_ele_new.fbx";//仮
        else if (Phase == BuildingPhase::Damaged) path = "asset/build_ele_damage.fbx";//仮
        else                                     path = "asset/build_ele_break.fbx";//仮
        break;

    default:
        path = "asset/build_default.fbx"; // デフォルトモデル
        break;
    }

    // モデル読み込み
    Model = ModelLoad(path);
}
    
//==============================================
// フェーズ変更（New / Damaged / Broken）
//==============================================
void Building::SetPhase(BuildingPhase phase)
{
    Phase = phase;               // 新しいフェーズを設定
    LoadModelForPhase();         // モデルを再読み込み
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
    if (Model)
        ModelDraw(Model);       // モデル描画
}
