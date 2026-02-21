// attack.cpp

#include <DirectXMath.h>
#include <d3d11.h>
using namespace DirectX;
#include "attack.h"
#include "sprite.h"
#include "shader.h"
#include "Camera.h"
#include "collider.h"
#include "field.h"
#include "Building.h"
#include "debug_ostream.h"
#include "player.h"
#include "keyboard.h"
#include "DamageText.h"
#include "Effect.h"
#include "input.h"
#include "hp.h"

#include "color.h"

// �O���[�o���ϐ�
static ID3D11Device* g_pDevice = NULL;
static ID3D11DeviceContext* g_pContext = NULL;

// ���_�o�b�t�@
static ID3D11Buffer* g_VertexBuffer;

// �C���f�b�N�X�o�b�t�@
static ID3D11Buffer* g_IndexBuffer;

// �e�N�X�`���ϐ�
static ID3D11ShaderResourceView* g_Attack_Texture[PLAYER_MAX];

// �I�u�W�F�N�g
static ATTACK_OBJECT Attack[PLAYER_MAX];

// �}�N����`
#define ATTACK_VERTEX (24)

static Vertex2 Attack_vdata[ATTACK_VERTEX] =
{
	// -Z�� (�@��: 0,0,-1)
	{// ���_0 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),		// ���W
		XMFLOAT3(0.0f, 0.0f, -1.0f),		// �@��
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),	// �J���[
		XMFLOAT2(0.0f,0.0f)					// �e�N�X�`�����W
	},
	{// ���_1 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// ���_2 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// ���_3 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// +X�� (�@��: 1,0,0)
	{// ���_4 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// ���_5 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// ���_6 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// ���_7 RIGHT-BOTTM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// +Z�� (�@��: 0,0,1)
	{// ���_8 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// ���_9 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// ���_10 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// ���_11 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// -X�� (�@��: -1,0,0)
	{// ���_12 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// ���_13 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// ���_14 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// ���_15 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// +Y�� (�@��: 0,1,0)
	{// ���_16 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// ���_17 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// ���_18 LEFT-BOTTOM
		XMFLOAT3(-0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// ���_19 RIGHT-BOTTOM
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// -Y�� (�@��: 0,-1,0)
	{// ���_20 LEFT-TOP
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// ���_21 RIGHT-TOP
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// ���_22 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// ���_23 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},
};

// �C���f�b�N�X�z��
static UINT Attack_idxdata[6 * 6]
{
	 0,  1,  2,  2,  1,  3, // -Z��
	 4,  5,  6,  6,  5,  7, // +X��
	 8,  9, 10, 10,  9, 11, // +Z��
	12, 13, 14, 14, 13, 15, // -X��
	16, 17, 18, 18, 17, 19, // +Y��
	20, 21, 22, 22, 21, 23, // -Y��
};

void Attack_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	for (int p = 0; p < PLAYER_MAX; p++)
	{
		Attack[p].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Attack[p].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Attack[p].scaling = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	// ���_�o�b�t�@�쐬
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex2) * ATTACK_VERTEX; // �i�[�ł��钸�_�� * ���_�T�C�Y
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	g_pDevice = pDevice;
	g_pContext = pContext;

	// �e�N�X�`���ǂݍ���
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(),image.GetImageCount(), metadata, &g_Attack_Texture[0]);
	assert(g_Attack_Texture[0]);

	LoadFromWICFile(L"Asset\\Texture\\SkyBlue.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(),image.GetImageCount(), metadata, &g_Attack_Texture[1]);
	assert(g_Attack_Texture[1]);

	// �C���f�b�N�X�o�b�t�@�쐬
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(UINT) * 6 * 6; // �i�[�ł��钸�_�� * ���_�T�C�Y

		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&bd, NULL, &g_IndexBuffer);

		// �C���f�b�N�X�o�b�t�@�֏�������
		D3D11_MAPPED_SUBRESOURCE msr;
		pContext->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		UINT* index = (UINT*)msr.pData;

		// �C���f�b�N�X�f�[�^���o�b�t�@�փR�s�[
		CopyMemory(&index[0], &Attack_idxdata[0], sizeof(UINT) * 6 * 6);
		pContext->Unmap(g_IndexBuffer, 0);
	}
}

void Attack_Finalize()
{
	if (g_VertexBuffer != NULL)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
	if (g_IndexBuffer != NULL)
	{
		g_IndexBuffer->Release();
		g_IndexBuffer = NULL;
	}

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (g_Attack_Texture[i])
		{
			g_Attack_Texture[i]->Release();
			g_Attack_Texture[i] = NULL;
		}
	}
}

void Attack_Update(int playerIndex)
{
	// �͈̓`�F�b�N 0 1 2 3 �ȊO�Ȃ� return
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	ATTACK_OBJECT& atttackObject = Attack[playerIndex];

	if (player.isAttacking == true)
	{
		float Player_RotationY = player.rotation.y;
		float rad = XMConvertToRadians(Player_RotationY);

		// �i�s�������v�Z
		XMFLOAT3 dir =
		{
			sinf(rad),	// X����
			0.0f,		// Y�����i�����j
			cosf(rad)	// Z����
		};

		// �v���C���[�̑O���ɂ��Ԃ��Ԃ�z�u
		atttackObject.position.x = dir.x * player.scaling.x + player.position.x;
		atttackObject.position.y = player.position.y;
		atttackObject.position.z = dir.z * player.scaling.z + player.position.z;

		// �U���^�C�}�[�X�V
		player.attackTimer += DELTA_TIME;

		// �v���C���[���̍U�����Ԃ��o�߂�����U���I��
		if (player.attackTimer >= ATTACKING_TIME)
		{
			player.isAttacking = false;
			player.attackTimer = 0.0f;
		}
	}

	// -------------------------------------------------------------
	// �����蔻��
	// -------------------------------------------------------------
	// AABB�̍X�V
	CalculateAABB(atttackObject.boundingBox, atttackObject.position, XMFLOAT3(1.0f, 1.0f, 1.0f));

	int buildingCount = GetBuildingCount();			// �����擾
	Building** buildingObjects = GetBuildings();	// ���X�g���擾

	// �S�Ẵt�B�[���h�I�u�W�F�N�g�ƏՓ˔�����s��
	for (int i = 0; i < buildingCount; ++i)
	{
		// ��A�N�e�B�u�ȃI�u�W�F�N�g���X�L�b�v�i��d�ŃQ�[�W�����Z����邱�Ƃ�h�����߁j
		if (!buildingObjects[i]->isActive) continue;

		// i�Ԗڂ̃t�B�[���h�I�u�W�F�N�g��AABB���擾
		// field.cpp��Initialize�Ōv�Z�ς݂̂��߁A���̂܂܎Q��
		AABB pStaticObjectAABB = buildingObjects[i]->boundingBox;

		// �v���C���[��AABB�ƃt�B�[���h�I�u�W�F�N�g��AABB��MTV���v�Z
		MTV collision = CalculateAABBMTV(atttackObject.boundingBox, pStaticObjectAABB);

		Keyboard_Keys_tag confirmKey[PLAYER_MAX] = { KK_SPACE , KK_ENTER, KK_V, KK_SPACE };

		// �����iFIELD_BUILDING�j�ɏՓ˂��Ă��āA���e�X�̃v���C���[�̂��Ԃ��ԃL�[��������Ă�����
		if (collision.isColliding)
		{
			BuildingType type = buildingObjects[i]->type;

			if (g_Input[playerIndex].A || Keyboard_IsKeyDown(confirmKey[playerIndex]))
			{
				// �e�����^�C�v���Ƃ̏���
				switch (type)
				{
				case BuildingType::Glass:
					player.breakCount_Glass += 1;						// ガラスを壊した数をプラス
					break;

				case BuildingType::Concrete:
					player.breakCount_Concrete += 1;					// コンクリートを壊した数をプラス
					break;

				case BuildingType::Plant:					
					player.breakCount_Plant += 1;						// 植物を壊した数をプラス
					break;

				case BuildingType::Electricity:
					player.breakCount_Electricity += 1;					// 電気を壊した数をプラス
					break;

				default:
					break;
				}

				buildingObjects[i]->isActive = false;				// 建物を非アクティブ化
				buildingObjects[i]->isDestroyed = true;				// 建物破壊フラグを有効
				player.evolutionGauge += player.evolutionGaugeRate;	// 進化ゲージをプラス
				player.brokenHistory.push_back(type);				// 最後に破壊した建物タイプを保存

				// HP回復
				player.hp += 10.0f;
				// HPの上限
				if (player.hp > PLAYER_MAX_HP)	player.hp = PLAYER_MAX_HP;

				player.isHealing = true;	// 回復中フラグを立てる
				
				// 満腹度増加
				player.satiety += 1.0f;
				// 満腹度の上限
				if (player.satiety > PLAYER_MAX_SATIETY)	player.satiety = PLAYER_MAX_SATIETY;

				// 効果音やエフェクトを再生

				// ヒットでスキルを終了
				player.isAttacking = false;
				player.attackTimer = 0.0f;

				// 更新済みAABB
				CalculateAABB(atttackObject.boundingBox, atttackObject.position, atttackObject.scaling);
			}

			// �Փ˂��Ă�����AMTV�̕������ʒu��߂�
			atttackObject.position.x += collision.translation.x;
			atttackObject.position.y += collision.translation.y;
			atttackObject.position.z += collision.translation.z;

			// �����߂���̐V����AABB���Čv�Z
			// ����ɂ��A�����t���[�����Ŏ��̃t�B�[���h�I�u�W�F�N�g�Ƃ̔���ɔ����܂��B
			CalculateAABB(atttackObject.boundingBox, atttackObject.position, atttackObject.scaling);

			// �f�o�b�O�o��
			//hal::dout << "�ՓˁI�����߂���: " << collision.overlap << " @ " << (collision.translation.x != 0 ? "X��" : (collision.translation.y != 0 ? "Y��" : "Z��")) << std::endl;

			// �������@#include "debug_ostream.h"�@�̃C���N���[�h�Ńf�o�b�O�m�F
		}
	}

	// --- �i������ ---
	if (player.evolutionGauge >= EVOLUTIONGAUGE_MAX)
	{
		// �v���C���[�𖳓G�ɂ���
		player.isInvincible = true;
		player.invincibleTimer = 0.0f;

		// 進化フラグを立てる
		player.isEvolving = true;
		player.evolvingTimer += DELTA_TIME;

		// 現在のフォーム（進化前の状態）を保存
		Form currentForm = player.form;

		// 1. �i���i�K��1�i�߂�
		player.form = static_cast<Form>(static_cast<int>(player.form) + 1);

		// 2. ��3�`�Ԃ܂ł����i�����Ȃ��悤�ɐ���
		if (player.form >= Form::Third)
		{
			player.form = Form::Third;
		}

		// 3. �^�C�v���胍�W�b�N
		//    Type�̌���́ANormal���� FirstEvolution�ɐi������ꍇ�̂ݎ��s
		if (currentForm == Form::First)
		{
			// 4��ނ̔j�󂵂���������z��Ɋi�[
			const int counts[4] =
			{
				player.breakCount_Glass,		// idx 0
				player.breakCount_Concrete,		// idx 1
				player.breakCount_Plant,		// idx 2
				player.breakCount_Electricity	// idx 3
			};

			// �Ή�����^�C�v��`
			const BuildingType types[4] =
			{
				BuildingType::Glass,
				BuildingType::Concrete,
				BuildingType::Plant,
				BuildingType::Electricity
			};

			// --- Step 1: �ő�J�E���g��(maxCount)�����߂� ---
			int maxCount = 0;
			for (int i = 0; i < 4; i++)
			{
				if (counts[i] > maxCount)
				{
					maxCount = counts[i];
				}
			}

			// --- Step 2: �������u�ŐV�v����u�ߋ��v�֑k���ď��҂����߂� ---
			int maxIdx = 0;

			// vector����납���
			for (int i = player.brokenHistory.size() - 1; i >= 0; i--)
			{
				BuildingType historyType = player.brokenHistory[i];
				int typeIdx = -1;

				// �^�C�v���C���f�b�N�X�ԍ��ɕϊ�
				for (int j = 0; j < 4; j++)
				{
					if (historyType == types[j])
					{
						typeIdx = j;
						break;
					}
				}

				// �u�����Ă��闚���̃^�C�v�v���u�ő�J�E���g�������O���[�v�v�̈�����H
				if (typeIdx != -1 && counts[typeIdx] == maxCount)
				{
					maxIdx = typeIdx;
					break; // �����������_�Ŋm��I
				}
			}
			// --- Step 3: �ŏI�^�C�v���f ---
			switch (maxIdx)
			{
			case 0: player.type = PlayerType::Glass;		break;
			case 1: player.type = PlayerType::Concrete;		break;
			case 2: player.type = PlayerType::Plant;		break;
			case 3: player.type = PlayerType::Electricity;	break;
			}

			// �^�C�v���Œ肳�ꂽ�t���O�𗧂Ă�
			player.isTypeFixed = true;
		}

		// 4. ���Z�b�g���� (������s)
		//    �^�C�v����� if �u���b�N�̊O�ɏo�����ƂŁA�ǂ̃t�H�[���i�K����̐i���ł����Z�b�g�����


		// ��3�`�Ԃɓ��B��������Ȃ�G�t�F�N�g���Z�b�g
		/*if (playerObject->form == Form::Third && currentForm != Form::Third)
		{
			XMFLOAT2 pos = { 170.0f, 600.0f };
			XMFLOAT2 size = { 300.0f, 300.0f };
			Effect_Set(0, pos, size);
		}*/

		// ��3�`�Ԃɓ��B��������Ȃ�G�t�F�N�g���Z�b�g�i�v���C���[�ԍ��ʈʒu�E�^�C�v�ʃe�N�X�`���j
		if (player.form == Form::Third && currentForm != Form::Third)
		{
			float screenX = SCREEN_ADJUST_X;
			float screenY = 620.0f * SCREEN_ADJUST_Y;

			// プレイヤーごとの画面上のエフェクト位置
			const XMFLOAT2 playerEffectPos[PLAYER_MAX] =
			{
				{  170.0f * screenX, screenY }, // プレイヤー1
				{  490.0f * screenX, screenY }, // プレイヤー2
				{  810.0f * screenX, screenY }, // プレイヤー3
				{ 1130.0f * screenX, screenY }  // プレイヤー4
			};

			// �i���^�C�v�ʂ̃e�N�X�`���ԍ��iEffect �̃e�N�X�`���z��ƍ��킹�邱�Ɓj
			int effectTexNo = 0; // �f�t�H���g
			switch (player.type)
			{
			case PlayerType::Glass:			effectTexNo = 0; break;
			case PlayerType::Concrete:		effectTexNo = 1; break;
			case PlayerType::Plant:			effectTexNo = 2; break;
			case PlayerType::Electricity:	effectTexNo = 3; break;
			default:						effectTexNo = 0; break;
			}

			// �v���C���[�ԍ��� playerIndex�i0�x�[�X�j
			XMFLOAT2 pos = playerEffectPos[playerIndex];
			XMFLOAT2 size = { 350.0f, 350.0f };

			Effect_SetUI(effectTexNo, pos, size);
		}

		player.brokenHistory.clear(); // �������N���A����
		player.evolutionGauge = 0;
		player.breakCount_Glass = 0;
		player.breakCount_Concrete = 0;
		player.breakCount_Plant = 0;
		player.breakCount_Electricity = 0;
	}
}

void Attack_Draw(int playerIndex)
{
	// �͈̓`�F�b�N 0 1 2 3 �ȊO�Ȃ� return
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	// �Q�Ƃ����
	ATTACK_OBJECT& attackObject = Attack[playerIndex];
	ID3D11ShaderResourceView* tex = g_Attack_Texture[playerIndex];

	// =====================
	// ���[���h�s��̍쐬
	// =====================

	// �X�P�[�����O�s��̍쐬
	XMMATRIX ScalingMatrix = XMMatrixScaling
	(
		attackObject.scaling.x,
		attackObject.scaling.y,
		attackObject.scaling.z
	);

	// ��]�s��̍쐬
	XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw
	(
		XMConvertToRadians(attackObject.rotation.x),
		XMConvertToRadians(attackObject.rotation.y),
		XMConvertToRadians(attackObject.rotation.z)
	);

	// ���s�ړ��s��̍쐬
	XMMATRIX TranslationMatrix = XMMatrixTranslation
	(
		attackObject.position.x,
		attackObject.position.y,
		attackObject.position.z
	);

	XMMATRIX WorldMatrix = ScalingMatrix * RotationMatrix * TranslationMatrix;

	// �v���W�F�N�V�����s��쐬
	XMMATRIX projection = GetProjectionMatrix();

	// �r���[�s��쐬
	XMMATRIX view = GetViewMatrix();

	// �ŏI�I�ȕϊ��s����쐬
	XMMATRIX WVP = WorldMatrix * view * projection;

	// �ϊ��s��𒸓_�V�F�[�_�[�փZ�b�g
	Shader_SetMatrix(WVP);

	LIGHT light{};
	light.Enable = TRUE;
	// ���̌����i���[���h��ԁj�V�F�[�_�[���ŒP�ʉ����Ďg���Ă���z��
	light.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
	// �g�U���Ɗ���
	light.Diffuse = XMFLOAT4(1.5f, 1.5f, 1.5f, 1.0f);
	light.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	Shader_SetLight(light);

	// �V�F�[�_�[��`��p�C�v���C���֐ݒ�
	Shader_Begin();

	// �s�����ŕ`�悷�邽�߃u�����h�𖳌������A�`��J���[�̃A���t�@��1�ɌŒ肷��
	SetBlendState(BLENDSTATE_NONE);
	Shader_SetColor(color::white);

	// ���_�V�F�[�_�[��`��p�C�v���C���֐ݒ�
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex2* vertex = (Vertex2*)msr.pData;

	// ���_�f�[�^�𒸓_�o�b�t�@�փR�s�[����
	CopyMemory(&vertex[0], &Attack_vdata[0], sizeof(Vertex2) * ATTACK_VERTEX);

	// �R�s�[����
	g_pContext->Unmap(g_VertexBuffer, 0);

	// �e�N�X�`�����Z�b�g
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// ���_�o�b�t�@���Z�b�g
	UINT stride = sizeof(Vertex2);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �C���f�b�N�X�o�b�t�@���Z�b�g
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// �`�悷��|���S���̎�ނ��Z�b�g 3���_�Ń|���S��1���Ƃ��ĕ\��
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_pContext->DrawIndexed(6 * 6, 0, 0);

	SetBlendState(BLENDSTATE_ALPHA);
}

void AttackPlayerCollisions()
{
	// �e�v���C���[�̍U���I�u�W�F�N�g�����[�v���āA���v���C���[�S���ɓ����蔻����s��
	for (int atk = 0; atk < PLAYER_MAX; ++atk)
	{
		if (atk < 0 || atk >= PLAYER_MAX) continue; // �ǉ��̈��S�`�F�b�N
		ATTACK_OBJECT& attackObject = Attack[atk];

		// player �͊����� GetPlayer ���g���ăk���`�F�b�N
		PLAYEROBJECT* attackerPtr = GetPlayer(atk);
		if (attackerPtr == nullptr) continue;
		PLAYEROBJECT& attacker = *attackerPtr;

		if (!attacker.isAttacking) continue;	// �U�����̂ݔ���

		// �U���I�u�W�F�N�g�ƍU���҂� AABB ���X�V
		CalculateAABB(attackObject.boundingBox, attackObject.position, attackObject.scaling);
		CalculateAABB(attacker.boundingBox, attacker.position, attacker.scaling);

		// �U���҂̌����x�N�g�����X�V�irotation.y ����Z�o�j
		{
			float rad = XMConvertToRadians(attacker.rotation.y);
			attacker.dir.x = sinf(rad);
			attacker.dir.z = cosf(rad);
		}

		// --- �v���C���[���Ŏg���Ă���`��X�P�[���E�q�b�g�{�b�N�X�䗦�ƍ��킹�� ---
		const float RENDER_SCALE = 2.0f;
		const float HITBOX_HEIGHT_SCALE = 1.0f;
		// Player �Ɠ����Z��/���Ӓ�`���g��
		const float HITBOX_SHORT = 0.35f;
		const float HITBOX_LONG  = 0.65f;

		// �U����������ΏۂƂ��đ��v���C���[�S�����`�F�b�N
		for (int def = 0; def < PLAYER_MAX; ++def)
		{
			if (def == atk) continue; // �����ɂ͓�����Ȃ�

			PLAYEROBJECT* defenderObject = GetPlayer(def);
			if (defenderObject == nullptr) continue;
			PLAYEROBJECT& defender = *defenderObject;

			if (!defender.active) continue;
			// ��e���△�G�Ȃ�X�L�b�v
			if (defender.isInvincible) continue;

			// defender �̌�������Z��/���ӂ����߂�
			float radDef = XMConvertToRadians(defender.rotation.y);
			float defFacingX = sinf(radDef);
			float defFacingZ = cosf(radDef);
			bool defFacingZDominant = fabsf(defFacingZ) >= fabsf(defFacingX);

			float widthScale = defFacingZDominant ? HITBOX_SHORT : HITBOX_LONG;		// X�����X�P�[��
			float depthScale = defFacingZDominant ? HITBOX_LONG  : HITBOX_SHORT;	// Z�����X�P�[��

			// ��2�`�� ��3�`�Ԃ�X��Z�����ɂ���
			if (defender.form == Form::Second || defender.form == Form::Third)
			{
				widthScale = 0.3f;
				depthScale = 0.3f;
			}

			// defender �p�̃q�b�g�{�b�N�X�X�P�[�����v�Z���� AABB �����
			XMFLOAT3 defenderHitboxScaling =
			{
				defender.scaling.x * RENDER_SCALE * widthScale,
				defender.scaling.y * RENDER_SCALE * HITBOX_HEIGHT_SCALE,
				defender.scaling.z * RENDER_SCALE * depthScale
			};
			CalculateAABB(defender.boundingBox, defender.position, defenderHitboxScaling);

			// ����idefender AABB �� �U���I�u�W�F�N�g AABB�j
			MTV col = CalculateAABBMTV(defender.boundingBox, attackObject.boundingBox);

			if (col.isColliding)
			{
				//// �m�b�N�o�b�N�i�U���҂̌����ƍU���͂��g�p�j
				//defender.position.x += attacker.dir.x/* * attacker.power*/;
				//defender.position.y += attacker.power;
				//defender.position.z += attacker.dir.z/* * attacker.power*/;

				// ������΂������i������傫������Ƃ߂������ԁI�j
				float knockbackPower = 0.4f;
				float liftUpPower = 0.2f;    // ������ɕ�������Ɛ�����΂��ꂽ�����o���

				// ���W�𒼐ڂ�����̂ł͂Ȃ��A���x�ivelocity�j�ɗ͂𗭂߂�
				defender.velocity.x = attacker.dir.x * knockbackPower;
				defender.velocity.y = liftUpPower;
				defender.velocity.z = attacker.dir.z * knockbackPower;

				// �_���[�W�p�ϐ�
				float rawDamage = attacker.attack * defender.defense;

				// �_���[�W�i�h��Ōy���j
				defender.hp -= rawDamage;
				if (defender.hp < 0.0f) defender.hp = 0.0f;

				TriggerbyHPShake(def, 8.0f,20.0f,1.5f);

				// スタンゲージ増加
				defender.stunGauge += 0.5f;

				// �_���[�W������\���i����ɃI�t�Z�b�g�j
				int dmgInt = static_cast<int>(rawDamage + 0.5f);
				XMFLOAT3 hitPos = defender.position;
				hitPos.y += defender.scaling.y + 0.3f;
				SetDamageText(hitPos, dmgInt, TextColor::Blue);

				// �_���[�W�t���O�E�^�C�}�[�i�A�j��/UI �p�j
				defender.isAttacked = true;
				defender.attackedTimer = 0.0f;

				// �Čv�Z
				CalculateAABB(defender.boundingBox, defender.position, defenderHitboxScaling);
				CalculateAABB(attackObject.boundingBox, attackObject.position, attackObject.scaling);
			}
		}
	}
}

ATTACK_OBJECT* GetAttack(int playerIndex)
{
	// �͈̓`�F�b�N 0 1 2 3 �ȊO�Ȃ� nullptr ��Ԃ�
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX)	return nullptr;

	return &Attack[playerIndex];
}