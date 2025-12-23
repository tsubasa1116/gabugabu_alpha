#pragma once

#include "d3d11.h"
#include "collider.h"

struct ATTACK_OBJECT
{
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scaling;
	XMFLOAT3 velocity;
	//bool Use;

	AABB boundingBox;
};

void Attack_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Attack_Finalize();
void Attack_Update(int playerIndex);
void Attack_Draw(int playerIndex);

ATTACK_OBJECT* GetAttack(int playerIndex);
