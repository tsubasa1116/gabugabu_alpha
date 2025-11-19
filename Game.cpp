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
#include	"Block.h"
#include	"field.h"
#include	"Effect.h"
#include	"score.h"
#include	"Audio.h"

#include	"Polygon3D.h"
#include	"Camera.h"

#include "Ball.h"

#include	"direct3d.h"//<<<<<<<<<<<<<<<<<<<

//======================================================
//	構造謡宣言
//======================================================
LIGHTOBJECT		Light;//<<<<<<ライト管理オブジェクト

//======================================================
//	グローバル変数
//======================================================
static	int		g_BgmID = NULL;	//サウンド管理ID

//======================================================
//	初期化関数
//======================================================
void Game_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{

	Field_Initialize(pDevice, pContext); // フィールドの初期化
	BallInitialize(pDevice, pContext); // ボールの初期化

	//Player_Initialize(pDevice, pContext); // ポリゴンの初期化
	//Block_Initialize(pDevice, pContext);//ブロックの初期化
	//Effect_Initialize(pDevice, pContext);//エフェクト初期化
	//Score_Initialize(pDevice, pContext);//スコア初期化

	Polygon3D_Initialize(pDevice, pContext);//３Dテスト初期化

	Camera_Initialize();	//カメラ初期化



	g_BgmID = LoadAudio("asset\\Audio\\bgm.wav");	//サウンドロード
	//PlayAudio(g_BgmID, true);	//再生開始（ループあり）
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

}

//======================================================
//	終了処理関数
//======================================================
void Game_Finalize()
{
	Field_Finalize();
	BallFinalize();
	//Block_Finalize();
	//Player_Finalize();	// ポリゴンの終了処理
	//Effect_Finalize();
	//Score_Finalize();
	Polygon3D_Finalize();
	Camera_Finalize();

	UnloadAudio(g_BgmID);//サウンドの解放
}

//======================================================
//	更新処理
//======================================================
void Game_Update()
{
	//更新処理
	Camera_Update();
	BallUpdate();
	Field_Update();
	//Player_Update();
	//Block_Update();
	//Effect_Update();
	//Score_Update();
	Polygon3D_Update();

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
	Field_Draw();
	BallDraw();
	Polygon3D_Draw();

	//2D描画
	Light.SetEnable(FALSE);			//ライティングOFF
	Shader_SetLight(Light.Light);	//ライト構造体をシェーダーへセット
	SetDepthTest(FALSE);



	//Block_Draw();
	//Player_Draw();
	//Effect_Draw();
	//Score_Draw();



}

