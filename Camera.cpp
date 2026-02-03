//======================================================
//	Camera.cpp[]
// 
//	制作者：前野翼			日付：2024//
//======================================================

#include "Camera.h"
#include "keyboard.h"
#include "Ball.h"
#include "Player.h"
#include "input.h"

// グローバル変数
static CAMERA	CameraObject;
static XMFLOAT3	g_BallPosOld;
const float		CAMERA_MOVE_SPEED = 0.2f;
static XMFLOAT3	s_TargetPos;			// 目標カメラ位置
static XMFLOAT3	s_TargetAt;				// 目標注視点位置
static bool		s_IsLerping = false;	// Lerp中かどうか
static float	s_LerpTime = 0.0f;		// Lerpの進捗度 (0.0fから1.0fへ変化)
const float		LERP_SPEED = 0.05f;		// Lerpの速さ (毎フレーム進む割合)

void Camera_Initialize()
{
	// セッターを使った初期化
	XMFLOAT3 pos = XMFLOAT3(0.0f, 10.0f, -10.0f);
	XMFLOAT3 at = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	SetCameraPosition(pos);
	SetCameraAtPosition(at);
	SetCameraUpVector(up);

	CameraObject.fov = 45.0f;
	float width = (float)Direct3D_GetBackBufferWidth();
	float height = (float)Direct3D_GetBackBufferHeight();
	CameraObject.aspect = width / height;
	CameraObject.nearClip = 0.5f;
	CameraObject.farClip = 1000.0f;

	//g_BallPosOld = GetBallPosition();
}

void Camera_Finalize()
{
	return;
}

void Camera_Update()
{
	// カメラ移動方向ベクトル
	XMFLOAT3 vec = {};
	if (Keyboard_IsKeyDown(KK_I))
	{
		vec.z = 1.0f;
	}
	if (Keyboard_IsKeyDown(KK_K))
	{
		vec.z = -1.0f;
	}
	if (Keyboard_IsKeyDown(KK_L))
	{
		vec.x = 1.0f;
	}
	if (Keyboard_IsKeyDown(KK_J))
	{
		vec.x = -1.0f;
	}
	if (Keyboard_IsKeyDown(KK_U))
	{
		vec.y = 1.0f;
	}
	if (Keyboard_IsKeyDown(KK_O))
	{
		vec.y = -1.0f;
	}


	if (g_Input->Up)
	{
		vec.z = 1.0f;
	}
	if (g_Input->Down)
	{
		vec.z = -1.0f;
	}
	if (g_Input->Right)
	{
		vec.x = 1.0f;
	}
	if (g_Input->Left)
	{
		vec.x = -1.0f;
	}


	// vecの正規化
	float len = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	if (len != 0.0f)
	{	// 0除算回避
		vec.x /= len;
		vec.y /= len;
		vec.z /= len;
	}
	
	// 移動量ベクトル
	vec.x *= CAMERA_MOVE_SPEED;
	vec.y *= CAMERA_MOVE_SPEED * 0.1f;
	vec.z *= CAMERA_MOVE_SPEED;

	// 座標と注視点へ移動量を加算
	CameraObject.position.x += vec.x;
	CameraObject.position.y += vec.y;
	CameraObject.position.z += vec.z;
	CameraObject.atPosition.x += vec.x;
	CameraObject.atPosition.y += vec.y;
	CameraObject.atPosition.z += vec.z;

	// fovの変更
	if (Keyboard_IsKeyDown(KK_Z))
	{
		CameraObject.fov += 0.3f;
		if (CameraObject.fov > 160.0f)
		{
			CameraObject.fov = 160.0f;
		}
	}
	if (Keyboard_IsKeyDown(KK_X))
	{
		CameraObject.fov -= 0.3f;
		if (CameraObject.fov < 5.0f)
		{
			CameraObject.fov = 5.0f;
		}
	}

	// ------------------------------------------------------------------
	// 視点切り替え
	// ------------------------------------------------------------------
	// 視点を切り替えるフラグ
	static int s_CurrentViewIndex = 0;

	// 全視点の数
	const int NUM_VIEWS = 3;

	// Cキーが押された瞬間を検出
	if (Keyboard_IsKeyDownTrigger(KK_C))
	{
		// 視点インデックスをインクリメントし、全視点数で割った余りを取る（ループ処理）
		s_CurrentViewIndex = (s_CurrentViewIndex + 1) % NUM_VIEWS;

		// Lerp開始フラグを立て、時間をリセット
		s_IsLerping = true;
		s_LerpTime = 0.0f;

		// カメラ移動による注視点の保存 (注視点(AtPosition)は第1形態プレイヤー位置)
		float current_at_x = CameraObject.atPosition.x;
		float current_at_y = CameraObject.atPosition.y;
		float current_at_z = CameraObject.atPosition.z;

		// --- 目標視点の設定 ---
		switch (s_CurrentViewIndex)
		{
		case 0: // 第1形態視点 (Normal View)
			s_TargetPos = XMFLOAT3(current_at_x, current_at_y + 10.0f, current_at_z - 10.0f);
			s_TargetAt = XMFLOAT3(current_at_x, current_at_y, current_at_z);
			break;

		case 1: // トップダウン視点 (Top Down View)
			// Z座標をわずかにずらすことで、DirectXのView MatrixがZ軸と平行にならないようにする
			s_TargetPos = XMFLOAT3(current_at_x, current_at_y + 10.0f, current_at_z - 0.00001f);
			s_TargetAt = XMFLOAT3(current_at_x, current_at_y, current_at_z);
			break;

		case 2: // 新しい斜め上視点 (High Angle View)
			s_TargetPos = XMFLOAT3(current_at_x, 0.0f, current_at_z - 0.00001f); // XZ平面で斜めに配置、Yを高く
			s_TargetAt = XMFLOAT3(current_at_x, current_at_y, current_at_z); // 注視点は変わらず
			break;
		}
	}
	// ------------------------------------------------------------------

	// ------------------------------------------------------------------
	// Lerpによるカメラ位置の更新処理
	// ------------------------------------------------------------------
	if (s_IsLerping)	// Lerpフラグが立った時
	{
		// 時間を進める
		s_LerpTime += LERP_SPEED;

		// 1.0fを超えないようにクランプする
		if (s_LerpTime >= 1.0f)
		{
			s_LerpTime = 1.0f;
			s_IsLerping = false; // Lerp終了！
		}

		// Lerpを実行
		// 現在のカメラ位置 = 現在の位置 + (目標位置 - 現在の位置) * 進捗度t

		// 視点位置の補間
		XMFLOAT3 currentPos = CameraObject.position;
		XMFLOAT3 nextPos = {
			currentPos.x + (s_TargetPos.x - currentPos.x) * s_LerpTime,
			currentPos.y + (s_TargetPos.y - currentPos.y) * s_LerpTime,
			currentPos.z + (s_TargetPos.z - currentPos.z) * s_LerpTime
		};

		// 注視点位置の補間
		XMFLOAT3 currentAt = CameraObject.atPosition;
		XMFLOAT3 nextAt = {
			currentAt.x + (s_TargetAt.x - currentAt.x) * s_LerpTime,
			currentAt.y + (s_TargetAt.y - currentAt.y) * s_LerpTime,
			currentAt.z + (s_TargetAt.z - currentAt.z) * s_LerpTime
		};

		SetCameraPosition(nextPos);
		SetCameraAtPosition(nextAt);
	}
	// ------------------------------------------------------------------

	// カメラのリセット
	if (Keyboard_IsKeyDown(KK_R))
	{
		XMFLOAT3 pos = XMFLOAT3(0.0f, 10.0f, -10.0f);
		XMFLOAT3 at = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		SetCameraPosition(pos);
		SetCameraAtPosition(at);
		SetCameraUpVector(up);

		CameraObject.fov = 45.0f;
	}

	return;
}

void Camera_Draw()
{ 
	//プロジェクション行列作成
	CameraObject.projection = XMMatrixPerspectiveFovLH
	(
		XMConvertToRadians(CameraObject.fov),
		CameraObject.aspect,
		CameraObject.nearClip,
		CameraObject.farClip
	);

	// ★★★ 平行投影行列を作成する ★★★
	// プロジェクション行列作成
	// XMMatrixOrthographicLH(幅, 高さ, nearClip, farClip) を使用する
	// ※幅と高さは、画面サイズをそのまま使うのが一般的です
	//float width = (float)Direct3D_GetBackBufferWidth();
	//width = 12.80f;
	//float height = (float)Direct3D_GetBackBufferHeight();
	//height = 7.20f;

	//CameraObject.projection = XMMatrixOrthographicLH
	//(
	//	// 幅と高さ。この値がそのまま描画範囲（スクリーンサイズ）になる
	//	width / CameraObject.aspect, // 幅 (アスペクト比で補正)
	//	height / CameraObject.aspect, // 高さ (アスペクト比で補正)
	//	CameraObject.nearClip,
	//	CameraObject.farClip
	//);

	//ビュー行列作成
	XMVECTOR	vpos = XMVectorSet(
		CameraObject.position.x,
		CameraObject.position.y,
		CameraObject.position.z,
		0.0f);
	XMVECTOR	vAt = XMVectorSet(
		CameraObject.atPosition.x,
		CameraObject.atPosition.y,
		CameraObject.atPosition.z,
		0.0f
	);
	XMVECTOR	vUp = XMVectorSet(
		CameraObject.upVector.x,
		CameraObject.upVector.y,
		CameraObject.upVector.z,
		0.0f
	);
	CameraObject.view = XMMatrixLookAtLH(
		vpos,
		vAt,
		vUp
	);

	return;

}

void	SetCameraFov(float fov)			{ CameraObject.fov = fov; }
void	SetCameraAspect(float asp)		{ CameraObject.aspect = asp; }
void	SetCameraClip(float n, float f)	{ CameraObject.nearClip = n; CameraObject.farClip = f; }

void	SetCameraPosition(XMFLOAT3 pos)	{ CameraObject.position = pos; }
void	SetCameraAtPosition(XMFLOAT3 at) { CameraObject.atPosition = at; }
void	SetCameraUpVector(XMFLOAT3 up)	{ CameraObject.upVector = up; }

XMMATRIX	GetViewMatrix()			{ return	CameraObject.view; }
XMMATRIX	GetProjectionMatrix()	{ return	CameraObject.projection; }



