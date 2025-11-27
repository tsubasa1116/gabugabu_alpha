#pragma once

//Ball.h

#include	<d3d11.h>
#include	<DirectXMath.h>
#include	"direct3d.h"
using namespace DirectX;

#include	"model.h"

void	BallInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void	BallFinalize();
void	BallUpdate();
void	BallDraw();
void BallDrawHP();

XMFLOAT3 GetBallPosition();


//ボールの状態
enum BALL_STATE
{
	BALL_STATE_IDLE = 0,	//何もしない
	BALL_STATE_MOVE,		//移動
	BALL_STATE_DIRECTION,	//方向指示
	BALL_STATE_POWER,		//威力指示
};

//ボール構造体
class BALL
{
	public:
		XMFLOAT3	Position;	//表示座標
		XMFLOAT3	Rotation;	//回転角
		XMFLOAT3	Scaling;	//拡大率
		XMFLOAT3	Velocity;	//速度

		BALL_STATE	State;		//状態
		MODEL*		Model;		//モデルデータ]

		// (仮)
		int HP;
		int maxHP;
};







