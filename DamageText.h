#pragma once

#include <DirectXMath.h>
#include "direct3d.h"
#include "color.h"
#include "makeText.h"

enum TextColor;

bool DamageText_Initialize();
void DamageText_Finalize();
void DamageText_Update();
void DamageText_Draw();
void SetDamageText(const XMFLOAT3& worldPos, int damage, TextColor color);