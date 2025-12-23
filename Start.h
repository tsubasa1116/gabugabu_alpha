
//Start.h
#pragma once

#include "direct3d.h"


void Start_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Start_Finalize();
void Start_Update();
void Start_Draw();

