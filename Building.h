#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "shader.h"
#include "collider.h"
#include "model.h"

using namespace DirectX;

//=========================================
// 建物の種類
//=========================================
enum class BuildingType
{
	Glass,      // ガラス建物
	Concrete,   // コンクリート建物
	Plant,      // 植物建物
	Electric,   // 電気建物
	None,
	Max
};

//=========================================
// 建物の状態（将来拡張用）
//=========================================
enum class BuildingPhase
{
	New,        // 新品
	Damaged,    // 破損
	Broken      // 破壊
};

//=========================================
// Building クラス
//=========================================
class Building
{
private:
	// モデル＆テクスチャの番号
	// （同じ番号で両方を管理）
	int m_ModelIndex;

	// 種類・フェーズに応じて
	// モデルとテクスチャを読み込む
	void LoadModelForPhase();

public:
	// トランスフォーム
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scaling;

	// 種類・状態
	BuildingType  Type;
	BuildingPhase Phase;

	// 当たり判定（未使用）
	AABB boundingBox;

	// モデル
	MODEL* m_Model;

	// 有効フラグ
	bool isActive;

	//=================================
	// コンストラクタ
	//=================================
	Building(BuildingType type, XMFLOAT3 pos, int modelIndex = 0);

	// デストラクタ
	~Building();

	// 更新
	void Update();

	// 描画
	void Draw(bool s_IsKonamiCodeEntered);

	// 状態変更
	void SetPhase(BuildingPhase phase);

	// ゲッター
	BuildingType  GetType()  const { return Type; }
	BuildingPhase GetPhase() const { return Phase; }
};

//=========================================
// Building 管理用関数
//=========================================
void Building_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Building_Finalize();
void Building_DrawAll(bool s_IsKonamiCodeEntered);

int GetBuildingCount();
Building** GetBuildings();


