
//Sound.h
#pragma once

#include "direct3d.h"


void Sound_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Sound_Finalize();
void Sound_Update();
void Sound_Draw();

