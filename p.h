/*==============================================================================

   ƒ|ƒŠƒSƒ“•`‰æ [player.h]
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef PLAYER_H
#define PLAYER_H

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "shader.h"
#include "debug_ostream.h"
#include "collider.h"

struct P
{
	int HP;
	int MaxHP;
	int stock;
	XMFLOAT3 pos;
	XMFLOAT3 scale;
	XMFLOAT4 color;
	bool active;

	float fi, wa, wi, ea;
	float gaugeOuter;

	AABB box;
};


void P_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void P_Finalize(void);
void P_Update();
void P_Draw(void);
void PStock_Draw(int i);

P* GetP();

void Player_UpdateAABB();
#endif // PLAYER_H
