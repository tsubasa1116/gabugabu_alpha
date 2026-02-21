// =====================================================
//	player.cpp
// 
//	����ҁF�����D�n			���t�F2026/01/27
//======================================================
#include <d3d11.h>
#include <iostream>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "shader.h"
#include "keyboard.h"
#include "sprite.h"
#include "color.h"
#include "hp.h"
#include "gauge.h"
#include "Effect.h"
#include "player.h"
#include "Camera.h"
#include "input.h"
#include "skill.h"
#include "special.h"
#include "field.h"
#include "collider.h"
#include "debug_render.h"
#include "debug_ostream.h"
#include "attack.h" 
#include "DamageText.h"
#include "makeText.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <chrono>
#include <codecvt>
#include <vector>
#include <algorithm>
#include "Effect.h"

//======================================================
//	�}�N����`
//======================================================
#define GAUGE_POS_X	(69.0f * (SCREEN_WIDTH / 1280.0f))	
#define GAUGE_POS_Y	(8.0f *  (SCREEN_HEIGHT / 720.0f))	
#define	HPBER_SIZE_X (270.0f * (SCREEN_WIDTH / 1280.0f))
#define	HPBER_SIZE_Y (270.0f * (SCREEN_HEIGHT / 720.0f))

//======================================================
//	�O���[�o���ϐ�
//======================================================
// �I�u�W�F�N�g
PLAYEROBJECT player[PLAYER_MAX];

static ID3D11Device* g_pDevice = NULL;
static ID3D11DeviceContext* g_pContext = NULL;
static hp HPBar[PLAYER_MAX];

// ���_�o�b�t�@
static ID3D11Buffer* g_VertexBuffer = NULL;

// �C���f�b�N�X�o�b�t�@
static ID3D11Buffer* g_IndexBuffer = NULL;

// �e�N�X�`���ϐ�
static ID3D11ShaderResourceView* g_Texture[17];

// �v���C���[ �A�j���[�V�����p�ϐ�
static int   g_animFrame[PLAYER_MAX] = { 0 };
static float g_animTimer[PLAYER_MAX] = { 0.0f };
static const float ANIM_FRAME_TIME = 0.15f;	// 1�t���[��������̕b��
static const int   SHEET_COLS = 16;
static const int   SHEET_ROWS = 16;

static int g_victoryState[PLAYER_MAX] = { 0 };			// 0 = �Ȃ�, 1 = ���� �Đ���, 2 = ���[�v
static float g_downHoldTimer[PLAYER_MAX] = { 0.0f };	// �ŏI�t���[���z�[���h�p�^�C�}�[�i�v���C���[���j
static bool g_specialAnimStarted[PLAYER_MAX] = { false, false, false, false };

// ���ʁE���S���̊Ǘ�
static std::vector<int> g_deathOrder;	// ���S�����v���C���[�̃C���f�b�N�X�i��Ɏ��񂾎҂��擪�j

// ���_�z��
static Vertex2 vdata[PLAYER_VERTEX] =
{
	{// ���_0 LEFT-TOP
		XMFLOAT3(-COORDINATE, COORDINATE, 0.0f),	// ���W
		XMFLOAT3(0.0f, 0.0f, -1.0f),				// �@���x�N�g��
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),			// �J���[
		XMFLOAT2(0.0f, 0.0f)						// �e�N�X�`�����W
	},
	{// ���_1 RIGHT-TOP
		XMFLOAT3(COORDINATE, COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(TEXCOORD, 0.0f)
	},
	{// ���_2 LEFT-BOTTOM
		XMFLOAT3(-COORDINATE, -COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f, TEXCOORD)
	},
	{// ���_3 RIGHT-BOTTOM
		XMFLOAT3(COORDINATE, -COORDINATE, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(TEXCOORD, TEXCOORD)
	},
};

// �C���f�b�N�X�z��
static UINT idxdata[6]
{
	 0, 1, 2, 2, 1, 3, // -Z��
};

static float top_y = 0;	// �Z�p�`��top-y���[�̃f�o�b�O�\��

//======================================================
//	�������֐�
//======================================================
void Player_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// �v���C���[�\���̏�����
	player[0].position = XMFLOAT3(-3.0f, 4.0f, 0.0f);
	player[1].position = XMFLOAT3(1.5f, 4.0f, 2.0f);
	player[2].position = XMFLOAT3(-4.0f, 4.0f, -3.0f);
	player[3].position = XMFLOAT3(4.0f, 4.0f, 1.0f);

	player[0].form = Form::First;
	player[1].form = Form::First;
	player[2].form = Form::First;
	player[3].form = Form::First;
	player[0].type = PlayerType::None;
	player[1].type = PlayerType::None;
	player[2].type = PlayerType::None;
	player[3].type = PlayerType::None;
	//player[0].form = Form::Third;
	//player[1].form = Form::Third;
	//player[2].form = Form::Third;
	//player[3].form = Form::Third;
	//player[0].type = PlayerType::Glass;
	//player[1].type = PlayerType::Concrete;
	//player[2].type = PlayerType::Plant;
	//player[3].type = PlayerType::Plant;

	for (int p = 0; p < PLAYER_MAX; p++)
	{
		player[p].oldPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[p].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[p].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
		player[p].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[p].hp = PLAYER_MAX_HP;
		player[p].attack = 0.0f;
		player[p].power = 0.0f;
		player[p].speed = 0.0f;
		player[p].defense = 1.0f;
		player[p].stock = 3;
		player[p].rank = 0;
		player[p].active = true;
		player[p].satiety = 0.0f;
		player[p].isAttacking = false;
		player[p].attackTimer = 0.0f;
		player[p].isAttacked = false;
		player[p].attackedTimer = 0.0f;
		player[p].isHealing = false;
		player[p].healingTimer = 0.0f;
		player[p].isEvolving = false;
		player[p].evolvingTimer = 0.0f;
		player[p].useSkill = false;
		player[p].skillTimer = 0.0f;
		player[p].skillCoolTimer = 0.0f;
		player[p].useSpecial = false;
		player[p].specialTimer = 0.0f;
		player[p].isInvincible = false;
		player[p].invincibleTimer = 0.0f;
		player[p].stunGauge = 0.0f;
		player[p].isStunning = false;
		player[p].stunTimer = 0.0f;
		player[p].isDown = false;
		player[p].downTimer = 0.0f;
		player[p].isPoisoned = false;
		player[p].poisonTimer = 0.0f;
		player[p].duringRespawn = false;
		player[p].respawnTimer = 0.0f;
		player[p].isEggBreaking = false;
		player[p].eggBreakingTimer = 0.0f;
		player[p].lastDir = PlayerDir::Down; // 正面
		player[p].isMoving = false;
		player[p].isShadowEnabled = false;
		player[p].evolutionGauge = 0.0f;
		player[p].evolutionGaugeRate = 0.3f;
		player[p].breakCount_Glass = 0;
		player[p].breakCount_Concrete = 0;
		player[p].breakCount_Plant = 0;
		player[p].breakCount_Electricity = 0;
	}

	// ���_�o�b�t�@�쐬
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));	// 0でクリア
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * PLAYER_VERTEX;	// 格納できる頂点数*頂点サイズ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	g_pDevice = pDevice;
	g_pContext = pContext;

#ifdef _DEBUG_
	// �e�N�X�`�����[�h���Ԍv��
	auto tex_start = std::chrono::high_resolution_clock::now();
	LoadTextureList(pDevice);
	auto tex_end = std::chrono::high_resolution_clock::now();
	auto tex_ms = std::chrono::duration_cast<std::chrono::milliseconds>(tex_end - tex_start).count();
	hal::dout << "�e�N�X�`�����[�h����: " << tex_ms << " ms" << std::endl;
#else
	// �e�N�X�`���ǂݍ���
	LoadTextureList(pDevice);
#endif

	// �C���f�b�N�X�o�b�t�@�쐬
	{
		D3D11_BUFFER_DESC	bd;
		ZeroMemory(&bd, sizeof(bd));	// 0�ŃN���A
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(UINT) * 6 * 6;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&bd, NULL, &g_IndexBuffer);

		// �C���f�b�N�X�o�b�t�@�֏�������
		D3D11_MAPPED_SUBRESOURCE msr;
		pContext->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		UINT* index = (UINT*)msr.pData;

		// �C���f�b�N�X�f�[�^���o�b�t�@�փR�s�[
		CopyMemory(&index[0], &idxdata[0], sizeof(UINT) * 6 * 6);
		pContext->Unmap(g_IndexBuffer, 0);
	}
	// �f�o�b�O�����_���[������
	Debug_Initialize(pDevice, pContext);

	float screenX = SCREEN_ADJUST_X;
	float screenY = 650.0f * SCREEN_ADJUST_Y;

	InitializeHP(pDevice, pContext, &HPBar[0], {  160.0f * screenX, screenY }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);
	InitializeHP(pDevice, pContext, &HPBar[1], {  480.0f * screenX, screenY }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);
	InitializeHP(pDevice, pContext, &HPBar[2], {  800.0f * screenX, screenY }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);
	InitializeHP(pDevice, pContext, &HPBar[3], { 1120.0f * screenX, screenY }, { HPBER_SIZE_X, HPBER_SIZE_Y }, color::white, color::green);

	HPBar[0].gaugeIndex = 0;
	HPBar[1].gaugeIndex = 1;
	HPBar[2].gaugeIndex = 2;
	HPBar[3].gaugeIndex = 3;

	// �A�j���[�V�����̏�����
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		g_animFrame[i] = 0;
		g_animTimer[i] = 0.0f;
	}

	// ���ʏ���������
	g_deathOrder.clear();
}

static void LoadTextureList(ID3D11Device* pDevice)
{
	TexMetadata metadata;
	ScratchImage image;

	struct TexEntry { int idx; const wchar_t* path;};

	const TexEntry texList[] = 
	{
		{  0, L"asset\\texture\\characterMiniRed_v2.png"},			// 第1形態 P1 赤
		{  1, L"asset\\texture\\characterMiniBlue_v1.png"},			// 第1形態 P2 青
		{  2, L"asset\\texture\\characterMiniYellow_v1.png"},		// 第1形態 P3 黄
		{  3, L"asset\\texture\\characterMiniGreen_v1.png"},		// 第1形態 P4 緑
		{  4, L"asset\\texture\\characterMidGlass_v1.png"},			// 第2形態 ガラス
		{  5, L"asset\\texture\\characterMidConcrete_v1.png" },		// 第2形態 コンクリート
		{  6, L"asset\\texture\\characterMidTree_v1.png" },			// 第2形態 植物
		{  7, L"asset\\texture\\characterMidElectricity_v1.png" },	// 第2形態 電気
		{  8, L"asset\\texture\\characterBigGlass_v2.png" },		// 第3形態 ガラス
		{  9, L"asset\\texture\\characterBigConcrete_v2.png" },		// 第3形態 コンクリート
		{ 10, L"asset\\texture\\characterBigTree_v2.png" },			// 第3形態 植物
		{ 11, L"asset\\texture\\characterBigElectricity_v2.png" },	// 第3形態 電気
		{ 12, L"asset\\texture\\characterBigSP_v3.png" },			// 第3形態 スペシャル
		{ 13, L"asset\\texture\\uiStockRed_v4.png"},				// UI ストック 赤
		{ 14, L"asset\\texture\\uiStockBlue_v4.png"},				// UI ストック 青
		{ 15, L"asset\\texture\\uiStockYellow_v4.png" },			// UI ストック 黄
		{ 16, L"asset\\texture\\uiStockGreen_v4.png" },				// UI ストック 緑
	};

	for (const auto& e : texList)
	{
		auto start = std::chrono::high_resolution_clock::now();

		// �R�����g�����Ă���v�f�͔z��G���g�����̂��R�����g�A�E�g���Ă��邽�߂����ɂ͗��Ȃ��B
		HRESULT hr = LoadFromWICFile(e.path, WIC_FLAGS_NONE, &metadata, image);
		if (SUCCEEDED(hr))
		{
			if (FAILED(CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture[e.idx])))
			{
				// �쐬���s���� nullptr �������đ��s
				g_Texture[e.idx] = nullptr;
			}
		}
		// �ǂݍ��ݎ��s�� nullptr �������đ��s
		else	g_Texture[e.idx] = nullptr;

		auto end = std::chrono::high_resolution_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		// std::wstring �� std::string �ɕϊ����ďo��
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		hal::dout << "�e�N�X�`�����[�h: " << conv.to_bytes(e.path) << " " << ms << " ms" << std::endl;
	}
}

//======================================================
//	�I�������֐�
//======================================================
void Player_Finalize()
{
	// �V�F�[�_�[�Ƀo�C���h����Ă��� SRV ���A���o�C���h�i���S�̂��ߑS�v�f���j
	const size_t TEX_COUNT = sizeof(g_Texture) / sizeof(g_Texture[0]);
	if (g_pContext)
	{
		// �Œ蒷�z����g���Ċm���� nullptr ��n���iAPI �͐��z���v���j
		ID3D11ShaderResourceView* nullSRV[25] = {};
		g_pContext->PSSetShaderResources(0, static_cast<UINT>(TEX_COUNT), nullSRV);
	}

	// �C���f�b�N�X�^���_�o�b�t�@�̉���iNULL �`�F�b�N��� nullptr �ɐݒ�j
	if (g_IndexBuffer != nullptr)
	{
		g_IndexBuffer->Release();
		g_IndexBuffer = nullptr;
	}

	if (g_VertexBuffer != nullptr)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = nullptr;
	}

	// �e�N�X�`���z��S�v�f�����S�ɉ���i�R�����g�����Ė����[�h�̗v�f�� nullptr �`�F�b�N�ň��S�j
	for (size_t i = 0; i < TEX_COUNT; ++i)
	{
		if (g_Texture[i] != nullptr)
		{
			g_Texture[i]->Release();
			g_Texture[i] = nullptr;
		}
	}

	// �f�o�C�X�^�R���e�L�X�g�͊O���Ǘ��̂��߉�����Ȃ����A�Q�Ƃ̓N���A���Ă���
	g_pContext = nullptr;
	g_pDevice = nullptr;

	// �f�o�b�O�����_���[�̏I������
	Debug_Finalize();
}

// ======================================================
// �ړ��֐��i�v�ύX�j
// ------------------------------------------------------
// �ړ��x�N�g���ƌ����Ă�������x�N�g���͕ʂŎ�������������
// ======================================================

// 入力(ローカル)をカメラ基準でワールドXZへ変換する（平面移動用）
static inline XMFLOAT3 ToWorldMoveDirByCamera(const XMFLOAT2& input)
{
	// input.x: 右(+), input.y: 上(+)
	XMMATRIX view = GetViewMatrix();
	XMMATRIX invView = XMMatrixInverse(nullptr, view);

	// invView の行からカメラ軸を取得（world）
	XMFLOAT3 right = XMFLOAT3(invView.r[0].m128_f32[0], invView.r[0].m128_f32[1], invView.r[0].m128_f32[2]);
	XMFLOAT3 forward = XMFLOAT3(invView.r[2].m128_f32[0], invView.r[2].m128_f32[1], invView.r[2].m128_f32[2]);

	// XZ平面へ射影（Y成分を捨てる）
	right.y = 0.0f;
	forward.y = 0.0f;

	// 正規化（カメラが真上に近い等でゼロ割りを避ける）
	{
		float rl = sqrtf(right.x * right.x + right.z * right.z);
		if (rl > 0.0001f) { right.x /= rl; right.z /= rl; }
	}
	{
		float fl = sqrtf(forward.x * forward.x + forward.z * forward.z);
		if (fl > 0.0001f) { forward.x /= fl; forward.z /= fl; }
	}

	// ローカル入力をワールドへ合成
	XMFLOAT3 worldDir;
	worldDir.x = right.x * input.x + forward.x * input.y;
	worldDir.y = 0.0f;
	worldDir.z = right.z * input.x + forward.z * input.y;
	return worldDir;
}

void Move(PLAYEROBJECT& player, XMFLOAT3 moveDir)
{
	// �i�݂��������i3�����j
	float length = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);

	if (length > 0.0f)
	{
		// �x�N�g���̐��K��
		moveDir.x /= length;
		moveDir.z /= length;

		// �ڕW�p�x�����߂�
		float targetAngle = atan2f(moveDir.x, moveDir.z);	// �x�N�g���̊p�x
		targetAngle = XMConvertToDegrees(targetAngle);		// ���W�A�� -> �x

		// �����𒲐��i180�x�����Ȃ��悤�Ɂj
		float diff = targetAngle - player.moveAngle;	// �p�x��
		if (diff > 180.0f) diff -= 360.0f;
		if (diff < -180.0f) diff += 360.0f;

		static float angSpeed = 0.9f;

		// �X���[�Y�ɕ�ԁi0.1f����ԃX�s�[�h�j
		player.moveAngle += diff * angSpeed;

		player.rotation.y = player.moveAngle;	// �p�x�̔��f

		// �O�i
		float rad = XMConvertToRadians(player.moveAngle);
		player.position.x += sinf(rad) * player.speed;
		player.position.z += cosf(rad) * player.speed;
	}
}

//======================================================
// �X�V�֐�
//======================================================
void Player_Update()
{
	// �f�o�b�O�p ImGui �E�B���h�E
	ImGui::Begin("Player Debug");

	// �e�v���C���[�ɑΉ����锭���L�[
	const Keyboard_Keys_tag attackKeys[PLAYER_MAX] = { KK_SPACE, KK_ENTER, KK_V, KK_SPACE };

	const Keyboard_Keys_tag specialKeys[PLAYER_MAX] = { KK_D7, KK_D8, KK_D9, KK_D0 };

	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		// �v���C���[���Ƃ� ID �𕪂���i���ꃉ�x���Փˉ���j
		ImGui::PushID(p);

		ImGui::Text("Player %d", p + 1);
		ImGui::Indent();

		ImGui::SliderFloat("poisonTimer", &player[p].poisonTimer, 0.0f, 5.0f);
		ImGui::SliderFloat("specialTimer", &player[p].specialTimer, 0.0f, 10.0f);
		ImGui::SliderFloat("stunGauge", &player[p].stunGauge, 0.0f, 10.0f);
		ImGui::SliderFloat("satiety", &player[p].satiety, 0.0f, 6.0f);
		ImGui::BulletText("isEggBreaking     : %d", player[p].isEggBreaking);
		ImGui::BulletText("isShadowEnabled   : %d", player[p].isShadowEnabled);
		ImGui::BulletText("isHealing         : %d", player[p].isHealing);
		ImGui::BulletText("isPoisoned        : %d", player[p].isPoisoned);
		ImGui::BulletText("isInvincible      : %d", player[p].isInvincible);
		ImGui::BulletText("useSkill          : %d", player[p].useSkill);
		ImGui::BulletText("EvolutionGauge    : %.1f", player[p].evolutionGauge);
		ImGui::BulletText("EvolutionGaugeRate: %.1f", player[p].evolutionGaugeRate);

		if (ImGui::Button("hp -1"))			player[p].hp -= 0.1f;
		if (ImGui::Button("gl +1"))			player[p].breakCount_Glass += 1;
		else if (ImGui::Button("pl +1"))	player[p].breakCount_Plant += 1;
		else if (ImGui::Button("co +1"))	player[p].breakCount_Concrete += 1;
		else if (ImGui::Button("el +1"))	player[p].breakCount_Electricity += 1;

		ImGui::SliderFloat("HP", &player[p].hp, 0.0f, 500.0f);
		ImGui::SliderFloat("Outer", &player[p].evolutionGauge, 0.0f, 1.0f);
		ImGui::BulletText("2 Concrete breaks : %d", player[p].breakCount_Concrete);
		ImGui::BulletText("3 Plant breaks    : %d", player[p].breakCount_Plant);
		ImGui::BulletText("4 Electricity breaks : %d", player[p].breakCount_Electricity);

		// �������X�g�̃T�C�Y��\��
		size_t historySize = player[p].brokenHistory.size();
		ImGui::BulletText("brokenHistory Size : %zu", historySize);

		if (historySize > 0)
		{
			ImGui::Indent(); // ����������Ɉ�i�C���f���g
			ImGui::Text("History (Latest -> Oldest):");

			// �������ŐV�i�����j����Â����փ��[�v���ĕ\��
			for (int i = (int)historySize - 1; i >= 0; --i)
			{
				// BuildingType �� enum�^�i�����l�j�Ȃ̂ŁA���̂܂� %d �ŕ\���\
				// �܂��́AImGui::Text�Ő��`���ĕ\������

				// ��1: �����̃C���f�b�N�X�ƒl�𒼐ڕ\��
				// ImGui::BulletText("[%d]: %d", p, (int)object[p].brokenHistory[p]);

				// ��2: �����̒l�����ɕ��ׂĕ\��
				ImGui::SameLine(); // �����s�ɕ\��
				// �����̒l�i�����j�𕶎���ɕϊ����Ă���\��
				ImGui::Text("%d", (int)player[p].brokenHistory[i]);
			}

			// ���������ɕ��т����Ȃ��悤���s
			ImGui::NewLine();
			ImGui::Unindent();
		}

		ImGui::Unindent();
		ImGui::Separator();
		ImGui::PopID();

		if (!player[p].active) continue;

		// ���[���h���W���X�N���[�����W�ɕϊ�
		XMFLOAT3 worldPos = player[p].position;
		worldPos.y += 2.0f; // �v���C���[�̏���ɕ\��

		XMVECTOR posVec = XMLoadFloat3(&worldPos);
		XMMATRIX view = GetViewMatrix();
		XMMATRIX proj = GetProjectionMatrix();
		XMMATRIX viewProj = view * proj;

		// ビューポート変換
		XMVECTOR screenPos = XMVector3Project
		(
			posVec,
			0.0f, 0.0f,
			SCREEN_WIDTH, SCREEN_HEIGHT,
			0.0f, 1.0f,
			proj, view,
			XMMatrixIdentity()
		);

		// Z�l�`�F�b�N�i�J�����̌��Ȃ�`�悵�Ȃ��j
		float screenZ = XMVectorGetZ(screenPos);
		if (screenZ > 0.0f && screenZ < 1.0f)
		{
			float screenX = XMVectorGetX(screenPos);
			float screenY = XMVectorGetY(screenPos);

			// �e�L�X�g�`��iUpdate���ł͌Ăяo���Ȃ��ADraw���ŕ`�悷��j
			// �����ł͍��W��ۑ����Ă���
			player[p].screenPos = XMFLOAT2(screenX, screenY);
			player[p].isOnScreen = true;
		}
		else	player[p].isOnScreen = false;

		// -------------------------------------------------------------
		// �ϐg
		// -------------------------------------------------------------
		switch (player[p].form)
		{
		case Form::First: // ��1�`��
			player[p].scaling.x = 0.5f;
			player[p].scaling.y = 0.5f;
			player[p].scaling.z = 0.5f;
			player[p].attack = 10.0f;
			player[p].power = 1.0f;
			player[p].speed = 0.06f;
			break;

		case Form::Second: // ��2�`��
			player[p].scaling.x = 0.8f;
			player[p].scaling.y = 0.8f;
			player[p].scaling.z = 0.8f;
			player[p].attack = 15.0f;
			player[p].power = 1.5f;
			player[p].speed = 0.05f;
			break;

		case Form::Third: // ��3�`��
			player[p].scaling.x = 1.2f;
			player[p].scaling.y = 1.2f;
			player[p].scaling.z = 1.2f;
			player[p].attack = 20.0f;
			player[p].power = 2.0f;
			player[p].speed = 0.04f;
			break;
		default:
			break;
		}

		// 回復フラグの更新
		if (player[p].isHealing)
		{
			player[p].healingTimer += DELTA_TIME;	// 回復タイマーを更新

			if (player[p].healingTimer >= HEALING_TIME)
			{
				player[p].isHealing = false;	// 回復終了
				player[p].healingTimer = 0.0f;	// タイマーリセット
			}
		}

		// 進化フラグの更新
		if (player[p].isEvolving)
		{
			player[p].evolvingTimer += DELTA_TIME;	// 進化タイマーを更新

			if (player[p].evolvingTimer >= EVOLVING_TIME)
			{
				player[p].isEvolving = false;	// 進化終了
				player[p].evolvingTimer = 0.0f;	// タイマーリセット
			}
		}

		// 満腹度の減少
		player[p].satiety -= DELTA_TIME;
		if (player[p].satiety < 0.0f)	player[p].satiety = 0.0f;
		//// 満腹度が1未満ならHPを減少させる
		//if (player[p].satiety < 1.0f)	player[p].hp -= 0.05f;

		// リスポーン処理
		if (player[p].duringRespawn)
		{
			player[p].respawnTimer += DELTA_TIME;

			// Y座標を4に固定
			player[p].position.y = 4.0f;

			// 攻撃ボタン押下または5秒経過で落下開始
			if (g_Input[p].A || Keyboard_IsKeyDownTrigger(attackKeys[p]) || player[p].respawnTimer >= 5.0f)
			{
				player[p].duringRespawn = false;
				player[p].respawnTimer = 0.0f;
				player[p].isInvincible = true;
				player[p].invincibleTimer = 0.0f;
				player[p].isEggBreaking = true;
				player[p].eggBreakingTimer = 0.0f;
			}
		}
		else
		{
			// y軸の移動量 (重力 + ジャンプ)
			// 重力加速度のない簡易的な重力
			player[p].position.y += -0.1f;
		}

		// 卵エフェクトが割れる時間
		if (player[p].isEggBreaking)
		{
			player[p].eggBreakingTimer += DELTA_TIME;

			if (player[p].eggBreakingTimer >= EGG_BREAKING_TIME)
			{
				player[p].isEggBreaking = false;
				player[p].eggBreakingTimer = 0.0f;
			}
		}

		// 毒の処理
		if (player[p].poisonTimer > 0.0f)
		{
			// �ŏ�Ԃ̊ԁA�_���[�W��^����
			player[p].hp -= SPECIAL_PLANT_DAMAGE * player[p].defense;

			// �Ń^�C�}�[��i�߂�
			player[p].poisonTimer -= DELTA_TIME;

			// �Ń^�C�}�[��0�ɂȂ�����ŏ�Ԃ�����
			if (player[p].poisonTimer <= 0.0f)
			{
				player[p].isPoisoned = false;
				player[p].poisonTimer = 0.0f;
			}
		}

		// �X�^���Q�[�W���ő�ŃX�^���t���O�𗧂Ă�
		if (player[p].stunGauge >= STUNGAUGE_MAX)
		{
			player[p].isStunning = true;
			player[p].stunGauge = STUNGAUGE_MAX;
		}
		// �X�^�����̏���
		if (player[p].isStunning)
		{
			// �X�^���^�C�}�[��i�߂�
			player[p].stunTimer += DELTA_TIME;

			// ���Ԍo�߂ŃX�^������
			if (player[p].stunTimer >= STUN_TIME)
			{
				player[p].isStunning = false;	// �X�^������
				player[p].stunTimer = 0.0f;		// �X�^���^�C�}�[���Z�b�g
				player[p].stunGauge = 0.0f;		// �X�^���Q�[�W���Z�b�g
			}

			// �X�^�����͈ړ��x�N�g�������S�Ƀ[���ɂ���
			player[p].moveDir = { 0.0f, 0.0f, 0.0f };

			player[p].isMoving = false;

			player[p].isAttacking = false;
			player[p].useSkill = false;
			player[p].useSpecial = false;
		}
		else // �X�^�����Ă��Ȃ��ꍇ�̏���
		{
			// �X�^�����Ă��Ȃ��Ԃ̓X�^���Q�[�W������������
			player[p].stunGauge -= DELTA_TIME;

			// �X�^���Q�[�W��0�����ɂȂ�Ȃ��悤�ɃN�����v
			if (player[p].stunGauge < 0.0f)	player[p].stunGauge = 0.0f;
		}

		// �X�^�����E�_�E�����łȂ���Α�1�`�ԍs�� 1�ʊm���̓A�j���[�V�����̂�
		if(!player[p].isStunning && !player[p].isDown && player[p].rank != 1 && player[p].active)
		{
			// �����g���K�[���͂��`�F�b�N���čU���t���O�𗧂Ă�
			if (Keyboard_IsKeyDownTrigger(attackKeys[p]))
			{
				player[p].isAttacking = true;

				// ��2�E��3�`�Ԃ̏ꍇ�A�X�L���g�p�t���O�����Ă�
				if (player[p].type != PlayerType::None)	player[p].useSkill = true;
			}
			if (g_Input[p].A)	player[p].isAttacking = true;

			// ��2�E��3�`�Ԃ̏ꍇ�X�L���g�p�t���O���Ă�
			if (g_Input[p].X)	if (player[p].type != PlayerType::None)	player[p].useSkill = true;

			// �����g���K�[���͂��`�F�b�N���ăX�y�V�����g�p�t���O�𗧂Ă�
			if (player[p].form == Form::Third && Keyboard_IsKeyDownTrigger(specialKeys[p]))	player[p].useSpecial = true;
			
			// �{�^�����͂��`�F�b�N���ăX�y�V�����g�p�t���O�𗧂Ă�
			if (player[p].form == Form::Third && g_Input[p].ZR)	player[p].useSpecial = true;

			// �t���O����������X�V�������Ăяo��
			if (player[p].isAttacking)	Attack_Update(p);	AttackPlayerCollisions();	// �U��
			if (player[p].useSkill)		Skill_Update(p);								// �X�L��
			if (player[p].useSpecial)	Special_Update(p);								// �X�y�V����

			// ���݂̃v���C���[ p �̈ړ��x�N�g�����������Z�b�g
			player[p].moveDir = { 0.0f, 0.0f, 0.0f };
			
			XMFLOAT2 moveInput = { 0.0f, 0.0f };

			// �X�y�V���� �R���N���[�g�g�p���͈ړ��s��
			if (player[p].useSpecial && player[p].type == PlayerType::Concrete)
			{
				player[p].moveDir = { 0.0f, 0.0f, 0.0f };
				player[p].isMoving = false;
			}
			// �X�y�V���� �R���N���[�g�g�p���łȂ���Έړ�����
			else
			{
				player[p].moveInput2D = { 0.0f, 0.0f };

				if (p == 0) // プレイヤー0 (WASD) 攻撃 Space
				{
					if (g_Input[0].LStickX > 0.0f) { moveInput.x += 1.0f; player[0].isMoving = true; }
					if (g_Input[0].LStickX < 0.0f) { moveInput.x -= 1.0f; player[0].isMoving = true; }
					if (g_Input[0].LStickY < 0.0f) { moveInput.y += 1.0f; player[0].isMoving = true; }
					if (g_Input[0].LStickY > 0.0f) { moveInput.y -= 1.0f; player[0].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_W))  { moveInput.y += 1.0f; player[0].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_S))  { moveInput.y -= 1.0f; player[0].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_A))  { moveInput.x -= 1.0f; player[0].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_D))  { moveInput.x += 1.0f; player[0].isMoving = true; }
					if (moveInput.x == 0.0f && moveInput.y == 0.0f)	player[0].isMoving = false;
				}
				else if (p == 1) // �v���C���[1 (���L�[) �U�� Enter
				{
					if (g_Input[1].LStickX > 0.0f)	  { moveInput.x += 1.0f; player[1].isMoving = true; }
					if (g_Input[1].LStickX < 0.0f)	  { moveInput.x -= 1.0f; player[1].isMoving = true; }
					if (g_Input[1].LStickY < 0.0f)	  { moveInput.y += 1.0f; player[1].isMoving = true; }
					if (g_Input[1].LStickY > 0.0f)	  { moveInput.y -= 1.0f; player[1].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_UP))	  { moveInput.y += 1.0f; player[1].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_DOWN))  { moveInput.y -= 1.0f; player[1].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_LEFT))  { moveInput.x -= 1.0f; player[1].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_RIGHT)) { moveInput.x += 1.0f; player[1].isMoving = true; }
					if (moveInput.x == 0.0f && moveInput.y == 0.0f)	player[1].isMoving = false;
				}
				else if (p == 2) // �v���C���[2 (TFGH) �U�� V
				{
					if (g_Input[2].LStickX > 0.0f) { moveInput.x += 1.0f; player[2].isMoving = true; }
					if (g_Input[2].LStickX < 0.0f) { moveInput.x -= 1.0f; player[2].isMoving = true; }
					if (g_Input[2].LStickY < 0.0f) { moveInput.y += 1.0f; player[2].isMoving = true; }
					if (g_Input[2].LStickY > 0.0f) { moveInput.y -= 1.0f; player[2].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_T))  { moveInput.y += 1.0f; player[2].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_G))  { moveInput.y -= 1.0f; player[2].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_F))  { moveInput.x -= 1.0f; player[2].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_H))  { moveInput.x += 1.0f; player[2].isMoving = true; }
					if (moveInput.x == 0.0f && moveInput.y == 0.0f)	player[2].isMoving = false;
				}
				if (p == 3) // �v���C���[3 (WASD) �U�� Space
				{
					if (g_Input[3].LStickX > 0.0f) { moveInput.x += 1.0f; player[3].isMoving = true; }
					if (g_Input[3].LStickX < 0.0f) { moveInput.x -= 1.0f; player[3].isMoving = true; }
					if (g_Input[3].LStickY > 0.0f) { moveInput.y += 1.0f; player[3].isMoving = true; }
					if (g_Input[3].LStickY < 0.0f) { moveInput.y -= 1.0f; player[3].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_W))  { moveInput.y += 1.0f; player[3].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_S))  { moveInput.y -= 1.0f; player[3].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_A))  { moveInput.x -= 1.0f; player[3].isMoving = true; }
					if (Keyboard_IsKeyDown(KK_D))  { moveInput.x += 1.0f; player[3].isMoving = true; }
					if (moveInput.x == 0.0f && moveInput.y == 0.0f)	player[3].isMoving = false;
				}
				// 入力を保存（プレイヤーの向き用）
				player[p].moveInput2D = moveInput;

				// 移動はカメラ基準をワールドにする
				player[p].moveDir = ToWorldMoveDirByCamera(moveInput);
			}

			// 現在のプレイヤーpだけを動かす
			Move(player[p], player[p].moveDir);

			// 移動中ならlastDirを更新
			if (player[p].isMoving)
			{
				float dx = player[p].moveInput2D.x;
				float dz = player[p].moveInput2D.y;

				     if (dx < 0.0f && dz < 0.0f) player[p].lastDir = PlayerDir::Down_Left;
				else if (dx < 0.0f && dz > 0.0f) player[p].lastDir = PlayerDir::Up_Left;
				else if (dx > 0.0f && dz > 0.0f) player[p].lastDir = PlayerDir::Up_Right;
				else if (dx > 0.0f && dz < 0.0f) player[p].lastDir = PlayerDir::Down_Right;
				else if (dz < 0.0f)              player[p].lastDir = PlayerDir::Down;
				else if (dx < 0.0f)              player[p].lastDir = PlayerDir::Left;
				else if (dz > 0.0f)              player[p].lastDir = PlayerDir::Up;
				else if (dx > 0.0f)              player[p].lastDir = PlayerDir::Right;
			}
		}

		// �v���C���[���Ƃ̃X�L���N�[���^�C���𖈃t���[�����Z
		if (player[p].skillCoolTimer > 0.0f)
		{
			player[p].skillCoolTimer -= DELTA_TIME;
			if (player[p].skillCoolTimer < 0.0f) player[p].skillCoolTimer = 0.0f;
		}

		// HPが0以下の処理
		if (player[p].hp <= 0.0f && player[p].active && !player[p].isDown)
		{
			// �_�E����ԂɈڍs���ă^�C�}�[�����Z�b�g
			player[p].isDown = true;
			player[p].downTimer = 0.0f;
			Effect_ClearUI(p);
		}

		// �_�E����Ԃ̃^�C�}�[�X�V�ƃ��X�|�[������
		if (player[p].isDown)
		{
			// �s����~
			player[p].moveDir = { 0.0f, 0.0f, 0.0f };
			player[p].isAttacking = false;
			player[p].useSkill = false;
			player[p].useSpecial = false;

			// �_�E���^�C�}�[�X�V
			player[p].downTimer += DELTA_TIME;

			// �v���C���[���̃_�E�����Ԃ��o�߂����烊�X�|�[������
			if (player[p].downTimer >= DOWN_TIME)
			{
				// �c�@��1���炷
				player[p].stock -= 1;

				if (player[p].stock > 0)	Player_Respawn(p);
				else
				{
					// �c�@�����ŕ����Ȃ�
					player[p].active = false;
					player[p].isDown = false;
					player[p].downTimer = 0.0f;

					// ���ʓo�^�i�����ŏd���o�^��h�~�j
					Ranking(p);
				}
			}
		}

		// 落下処理 影エフェクト非表示
		if (player[p].position.y < -1.0f)
		{
			player[p].isShadowEnabled = false;
		}

		if (player[p].active && player[p].position.y <= -10.0f)
		{
			Effect_ClearUI(p);
			// 残機を一つ減らす
			player[p].stock -= 1;

			// ���X�|�[���i�ʒu�E�X�e�[�g���Z�b�g�j
			if (player[p].stock > 0)	Player_Respawn(p);
			else
			{
				// �c�@�����Ŋ��S�ɔ�A�N�e�B�u��
				player[p].active = false;
				// ���ʓo�^
				Ranking(p);
			}
		}

		// �_���[�W���󂯂����̏���
		if (player[p].isAttacked)
		{
			// �_���[�W�^�C�}�[�X�V
			player[p].attackedTimer += DELTA_TIME;

			// �v���C���[���̃_���[�W���Ԃ��o�߂�����_���[�W�I��
			if (player[p].attackedTimer >= ATTACKED_TIME)
			{
				player[p].isAttacked = false;
				player[p].attackedTimer = 0.0f;
			}
		}

		// �i�����̖��G����
		if (player[p].isInvincible)
		{
			// ���G�^�C�}�[�X�V
			player[p].invincibleTimer += DELTA_TIME;

			// プレイヤー毎の無敵時間が経過したら無敵終了
			if (player[p].invincibleTimer >= EVOLVING_TIME)
			{
				player[p].isInvincible = false;
				player[p].invincibleTimer = 0.0f;
			}
		}

		// �X�y�V�����J�n���̃t���[���������i�A�j���[�V�����X�V�^�C�~���O�Ɉˑ����Ȃ��j
		if (player[p].useSpecial && !g_specialAnimStarted[p])
		{
			int type = -1;
				 if(player[p].type == PlayerType::Concrete)		type = 0;
			else if(player[p].type == PlayerType::Electricity)	type = 1;
			else if(player[p].type == PlayerType::Glass)		type = 2;
			else if(player[p].type == PlayerType::Plant)		type = 3;

			int start = type * 64;
				 if (player[p].lastDir == PlayerDir::Down)		start += 0;
			else if (player[p].lastDir == PlayerDir::Down_Left)	start += 8;
			else if (player[p].lastDir == PlayerDir::Left)		start += 16;
			else if (player[p].lastDir == PlayerDir::Up_Left)	start += 24;
			else if (player[p].lastDir == PlayerDir::Up)		start += 32;
			else if (player[p].lastDir == PlayerDir::Up_Right)	start += 40;
			else if (player[p].lastDir == PlayerDir::Right)		start += 48;
			else if (player[p].lastDir == PlayerDir::Down_Right)start += 56;

			g_animFrame[p] = start;
			g_specialAnimStarted[p] = true;
		}
		// �X�y�V�����I�����̃t���[�����Z�b�g
		if (!player[p].useSpecial && g_specialAnimStarted[p])
		{
			g_specialAnimStarted[p] = false;

			// �ʏ�e�N�X�`���̑ҋ@�A�j���[�V�����J�n�t���[���Ƀ��Z�b�g
			int start = 0;
				 if (player[p].lastDir == PlayerDir::Down)		start = 0;
			else if (player[p].lastDir == PlayerDir::Down_Left)	start = 26;
			else if (player[p].lastDir == PlayerDir::Left)		start = 52;
			else if (player[p].lastDir == PlayerDir::Up_Left)	start = 78;
			else if (player[p].lastDir == PlayerDir::Up)		start = 104;
			else if (player[p].lastDir == PlayerDir::Up_Right)	start = 130;
			else if (player[p].lastDir == PlayerDir::Right)		start = 156;
			else if (player[p].lastDir == PlayerDir::Down_Right)start = 182;

			g_animFrame[p] = start;
		}

		// �v���C���[ �A�j���[�V�����X�V
		g_animTimer[p] += DELTA_TIME;

		// エフェクト アニメーション
		Effect_UpdateForPlayer(p);

		if (g_animTimer[p] >= ANIM_FRAME_TIME)
		{
			int advance = (int)(g_animTimer[p] / ANIM_FRAME_TIME);
			g_animTimer[p] -= advance * ANIM_FRAME_TIME;

			// 勝利 第1形態 13コマ(ラスト5コマ ループ) 第2形態 20コマ(ラスト9コマ ループ) 第3形態 21コマ(ラストコマ ループ)
			//if (Keyboard_IsKeyDown(KK_TAB) || g_victoryState[p] != 0)
			if (player[p].rank == 1 || g_victoryState[p] != 0)
			{
				//if (Keyboard_IsKeyDown(KK_TAB) && g_victoryState[p] == 0)
				if (player[p].rank == 1 && g_victoryState[p] == 0)
				{
					g_victoryState[p] = 1;
					g_animFrame[p] = 208;	// ����Đ��J�n�t���[��
				}

				if (g_victoryState[p] == 1)
				{
					// ����Đ� �t���[����P������
					g_animFrame[p] += advance;

					// ��1�`�� 220 ��\��������Ƀ��[�v�̈�ֈڍs����
					if (g_animFrame[p] > 220 && player[p].form == Form::First)
					{
						g_victoryState[p] = 2;
						g_animFrame[p] = 216;	// ���[�v�J�n�t���[��
					}
					// ��2�`�� 227 ��\��������Ƀ��[�v�̈�ֈڍs����
					if (g_animFrame[p] > 227 && player[p].form == Form::Second)
					{
						g_victoryState[p] = 2;
						g_animFrame[p] = 219;	// ���[�v�J�n�t���[��
					}
					// ��3�`�� 228 ��\��������Ƀ��[�v�̈�ֈڍs���� 229�R�}�ڂ͎g�p���Ȃ�
					if (g_animFrame[p] > 228 && player[p].form == Form::Third)
					{
						g_victoryState[p] = 2;
						g_animFrame[p] = 221;	// ���[�v�J�n�t���[��
					}
				}
				else if (g_victoryState[p] == 2)
				{
					switch (player[p].form)
					{
					case Form::First:	LoopRange(g_animFrame[p], 216, 5, advance);	// ��1�`�� 216�`220�����[�v
						break;
					case Form::Second:	LoopRange(g_animFrame[p], 219, 9, advance);	// ��2�`�� 219�`227�����[�v
						break;
					case Form::Third:	LoopRange(g_animFrame[p], 221, 8, advance);	// ��3�`�� 221�`228�����[�v 229�R�}�ڂ͎g�p���Ȃ�
						break;
					}
				}
			}

			// �_�E�� 5�R�} (�_���[�W 2�R�} + �_�E�� 3�R�}) �ŏI�R�}�Œ�~
			else if (player[p].isDown == true)
			{
				// �����ɉ������J�n�t���[��������
				int start = 15; // �f�t�H���g�iDown�j
					 if (player[p].lastDir == PlayerDir::Down)		 start = 15;
				else if (player[p].lastDir == PlayerDir::Down_Left)	 start = 41;
				else if (player[p].lastDir == PlayerDir::Left)		 start = 67;
				else if (player[p].lastDir == PlayerDir::Up_Left)	 start = 93;
				else if (player[p].lastDir == PlayerDir::Up)		 start = 119;
				else if (player[p].lastDir == PlayerDir::Up_Right)	 start = 145;
				else if (player[p].lastDir == PlayerDir::Right)		 start = 171;
				else if (player[p].lastDir == PlayerDir::Down_Right) start = 197;

				const int count = 5;
				const int lastFrame = start + count - 1;

				// advance �ɑΉ�����o�ߕb�ig_animTimer�ł܂Ƃ߂Đi�߂����j
				float elapsedSec = (float)advance * ANIM_FRAME_TIME;

				// �t���[�����͈͊O�Ȃ�J�n�t���[���ɕ␳���^�C�}�[���Z�b�g
				if (g_animFrame[p] < start || g_animFrame[p] > lastFrame)
				{
					g_animFrame[p] = start;
					g_downHoldTimer[p] = 0.0f;
				}

				// �ŏI�t���[���ȊO�Ȃ��1�`�Ԑi�s�i���[�v�j
				if (g_animFrame[p] != lastFrame)
				{
					LoopRange(g_animFrame[p], start, count, advance);
					g_downHoldTimer[p] = 0.0f; // ���B�O�̓z�[���h�^�C�}�[�����Z�b�g
				}
				else
				{
					// �ŏI�t���[���ɓ��B �z�[���h��i�߂�
					g_downHoldTimer[p] += elapsedSec;

					// �z�[���h�����������玟�ɐi�߂�i�����ł�1�t���[���������i�߂�j
					if (g_downHoldTimer[p] >= DOWN_TIME)
					{
						g_downHoldTimer[p] = 0.0f;
						// 1�t���[�����i�߂�i���[�v�ɂ�� start �ɖ߂�j
						LoopRange(g_animFrame[p], start, count, 1);
					}
				}
			}
			// �X�y�V���� 8�R�}
			else if (player[p].useSpecial)
			{
				int type = -1;
					 if(player[p].type == PlayerType::Concrete)		type = 0;
				else if(player[p].type == PlayerType::Electricity)	type = 1;
				else if(player[p].type == PlayerType::Glass)		type = 2;
				else if(player[p].type == PlayerType::Plant)		type = 3;

				// �����ɉ������J�n�t���[��������
				int start = type * 64;
					 if (player[p].lastDir == PlayerDir::Down)		start += 0;
				else if (player[p].lastDir == PlayerDir::Down_Left)	start += 8;
				else if (player[p].lastDir == PlayerDir::Left)		start += 16;
				else if (player[p].lastDir == PlayerDir::Up_Left)	start += 24;
				else if (player[p].lastDir == PlayerDir::Up)		start += 32;
				else if (player[p].lastDir == PlayerDir::Up_Right)	start += 40;
				else if (player[p].lastDir == PlayerDir::Right)		start += 48;
				else if (player[p].lastDir == PlayerDir::Down_Right)start += 56;

				const int count = 8;

				LoopRange(g_animFrame[p], start, count, advance);
			}
			// �_���[�W 3�R�}
			else if (player[p].isAttacked == true || player[p].isStunning)
			{
					 if (player[p].lastDir == PlayerDir::Down)		LoopRange(g_animFrame[p],  14, 3, advance);	//  ��   14�`16 
				else if (player[p].lastDir == PlayerDir::Down_Left)	LoopRange(g_animFrame[p],  40, 3, advance);	// ����  40�`42
				else if (player[p].lastDir == PlayerDir::Left)		LoopRange(g_animFrame[p],  66, 3, advance);	//  ��   66�`68
				else if (player[p].lastDir == PlayerDir::Up_Left)	LoopRange(g_animFrame[p],  92, 3, advance);	// ����  92�`94
				else if (player[p].lastDir == PlayerDir::Up)		LoopRange(g_animFrame[p], 118, 3, advance);	//  ��  118�`120
				else if (player[p].lastDir == PlayerDir::Up_Right)	LoopRange(g_animFrame[p], 144, 3, advance);	// �E�� 144�`146
				else if (player[p].lastDir == PlayerDir::Right)		LoopRange(g_animFrame[p], 170, 3, advance);	//  �E  170�`172
				else if (player[p].lastDir == PlayerDir::Down_Right)LoopRange(g_animFrame[p], 196, 3, advance);	// �E�� 196�`198
			}
			// �U�� 6�R�}
			else if (player[p].isAttacking == true)
			{
					 if (player[p].lastDir == PlayerDir::Down)		LoopRange(g_animFrame[p],  20, 6, advance);	//  ��   20�`25
				else if (player[p].lastDir == PlayerDir::Down_Left)	LoopRange(g_animFrame[p],  46, 6, advance);	// ����  46�`51
				else if (player[p].lastDir == PlayerDir::Left)		LoopRange(g_animFrame[p],  72, 6, advance);	//  ��   72�`77
				else if (player[p].lastDir == PlayerDir::Up_Left)	LoopRange(g_animFrame[p],  98, 6, advance);	// ����  98�`103
				else if (player[p].lastDir == PlayerDir::Up)		LoopRange(g_animFrame[p], 124, 6, advance);	//  ��  124�`129
				else if (player[p].lastDir == PlayerDir::Up_Right)	LoopRange(g_animFrame[p], 150, 6, advance);	// �E�� 150�`155
				else if (player[p].lastDir == PlayerDir::Right)		LoopRange(g_animFrame[p], 176, 6, advance);	//  �E  176�`181
				else if (player[p].lastDir == PlayerDir::Down_Right)LoopRange(g_animFrame[p], 202, 6, advance);	// �E�� 202�`207
			}
			// 移動 8コマ （リスポーン中を除く）
			else if (!player[p].duringRespawn && player[p].isMoving == true)
			{
				float dx = player[p].moveInput2D.x;
				float dz = player[p].moveInput2D.y;

					 if (dx < 0.0f && dz < 0.0f) LoopRange(g_animFrame[p], 32, 8, advance);
				else if (dx < 0.0f && dz > 0.0f) LoopRange(g_animFrame[p], 84, 8, advance);
				else if (dx > 0.0f && dz > 0.0f) LoopRange(g_animFrame[p], 136, 8, advance);
				else if (dx > 0.0f && dz < 0.0f) LoopRange(g_animFrame[p], 188, 8, advance);
				else if (dz < 0.0f)              LoopRange(g_animFrame[p], 6, 8, advance);
				else if (dx < 0.0f)              LoopRange(g_animFrame[p], 58, 8, advance);
				else if (dz > 0.0f)              LoopRange(g_animFrame[p], 110, 8, advance);
				else if (dx > 0.0f)              LoopRange(g_animFrame[p], 162, 8, advance);
			}
			// �ҋ@ 6�R�}
			else if (player[p].isMoving == false)
			{
					 if (player[p].lastDir == PlayerDir::Down)		LoopRange(g_animFrame[p],   0, 6, advance);	//  ��    0�`5
				else if (player[p].lastDir == PlayerDir::Down_Left)	LoopRange(g_animFrame[p],  26, 6, advance);	// ����  26�`31
				else if (player[p].lastDir == PlayerDir::Left)		LoopRange(g_animFrame[p],  52, 6, advance);	//  ��   52�`57
				else if (player[p].lastDir == PlayerDir::Up_Left)	LoopRange(g_animFrame[p],  78, 6, advance);	// ����  78�`83 
				else if (player[p].lastDir == PlayerDir::Up)		LoopRange(g_animFrame[p], 104, 6, advance);	//  ��  104�`109
				else if (player[p].lastDir == PlayerDir::Up_Right)	LoopRange(g_animFrame[p], 130, 6, advance);	// �E�� 130�`135
				else if (player[p].lastDir == PlayerDir::Right)		LoopRange(g_animFrame[p], 156, 6, advance);	//  �E  156�`161
				else if (player[p].lastDir == PlayerDir::Down_Right)LoopRange(g_animFrame[p], 182, 6, advance);	// �E�� 182�`187		
			}
		}



	
		static XMFLOAT3 posBuff = player[p].position;	// �f�o�b�O�\�����W

		// 描画で使っているスプライト倍率と同じ値を物理にも使う
		const float renderScale = 2.0f;	// Draw 側の spriteScale に合わせる
		// 描画スケールを反映したスケール（表示用）
		XMFLOAT3 physicsScaling = XMFLOAT3(player[p].scaling.x * renderScale, player[p].scaling.y * renderScale, player[p].scaling.z * renderScale);


		////////////////////////////////////////////////////////////////////////////////////////////
		// TODO:

		// --- �v���C���[�p�q�b�g�{�b�N�X�䗦�i�����Œ��Z��؂�ւ���j ---
		// �����͌Œ�A�����ʂ͌����ɉ����Ē��Z��؂�ւ���
		const float HITBOX_HEIGHT_SCALE = 1.0f;
		const float HITBOX_SHORT = 0.35f;	// 向きと直交する短辺
		const float HITBOX_LONG  = 0.65f;	// 向きに沿った長辺

		// ��]����O���x�N�g�����Z�o���āA�ǂ���̎����D�������肷��
		float radFacing = XMConvertToRadians(player[p].rotation.y);
		float facingX = sinf(radFacing);
		float facingZ = cosf(radFacing);
		bool facingZDominant = fabsf(facingZ) >= fabsf(facingX);

		float widthScale = facingZDominant ? HITBOX_SHORT : HITBOX_LONG;	// X�����X�P�[��
		float depthScale = facingZDominant ? HITBOX_LONG  : HITBOX_SHORT;	// Z�����X�P�[��

		// ��2�`�� ��3�`�Ԃ�X��Z�����ɂ���
		if (player[p].form == Form::Second || player[p].form == Form::Third)
		{
			widthScale = 0.25f;
			depthScale = 0.25f;
		}

		XMFLOAT3 hitboxScaling = XMFLOAT3
		(
			player[p].scaling.x * renderScale * widthScale,
			player[p].scaling.y * renderScale * HITBOX_HEIGHT_SCALE,
			player[p].scaling.z * renderScale * depthScale
		);


		/////////////////////////////////////////////////////////////////////////////////////
		// TODO:�����蔻��̌�����
		// TODO:�����Ƃ̂ق����������蔻��Ƃ͕ʂɁA�U����H�炤�p�̑傫�߂̓����蔻������
		
		// AABB �����݂̈ʒu�E�X�P�[���i�q�b�g�{�b�N�X�j�ōX�V���Ă����i�Փ˔���Ŏg�p�j
		CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);

		posBuff = player[p].position;

		// 地面の高さ（最低ライン）
		//float groundHeight = -10.0f;	// 奈落の底
		//bool isShadowEnabled = false;		// 地面に足がついているかフラグ

		// �}�b�v�f�[�^�i�n�ʁj�Ƃ̓����蔻��
		int fieldCount = GetFieldObjectCount();
		MAPDATA* fieldObjects = GetFieldObjects();

		for (int j = 0; j < fieldCount; ++j)
		{
			// �A�N�e�B�u����Ȃ��A�܂��� no �� MAX �Ȃ�X�L�b�v
			if (!fieldObjects[j].isActive || fieldObjects[j].no == FIELD::FIELD_MAX)
			{
				continue;
			}

			// �v���C���[��AABB�i�̂̈ꕔ�j���Z�p���ɏ���Ă��邩
			if (CheckAABBHexCollision(player[p].boundingBox, fieldObjects[j].boundingBox))
			{
				// �^�C���̏�ʂ�Y���W���v�Z
				float tileTopY = fieldObjects[j].pos.y + (fieldObjects[j].boundingBox.height / 2.0f);	// -1 + 1.5 = 0.5

				// �v���C���[�̒�ʂ��^�C���̏�ʈȉ���
				if (player[p].boundingBox.Min.y <= tileTopY)
				{
					const float baseHalfHeight = COORDINATE;
					// ���n�ł͌����ڂ̍����i�`��X�P�[���j����Ɍv�Z���Ă��邽�� physicsScaling ���g�p
					float halfHeight = baseHalfHeight * player[p].scaling.y * renderScale;

					// ���n������i�߂荞�݂��N���Ȃ��悤�Œ�l�Ƃ��ĕ␳�j
					float targetY = tileTopY + halfHeight;
					if (player[p].position.y < targetY)
					{
						player[p].position.y = targetY;
						player[p].isShadowEnabled = true; // 影エフェクト非表示
					}

					// AABB ���Čv�Z���Đ�������ۂi�`��X�P�[�����l���j
					// �q�b�g�{�b�N�X�i�����ɉ����������`�j�ōČv�Z����
					CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);

					top_y = tileTopY;

					break;
				}
			}
		}

		// -------------------------------------------------------------------------------------
		// �����Ƃ̓����蔻��
		// -------------------------------------------------------------------------------------
		int buildingCount = GetBuildingCount();			// �����擾
		Building** buildingObjects = GetBuildings();	// ���X�g���擾

		for (int j = 0; j < buildingCount; ++j)
		{
			// �A�N�e�B�u�łȂ��Ȃ疳��
			if (!buildingObjects[j]->isActive)	continue;

			// y���W�̒���
			// Building::Draw() �� position.y + 1.0f ���Ă���̂ŁA����p�̍��W�����킹��
			XMFLOAT3 colliderPos = buildingObjects[j]->position;
			colliderPos.y += 1.0f;

			// �R���C�_�[�̍쐬�ƍX�V�i�␳�������W colliderPos ���g���j
			CalculateAABB(buildingObjects[j]->boundingBox, colliderPos, buildingObjects[j]->scaling);

			// �v���C���[ �� �����̓����蔻��
			MTV collision = CalculateAABBMTV(player[p].boundingBox, buildingObjects[j]->boundingBox);

			if (collision.isColliding)
			{
				// �Փ˂��Ă�����AMTV�̕������ʒu��߂�
				player[p].position.x += collision.translation.x;
				player[p].position.y += collision.translation.y;
				player[p].position.z += collision.translation.z;

				// �����߂���̐V����AABB���Čv�Z�i�`��X�P�[���𔽉f�j
				// �q�b�g�{�b�N�X�i�����ɉ����������`�j�ōČv�Z����
				CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);
			}
		}

		// �v���C���[�ɑΉ�����U���I�u�W�F�N�g�� PLAYER_MAX �����[�v���ăX�P�[�����O����
		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			ATTACK_OBJECT* attackObject = GetAttack(p); // GetAttack �� 1-based
			if (attackObject == nullptr) continue;

			// �v���C���[���̃X�P�[���ɍ��킹��i�U���I�u�W�F�N�g�͔����j
			attackObject->scaling.x = player[p].scaling.x * 0.5f;
			attackObject->scaling.y = player[p].scaling.y * 0.5f;
			attackObject->scaling.z = player[p].scaling.z * 0.5f;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////
		// TODO:

		// -------------------------------------------------------------
		// �v���C���[�I�u�W�F�N�g���m�̓����蔻��iPLAYER_MAX���Ή��j
		// -------------------------------------------------------------
		for (int otherIndex = p + 1; otherIndex < PLAYER_MAX; ++otherIndex)
		{
			// ��A�N�e�B�u�͖���
			if (!player[otherIndex].active) continue;

			// ���v���C���[�� AABB ���X�V�i�����Œ�`�ς݂� hitboxScalingOther ���g�p�j
			CalculateAABB(player[otherIndex].boundingBox, player[otherIndex].position, hitboxScaling);

			// �Փ˃`�F�b�N�i�y�A p <-> otherIndex ����x��������j
			MTV collision_player = CalculateAABBMTV(player[p].boundingBox, player[otherIndex].boundingBox);

			if (collision_player.isColliding)
			{
				// �����x�N�g�����X�V�irotation.y ����Z�o�j
				{
					float rad_p = XMConvertToRadians(player[p].rotation.y);
					player[p].dir.x = sinf(rad_p);
					player[p].dir.z = cosf(rad_p);
				}


				{
					float rad_o = XMConvertToRadians(player[otherIndex].rotation.y);
					player[otherIndex].dir.x = sinf(rad_o);
					player[otherIndex].dir.z = cosf(rad_o);
				}

				// �����߂��� (MTV) �𔼕��ɂ��đo���ɓK�p
				XMFLOAT3 half_translation =
				{
					collision_player.translation.x * 0.5f,
					collision_player.translation.y * 0.5f,
					collision_player.translation.z * 0.5f
				};

				// object[p] �� MTV �̔�����������
				player[p].position.x += half_translation.x;
				player[p].position.y += half_translation.y;
				player[p].position.z += half_translation.z;

				// object[otherIndex] ���t�����ɔ�����������
				player[otherIndex].position.x -= half_translation.x;
				player[otherIndex].position.y -= half_translation.y;
				player[otherIndex].position.z -= half_translation.z;

				// �����߂���̐V����AABB���Čv�Z (�q�b�g�{�b�N�X��)
				CalculateAABB(player[p].boundingBox, player[p].position, hitboxScaling);
				CalculateAABB(player[otherIndex].boundingBox, player[otherIndex].position, hitboxScaling);
			}
		}

		SetHPValue(&HPBar[p], (int)player[p].hp, (int)PLAYER_MAX_HP);
		UpdateHP(&HPBar[p]);
	}
	ImGui::End();
}

//======================================================
//	�`��֐�
//======================================================
void Player_Draw(bool s_IsKonamiCodeEntered)
{
	// �U���E�X�L���E�X�y�V�����`��
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (player[p].active && player[p].isAttacking)	Attack_Draw(p);
		//if (player[p].active && player[p].useSkill)		Skill_Draw(p);
		if (player[p].active && player[p].useSpecial)	Special_Draw(p);
	}

	LIGHT light{};
	light.Enable = TRUE;
	// ���̌����i���[���h��ԁj�V�F�[�_�[���ŒP�ʉ����Ďg���Ă���z��
	light.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
	// �g�U���Ɗ���
	light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	Shader_SetLight(light);

	static bool input1 = false;
	// �f�o�b�O���[�h���̂݃L�[���͂��󂯕t����
	if (s_IsKonamiCodeEntered)
	{
		if (Keyboard_IsKeyDownTrigger(KK_D1)) input1 = !input1;	// �t���O���]
	}
	
	Shader_Begin(); 

	// ========================================================
	// ���̃v���C���[����O�̃v���C���[�ɉB��Ȃ��悤�ɕ`��
	// ========================================================

	// �v���W�F�N�V�����E�r���[�s����Ɏ擾
	XMMATRIX projection = GetProjectionMatrix();
	XMMATRIX view = GetViewMatrix();

	// �J�����ʒu���Z�o�iView �̋t�s��� r[3] �����[���h��Ԃ̃J�����ʒu�j
	XMMATRIX invView = XMMatrixInverse(nullptr, view);
	XMFLOAT3 camPos;
	camPos.x = invView.r[3].m128_f32[0];
	camPos.y = invView.r[3].m128_f32[1];
	camPos.z = invView.r[3].m128_f32[2];

	// �v���C���[��`�悷�郉���_�iProjection, View ���L���v�`���j
	auto DrawPlayerInternal = [&](int idx)
	{
		if (!player[idx].active) return;

		// プレイヤーの影エフェクト描画
		EffectShadow_DrawForPlayer(idx);

		const float spriteScale = 3.5f;	// 表示倍率

		// ���[���h�s��i�r���{�[�h���̊������W�b�N�𓥏P�j
		XMMATRIX ScalingMatrix = XMMatrixScaling(
			player[idx].scaling.x * spriteScale,
			player[idx].scaling.y * spriteScale,
			player[idx].scaling.z * spriteScale
		);

		XMMATRIX vm = GetViewMatrix();	// �J�����̍s��
		vm.r[3].m128_f32[0] = 0.0f;
		vm.r[3].m128_f32[1] = 0.0f;
		vm.r[3].m128_f32[2] = 0.0f;
		vm.r[3].m128_f32[3] = 1.0f;
		vm = XMMatrixTranspose(vm);
		vm.r[3].m128_f32[0] = player[idx].position.x;
		vm.r[3].m128_f32[1] = player[idx].position.y;
		vm.r[3].m128_f32[2] = player[idx].position.z;
		vm.r[3].m128_f32[3] = 1.0f;

		// World �s��i�r���{�[�h�p�j���V�F�[�_�[�ɓn��
		XMMATRIX WorldMatrix = ScalingMatrix * vm;
		Shader_SetWorldMatrix(WorldMatrix);

		XMMATRIX WVP = ScalingMatrix * vm * view * projection;

		Shader_SetMatrix(WVP);
		Shader_Begin();
		SetBlendState(BLENDSTATE_ALPHA);

		// ���_�o�b�t�@�Ƀf�[�^�R�s�[�i�t���[���ɉ�����UV������������j
		D3D11_MAPPED_SUBRESOURCE msr;

		// �R�s�[����vdata �����[�J���z��ɃR�s�[���� UV �𒲐�
		Vertex2 localV[PLAYER_VERTEX];
		CopyMemory(&localV[0], &vdata[0], sizeof(Vertex2) * PLAYER_VERTEX);

		// ���݂̃t���[������ UV ���v�Z
		int frame = g_animFrame[idx];
		int col = frame % SHEET_COLS;
		int row = frame / SHEET_COLS;
		float u0 = (float)col / (float)SHEET_COLS;
		float v0 = (float)row / (float)SHEET_ROWS;
		float u1 = u0 + 1.0f / (float)SHEET_COLS;
		float v1 = v0 + 1.0f / (float)SHEET_ROWS;

		// ���_�̃e�N�X�`�����W���㏑��
		localV[0].tex = XMFLOAT2(u0, v0);	// LEFT-TOP
		localV[1].tex = XMFLOAT2(u1, v0);	// RIGHT-TOP
		localV[2].tex = XMFLOAT2(u0, v1);	// LEFT-BOTTOM
		localV[3].tex = XMFLOAT2(u1, v1);	// RIGHT-BOTTOM
		
		// �o�b�t�@�֏�������
		g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		Vertex2* vertex = (Vertex2*)msr.pData;
		CopyMemory(vertex, &localV[0], sizeof(Vertex2) * PLAYER_VERTEX);
		g_pContext->Unmap(g_VertexBuffer, 0);

		ID3D11ShaderResourceView* srv = nullptr;

		// �`�Ԃƃ^�C�v�ɉ������e�N�X�`����ݒ�
		switch (player[idx].form)
		{
		// ��1�`��
		case Form::First:
				 if(idx == 0)	{ srv = g_Texture[0];	break; }
			else if(idx == 1)	{ srv = g_Texture[1];	break; }
			else if(idx == 2)	{ srv = g_Texture[2];	break; }
			else if(idx == 3)	{ srv = g_Texture[3];	break; }
			// ��2�`��
		case Form::Second:
			switch (player[idx].type)
			{
			case PlayerType::Glass:			srv = g_Texture[4];	break;				
			case PlayerType::Concrete:		srv = g_Texture[5];	break;
			case PlayerType::Plant:			srv = g_Texture[6];	break;
			case PlayerType::Electricity:	srv = g_Texture[7];	break;
			default: break;
			}
			break;
		// ��3�`��
		case Form::Third:
			switch (player[idx].type)
			{
			case PlayerType::Glass:			srv = g_Texture[8];		break;
			case PlayerType::Concrete:		srv = g_Texture[9];		break;
			case PlayerType::Plant:			srv = g_Texture[10];	break;
			case PlayerType::Electricity:	srv = g_Texture[11];	break;
			default: break;
			}
			break;
		default: break;
		}

		// �X�y�V�����g�p���͐�p�e�N�X�`��
		if (player[idx].useSpecial)			srv = g_Texture[12];

		g_pContext->PSSetShaderResources(0, 1, &srv);

		// �v���C���[���ƂɈقȂ�F��ݒ�
		if (player[idx].isPoisoned)
		{
			switch (idx)
			{
			// Lerp = 1.乗算色 2.補間する色 3.補間の度合い
			case 0:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			case 1:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			case 2:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			case 3:		Shader_SetColorLerp(color::white, color::purple, 0.7f); break;
			default:	Shader_SetColor(color::white); break;
			}
		}
		else			Shader_SetColor(color::white); // 通常色

		// �o�b�t�@�Z�b�g & �`��
		UINT stride = sizeof(Vertex2);
		UINT offset = 0;
		g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
		g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		g_pContext->DrawIndexed(6, 0, 0);

		// エフェクト描画 （プレイヤーの手前）
		EffectFront_DrawForPlayer(idx);
	};

	// -----------------------------------
	// �����`��̂��߂̃\�[�g�i�������j
	// -----------------------------------
	std::vector<std::pair<float, int>> list;	// (�������, index)
	list.reserve(PLAYER_MAX);

	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (!player[p].active) continue;

		float dx = player[p].position.x - camPos.x;
		float dy = player[p].position.y - camPos.y;
		float dz = player[p].position.z - camPos.z;
		float dist2 = dx * dx + dy * dy + dz * dz;
		list.emplace_back(dist2, p);
	}

	// �������i�傫�����j�Ƀ\�[�g
	std::sort(list.begin(), list.end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
		{
		return a.first > b.first;
		});

	// ���߃����_�����O�F�[�x�e�X�g�͗L���A�[�x�������݂͖����iSetDepthReadOnly ���g�p�j
	SetDepthTest(true);
	SetDepthReadOnly();	// �[�x�e�X�g�͂��邪�[�x�o�b�t�@�ւ̏������݂͂��Ȃ�

	// �\�[�g���i�������̂���`��j
	for (auto& p : list)	DrawPlayerInternal(p.second);

	// 3D�I�u�W�F�N�g�͐[�x�e�X�g�𖳌��ɂ��ĕ`��
	SetDepthTest(false);

	// 3D�I�u�W�F�N�g�i�v���C���[�j�̕`�悪�I�������...
	SetDepthTest(false); // �R���C�_�[���őO�ʂɏo�������Ȃ炱���OK

	/////////////////////////////////////////////////////////////////////////////////////
	// TODO:�����蔻��̉���
	if (s_IsKonamiCodeEntered)
	{
		// �v���C���[�̕`��Ɏg��ꂽ�s����N���A����
		Shader_SetMatrix(XMMatrixIdentity() * GetViewMatrix() * GetProjectionMatrix()); // WVP�s���Identity * View * Projection�ɐݒ�

		// 3. ���߂�F�����������Ȃ�Ȃ��悤�Ƀu�����h�X�e�[�g�����Z�b�g
		SetBlendState(BLENDSTATE_NONE); // �g���Ȃ�A���t�@�Ȃ��ł�OK

		for (int i = 0; i < PLAYER_MAX; i++)
		{
			if (!player[i].active) continue;

			// 4. �F���Z�b�g�i�F�ɂ���Ȃ��4�����̃A���t�@��1.0f�ɁI�j
			Shader_SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

				// AABBを描画
				// AABBのMin/Maxは既にワールド座標なので、行列はリセットしたまま描写すればOK
				Debug_DrawAABB(player[i].boundingBox, XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));
			}
		}
	}
}


void Player_DrawHP()
{
	Shader_Begin();

	// ��UI�X�e�[�^�X�`��
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		SetBlendState(BLENDSTATE_ALPHA);

		DrawHP(&HPBar[i], i + 2);
		XMFLOAT2 hp = HPBar[i].pos;

		// スキルゲージ表示用の値を計算する
		float skillFill = 1.0f;

		// スキル未所持なら0
		if (player[i].type == PlayerType::None)
		{
			skillFill = 0.0f;
		}
		else
		{
			// クールタイマーが0なら利用可能
			if (player[i].skillCoolTimer <= 0.0f)
			{
				skillFill = 1.0f;
			}
			else
			{
				// typeに応じたクールタイムを取得
				float coolTime = 0.0f;
				switch (player[i].type)
				{
				case PlayerType::Glass:			coolTime = SKILL_GLASS_COOLTIME; break;
				case PlayerType::Concrete:		coolTime = SKILL_CONCRETE_COOLTIME; break;
				case PlayerType::Plant:			coolTime = SKILL_PLANT_COOLTIME; break;
				case PlayerType::Electricity:	coolTime = SKILL_ELECTRICITY_COOLTIME; break;
				default: coolTime = 0.0f; break;
				}

				// クールタイムが0の時は1.0fを返す
				if (coolTime <= 0.0f)
				{
					skillFill = 1.0f;
				}
				else
				{
					// 使用直後　skillCoolTimer == coolTime => fill = 0.0
					// クール終了　skillCoolTimer == 0 => fill = 1.0
					skillFill = 1.0f - (player[i].skillCoolTimer / coolTime);
					if (skillFill < 0.0f) skillFill = 0.0f;
					if (skillFill > 1.0f) skillFill = 1.0f;
				}
			}
		}

		// 進化が固定されたら、タイプのゲージを最大値で表示する
		if (player[i].isTypeFixed)
		{
			float glass = 0.0f;
			float concrete = 0.0f;
			float plant = 0.0f;
			float electricity = 0.0f;

			switch (player[i].type)
			{
			case PlayerType::Glass:			glass = 1.0f;		break;
			case PlayerType::Concrete:		concrete = 1.0f;	break;
			case PlayerType::Plant:			plant = 1.0f;		break;
			case PlayerType::Electricity:	electricity = 1.0f;	break;
			default: break;
			}

			Gauge_Set(i, glass, concrete, plant, electricity,
				player[i].evolutionGauge, skillFill, { hp.x - GAUGE_POS_X , hp.y + GAUGE_POS_Y }, player[i].type);
		}
		else
		{
			// �Œ�O�̓J�E���g�������̂܂ܕ\������
			Gauge_Set(i, player[i].breakCount_Glass, player[i].breakCount_Concrete, player[i].breakCount_Plant, player[i].breakCount_Electricity,
				player[i].evolutionGauge, skillFill, { hp.x - GAUGE_POS_X , hp.y + GAUGE_POS_Y }, player[i].type);
		}


		if (Player_CanUseSpecial(i))
		{
			Effect_Set(26, { (hp.x + 12.0f * SCREEN_ADJUST_X), hp.y - (100.0f * SCREEN_ADJUST_Y) }, { (162.0f * SCREEN_ADJUST_X), (60.0f * SCREEN_ADJUST_Y) }, i);
		}
		if (!Player_CanUseSpecial(i))
		{
			Effect_Clear(i);
		}


		// 通常ゲージ（内＋外）は常に描画
		// スキルゲージは属性確定のときのみ描画
		Gauge_DrawBasic(i);

		if (player[i].isTypeFixed)
		{
			Gauge_DrawSkill(i);
		}

		Shader_Begin();

		Player_DrawStock(i);
	}
}

void Player_Respawn(int playerIndex)
{
	// �͈̓`�F�b�N 0 1 2 3 �ȊO�Ȃ� return
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	// �c�@��1�ȏ゠��ꍇ
	if (player[playerIndex].active == true)
	{
		player[playerIndex].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[playerIndex].scaling = XMFLOAT3(0.5f, 0.5f, 0.5f);
		player[playerIndex].hp = PLAYER_MAX_HP;
		player[playerIndex].attack = 0.0f;
		player[playerIndex].power = 0.0f;
		player[playerIndex].speed = 0.0f;
		player[playerIndex].defense = 1.0f;
		player[playerIndex].dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[playerIndex].active = true;
		player[playerIndex].satiety = 0.0f;
		player[playerIndex].isAttacking = false;
		player[playerIndex].attackTimer = 0.0f;
		player[playerIndex].isAttacked = false;
		player[playerIndex].attackedTimer = 0.0f;
		player[playerIndex].isHealing = false;
		player[playerIndex].healingTimer = 0.0f;
		player[playerIndex].isEvolving = false;
		player[playerIndex].evolvingTimer = 0.0f;
		player[playerIndex].useSkill = false;
		player[playerIndex].skillTimer = 0.0f;
		player[playerIndex].skillCoolTimer = 0.0f;
		player[playerIndex].useSpecial = false;
		player[playerIndex].specialTimer = 0.0f;
		player[playerIndex].isInvincible = false;
		player[playerIndex].invincibleTimer = 0.0f;
		player[playerIndex].stunGauge = 0.0f;
		player[playerIndex].isStunning = false;
		player[playerIndex].stunTimer = 0.0f;
		player[playerIndex].isDown = false;
		player[playerIndex].downTimer = 0.0f;
		player[playerIndex].isPoisoned = false;
		player[playerIndex].poisonTimer = 0.0f;
		player[playerIndex].duringRespawn = true;
		player[playerIndex].respawnTimer = 0.0f;
		player[playerIndex].isEggBreaking = false;
		player[playerIndex].eggBreakingTimer = 0.0f;
		player[playerIndex].lastDir = PlayerDir::Down; // 正面
		player[playerIndex].isMoving = false;
		player[playerIndex].isShadowEnabled = true;
		player[playerIndex].form = Form::First;
		player[playerIndex].type = PlayerType::None;
		player[playerIndex].evolutionGauge = 0;
		player[playerIndex].evolutionGaugeRate = 0.5f;
		player[playerIndex].breakCount_Glass = 0;
		player[playerIndex].breakCount_Concrete = 0;
		player[playerIndex].breakCount_Plant = 0;
		player[playerIndex].breakCount_Electricity = 0;
		player[playerIndex].brokenHistory.clear();
		player[playerIndex].knockback_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		player[playerIndex].is_knocked_back = false;
		player[playerIndex].knockback_duration = 0.0f;
		player[playerIndex].isTypeFixed = false;
	}

	if (playerIndex == 0) player[0].position = XMFLOAT3(-4.0f, 4.0f, 0.0f);
	if (playerIndex == 1) player[1].position = XMFLOAT3(1.5f, 4.0f, 2.0f);
	if (playerIndex == 2) player[2].position = XMFLOAT3(-4.0f, 4.0f, -3.0f);
	if (playerIndex == 3) player[3].position = XMFLOAT3(4.0f, 4.0f, 1.0f);
}

inline void LoopRange(int& animFrame, int start, int count, int advance)
{
	int relative = (animFrame - start + advance) % count;
	if (relative < 0) relative += count;
	animFrame = start + relative;
}

//==================================
// �c�@�`��
//==================================
void Player_DrawStock(int i)
{
	Shader_Begin();
	Shader_BeginUI();

	// HPバー位置取得・ゲージ座標設定
	float bx = HPBar[i].pos.x - (60.0f * SCREEN_ADJUST_X);
	float by = HPBar[i].pos.y + (60.0f * SCREEN_ADJUST_Y);


	// �v���C���[���Ƃ̃X�g�b�N�`��
	for (int j = 0; j < player[i].stock; j++)
	{
		// ストック描画変数
		XMFLOAT2 pos = { bx + (j * 30.0f * SCREEN_ADJUST_X), by };	// 横並び
		XMFLOAT2 size = { (260.0f * SCREEN_ADJUST_X), (260.0f * SCREEN_ADJUST_Y) };

		g_pContext->PSSetShaderResources(0, 1, &g_Texture[i + 13]);
	
		SetBlendState(BLENDSTATE_ALPHA);
		DrawSprite(pos, size, color::white);
	}
}

void Player_DrawText()
{
	for (int p = 0; p < PLAYER_MAX; ++p)
	{
		if (!player[p].active || !player[p].isOnScreen) continue;

		wchar_t playerLabel[8];
		swprintf_s(playerLabel, L"P%d", p + 1);

		// �v���C���[���ƂɐF�ݒ�
		TextColor textColor;
		switch (p)
		{
		case 0:
			textColor = TextColor::P1color;
			break;
		case 1:
			textColor = TextColor::P2color;
			break;
		case 2:
			textColor = TextColor::P3color;
			break;
		case 3:
			textColor = TextColor::P4color;
			break;
		default:
			textColor = TextColor::White;
			break;
		}

		// �t�H���g�T�C�Y�̔������x���ɂ��炷
		float offsetX = 15.0f;

		DrawTextEx(
			playerLabel,
			player[p].screenPos.x - offsetX,
			player[p].screenPos.y - 10.0f,	// �e�L�X�g�̍�������ɕ\��
			40.0f,							// �t�H���g�T�C�Y
			L"Impact",
			textColor
		);
	}
}

static void Ranking(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;
	// ��d�o�^�h�~
	if (player[playerIndex].rank != 0) return;

	// ���S���ɒǉ�
	g_deathOrder.push_back(playerIndex);
	size_t pos = g_deathOrder.size();

	// ��Ɏ��񂾃v���C���[���Ꮗ�ʂɂȂ�ipos=1 -> 4�ʁj
	player[playerIndex].rank = PLAYER_MAX - (int)(pos - 1);

	// �Ō�̈�l���m�肵����c���1�ʂɂ���
	if (g_deathOrder.size() == (size_t)(PLAYER_MAX - 1))
	{
		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			if (player[p].rank == 0)
			{
				player[p].rank = 1;
				break;
			}
		}
	}
}

PLAYEROBJECT* GetPlayer(int playerIndex)
{
	// �͈̓`�F�b�N 0 1 2 3 �ȊO�Ȃ� nullptr ��Ԃ�
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX)	return nullptr;

	return &player[playerIndex];
}

void TriggerbyHPShake(int playerIndex, float amplitude, float duration, float speed)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	
		SetHPShake(&HPBar[playerIndex], amplitude, duration, speed, playerIndex + 6);
	
}


bool Player_CanUseSpecial(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return false;

	PLAYEROBJECT& pl = player[playerIndex];

	if (!pl.active) return false;
	if (pl.isStunning) return false;
	if (pl.isDown) return false;
	if (pl.rank == 1) return false;

	// 形態が第3形態であること
	if (pl.form != Form::Third) return false;

	// タイプが未設定だとスペシャルがないからタイプもチェック
	if (pl.type == PlayerType::None) return false;

	// すべて通ったらtrue
	return true;
}