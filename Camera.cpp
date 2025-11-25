//======================================================
//	camera.cpp[]
// 
//	制作者：前野翼			日付：2024//
//======================================================
//Camera.cpp


#include	"Camera.h"
#include	"keyboard.h"
#include	"Ball.h"

//グローバル変数
static	CAMERA	CameraObject;

XMFLOAT3		g_BallPosOld;//<<<<<<<<<<<<<<

void	Camera_Initialize()
{ 
	// セッターを使った初期化
	XMFLOAT3 pos = XMFLOAT3(0.0f, 10.0f, -10.0f);
	XMFLOAT3 at = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	SetCameraPosition(pos);
	SetCameraAtPosition(at);
	SetCameraUpVector(up);

	CameraObject.Fov = 45.0f;
	float width = (float)Direct3D_GetBackBufferWidth();
	float height = (float)Direct3D_GetBackBufferHeight();
	CameraObject.Aspect = width / height;
	CameraObject.NearClip = 0.5f;
	CameraObject.FarClip = 1000.0f;

	//g_BallPosOld = GetBallPosition();//<<<<<<<<<<<<<<<<

}

void	Camera_Finalize()
{
	return;
}
void	Camera_Update()
{
	//ボールの座標取得<<<<<<<<<<<<<<<<<<<<<<
	XMFLOAT3	pos = g_BallPosOld;
	g_BallPosOld = GetBallPosition();

	//CameraObject.Position.x += pos.x;
	//CameraObject.Position.z += pos.z;

	////前回のボールと現在のボールの座標の差分<<<<<<<<<<<<<<<
	//pos.x = g_BallPosOld.x - pos.x;
	//pos.y = g_BallPosOld.y - pos.y;
	//pos.z = g_BallPosOld.z - pos.z;

	////注意点としてセット<<<<<<<<<<<<<<<<<<<<<<<
	//CameraObject.AtPosition.x = g_BallPosOld.x;
	//CameraObject.AtPosition.y = g_BallPosOld.y;
	//CameraObject.AtPosition.z = g_BallPosOld.z;


	////注視点を中心にカメラの位置を回転（Y軸回転）
	//float	Rotation = 0.0f;
	//if (Keyboard_IsKeyDown(KK_A))
	//{
	//	Rotation = 1.0f;
	//}
	//if (Keyboard_IsKeyDown(KK_D))
	//{
	//	Rotation = -1.0f;
	//}

	////注視点からカメラへのベクトル
	//XMFLOAT2	vec;
	//vec.x = CameraObject.Position.x - CameraObject.AtPosition.x;
	//vec.y = CameraObject.Position.z - CameraObject.AtPosition.z;
	////ベクトルの回転
	//float	co = cosf(XMConvertToRadians(Rotation));
	//float	si = sinf(XMConvertToRadians(Rotation));
	//CameraObject.Position.x = (vec.x * co - vec.y * si);
	//CameraObject.Position.z = (vec.x * si + vec.y * co);
	//CameraObject.Position.x += CameraObject.AtPosition.x;
	//CameraObject.Position.z += CameraObject.AtPosition.z;

	////vecを正規化する
	//float len = sqrtf(vec.x * vec.x + vec.y * vec.y);
	//vec.x /= len;
	//vec.y /= len;

	////注視点の方向へ移動する
	//float	speed = 0.0f;
	//if (Keyboard_IsKeyDown(KK_W))
	//{
	//	speed = -0.1f;
	//}
	//if (Keyboard_IsKeyDown(KK_S))
	//{
	//	speed = 0.1f;
	//}

	////今回の移動量ベクトル
	//vec.x *= speed;
	//vec.y *= speed;

	////座標と注視点へ移動量を加算
	//CameraObject.Position.x += vec.x;
	//CameraObject.Position.z += vec.y;
	//CameraObject.AtPosition.x += vec.x;
	//CameraObject.AtPosition.z += vec.y;


	//FOVの変更
	if (Keyboard_IsKeyDown(KK_Z))
	{
		CameraObject.Fov += 0.3f;
		if (CameraObject.Fov > 160.0f)
		{
			CameraObject.Fov = 160.0f;
		}

	}
	if (Keyboard_IsKeyDown(KK_X))
	{
		CameraObject.Fov -= 0.3f;
		if (CameraObject.Fov < 5.0f)
		{
			CameraObject.Fov = 5.0f;
		}
	}



	return;
}
void	Camera_Draw()
{ 
	//プロジェクション行列作成
	CameraObject.Projection = XMMatrixPerspectiveFovLH
	(
		XMConvertToRadians(CameraObject.Fov),
		CameraObject.Aspect,
		CameraObject.NearClip,
		CameraObject.FarClip
	);

	//// ★★★ 修正箇所はここ！：平行投影行列を作成する ★★★
	//// プロジェクション行列作成
	//// XMMatrixOrthographicLH(幅, 高さ, NearClip, FarClip) を使用する
	//// ※幅と高さは、画面サイズをそのまま使うのが一般的です
	//float width = (float)Direct3D_GetBackBufferWidth();
	//float height = (float)Direct3D_GetBackBufferHeight();

	//CameraObject.Projection = XMMatrixOrthographicLH
	//(
	//	// 幅と高さ。この値がそのまま描画範囲（スクリーンサイズ）になる
	//	width / CameraObject.Aspect, // 幅 (アスペクト比で補正)
	//	height / CameraObject.Aspect, // 高さ (アスペクト比で補正)
	//	CameraObject.NearClip,
	//	CameraObject.FarClip
	//);

	//ビュー行列作成
	XMVECTOR	vpos = XMVectorSet(
		CameraObject.Position.x,
		CameraObject.Position.y,
		CameraObject.Position.z,
		0.0f);
	XMVECTOR	vAt = XMVectorSet(
		CameraObject.AtPosition.x,
		CameraObject.AtPosition.y,
		CameraObject.AtPosition.z,
		0.0f
	);
	XMVECTOR	vUp = XMVectorSet(
		CameraObject.UpVector.x,
		CameraObject.UpVector.y,
		CameraObject.UpVector.z,
		0.0f
	);
	CameraObject.View = XMMatrixLookAtLH(
		vpos,
		vAt,
		vUp
	);

	return;

}

void	SetCameraFov(float fov)			{ CameraObject.Fov = fov; }
void	SetCameraAspect(float asp)		{ CameraObject.Aspect = asp; }
void	SetCameraClip(float n, float f)	{ CameraObject.NearClip = n; CameraObject.FarClip = f; }

void	SetCameraPosition(XMFLOAT3 pos)	{ CameraObject.Position = pos; }
void	SetCameraAtPosition(XMFLOAT3 at) { CameraObject.AtPosition = at; }
void	SetCameraUpVector(XMFLOAT3 up)	{ CameraObject.UpVector = up; }

XMMATRIX	GetViewMatrix()			{ return	CameraObject.View; }
XMMATRIX	GetProjectionMatrix()	{ return	CameraObject.Projection; }



