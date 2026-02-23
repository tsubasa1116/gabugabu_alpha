// Effect.h

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "player.h"

// マクロ定義
#define BUILDING_EFFECT_MAX (50)	// 建物エフェクトの最大数

class EFFECT
{
public:
	bool enable;
	XMFLOAT3 pos;
	XMFLOAT2 size;
	float frameCnt;	// アニメーションカウンター
	int texNo;
	int playerIndex;
	float scaleTimer;
	bool scaleGrowing;
	XMFLOAT2 baseSize;
};

struct PLAYER_EFFECT_ANIM
{
	int evolutionFrame = 0;
	float evolutionTimer = 0.0f;
	int evolutionPhase = 0;		// 0:未進化, 1:進化1, 2:進化2, 3:終了
	int skillFrame = 0;
	float skillTimer = 0.0f;
	int specialFrame = 0;
	float specialTimer = 0.0f;
	int poisonFrame = 0;
	float poisonTimer = 0.0f;
	int attackedFrame = 0;
	float attackedTimer = 0.0f;
	int healingFrame = 0;
	float healingTimer = 0.0f;
	int respawnFrame = 0;
	float respawnTimer = 0.0f;
	int shadowFrame = 0;
	float shadowTimer = 0.0f;
};

struct BUILDING_EFFECT_ANIM
{
	int hitFrame = 0;
	float hitTimer = 0.0f;
	bool hitPlaying = false;	// 再生中フラグ
	bool hitFinished = false;	// 再生完了フラグ（1回だけ再生）
	int hitStartFrame = 0;		// 開始フレーム
	int hitEndFrame = 0;		// 終了フレーム
};

struct EFFECT_LAYER
{
	int texNo;
	int frame;
	int sheetCols;
	int sheetRows;
};

// テクスチャごとの設定
struct EffectConfig {
	int maxFrame;  // 全体フレーム数
	int loopStart; // ループ開始フレーム
	int loopEnd;   // ループ終了フレーム (ここを超えたらloopStartに戻る)
	bool isLoop;   // ループフラグ
	float speed;   // 再生速度（1.0fが標準）
	int spriteY;
};

// メイン処理関数
void Effect_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Effect_Finalize();
void Effect_Update();
void Effect_Draw();
void Effect_SetUI(int texNo, XMFLOAT2 pos, XMFLOAT2 size);
void Effect_Set(int texNo, XMFLOAT2 pos, XMFLOAT2 size, int playerIndex);
void Effect_ClearUI(int pIndex);
void Effect_Clear(int pIndex);

// プレイヤー付近に表示するエフェクト関数
void Effect_UpdateForPlayer(int playerIndex);
void EffectFront_DrawForPlayer(int playerIndex);	// プレイヤーの前に表示するエフェクト（スキルなど）
void EffectShadow_DrawForPlayer(int playerIndex);	// プレイヤーの影エフェクト

// 建物付近に表示するエフェクト関数
void Effect_UpdateForBuilding(int buildingIndex);
void Effect_DrawForBuilding(int buildingIndex);

// 建物エフェクト一括処理関数（軽量版）
void Effect_UpdateAllBuildings();
void Effect_DrawAllBuildings();