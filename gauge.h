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

void Gauge_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Gauge_Finalize(void);
void Gauge_Update(void);
void Gauge_Draw(void);


#endif // gauge_H
