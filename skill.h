#pragma once

#include "d3d11.h"
#include "collider.h"

struct SKILL1_OBJECT
{
	XMFLOAT3 Position;
	XMFLOAT3 Rotation;
	XMFLOAT3 Scaling;
	XMFLOAT3 Velocity;
	bool Use;

	AABB boundingBox;
};

struct SKILL2_OBJECT
{
	XMFLOAT3 Position;
	XMFLOAT3 Rotation;
	XMFLOAT3 Scaling;
	XMFLOAT3 Velocity;
	bool Use;

	AABB boundingBox;
};

void Player1_Skill_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Player2_Skill_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Skill_Finalize();
void Player1_Skill_Update();
void Player2_Skill_Update();
void Player1_Skill_Draw();
void Player2_Skill_Draw();

SKILL1_OBJECT* GetSkill1Objects();
SKILL2_OBJECT* GetSkill2Objects();
int GetSkill1ObjectCount();
int GetSkill2ObjectCount();
