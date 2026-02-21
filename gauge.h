/*==============================================================================

   ポリゴン描画 [gauge.h]
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef gauge_H
#define gauge_H

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "shader.h"
#include "debug_ostream.h"
#include "player.h"

#define GAUGE_PLAYER_MAX (4)

struct GaugeData
{
    float fire, water, wind, earth;
    float outer;
    float skill;
    XMFLOAT2 pos;
    XMFLOAT4 outercolor;
    XMFLOAT2 shakeOffset;
    PlayerType type;
};

extern GaugeData g_Gauge[GAUGE_PLAYER_MAX];

void Gauge_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Gauge_Finalize(void);
void Gauge_Update(void);

//==========================================
// 描画セット（Setで設定しDrawで描画する）
//==========================================
void Gauge_Set(int i, float fire, float water, float wind, float earth, float outer, float skill, const XMFLOAT2& pos, PlayerType type);
void Gauge_Draw(int i);

// 通常ゲージ描画（内ゲージ＋外ゲージ）
void Gauge_DrawBasic(int i);
// スキルゲージ描画（下面／テキスト／上面）
void Gauge_DrawSkill(int i);

void Gauge_SetShakeOffset(int i, const XMFLOAT2& offset);

#endif // gauge_H
