//======================================================
//	game.cpp[]
// 
//	制作者：前野翼			日付：2024//
//======================================================
//Game.cpp

#include	"Manager.h"
#include	"sprite.h"
#include	"Game.h"
#include	"keyboard.h"

#include	"player.h"
#include    "p.h"
#include	"Block.h"
#include	"field.h"
#include	"building.h"
#include	"Effect.h"
#include	"score.h"
#include	"Audio.h"
#include    "gauge.h"

#include	"Polygon3D.h"
#include	"Camera.h"
#include	"fade.h"

#include "Ball.h"
#include "skill.h"

#include "skill.h"



#include	"direct3d.h"//<<<<<<<<<<<<<<<<<<<



//======================================================
//	構造謡宣言
//======================================================
LIGHTOBJECT		Light;//<<<<<<ライト管理オブジェクト

//======================================================
//	グローバル変数
//======================================================
static	int		g_BgmID = NULL;	//サウンド管理ID
bool input2 = false;

const int KONAMI_CODE[] = {
	KK_UP, KK_UP, KK_DOWN, KK_DOWN,
	KK_LEFT, KK_RIGHT, KK_LEFT, KK_RIGHT,
	KK_B, KK_A
};

// コマンドの長さ
const int KONAMI_CODE_LENGTH = sizeof(KONAMI_CODE) / sizeof(KONAMI_CODE[0]);

// 現在、コマンド入力のどこまで進んでいるかを追跡するインデックス
static int s_KonamiCodeIndex = 0;

// コマンドが入力されたときに立つフラグ
static bool s_IsKonamiCodeEntered = false;

// 押されたキーが期待されているキーと一致しているかの確認をする
void CheckKonamiCode(int currentKeyCode)
{
	// 現在期待されているキーが押されたか？
	if (currentKeyCode == KONAMI_CODE[s_KonamiCodeIndex])
	{
		// 期待通りの入力だったので、インデックスを進める
		s_KonamiCodeIndex++;

		// コマンドの最後まで到達したか？
		if (s_KonamiCodeIndex >= KONAMI_CODE_LENGTH)
		{
			// コマンド入力完了！フラグを立てる
			s_IsKonamiCodeEntered = !s_IsKonamiCodeEntered;

			// コマンドは完了したので、インデックスをリセットするか、-1などの完了状態にする
			s_KonamiCodeIndex = 0; // または s_KonamiCodeIndex = -1;
		}
	}
	else
	{
		// 期待されていないキーが押された場合、シーケンスは失敗。最初からやり直し
		s_KonamiCodeIndex = 0;

		// ただし、失敗したキーがコマンドの最初のキーである場合、
		// 最初のキーからやり直す可能性を考慮するなら、以下のように再チェックしても良い
		if (currentKeyCode == KONAMI_CODE[0])
		{
			s_KonamiCodeIndex = 1;
		}
	}
}

//======================================================
//	初期化関数
//======================================================
void Game_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	Field_Initialize(pDevice, pContext); // フィールドの初期化
	//BallInitialize(pDevice, pContext); // ボールの初期化

	P_Initialize(pDevice, pContext); // プレイヤーの初期化
	//Player_Initialize(pDevice, pContext); // ポリゴンの初期化
	//Block_Initialize(pDevice, pContext);//ブロックの初期化
	//Effect_Initialize(pDevice, pContext);//エフェクト初期化
	//Score_Initialize(pDevice, pContext);//スコア初期化
	Skill_Initialize(pDevice, pContext);

	Polygon3D_Initialize(pDevice, pContext);//３Dテスト初期化

	Camera_Initialize();	//カメラ初期化

	g_BgmID = LoadAudio("asset\\Audio\\bgm.wav");	//サウンドロード
	//PlayAudio(g_BgmID, true);		//再生開始（ループあり）
	//PlayAudio(g_BgmID);			//再生開始（ループなし）
	//PlayAudio(g_BgmID, false);	//再生開始（ループなし）

	//ライト初期化
	XMFLOAT4	para;

	para = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);//環境光の色
	Light.SetAmbient(para);

	para = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);//光の色
	Light.SetDiffuse(para);

	para = XMFLOAT4(0.5f, -1.0f, 0.0f, 1.0f);//光方向
	float	len = sqrtf(para.x * para.x + para.y * para.y + para.z * para.z);
	para.x /= len;
	para.y /= len;
	para.z /= len;
	Light.SetDirection(para);//光の方向（正規化済）

	//�Q�[�����U���g�֑J��
	if (Keyboard_IsKeyDown(KK_F1))
	{
		XMFLOAT4	color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		SetFade(60.0f, color, FADE_IN, SCENE_RESULT);
	}

}

//======================================================
//	終了処理関数
//======================================================
void Game_Finalize()
{
	Field_Finalize();
	//BallFinalize();
	P_Finalize();
	//Block_Finalize();
	//Player_Finalize();	// ポリゴンの終了処理
	//Effect_Finalize();
	//Score_Finalize();
	Polygon3D_Finalize();
	Camera_Finalize();
	Skill_Finalize();

	UnloadAudio(g_BgmID);//サウンドの解放
}

//======================================================
//	更新処理
//======================================================
void Game_Update()
{
	// ------------------------------------
	//  コナミコマンド検出
	// ------------------------------------
	// コマンドで使用する全てのキーの押下トリガーをチェックし、検出関数に渡す
		 if (Keyboard_IsKeyDownTrigger(KK_UP))		CheckKonamiCode(KK_UP);
	else if (Keyboard_IsKeyDownTrigger(KK_DOWN))	CheckKonamiCode(KK_DOWN);
	else if (Keyboard_IsKeyDownTrigger(KK_LEFT))	CheckKonamiCode(KK_LEFT);
	else if (Keyboard_IsKeyDownTrigger(KK_RIGHT))	CheckKonamiCode(KK_RIGHT);
	else if (Keyboard_IsKeyDownTrigger(KK_B))		CheckKonamiCode(KK_B);
	else if (Keyboard_IsKeyDownTrigger(KK_A))		CheckKonamiCode(KK_A);
	// ------------------------------------
	//更新処理
	Camera_Update();
	//BallUpdate();
	Field_Update();
	P_Update();
	//Player_Update();
	//Block_Update();
	//Effect_Update();
	//Score_Update();
	Polygon3D_Update();
	Gauge_Update();


	//�Q�[���V�[���֑J��
	if (Keyboard_IsKeyDownTrigger(KK_F1) && (GetFadeState() == FADE_NONE))
	{
		//�t�F�[�h�A�E�g�����ăV�[����؂�ւ���
		XMFLOAT4	color(0.0f, 0.0f, 0.0f, 1.0f);
		SetFade(40.0f, color, FADE_OUT, SCENE_RESULT);
	}
}

//======================================================
//	描画関数
//======================================================
void Game_Draw()
{ 
	Light.SetEnable(TRUE);			//ライティングON
	Shader_SetLight(Light.Light);	//ライト構造体をシェーダーへセット
	SetDepthTest(TRUE);

	Camera_Draw();		//Drawの最初で呼ぶ！
	Field_Draw(s_IsKonamiCodeEntered);
	//BallDraw();
	Polygon3D_Draw(s_IsKonamiCodeEntered);
	

	//2D描画
	Light.SetEnable(FALSE);			//ライティングOFF
	Shader_SetLight(Light.Light);	//ライト構造体をシェーダーへセット
	SetDepthTest(FALSE);


	P_Draw();
	//Block_Draw();
	//Player_Draw();
	//Effect_Draw();
	//Score_Draw();



}

