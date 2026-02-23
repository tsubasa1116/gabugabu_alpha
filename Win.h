// Win.h
#pragma once

#include "direct3d.h"


void Win_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Win_Finalize();
void Win_Update();
void Win_Draw();

void DrawSlidingBanner(ID3D11ShaderResourceView* tex, float y, float offset, float width, float height);
void DrawPlayerWinCrownBanner(ID3D11ShaderResourceView* texPlayerWin, TexMetadata& metaPlayerWin, ID3D11ShaderResourceView* texCrown, TexMetadata& metaCrown, float y, float offset, float scale);
