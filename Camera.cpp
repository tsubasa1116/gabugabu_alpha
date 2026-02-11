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
#include "debug_ostream.h"

// グローバル変数
static CAMERA	CameraObject;
const float		CAMERA_MOVE_SPEED = 0.2f; // カメラ移動速度
static XMFLOAT3	s_TargetPos;			  // 目標カメラ位置
static XMFLOAT3	s_TargetAt;				  // 目標注視点位置
static float	s_TargetFov = 45.0f;	  // 目標fov（平行投影幅としても使用）
static bool		s_IsLerping = false;	  // 目標と現在が十分に離れているか
const float		SMOOTH_FACTOR = 0.15f;	  // 1フレームあたりの進行率で、大きいほど速く追従する
const float		FOV_SMOOTH_FACTOR = 0.12f;// FOVの追従速度
const float		TARGET_EPSILON = 0.001f;  // 目標到達判定の閾値

CAMERAMODE cameraMode = CAMERAMODE_MANUAL;

static inline XMFLOAT3 LerpFloat3(const XMFLOAT3& a, const XMFLOAT3& b, float t)
{
	return XMFLOAT3(
		a.x + (b.x - a.x) * t,
		a.y + (b.y - a.y) * t,
		a.z + (b.z - a.z) * t
	);
}

static inline float LerpFloat(float a, float b, float t)
{
	return a + (b - a) * t;
}

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

	// 初期の目標値を現在値に合わせる
	s_TargetPos = CameraObject.position;
	s_TargetAt = CameraObject.atPosition;
	s_TargetFov = CameraObject.fov;

}

void Camera_Finalize()
{
	return;
}

float theta = {};
float lenght = 20.0f;
float kakudoz = {};
float kakudoy = {};
float kakudox = {};

void Camera_Update()
{
	if (Keyboard_IsKeyDown(KK_N))
		theta += 1;
	if (Keyboard_IsKeyDown(KK_B))
		theta -= 1;

	if (theta >= 89)
		theta = 89;
	if (theta <= 1)
		theta = 1;

	float posz = cosf(XMConvertToRadians(theta));
	float posy = sinf(XMConvertToRadians(theta));
	float posx = tanf(XMConvertToRadians(theta));

	kakudoz = posz * lenght;
	kakudoy = posy * lenght;
	kakudox = posx * lenght;

	SetCameraPosition(XMFLOAT3(0.0f, kakudoy, kakudoz));

	hal::dout << theta << std::endl;

	if(cameraMode == CAMERAMODE_MANUAL)
	{
		// カメラ移動方向ベクトル（目標値へ加算する）
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
		
		// 移動量ベクトル（目標値へ加算）
		vec.x *= CAMERA_MOVE_SPEED;
		vec.y *= CAMERA_MOVE_SPEED * 0.1f;
		vec.z *= CAMERA_MOVE_SPEED;
	
		// 直接現在値に加算せず目標値に加算する（滑らかに追従するため）
		s_TargetPos.x += vec.x;
		s_TargetPos.y += vec.y;
		s_TargetPos.z += vec.z;

		s_TargetAt.x += vec.x;
		s_TargetAt.y += vec.y;
		s_TargetAt.z += vec.z;
	
		// fovの変更（目標FOVを変更）
		if (Keyboard_IsKeyDown(KK_Z))
		{
			s_TargetFov += 0.3f;
			if (s_TargetFov > 160.0f)
			{
				s_TargetFov = 160.0f;
			}
		}
		if (Keyboard_IsKeyDown(KK_X))
		{
			s_TargetFov -= 0.3f;
			if (s_TargetFov < 5.0f)
			{
				s_TargetFov = 5.0f;
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
	
			// Lerp開始フラグを立てる
			s_IsLerping = true;
	
			// カメラ移動による注視点の保存 (注視点は第1形態プレイヤー位置)
			float current_at_x = CameraObject.atPosition.x;
			float current_at_y = CameraObject.atPosition.y;
			float current_at_z = CameraObject.atPosition.z;
	
			// 目標視点の設定
			switch (s_CurrentViewIndex)
			{
			case 0: // 第1形態視点
				s_TargetPos = XMFLOAT3(current_at_x, current_at_y + 10.0f, current_at_z - 10.0f);
				s_TargetAt = XMFLOAT3(current_at_x, current_at_y, current_at_z);
				break;
	
			case 1: // トップダウン視点
				// Z座標をずらして、DirectXのViewMatrixがZ軸と平行にならないようにする
				s_TargetPos = XMFLOAT3(current_at_x, current_at_y + 10.0f, current_at_z - 0.00001f);
				s_TargetAt = XMFLOAT3(current_at_x, current_at_y, current_at_z);
				break;
	
			case 2: // 新しい斜め上視点 
				s_TargetPos = XMFLOAT3(current_at_x, 0.0f, current_at_z - 0.00001f); // XZ平面で斜めに配置、Yを高く
				s_TargetAt = XMFLOAT3(current_at_x, current_at_y, current_at_z); // 注視点は変わらず
				break;
			}
		}

		// カメラのリセット（目標にリセット値を与える）
		if (Keyboard_IsKeyDown(KK_R))
		{
			XMFLOAT3 pos = XMFLOAT3(0.0f, 10.0f, -10.0f);
			XMFLOAT3 at = XMFLOAT3(0.0f, 0.0f, 0.0f);
			XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
			// 目標値をリセット
			s_TargetPos = pos;
			s_TargetAt = at;
			SetCameraUpVector(up);

			s_TargetFov = 45.0f;
		}

		if (Keyboard_IsKeyDown(KK_M))
		{
			cameraMode = CAMERAMODE_AUTO;
		}
	}

	else if(cameraMode == CAMERAMODE_AUTO)
	{
		Camera_UpdateAuto();
	}

	// 毎フレームの滑らか補間（目標値へ追従する）
	// 位置と注視点の補間
	CameraObject.position = LerpFloat3(CameraObject.position, s_TargetPos, SMOOTH_FACTOR);
	CameraObject.atPosition = LerpFloat3(CameraObject.atPosition, s_TargetAt, SMOOTH_FACTOR);

	// fovの補間
	CameraObject.fov = LerpFloat(CameraObject.fov, s_TargetFov, FOV_SMOOTH_FACTOR);

	// 到達判定（目標とほぼ同じなら完全に一致させる）
	auto closeEnough = [](const XMFLOAT3& a, const XMFLOAT3& b)->bool
	{
		float dx = a.x - b.x;
		float dy = a.y - b.y;
		float dz = a.z - b.z;
		return (dx*dx + dy*dy + dz*dz) <= (TARGET_EPSILON * TARGET_EPSILON);
	};

	if (closeEnough(CameraObject.position, s_TargetPos) && closeEnough(CameraObject.atPosition, s_TargetAt)
		&& fabsf(CameraObject.fov - s_TargetFov) <= TARGET_EPSILON)
	{
		// 完全に一致させてフラグ解除
		CameraObject.position = s_TargetPos;
		CameraObject.atPosition = s_TargetAt;
		CameraObject.fov = s_TargetFov;
		s_IsLerping = false;
	}
	else
	{
		// いずれかが未到達ならフラグオン
		s_IsLerping = true;
	}

	return;
}

void Camera_UpdateAuto()
{
	XMFLOAT3 center = { 0.0f, 0.0f, 0.0f };
	float minX = FLT_MAX, maxX = -FLT_MAX;
	float minZ = FLT_MAX, maxZ = -FLT_MAX;

	int playerCount = 0;
	for (int i = 0; i < 4; i++) {
		PLAYEROBJECT* player = GetPlayer(i);
		if (!player) continue;

		center.x += player->position.x;
		center.z += player->position.z;

		if (player->position.x < minX) minX = player->position.x;
		if (player->position.x > maxX) maxX = player->position.x;
		if (player->position.z < minZ) minZ = player->position.z;
		if (player->position.z > maxZ) maxZ = player->position.z;
		playerCount++;
	}

	if (playerCount > 0) {
		center.x /= (float)playerCount;
		center.z /= (float)playerCount;
	}

	// 注視点は中心（目標に設定）
	s_TargetAt = center;
	// カメラの目標位置を調整して、平行投影の立体感を表現する（即時代入しない）
	s_TargetPos = XMFLOAT3(center.x + 2.0f, center.y + 10.0f, center.z - 10.0f);

	// 平行投影用の表示範囲計算
	float spreadX = maxX - minX;
	float spreadZ = maxZ - minZ;

	// プレイヤー間の最大距離（幅）
	float maxSpread = (spreadX > spreadZ) ? spreadX : spreadZ;

	// マージンを足す
	// 平行投影での表示幅を計算し、この値が画面の横方向のワールド単位での幅になる
	float margin = 10.0f;
	float targetWidth = maxSpread + margin;

	// ズームの最小値
	if (targetWidth < 12.0f) targetWidth = 12.0f;

	// fovを平行投影の幅として利用（目標FOVに設定）
	s_TargetFov = targetWidth;
}


void Camera_Draw()
{
	// 平行投影行列を作成する
	// fovを画面の横幅として扱う
	float viewWidth = CameraObject.fov;
	float viewHeight = viewWidth / CameraObject.aspect;

	CameraObject.projection = XMMatrixOrthographicLH(
		viewWidth,    // 投影する範囲の幅
		viewHeight,   // 投影する範囲の高さ
		CameraObject.nearClip,
		CameraObject.farClip
	);

	// ビュー行列作成
	XMVECTOR vpos = XMVectorSet(CameraObject.position.x, CameraObject.position.y, CameraObject.position.z, 0.0f);
	XMVECTOR vAt = XMVectorSet(CameraObject.atPosition.x, CameraObject.atPosition.y, CameraObject.atPosition.z, 0.0f);
	XMVECTOR vUp = XMVectorSet(CameraObject.upVector.x, CameraObject.upVector.y, CameraObject.upVector.z, 0.0f);

	CameraObject.view = XMMatrixLookAtLH(vpos, vAt, vUp);
}

void SetCameraFov(float fov)         { CameraObject.fov = fov; s_TargetFov = fov; }
void SetCameraAspect(float asp)      { CameraObject.aspect = asp; }
void SetCameraClip(float n, float f) { CameraObject.nearClip = n; CameraObject.farClip = f; }

void SetCameraPosition(XMFLOAT3 pos)  { CameraObject.position = pos; s_TargetPos = pos; }
void SetCameraAtPosition(XMFLOAT3 at) { CameraObject.atPosition = at; s_TargetAt = at; }
void SetCameraUpVector(XMFLOAT3 up)	  { CameraObject.upVector = up; }

XMMATRIX GetViewMatrix()        { return	CameraObject.view; }
XMMATRIX GetProjectionMatrix()	{ return	CameraObject.projection; }



