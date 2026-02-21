//======================================================
//	main.cpp[]
// 
//	����ҁF�O�엃			���t�F2024//
//======================================================
//�E�B���h�E�̕\��
#include <SDKDDKVer.h>		// ���p�ł���ł���ʂ� Windows �v���b�g�t�H�[������`�����
#define WIN32_LEAN_AND_MEAN	// 32bit�A�v���ɂ͕s�v�ȏ���}�~���ăR���p�C�����Ԃ�Z�k
#include <windows.h>
#include "debug_ostream.h"	//�f�o�b�O�\��

#include <chrono>
#include <algorithm>
#include "direct3d.h"
#include "shader.h"
#include "polygon.h"
#include "field.h"
#include "sprite.h"
#include "keyboard.h"
#include "Effect.h"
#include "score.h"
#include "Player.h"
#include "Manager.h"
#include "Audio.h"
#include "input.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <SDL3/SDL.h>
#include "gamepad.h"

//#define		SCREEN_WIDTH	(1280)
//#define		SCREEN_HEIGHT	(720)
#define		SCREEN_WIDTH	(1920)
#define		SCREEN_HEIGHT	(1080)

//==================================
//�O���[�o���ϐ�
//==================================
#ifdef _DEBUG	//デバッグビルドの時だけ変数が作られる
int		g_CountFPS;			//FPSカウンター
char	g_DebugStr[2048];	//FPS表示文字列
#endif

#pragma comment(lib, "winmm.lib")

//=================================
//�}�N����`
//=================================
#define	 CLASS_NAME	"DX21 Window"
#define	 WINDOW_CAPTION	"�|���S���`��"

//===================================
//�v���g�^�C�v�錾
//===================================
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//�E�B���h�E�v���V�[�W��
//�R�[���o�b�N�֐��������l���Ăяo���Ă����֐�
LRESULT	CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
{
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device, context);
}

void BeginImGuiFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void EndImGuiFrame()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ShutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmd, int nCmdShow)
{
	// �����̏�����
	srand(timeGetTime());

	// �t���[�����[�g�v���p�ϐ�
	DWORD	dwExecLastTime;
	DWORD	dwFPSLastTime;
	DWORD	dwCurrentTime;
	DWORD	dwFrameCount;

	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	// �E�B���h�E�N���X�̓o�^�i�E�B���h�E�̎d�l�I�ȕ������߂�Windows�փZ�b�g����j
	WNDCLASS	wc;	// �\���̂�����
	ZeroMemory(&wc, sizeof(WNDCLASS));	// ���e���O�ŏ�����
	wc.lpfnWndProc = WndProc;			// �R�[���o�b�N�֐��̃|�C���^�[
	wc.lpszClassName = CLASS_NAME;		// ���̎d�l���̖��O
	wc.hInstance = hInstance;			// ���̃A�v���P�[�V�����̂���
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);	// �J�[�\���̎��
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);	// �E�B���h�E�̔w�i�F�͍�
	RegisterClass(&wc);	// �\���̂�Windows�փZ�b�g

	// �N���C�A���g�̈�̃T�C�Y��\����` (������left, top, right, bottom)
	RECT window_rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	// �t��HD
	//RECT window_rect = { -960, -540, SCREEN_WIDTH, SCREEN_HEIGHT };

	// �E�B���h�E�̃X�^�C��
	DWORD window_style = WS_OVERLAPPEDWINDOW;
	// �w�肵���N���C�A���g�̈���m�ۂ��邽�߂ɐV���ȋ�`���W���v�Z
	AdjustWindowRect(&window_rect, window_style, FALSE);
	// �������ꂽ��`�̉��Əc�̃T�C�Y���v�Z
	int window_width = window_rect.right - window_rect.left;
	int window_height = window_rect.bottom - window_rect.top;

	// �E�B���h�E�̍쐬
	HWND	hWnd = CreateWindow(
		CLASS_NAME,
		WINDOW_CAPTION,
		window_style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		window_width,
		window_height,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	// �쐬�����E�B���h�E��\������
	ShowWindow(hWnd, nCmdShow);	// �����ɏ]���ĕ\���A�܂��͔�\��
	// �E�B���h�E�����̍X�V�v��
	UpdateWindow(hWnd);

	Direct3D_Initialize(hWnd);
	Keyboard_Initialize();
	Shader_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext()); // �V�F�[�_�̏�����
	InitializeSprite();	// �X�v���C�g�̏�����
	InitAudio();		// �T�E���h�̏�����
	InitImGui(hWnd, Direct3D_GetDevice(), Direct3D_GetDeviceContext());

	Gamepad_Initialize();

	Manager_Initialize();

	// ���b�Z�[�W���[�v
	MSG	msg;
	ZeroMemory(&msg, sizeof(MSG));

	// �t���[�����[�g�v��������
	timeBeginPeriod(1);	// �^�C�}�[�̐��x��ݒ�
	dwExecLastTime = dwFPSLastTime = timeGetTime();	// ���݂̃^�C�}�[�l
	dwCurrentTime = dwFrameCount = 0;

	// �I�����b�Z�[�W������܂Ń��[�v����
	do {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{ // �E�B���h�E���b�Z�[�W�����Ă�����
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{ // �Q�[���̏���
			dwCurrentTime = timeGetTime();
			if ((dwCurrentTime - dwFPSLastTime) >= 1000)
			{
#ifdef _DEBUG
				g_CountFPS = dwFrameCount;
#endif
				dwFPSLastTime = dwCurrentTime;	// ���݂̃^�C�}�[�l�ۑ�
				dwFrameCount = 0;
			}

			if ((dwCurrentTime - dwExecLastTime) >= ((float)1000 / 60))
			{// 1/60s �o�߂���
				dwExecLastTime = dwCurrentTime;	// ���݂̃^�C�}�[�ƕۑ�
#ifdef _DEBUG
				// �E�B���h�E�L���v�V�����֌��݂�FPS��\��
				wsprintf(g_DebugStr, "DX21 �v���W�F�N�g ");
				wsprintf(&g_DebugStr[strlen(g_DebugStr)],
					" FPS : %d", g_CountFPS);
				SetWindowText(hWnd, g_DebugStr);
#endif
				// ======= ImGui������ =======
				BeginImGuiFrame();
				Manager_Update();

				// SDLゲームパッド入力更新
				Gamepad_Update();

				// ImGui描画テスト
#ifdef _DEBUG
				int gamepadCount = Gamepad_GetCount();
				if (gamepadCount > 0)
				{
					ImGui::Begin("Gamepad Debug");
					for (int p = 0; p < gamepadCount; p++)
					{
						
						ImGui::Text("=== Gamepad %d ===", p);
						ImGui::Text("Name: %s", Gamepad_GetName(p));
						ImGui::Text("LStick X : %.3f", g_Input[p].LStickX);
						ImGui::Text("LStick Y : %.3f", g_Input[p].LStickY);
						ImGui::Text("A Button : %d", g_Input[p].A);
						ImGui::Text("B Button : %d", g_Input[p].B);
						ImGui::Text("X Button : %d", g_Input[p].X);
						ImGui::Text("Y Button : %d", g_Input[p].Y);
						ImGui::Text("D-Pad: U:%d D:%d L:%d R:%d",
							g_Input[p].Up, g_Input[p].Down,
							g_Input[p].Left, g_Input[p].Right);
						ImGui::Separator();
						
					}
					ImGui::End();
				}
#endif
				// 描画処理
				Direct3D_Clear();	// バックバッファをクリア
				Manager_Draw();
				
				EndImGuiFrame();	// ImGui ��`��
				
				Direct3D_Present();
				keycopy();
				
				dwFrameCount++;		//�����񐔍X�V
			}
		}
	} while (msg.message != WM_QUIT);
	
	Manager_Finalize();
	UninitAudio();		// �T�E���h�̏I��
	Shader_Finalize();	// �V�F�[�_�̏I������

	Gamepad_Finalize();

	ShutdownImGui();
	Direct3D_Finalize();

	//�I������
	return (int)msg.wParam;
}

//=========================================
//�E�B���h�E�v���V�[�W��
// ���b�Z�[�W���[�v���ŌĂяo�����
//=========================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//HGDIOBJ hbrWhite, hbrGray;
	//HDC		hdc;	//�E�B���h�E��ʂ�\�����i�f�o�C�X�R���e�L�X�g ���o�͐�j
	//PAINTSTRUCT	ps;	//�E�B���h�E��ʂ̑傫���ȂǕ`��֘A�̏��

	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_ACTIVATEAPP:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard_ProcessMessage(uMsg, wParam, lParam);
		break;

	case WM_KEYDOWN:	// �L�[�������ꂽ
		if (wParam == VK_ESCAPE)	// �����ꂽ�̂�ESC�L�[
		{
			// �E�B���h�E����������N�G�X�g��Windows�ɑ���
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}

		Keyboard_ProcessMessage(uMsg, wParam, lParam);
		break;
	case WM_CLOSE:	// �E�B���h�E����Ȃ�������				
		//if (
		//	MessageBox(hWnd, "�{���ɏI�����Ă�낵���ł����H",
		//		"�m�F", MB_OKCANCEL | MB_DEFBUTTON2) == IDOK
		//	)
		//{//OK�������ꂽ�Ƃ�
		//	DestroyWindow(hWnd);//�I������葱����Windows�փ��N�G�X�g
		//}
		//else
		//{
		//	return 0;	//����ς�I���Ȃ�
		//}

		//break;
	case WM_DESTROY:			// �I������OK�ł���
		PostQuitMessage(0);		// �����̃��b�Z�[�W��0�𑗂�
		break;
	}
	// �K�v�̖������b�Z�[�W�͓K���ɏ��������ďI��
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

