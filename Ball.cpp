//======================================================
//	ball.cpp[]
// 
//	制作者：前野翼			日付：2024//
//======================================================
//Ball.cpp

#include	"keyboard.h"
#include	"Ball.h"
#include	"Camera.h"
#include	"shader.h"
#include	"color.h"
#include    "hp.h"

static HP b_HPBar;

// ボールオブジェクト
BALL	g_Ball;

ID3D11Device* g_pDevice;
ID3D11DeviceContext* g_pContext;


void	BallInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	g_Ball.Model = ModelLoad("asset\\model\\ball.fbx");

	g_Ball.Position = XMFLOAT3(0.0f, 0.5f, 0.0f);
	g_Ball.Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Ball.Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	g_Ball.Scaling = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Ball.State = BALL_STATE::BALL_STATE_MOVE;

	g_Ball.HP = 10; //仮
	g_Ball.maxHP = 10; //仮

	InitializeHP(pDevice, pContext, &b_HPBar, { 250.0f, 500.0f }, { 400.0f, 20.0f }, color::red, color::green);


}
void	BallFinalize()
{
	ModelRelease(g_Ball.Model);

}
void	BallUpdate()
{
	// HPバー
	SetHPValue(&b_HPBar, (int)g_Ball.HP, (int)g_Ball.maxHP);
	UpdateHP(&b_HPBar);

	switch (g_Ball.State)
	{
	case BALL_STATE::BALL_STATE_IDLE:
		break;
	case BALL_STATE::BALL_STATE_MOVE:
		break;
	case BALL_STATE::BALL_STATE_DIRECTION:
		break;
	case BALL_STATE::BALL_STATE_POWER:
		break;
	}

	//デバッグ
	g_Ball.Velocity = XMFLOAT3(0, 0, 0);
	if (Keyboard_IsKeyDown(KK_UP))
	{
		g_Ball.Velocity.z = 1.0f / 60.0f;
	}
	if (Keyboard_IsKeyDown(KK_DOWN))
	{
		g_Ball.Velocity.z = -1.0f / 60.0f;
	}
	if (Keyboard_IsKeyDown(KK_LEFT))
	{
		g_Ball.Velocity.x = -1.0f / 60.0f;
	}
	if (Keyboard_IsKeyDown(KK_RIGHT))
	{
		g_Ball.Velocity.x = 1.0f / 60.0f;
	}
	g_Ball.Position.x += g_Ball.Velocity.x;
	g_Ball.Position.y += g_Ball.Velocity.y;
	g_Ball.Position.z += g_Ball.Velocity.z;


}
void	BallDraw() 
{

	//ワールド行列作成
	XMMATRIX	scale = XMMatrixScaling(
		g_Ball.Scaling.x,
		g_Ball.Scaling.y,
		g_Ball.Scaling.z);
	XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
		g_Ball.Rotation.x,
		g_Ball.Rotation.y,
		g_Ball.Rotation.z);
	XMMATRIX	translation = XMMatrixTranslation(
		g_Ball.Position.x,
		g_Ball.Position.y,
		g_Ball.Position.z);
	XMMATRIX	world = scale * rotation * translation;

	//変換行列作成
	XMMATRIX	view = GetViewMatrix();
	XMMATRIX	projection = GetProjectionMatrix();
	XMMATRIX	wvp = world * view * projection;

	//シェーダーへ行列をセット
	Shader_SetWorldMatrix(world);
	Shader_SetMatrix(wvp);

	//モデルの描画リクエスト
	ModelDraw(g_Ball.Model);
}

void BallDrawHP()
{

	Shader_Begin();


	DrawHP(&b_HPBar);
}

XMFLOAT3 GetBallPosition()
{
	return g_Ball.Position;
}



