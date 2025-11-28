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
	XMFLOAT3 pos;
	XMFLOAT3 scale;
	XMFLOAT4 color;
	bool active;

	bool isIllum;
	float illumTimer;
	float illumTotal;
	float illumInterval;
	float illumInTimer;
	XMFLOAT4 illumColor;

	bool isStar;
	float starTimer;

	float fi, wa, wi, ea;
	float gaugeOuter;

	AABB box;
};


void P_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void P_Finalize(void);
void P_Update();
void P_Draw(void);
void P_Color(const XMFLOAT4& color, bool illum);

P* GetP();

void Player_UpdateAABB();
#endif // PLAYER_H
