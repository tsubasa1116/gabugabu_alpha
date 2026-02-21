//======================================================
//	field.cpp[]
//======================================================
#include "field.h"
#include "Camera.h"
#include "keyboard.h"
#include "collider.h"
#include "debug_render.h"
#include "model.h"
#include "Building.h"
#include "player.h"
#include "special.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "color.h"

//======================================================
//	�}�N����`
//======================================================
#define BOX_NUM_VERTEX	(24)
#define FIELD_TEX_MAX	(2)

//======================================================
//	�O���[�o���ϐ�
//======================================================
MODEL* Test = NULL;//�f�o�b�O

////�O���[�o���ϐ�
static	ID3D11Device* g_pDevice = NULL;
static	ID3D11DeviceContext* g_pContext = NULL;
////���_�o�b�t�@
//static	ID3D11Buffer* g_VertexBuffer = NULL;
////�C���f�b�N�X�o�b�t�@
//static	ID3D11Buffer* g_IndexBuffer = NULL;
//�e�N�X�`���ϐ�
//static ID3D11ShaderResourceView* g_Texture;

// FIELD enum (FIELD_BUILDING, FIELD_BOX) �̐������e�N�X�`�����Ǘ�
static ID3D11ShaderResourceView* g_Texture[FIELD_TEX_MAX];

// FIELD::no の値に対応するテクスチャファイル名
static const wchar_t* g_TexturePaths[FIELD_TEX_MAX] = 
{
	L"Asset\\Texture\\gure.jpg",
	L"Asset\\Texture\\fade.bmp"
};

static const char* g_ModelName[] = {
	"field",
	"field_v2",
	"field_v3",
	"propsConcreteMain_v2",		// 3�}�X�匚��
	"propsConcreteSub_v2",		// �}���V����
	"propsElectricitySub_v2",	// �ԂƐM��
	"propsGlassSub_v2",			// �r��
	"propsTreeSub_v2",			// �L�t��
	"build_glass_new"			// �ςȌ���
	"propsTowerMain_v3"			//�����^��-
};
static const char* g_ModelName1[] = {
	"raibu",
	"kitosaku"
};

//�}�b�v�f�[�^�z��
MAPDATA Map[] =
{
	// ===== 地面・特殊 =====			 
	{ {},{}, FIELD::FIELD_Electricity,1}, // 1kaku
	{ {},{}, FIELD::FIELD_Electricity,0}, // 2kaku
	{ {},{}, FIELD::FIELD_Plant,2},           // 3kaku
	{ {},{}, FIELD::FIELD_Electricity,0},           // 4kaku
	{ {},{}, FIELD::FIELD_Plant,2}, // 5kaku
	{ {},{}, FIELD::FIELD_Electricity,0},           // 6kaku
	{ {},{}, FIELD::FIELD_Plant,2},           // 7kaku
	{ {},{}, FIELD::FIELD_Concrete,2},           // 8kaku
	{ {},{}, FIELD::FIELD_Concrete,1},           // 9
	{ {},{}, FIELD::FIELD_Glass},           // 10
						  
	// ===== BOX 10 ===== 
	{ {},{}, FIELD::FIELD_Glass,}, // 11
	{ {},{}, FIELD::FIELD_Plant,2}, // 12kaku
	{ {},{}, FIELD::FIELD_Plant,2}, // 13kaku
	{ {},{}, FIELD::FIELD_Concrete,1}, // 14kaku
	{ {},{}, FIELD::FIELD_Glass}, // 15kaku
	{ {},{}, FIELD::FIELD_Electricity,4}, // 16
	{ {},{}, FIELD::FIELD_Electricity,0}, // 17kaku
	{ {},{}, FIELD::FIELD_Concrete,2}, // 18
	{ {},{}, FIELD::FIELD_Electricity,0}, // 19kaku
	{ {},{}, FIELD::FIELD_Concrete,1}, // 20kaku
						  
	// ===== BOX 20 ===== 
	{ {},{}, FIELD::FIELD_Plant,2 }, // 21
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 22
	{ {},{}, FIELD::FIELD_Plant,1 }, // 23
	{ {},{}, FIELD::FIELD_Plant,2}, // 24
	{ {},{}, FIELD::FIELD_Plant,2 }, // 25
	{ {},{}, FIELD::FIELD_Electricity,0 }, // 26
	{ {},{}, FIELD::FIELD_Concrete,2}, // 27
	{ {},{}, FIELD::FIELD_Plant,2 }, // 28
	{ {},{}, FIELD::FIELD_Plant,3 }, // 29
	{ {},{}, FIELD::FIELD_Glass }, // 30
						 
	// ===== BOX 30 =====
	{ {},{}, FIELD::FIELD_Electricity,0 }, // 31
	{ {},{}, FIELD::FIELD_Plant,2 }, // 32
	{ {},{}, FIELD::FIELD_Plant,2 }, // 33
	{ {},{}, FIELD::FIELD_Electricity,4}, // 34
	{ {},{}, FIELD::FIELD_Glass }, // 35
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 36
	{ {},{}, FIELD::FIELD_Plant,2 }, // 37
	{ {},{}, FIELD::FIELD_Glass,3 }, // 38
	{ {},{}, FIELD::FIELD_BOX }, // 39
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 40
						 
	// ===== BOX 40 =====
	{ {},{}, FIELD::FIELD_Plant,2 }, // 41
	{ {},{}, FIELD::FIELD_Plant,2 }, // 42
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 43
	{ {},{}, FIELD::FIELD_Glass }, // 44
	{ {},{}, FIELD::FIELD_Plant,2 }, // 45
	{ {},{}, FIELD::FIELD_Glass, }, // 46
	{ {},{}, FIELD::FIELD_Electricity,4}, // 47
	{ {},{}, FIELD::FIELD_Glass,1 }, // 48  左上デカい建物
	{ {},{}, FIELD::FIELD_Electricity,2 }, // 49   右上デカい建物
	{ {},{}, FIELD::FIELD_Electricity,4}, // 50
						 
	// ===== BOX 50 =====
	{ {},{}, FIELD::FIELD_Electricity }, // 51
	{ {},{}, FIELD::FIELD_Concrete }, // 52 右下デカい建物
	{ {},{}, FIELD::FIELD_Plant,4 }, // 53左下デカい
	{ {},{}, FIELD::FIELD_Electricity}, // 54
	{ {},{}, FIELD::FIELD_Plant,2}, // 55
	{ {},{}, FIELD::FIELD_Glass,3}, // 56
	{ {},{}, FIELD::FIELD_Electricity}, // 57
	{ {},{}, FIELD::FIELD_Plant,2}, // 58
	{ {},{}, FIELD::FIELD_Concrete,1}, // 59
	{ {},{}, FIELD::FIELD_Electricity,}, // 60
						  
	// ===== BOX 60 ===== 
	{ {},{}, FIELD::FIELD_Plant,2 }, // 61
	{ {},{}, FIELD::FIELD_Glass }, // 62
	{ {},{}, FIELD::FIELD_Electricity }, // 63
	{ {},{}, FIELD::FIELD_Electricity }, // 64
	{ {},{}, FIELD::FIELD_Electricity,3 }, // 65
	{ {},{}, FIELD::FIELD_Electricity }, // 66
	{ {},{}, FIELD::FIELD_Glass }, // 67
	{ {},{}, FIELD::FIELD_BOX }, // 68
	{ {},{}, FIELD::FIELD_Electricity }, // 69
	{ {},{}, FIELD::FIELD_Electricity }, // 70
						 
	// ===== BOX 70 =====
	{ {},{}, FIELD::FIELD_Plant,2 }, // 71
	{ {},{}, FIELD::FIELD_Plant,2 }, // 72
	{ {},{}, FIELD::FIELD_Electricity }, // 73
	{ {},{}, FIELD::FIELD_Electricity,3 },// 74
	{ {},{}, FIELD::FIELD_Plant,1 }, // 75
	{ {},{}, FIELD::FIELD_Plant,3 }, // 76
	{ {},{}, FIELD::FIELD_Plant,2 }, // 77
	{ {},{}, FIELD::FIELD_Plant,1 }, // 78
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 79
	{ {},{}, FIELD::FIELD_Plant,2 }, // 80
						  
	// ===== BOX 80 ===== 
	{ {},{}, FIELD::FIELD_Plant,1 }, // 81
	{ {},{}, FIELD::FIELD_Plant,2 }, // 82
	{ {},{}, FIELD::FIELD_Plant,3 }, // 83
	{ {},{}, FIELD::FIELD_Electricity ,4}, // 84
	{ {},{}, FIELD::FIELD_Plant,2 }, // 85
	{ {},{}, FIELD::FIELD_Plant,2 }, // 86
	{ {},{}, FIELD::FIELD_Plant,3 }, // 87
	{ {},{}, FIELD::FIELD_Glass },   // 88
	{ {},{}, FIELD::FIELD_Plant,2 }, // 89
	{ {},{}, FIELD::FIELD_Plant,2 }, // 90
	 					  
	// ===== BOX 90 ===== 
	{ {},{}, FIELD::FIELD_Plant,2 }, // 91
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 92
	{ {},{}, FIELD::FIELD_Plant,2 }, // 93
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 94
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 95
	{ {},{}, FIELD::FIELD_Concrete ,2}, // 96
	{ {},{}, FIELD::FIELD_Plant,3 }, // 97
	{ {},{}, FIELD::FIELD_Plant,2 }, // 98
	{ {},{}, FIELD::FIELD_Plant,2 }, // 99
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 100
						  
	// ===== BOX 100 =====
	{ {},{}, FIELD::FIELD_Plant,2}, // 101
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 102  いまのままだとここまでしかモデルが置けない
	{ {},{}, FIELD::FIELD_BOX }, // 103
	{ {},{}, FIELD::FIELD_BOX }, // 104
	{ {},{}, FIELD::FIELD_BOX }, // 105
	{ {},{}, FIELD::FIELD_BOX }, // 106
	{ {},{}, FIELD::FIELD_BOX }, // 107
	{ {},{}, FIELD::FIELD_BOX }, // 108
	{ {},{}, FIELD::FIELD_BOX }, // 109
	{ {},{}, FIELD::FIELD_BOX }, // 110
						  
	// ===== BOX 110 ==== BOX
	{ {},{}, FIELD::FIELD_BOX }, // 111
	{ {},{}, FIELD::FIELD_BOX }, // 112
	{ {},{}, FIELD::FIELD_BOX }, // 113
	{ {},{}, FIELD::FIELD_BOX }, // 114
	{ {},{}, FIELD::FIELD_BOX }, // 115
	{ {},{}, FIELD::FIELD_BOX }, // 116
	{ {},{}, FIELD::FIELD_BOX }, // 117
	{ {},{}, FIELD::FIELD_BOX }, // 118
	{ {},{}, FIELD::FIELD_BOX }, // 119
	{ {},{}, FIELD::FIELD_BOX }, // 120

	// ===== 終了マーカー（カウントしない）=====
	{ XMFLOAT3(2.0f,-1.0f,5.0f), {}, FIELD::FIELD_MAX }
};


//======================================================
//	�������֐�
//======================================================
void Field_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	char modelPath[256];
	snprintf(modelPath, sizeof(modelPath), "asset\\model\\%s.fbx", g_ModelName[1]);

	Test = ModelLoad(modelPath);//�f�o�b�O

	// �z��v�f���i�I���}�[�J�[ FIELD_MAX ���܂܂Ȃ��j
	int count = GetFieldObjectCount();
	if (count <= 1)
	{
		g_pDevice = pDevice;
		g_pContext = pContext;
		Building_Initialize(pDevice, pContext);
		return;
	}

	// ====== �Z�p�i�q���𑽐��������A���S�ɋ߂����̂��� N �I��Łu���~�`�v�ɔz�u ======
	// MAPDATA::radius �� hex size�icenter->corner�j�ƌ��Ȃ��iflat-top�j
	const float size = Map->radius;
	const float sqrt3 = sqrtf(3.0f);

	// �������X�P�[���i�K�v�Ȃ璲���j
	const float horizontalScale = 1.0f;

	// ���𐶐����邽�߂̃����O���i�]�T����������j
	// count ���ۂ��I�Ԃ��߁A���͑������߂ɐ�������imarginFactor�j
	const float marginFactor =5.0f; // 1.0 = �Œ��, 1.25 = �]�T 25%
	int rings = 1;
	while (1 + 3 * rings * (rings + 1) < static_cast<int>(count * marginFactor))
		++rings;

	// �����W(q,r)�𓯐S�����O�Ő����itotalCandidates >= count�j
	int totalCandidates = 1 + 3 * rings * (rings + 1);

	// �w���p�[�\���́i���[�J���j
	struct Candidate { int q; int r; float wx; float wz; float dist; };

	// ���I�m�ہi���[�J���� vector ���g��Ȃ��`�ɂ��ăC���N���[�h�s�v�Ɂj
	Candidate* candidates = new Candidate[totalCandidates];

	// ���S
	int idx = 0;
	candidates[idx].q = 0;
	candidates[idx].r = 0;
	candidates[idx].wx = 0.0f;
	candidates[idx].wz = 0.0f;
	candidates[idx].dist = 0.0f;
	++idx;

	// 6�����x�N�g���iaxial coords�j
	const int dirQ[6] = { 1, 1, 0, -1, -1, 0 };
	const int dirR[6] = { 0, -1, -1, 0, 1, 1 };

	for (int k = 1; idx < totalCandidates; ++k)
	{
		int q = -k;
		int r = k;
		for (int side = 0; side < 6 && idx < totalCandidates; ++side)
		{
			for (int step = 0; step < k && idx < totalCandidates; ++step)
			{
				float wx = size * 1.5f * static_cast<float>(q) * horizontalScale;
				float wz = size * sqrt3 * (static_cast<float>(r) + static_cast<float>(q) * 0.5f);
				float d = sqrtf(wx * wx + wz * wz);

				candidates[idx].q = q;
				candidates[idx].r = r;
				candidates[idx].wx = wx;
				candidates[idx].wz = wz;
				candidates[idx].dist = d;
				++idx;

				q += dirQ[side];
				r += dirR[side];
			}
		}
	}

	// ���S�ɋ߂����� count ��I�ԁi�ȈՑI���\�[�g���C�N�j
	// �I�� N = count�iMap �z��̗v�f���j
	int N = count;
	// ���S��: N ����␔�𒴂��Ȃ��悤��
	if (N > totalCandidates) N = totalCandidates;

	// �����I���F�擪 N ���������I�����C�c��𑖍����Ă��߂���Γ���ւ���iO(M*N)�������͂����܂ő傫���Ȃ��j
	// �܂��擪 N �� selected �Ƃ���i�z�������j
	Candidate* selected = new Candidate[N];
	for (int i = 0; i < N; ++i) selected[i] = candidates[i];

	// ���݂̍ŉ��C���f�b�N�X�����߂�֐�
	auto findWorstIndex = [&](int limit) -> int {
		int worst = 0;
		float maxd = selected[0].dist;
		for (int j = 1; j < limit; ++j)
		{
			if (selected[j].dist > maxd)
			{

				maxd = selected[j].dist;
				worst = j;
			}
		}
		return worst;
		};

	int worstIdx = findWorstIndex(N);

	// �c���������
	for (int i = N; i < totalCandidates; ++i)
	{
		if (candidates[i].dist < selected[worstIdx].dist)
		{
			// �u��
			selected[worstIdx] = candidates[i];
			// worstIndex ���Čv�Z
			worstIdx = findWorstIndex(N);
		}
	}

	// ������ selected[] �͒��S�ɋ߂� N �̌��i�����������͔C�Ӂj�Ȃ̂ŁA���S�ɋ߂����ɕ��בւ��邱�ƂŌ����ڂ���莩�R��
	// �ȈՓI�Ƀo�u���\�[�g�iN ���������̂ŏ\���j
	for (int a = 0; a < N - 1; ++a)
	{
		for (int b = 0; b < N - 1 - a; ++b)
		{
			if (selected[b].dist > selected[b + 1].dist)
			{
				Candidate tmp = selected[b];
				selected[b] = selected[b + 1];
				selected[b + 1] = tmp;
			}
		}
	}

	// Map �z��֊��蓖�āF���S�ɋ߂����ɔz�u���Ă���
	int assign = 0;
	for (int i = 0; i < count; ++i)
	{
		if (Map[i].no == FIELD::FIELD_MAX) break;

		if (assign < N)
		{
			Map[i].pos.x = selected[assign].wx;
			Map[i].pos.z = selected[assign].wz;
			Map[i].pos.y = -1.0f;
			Map[i].isActive = true;
			++assign;
		}
		else
		{
			// ���蓖�Ă��Ȃ������c��͔�\��
			Map[i].isActive = false;
		}
	}

	// ���
	delete[] candidates;
	delete[] selected;

	// ====================================================

	g_pDevice = pDevice;
	g_pContext = pContext;

	// --------------------------------------------------------------------
	// �����̃e�N�X�`����ǂݍ���
	// --------------------------------------------------------------------
	for (int i = 0; i < FIELD_TEX_MAX; ++i) // ��`�����e�N�X�`���̐��������[�v
	{
		TexMetadata metadata;
		ScratchImage image;
		// �z��ɒ�`�����p�X����e�N�X�`����ǂݍ���
		LoadFromWICFile(g_TexturePaths[i], WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(g_pDevice, image.GetImages(),
			image.GetImageCount(), metadata, &g_Texture[i]);
		assert(g_Texture[i]);
	}
	// --------------------------------------------------------------------

	Building_Initialize(pDevice, pContext);
}

//======================================================
//	�I�������֐�
//======================================================
void Field_Finalize(void)
{
	ModelRelease(Test);

	//SAFE_RELEASE(g_VertexBuffer);
	//SAFE_RELEASE(g_IndexBuffer);
	for (int i = 0; i < FIELD_TEX_MAX; ++i) SAFE_RELEASE(g_Texture[i]);

	Building_Finalize();
}

//======================================================
//	�`��֐�
//======================================================
void Field_Draw(bool s_IsKonamiCodeEntered)
{
	static bool input2 = false;
	// �f�o�b�O���[�h���̂݃L�[���͂��󂯕t����
	if (s_IsKonamiCodeEntered)
	{
		if (Keyboard_IsKeyDownTrigger(KK_D2))
		{
			input2 = !input2;	// �t���O���]
		}
	}
	//�V�F�[�_�[��`��p�C�v���C���֐ݒ�
	Shader_Begin();
	Shader_SetColor(color::white);

	//プロジェクション行列作成
	XMMATRIX	projection = GetProjectionMatrix();
	//ビュー行列作成
	XMMATRIX	view = GetViewMatrix();
	//先にVP変換行列を作っておく
	XMMATRIX VP = view * projection;

	//MAP�̕\��
	int i = 0;
	while (Map[i].no != FIELD_MAX)
	{
		// �����A�N�e�B�u����Ȃ�������A�`�悵�Ȃ��Ŏ���
		if (!Map[i].isActive)
		{
			i++; // i ��i�߂�̂�Y��Ȃ��ŁI
			continue; // ���̐�̕`�揈�����X�L�b�v
		}

		XMMATRIX World = XMMatrixScaling(3.0f, 3.0f, 1.0f) * XMMatrixRotationX(XMConvertToRadians(-90.0f)) * XMMatrixTranslation(Map[i].pos.x, Map[i].pos.y, Map[i].pos.z);

		Shader_SetWorldMatrix(World);
		Shader_SetMatrix(World * VP);

		// --------------------------------------------------------
		// map[i].no �̒l (int�ɃL���X�g) �ɑΉ�����e�N�X�`�����Z�b�g
		// --------------------------------------------------------
		int texIndex = (int)Map[i].no; // FIELD_BUILDING, FIELD_BOX�� 0, 1 �ɑΉ����Ă��邱�Ƃ𗘗p
		// �e�N�X�`���Z�b�g
		if (texIndex >= 0 && texIndex < FIELD_TEX_MAX)
			g_pContext->PSSetShaderResources(0, 1, &g_Texture[texIndex]);

		if (!s_IsKonamiCodeEntered || input2) ModelDraw(Test);

		//// �e�N�X�`�����p�C�v���C���������
		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		g_pContext->PSSetShaderResources(0, 1, &g_Texture[1]);
		//------------------------------------------------
		i++;
	}

	///////////////////////////////////////
	// TODO:boundingBox���Q�Ƃ�����
	
	// --- 3. �f�o�b�O�`��͑S���̃}�b�v��`���I�������Ɂu1�񂾂��v��� ---
	if (s_IsKonamiCodeEntered)
	{
		SetBlendState(BLENDSTATE_NONE);
		SetDepthTest(false); // �d�Ȃ�𖳎����Č�����悤��
		Shader_SetMatrix(VP); // ���[���h�s���Identity�ɂ���̂�VP������OK

		// �t�B�[���h�I�u�W�F�N�g�̘Z�p��
		int fieldCount = GetFieldObjectCount();
		MAPDATA* fieldObjects = GetFieldObjects();
		for (int j = 0; j < fieldCount; ++j)
		{
			if (!fieldObjects[j].isActive) continue;
			Debug_DrawHex(Map[j].boundingBox, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}

		// �v���C���[�̃X�y�V�����͈́i�~�j
		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			PLAYEROBJECT* player = GetPlayer(p);
			if (!player->active || !player->useSpecial) continue;

			// �����Ɋe�^�C�v��Debug_DrawCircle�������܂Ƃ߂�
			// (�������̃R�[�h�� player.type ���Ƃ̔��������)

			// �A���E�R���N���[�g�̃X�y�V�������g�p����Ă���ꍇ�A�~�̃t���[����ԐF�ŕ\��

			for (int p = 0; p < PLAYER_MAX; ++p)
			{
				PLAYEROBJECT* playerObject = GetPlayer(p);
				PLAYEROBJECT& player = *playerObject;
				if (!player.useSpecial) continue;
				// �A���E�R���N���[�g�̃X�y�V����

				if (player.type == PlayerType::Plant || player.type == PlayerType::Concrete)
				{
					// �~�̒��S�Ɣ��a��ݒ�
					XMFLOAT3 center = playerObject->position;
					float radius = 5.0f;

					// �ԐF�ŉ~��`��
					Debug_DrawCircle(center, radius, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
				}

				// �d�C�̃X�y�V����
				if (player.type == PlayerType::Electricity)
				{
					for (int i = 0; i < SPECIAL_ELECTRICITY_QUANTITY; ++i)
					{
						// �d�C�̉~�̒��S�Ɣ��a���擾
						XMFLOAT3 center = player.electricityCircles[i].center;
						float radius = player.electricityCircles[i].radius;

						// �ԐF�ŉ~��`��
						Debug_DrawCircle(center, radius, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
					}
				}

				// �K���X�̃X�y�V����
				if (player.type == PlayerType::Glass)
				{
					for (const auto& box : player.glassBoxes)
					{
						// �K���X�̉~�̒��S�Ɣ��a��ݒ�
						XMFLOAT3 center = box.position;
						float radius = 0.3f; // ���a0.3�̉~

						// �ԐF�ŉ~��`��
						Debug_DrawCircle(center, radius, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
					}
				}
			}
		}
	}
}

//======================================================
//	�X�V����
//======================================================
void Field_Update(void)
{
	int i = 0;
	while (Map[i].no != FIELD_MAX)
	{
		// �����A�N�e�B�u����Ȃ�������A�`�悵�Ȃ��Ŏ���
		if (!Map[i].isActive)
		{
			i++; // i ��i�߂�̂�Y��Ȃ��ŁI
			continue; // ���̐�̕`�揈�����X�L�b�v
		}

		Map[i].boundingBox.center = Map[i].pos;			// -1
		Map[i].boundingBox.radius = Map[i].radius;		// 1
		Map[i].boundingBox.height = Map[i].height;		// 3.0



		i++;
	}
}

// ======================================================
//	�Q�b�^�[
// ------------------------------------------------------
//	�t�B�[���h�̔z��̐擪�|�C���^��Ԃ�
// ======================================================
MAPDATA* GetFieldObjects()
{
	return Map;
}

// �t�B�[���h�I�u�W�F�N�g�̑�����Ԃ�
int GetFieldObjectCount()
{
	int count = 0;
	// map�z���FIELD_MAX���I���}�[�J�[�Ƃ��Ă���
	while (Map[count].no != FIELD_MAX)
	{
		count++;
	}
	return count;
}


