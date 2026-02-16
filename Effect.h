// Effect.h

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "player.h"

class EFFECT
{
public:
	bool enable;
	XMFLOAT3 pos;
	XMFLOAT2 size;
	int frameCnt;	// アニメーションカウンター
	int texNo;
};

struct EffectAnimState
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
	int runDustFrame = 0;
	float runDustTimer = 0.0f;
};

static EffectAnimState g_effectAnim[PLAYER_MAX];

struct EffectLayer
{
	int texNo;
	int frame;
	int sheetCols;
	int sheetRows;
};

// メイン処理関数
void Effect_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Effect_Finalize();
void Effect_Update();
void Effect_Draw();
void Effect_Set(int texNo, XMFLOAT2 pos, XMFLOAT2 size);
void Effect_Clear(int pIndex);

// プレイヤー付近に表示するエフェクト関数
void Effect_UpdateForPlayer(int playerIndex);
void Effect_DrawForPlayer(int playerIndex);
