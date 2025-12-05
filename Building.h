//Building.h
#pragma once
#include <string>

#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
#include "sprite.h"
#include "shader.h"
#include "collider.h"
#include "Building.h"
#include "model.h"

using namespace DirectX;

//=========================================
// 列挙型定義
//=========================================
enum class BuildingType {
	Glass,		// ガラス建物
	Concrete,	// コンクリ建物
	Plant,		// 植物建物
	Electric,	// 電気建物
	None,		// 未設定
	Max
};

enum class BuildingPhase {
	New,		// 新品
	Damaged,	// 壊れかけ
	Broken		// 壊れた
};

//=========================================
// Building クラス
//=========================================
class Building
{
private:
public:
	XMFLOAT3 position;		// 座標
	XMFLOAT3 rotation;		// 回転
	XMFLOAT3 scaling;		// スケール

	BuildingType Type;		// 建物の種類
	BuildingPhase Phase;	// 現在のフェーズ（状態）

	AABB boundingBox;

	MODEL* m_Model;			// 現在のフェーズ用モデル

	bool isActive;

	// フェーズや種類に応じたモデルを読み込む内部関数
	void LoadModelForPhase();


	// コンストラクタ（種類と座標を指定して生成）
	Building(BuildingType type, XMFLOAT3 pos);

	// デストラクタ
	~Building();

	// 更新
	void Update();

	// 描画
	void Draw();

	// フェーズ変更（New → Damaged → Broken）
	void SetPhase(BuildingPhase phase);

	// ゲッター
	BuildingPhase GetPhase() const { return Phase; }
	BuildingType GetType() const { return Type; }
	XMFLOAT3 GetPosition() const { return position; }
};

//=========================================
// 管理用関数（Building.cpp で実装）
//=========================================
void Building_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Building_Finalize();
void Building_DrawAll(bool s_IsKonamiCodeEntered);

// 建物の総数を取得
int GetBuildingCount();

// 建物のリスト（ポインタ配列）の先頭アドレスを取得
// g_Buildings は Building* の配列なので、戻り値は Building** 
Building** GetBuildings();