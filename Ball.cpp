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

}
void	BallFinalize()
{
	ModelRelease(g_Ball.Model);

}
void	BallUpdate()
{
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

XMFLOAT3 GetBallPosition()
{
	return g_Ball.Position;
}



