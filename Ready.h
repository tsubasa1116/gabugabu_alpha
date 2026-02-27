
//Ready.h
#pragma once

#include "direct3d.h"


void Ready_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Ready_Finalize();
void Ready_Update();
void Ready_Draw();

