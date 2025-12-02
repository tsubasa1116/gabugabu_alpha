//Building.h
#pragma once
#include <string>

// 建物の種類を列挙
enum class BuildingType {
    Glass,      // ガラス建物
    Concrete,   // コンクリ建物
    Plant,      // 植物建物
    Electric,   // 電気建物
    None        // 未設定
};

// 建物の状態（フェーズ）を列挙
enum class BuildingPhase {
    New,        // 新品
    Damaged,    // 壊れかけ
    Broken      // 壊れた
};

// 前方宣言
class MODEL;

// 建物クラス
class Building
{
private:
    BuildingType Type;      // 建物の種類
    BuildingPhase Phase;    // 現在のフェーズ（状態）

    MODEL* Model;           // 現在のフェーズ用モデル（3Dデータ）

    // フェーズや種類に応じたモデルを読み込む関数
    void LoadModelForPhase();

public:
    Building();             // コンストラクタ（初期化）
    ~Building();            // デストラクタ（モデル解放）

    // 初期化（建物の種類をセットし、モデルを読み込む）
    void Initialize(BuildingType type);

    // 毎フレーム更新用
    void Update();

    // 描画
    void Draw();

    // フェーズ変更（New → Damaged → Broken）
    void SetPhase(BuildingPhase phase);

    // ゲッター
    BuildingPhase GetPhase() const { return Phase; }
    BuildingType GetType() const { return Type; }
};
