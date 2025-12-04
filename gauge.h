#pragma once
/*==============================================================================

   ƒ|ƒŠƒSƒ“•`‰æ [gauge.h]
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

#define GAUGE_PLAYER_MAX 4

struct GaugeData
{
    float fire, water, wind, earth;
    float outer;
    XMFLOAT2 pos;
};

extern GaugeData g_Gauge[GAUGE_PLAYER_MAX];

void Gauge_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Gauge_Finalize(void);
void Gauge_Update(void);

//==========================================
// •`‰æƒZƒbƒgiSet‚Åİ’è‚µDraw‚Å•`‰æ‚·‚éj
//==========================================
void Gauge_Set(int i, float fire, float water, float wind, float earth,
               float outer, const XMFLOAT2& pos);
void Gauge_Draw(int i);


#endif // gauge_H
