/*==============================================================================

   シェーダー [shader.h]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SHADER_H
#define	SHADER_H

#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"

bool Shader_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Shader_Finalize();

void Shader_SetMatrix(const DirectX::XMMATRIX& matrix);

void Shader_SetWorldMatrix(const DirectX::XMMATRIX& matrix);
void Shader_SetLight(LIGHT light);

void Shader_Begin();
void Shader_BeginUI();
void Shader_BeginGauge();
void Shader_BeginOutGauge();
void Shader_BeginSkillGauge();
void Shader_BeginHpber();

void Shader_SetGaugeMulti(float glass, float concrete, float plant, float electric);
void Shader_SetOutGauge(float value, XMFLOAT4 color);
void Shader_SetSingleGauge(float fill);
void Shader_SetHpber(XMFLOAT4 colA, XMFLOAT4 colB, float al, float speed);
void Shader_SetColor(const XMFLOAT4& color);

// 線形補間カラー設定　mulColor=乗算色、lerpColor=補間する色、lerpFactor=補間の度合い
void Shader_SetColorLerp(const XMFLOAT4& mulColor, const XMFLOAT4& lerpColor, float lerpFactor);

void Shader_BeginDebugColor();

void Shader_SetDrawMode(int mode);

void Shader_SetGaugeTextures();
void Shader_SetOutGaugeTextures();
void Shader_SetSkillGaugeTextures(int typeIndex);
void Shader_SetSkillCoolGaugeTextures(int typeIndex);
void Shader_SetSkillTextTextures(int typeIndex);
void Shader_SetBlur();


#endif // SHADER_H
