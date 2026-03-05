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
enum class BuildingType {
	None,		// 未設定
	Glass,		// ガラス建物
	Concrete,	// コンクリ建物
	Plant,		// 植物建物
	Electricity,// 電気建物
	Max
};

//=========================================
// 建物の状態（将来拡張用）
//=========================================
enum class BuildingPhase
{
	New,		// 新品
	Damaged,	// 破損
	Broken		// 破壊
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
	int m_FieldIndex;
	// 種類・フェーズに応じて
	// モデルとテクスチャを読み込む
	//void LoadModelForPhase();
	float m_ShakeTimer = 0.0f;
	bool  m_IsShaking = false;
	XMFLOAT3 m_BasePosition;   // ★これを追加
public:

	// トランスフォーム
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scaling;

	// 種類・状態
	BuildingType  type;
	BuildingPhase Phase;

	// 当たり判定（未使用）
	AABB boundingBox;

	// モデル
	MODEL* m_Model;

	bool isActive;		// 有効フラグ
	bool isDestroyed;	// 建物破壊フラグ

	// プレイヤー接近時のテクスチャオフセット
	int m_TexOffset;
	bool m_IsPlayerNear;

	float m_Alpha = 1.0f;      // 1.0（不透明）～ 0.0（完全に透明）
	bool  m_IsFading = false;  // フェードアウト中かどうかのフラグ

	float m_FallSpeed = 0.0f;
	bool  m_IsFalling = false;

	float m_FallTimer = 0.0f;
	//=================================
	// コンストラクタ
	//=================================
	Building(BuildingType type, XMFLOAT3 pos, int modelIndex, int fieldIndex);

	// デストラクタ
	~Building();

	// 更新
	void Update();

	// 描画
	void Draw(bool s_IsKonamiCodeEntered);

	void Rebirth(); // 復活

	// 状態変更
	//void SetPhase(BuildingPhase phase);

	// ゲッター
	BuildingType  GetType()  const { return type; }
	BuildingPhase GetPhase() const { return Phase; }

	const AABB& GetAABB() const { return boundingBox; }

	// 追加: モデル名取得（FBX名）を返す。
	// Building.cpp に実装を追加しています。
	const char* GetModelName() const;

	float m_RespawnTimer = { 10.0f };	// 復活までの秒数
	float m_RebirthAnimTimer = 0.0f;
	int GetFieldIndex() const { return m_FieldIndex; }
	// 既存ファイル: Building.h

};

//=========================================
// Building 管理用関数
//=========================================
void Building_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Building_Finalize();
void Building_DrawAll(bool s_IsKonamiCodeEntered);
void Building_UpdateAll();

int GetBuildingCount();
Building** GetBuildings();