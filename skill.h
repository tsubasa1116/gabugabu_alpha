#pragma once

#include "d3d11.h"
#include "collider.h"

struct SKILL_OBJECT
{
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scaling;
	XMFLOAT3 velocity;
	//bool Use;

	AABB boundingBox;
};

void Skill_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Skill_Finalize();
void Player1_Skill_Update();
void Player2_Skill_Update();
void Player1_Skill_Draw();
void Player2_Skill_Draw();

SKILL_OBJECT* GetSkill(int index);
int GetSkill1ObjectCount();
int GetSkill2ObjectCount();
