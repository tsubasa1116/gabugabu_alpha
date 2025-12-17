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
#include "direct3d.h"//<<<<<<<<<<<<<<

bool Shader_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Shader_Finalize();

void Shader_SetMatrix(const DirectX::XMMATRIX& matrix);

void Shader_SetWorldMatrix(const DirectX::XMMATRIX& matrix);//<<<<<<<<
void Shader_SetLight(LIGHT light);//<<<<<<<<<<<<<<<<<<<


void Shader_Begin();
void Shader_BeginUI();
void Shader_BeginGauge();
void Shader_BeginOutGauge();
void Shader_BeginHpber();

void Shader_SetGaugeMulti(float fire, float water, float wind, float earth,
	XMFLOAT4 fireColor, XMFLOAT4 waterColor,
	XMFLOAT4 windColor, XMFLOAT4 earthColor);
void Shader_SetOutGauge(float value, XMFLOAT4 color);
void Shader_SetHpber(XMFLOAT4 colA, XMFLOAT4 colB, XMFLOAT2 cnt, float ints, float speed);
void Shader_SetColor(const XMFLOAT4& color);

void Shader_BeginDebugColor();


#endif // SHADER_H
