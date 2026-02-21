//======================================================
//	Game.cpp[]
// 
//	����ҁF�O�엃			���t�F2024//
//======================================================

#include "Manager.h"
#include "sprite.h"
#include "Game.h"
#include "keyboard.h"
#include "makeText.h"
#include "p.h"
#include "field.h"
#include "building.h"
#include "Effect.h"
#include "score.h"
#include "Audio.h"
#include "gauge.h"
#include "Polygon.h"
#include "Player.h"
#include "Camera.h"
#include "Ball.h"
#include "attack.h"
#include "skill.h"
#include "special.h"
#include "fade.h"
#include "DamageText.h"
#include "direct3d.h"
#include "SkyBall.h"
//======================================================
//	�\���w�錾
//======================================================
LIGHTOBJECT Light;

//======================================================
//	�O���[�o���ϐ�
//======================================================
static int g_BgmID = NULL;	// �T�E���h�Ǘ�ID
bool input2 = false;

const int KONAMI_CODE[] = {
	KK_UP, KK_UP, KK_DOWN, KK_DOWN,
	KK_LEFT, KK_RIGHT, KK_LEFT, KK_RIGHT,
	KK_B, KK_A
};

// �R�}���h�̒���
const int KONAMI_CODE_LENGTH = sizeof(KONAMI_CODE) / sizeof(KONAMI_CODE[0]);

// ���݁A�R�}���h���͂̂ǂ��܂Ői��ł��邩��ǐՂ���C���f�b�N�X
static int s_KonamiCodeIndex = 0;

// �R�}���h�����͂��ꂽ�Ƃ��ɗ��t���O
static bool s_IsKonamiCodeEntered = false;

// �����ꂽ�L�[�����҂���Ă���L�[�ƈ�v���Ă��邩�̊m�F������
void CheckKonamiCode(int currentKeyCode)
{
	// ���݊��҂���Ă���L�[�������ꂽ���H
	if (currentKeyCode == KONAMI_CODE[s_KonamiCodeIndex])
	{
		// ���Ғʂ�̓��͂������̂ŁA�C���f�b�N�X��i�߂�
		s_KonamiCodeIndex++;

		// �R�}���h�̍Ō�܂œ��B�������H
		if (s_KonamiCodeIndex >= KONAMI_CODE_LENGTH)
		{
			// �R�}���h���͊����I�t���O�𗧂Ă�
			s_IsKonamiCodeEntered = !s_IsKonamiCodeEntered;

			// �R�}���h�͊��������̂ŁA�C���f�b�N�X�����Z�b�g���邩�A-1�Ȃǂ̊�����Ԃɂ���
			s_KonamiCodeIndex = 0; // �܂��� s_KonamiCodeIndex = -1;
		}
	}
	else
	{
		// ���҂���Ă��Ȃ��L�[�������ꂽ�ꍇ�A�V�[�P���X�͎��s�B�ŏ������蒼��
		s_KonamiCodeIndex = 0;

		// �������A���s�����L�[���R�}���h�̍ŏ��̃L�[�ł���ꍇ�A
		// �ŏ��̃L�[�����蒼���\�����l������Ȃ�A�ȉ��̂悤�ɍă`�F�b�N���Ă��ǂ�
		if (currentKeyCode == KONAMI_CODE[0])
		{
			s_KonamiCodeIndex = 1;
		}
	}
}

//======================================================
//	�������֐�
//======================================================
void Game_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	Initialize_MakeText();
	CreateRenderTarget_MakeText();

	Player_Initialize(pDevice, pContext);
	Field_Initialize(pDevice, pContext);
	Effect_Initialize(pDevice, pContext);
	Attack_Initialize(pDevice, pContext);
	Skill_Initialize(pDevice, pContext);
	Special_Initialize(pDevice, pContext);
	Camera_Initialize();
	DamageText_Initialize();
	SkyBall_Initialize(pDevice, pContext);

	//BallInitialize(pDevice, pContext);
	//P_Initialize(pDevice, pContext);		// �v���C���[�̏�����
	//Score_Initialize(pDevice, pContext);

	g_BgmID = LoadAudio("asset\\Audio\\BGM_01.wav");	// �T�E���h���[�h
	PlayAudio(g_BgmID, true);		// �Đ��J�n(���[�v����)
	//PlayAudio(g_BgmID);			// �Đ��J�n�i���[�v�Ȃ��j
	//PlayAudio(g_BgmID, false);	// �Đ��J�n�i���[�v�Ȃ��j

	//���C�g������
	XMFLOAT4 para;

	para = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);	// �����̐F
	Light.SetAmbient(para);
	para = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);	// ���̐F
	Light.SetDiffuse(para);
	para = XMFLOAT4(0.5f, -1.0f, 0.0f, 1.0f);	// ������

	float len = sqrtf(para.x * para.x + para.y * para.y + para.z * para.z);
	para.x /= len;
	para.y /= len;
	para.z /= len;
	Light.SetDirection(para);	// ���̕����i���K���ρj
}

//======================================================
//	�I�������֐�
//======================================================
void Game_Finalize()
{
	Field_Finalize();
	Effect_Finalize();
	Player_Finalize();
	Camera_Finalize();
	Attack_Finalize();
	Skill_Finalize();
	Special_Finalize();
	SkyBall_Finalize();
	
	//BallFinalize();
	//P_Finalize();
	//Score_Finalize();

	UnloadAudio(g_BgmID);	// �T�E���h�̉��
	DamageText_Finalize();
}

//======================================================
//	�X�V����
//======================================================
void Game_Update()
{
	// ------------------------------------
	//  �R�i�~�R�}���h���o
	// ------------------------------------
	// �R�}���h�Ŏg�p����S�ẴL�[�̉����g���K�[���`�F�b�N���A���o�֐��ɓn��
		 if (Keyboard_IsKeyDownTrigger(KK_UP))		CheckKonamiCode(KK_UP);
	else if (Keyboard_IsKeyDownTrigger(KK_DOWN))	CheckKonamiCode(KK_DOWN);
	else if (Keyboard_IsKeyDownTrigger(KK_LEFT))	CheckKonamiCode(KK_LEFT);
	else if (Keyboard_IsKeyDownTrigger(KK_RIGHT))	CheckKonamiCode(KK_RIGHT);
	else if (Keyboard_IsKeyDownTrigger(KK_B))		CheckKonamiCode(KK_B);
	else if (Keyboard_IsKeyDownTrigger(KK_A))		CheckKonamiCode(KK_A);
	// ------------------------------------
	// �X�V����
	// ------------------------------------

	Player_Update();
	Field_Update();
	Effect_Update();
	Gauge_Update();
	Camera_Update();	// �v���C���[�̍X�V�̌�ɌĂ�
	SkyBall_Update();
	//BallUpdate();
	//P_Update();
	//Score_Update();
	DamageText_Update();
	
	//�Q�[���V�[���֑J��
	if (Keyboard_IsKeyDownTrigger(KK_F1) && (GetFadeState() == FADE_NONE))
	{
		// �t�F�[�h�A�E�g�����ăV�[����؂�ւ���
		XMFLOAT4 color(0.0f, 0.0f, 0.0f, 1.0f);
		SetFade(40.0f, color, FADE_OUT, SCENE_RESULT);
	}
}

//======================================================
//	�`��֐�
//======================================================
void Game_Draw()
{ 
	//2D�`��
	Light.SetEnable(FALSE);			// ���C�e�B���OOFF
	Shader_SetLight(Light.Light);	// ���C�g�\���̂��V�F�[�_�[�փZ�b�g
	SkyBall_Draw();
	SetDepthTest(FALSE);
	Camera_Draw();	// Draw�̍ŏ��ŌĂԁI

	Light.SetEnable(TRUE);			// ���C�e�B���OON
	Shader_SetLight(Light.Light);	// ���C�g�\���̂��V�F�[�_�[�փZ�b�g
	SetDepthTest(TRUE);

	Field_Draw(s_IsKonamiCodeEntered);
	Player_Draw(s_IsKonamiCodeEntered);

	//2D�`��
	Light.SetEnable(FALSE);			// ���C�e�B���OOFF
	Shader_SetLight(Light.Light);	// ���C�g�\���̂��V�F�[�_�[�փZ�b�g
	SetDepthTest(FALSE);
    
	Effect_Draw();
	Player_DrawHP();
	
	Player_DrawText();
	DamageText_Draw();
	//DrawTextEx(
	//	L"����ɂ��͐��E",			// �\�����镶��
	//	600, 400,					// �ʒu
	//	60.0f,						// �T�C�Y
	//	L"�ʂ˂�������������v7��",	// �t�H���g
	//	TextColor::Yellow			// �F
	//);
	
	//P_Draw();
}

