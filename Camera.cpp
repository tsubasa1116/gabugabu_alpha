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
#include "imgui.h"

// グローバル変数
static CAMERA   CameraObject;
const float     CAMERA_MOVE_SPEED = 0.2f; // カメラ移動速度
static XMFLOAT3 s_TargetPos;              // 目標カメラ位置
static XMFLOAT3 s_TargetAt;               // 目標注視点位置
static float    s_TargetFov = 45.0f;      // 目標fov
static bool     s_IsLerping = false;      // 目標と現在が十分に離れているか
const float     SMOOTH_FACTOR = 0.5f;     // 1フレームあたりの進行率で、大きいほど速く追従する
const float     FOV_SMOOTH_FACTOR = 0.15f;// fovの追従速度
const float     TARGET_EPSILON = 0.001f;  // 目標到達判定の閾値(しきいち)

// カメラシェイク用
static bool     s_IsShaking = false;      // シェイク中かどうか
static float    s_ShakeIntensity = 0.0f;  // シェイクの強度
static float    s_ShakeDuration = 0.0f;   // シェイクの残り時間
static float    s_ShakeTimer = 0.0f;      // シェイクの経過時間
static XMFLOAT3 s_ShakeOffset;            // シェイクによるオフセット

static bool     s_IsDeathFocus = false;       // 死亡フォーカス中かどうか
static int      s_FocusPlayerIndex = -1;      // フォーカス対象のインデックス
static float    s_DeathFocusDuration = 0.0f;  // フォーカス持続時間
static float    s_DeathFocusTimer = 0.0f;     // フォーカス経過時間
static XMFLOAT3 s_BeforeFocusPos;             // フォーカス前のカメラ位置
static XMFLOAT3 s_BeforeFocusAt;              // フォーカス前の注視点
static float    s_BeforeFocusFov;             // フォーカス前のfov
static CAMERAMODE s_BeforeFocusMode;          // フォーカス前のカメラモード

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

// ランダムな値を-1.0f～1.0fの範囲で生成する
static inline float RandomFloat()
{
	return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
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

	// シェイク
	s_IsShaking = false;
	s_ShakeIntensity = 0.0f;
	s_ShakeDuration = 0.0f;
	s_ShakeTimer = 0.0f;
	s_ShakeOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);

	cameraMode = CAMERAMODE_MANUAL;
}

void Camera_Finalize()
{
	return;
}

void Camera_Update()
{
	if(cameraMode == CAMERAMODE_MANUAL)
	{
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
		{	// 0で割らないようにする
			vec.x /= len;
			vec.y /= len;
			vec.z /= len;
		}
		
		// 移動量ベクトル（目標値へ加算）
		vec.x *= CAMERA_MOVE_SPEED;
		vec.y *= CAMERA_MOVE_SPEED * 0.1f;
		vec.z *= CAMERA_MOVE_SPEED;
	
		// 現在の値に加算せず、目標値に加算する（滑らかに追従するため）
		s_TargetPos.x += vec.x;
		s_TargetPos.y += vec.y;
		s_TargetPos.z += vec.z;

		s_TargetAt.x += vec.x;
		s_TargetAt.y += vec.y;
		s_TargetAt.z += vec.z;
	

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
	
			// 注視点を保存 (注視点は第1形態のプレイヤー位置)
			float current_at_x = CameraObject.atPosition.x;
			float current_at_y = CameraObject.atPosition.y;
			float current_at_z = CameraObject.atPosition.z;
	
			// 目標視点の設定
			switch (s_CurrentViewIndex)
			{
			case 0: // 第1形態視点
				s_TargetPos = XMFLOAT3(current_at_x, current_at_y + 10.0f, current_at_z - 10.0f);
				s_TargetAt  = XMFLOAT3(current_at_x, current_at_y, current_at_z);
				break;
	
			case 1: // トップダウン視点
				s_TargetPos = XMFLOAT3(current_at_x, current_at_y + 10.0f, current_at_z - 0.001f);
				s_TargetAt  = XMFLOAT3(current_at_x, current_at_y, current_at_z);
				break;
	
			case 2: // 斜め上視点 
				s_TargetPos = XMFLOAT3(current_at_x, 0.0f, current_at_z - 0.001f);
				s_TargetAt  = XMFLOAT3(current_at_x, current_at_y, current_at_z);
				break;
			}
		}

		if (Keyboard_IsKeyDown(KK_R))
		{
			XMFLOAT3 pos = XMFLOAT3(0.0f, 10.0f, -10.0f);
			XMFLOAT3 at  = XMFLOAT3(0.0f, 0.0f, 0.0f);
			XMFLOAT3 up  = XMFLOAT3(0.0f, 1.0f, 0.0f);
			// 目標値をリセット
			s_TargetPos = pos;
			s_TargetAt  = at;
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


		if (Keyboard_IsKeyDown(KK_M))
		{
			Camera_Initialize();
		}
	}

	// フレームの補間
	// 死亡フォーカス中の補間速度
	float lerpFactor = s_IsDeathFocus ? 0.25f : SMOOTH_FACTOR;
	float fovLerpFactor = s_IsDeathFocus ? 0.15f : FOV_SMOOTH_FACTOR;

	// 位置と注視点の補間
	CameraObject.position = LerpFloat3(CameraObject.position, s_TargetPos, lerpFactor);
	CameraObject.atPosition = LerpFloat3(CameraObject.atPosition, s_TargetAt, lerpFactor);

	// fovの補間
	CameraObject.fov = LerpFloat(CameraObject.fov, s_TargetFov, fovLerpFactor);

	// 目標とほぼ同じなら完全に一致させる判定
	auto nearCheck = [](const XMFLOAT3& a, const XMFLOAT3& b)->bool
		{
			float dx = a.x - b.x;
			float dy = a.y - b.y;
			float dz = a.z - b.z;
			return (dx * dx + dy * dy + dz * dz) <= (TARGET_EPSILON * TARGET_EPSILON);
		};

	if (nearCheck(CameraObject.position, s_TargetPos) &&
		nearCheck(CameraObject.atPosition, s_TargetAt) &&
		fabsf(CameraObject.fov - s_TargetFov) <= TARGET_EPSILON)
	{
		// 完全に一致させてフラグ解除
		CameraObject.position = s_TargetPos;
		CameraObject.atPosition = s_TargetAt;
		CameraObject.fov = s_TargetFov;
		s_IsLerping = false;
	}
	else
	{
		// いずれかが未到達ならtrue
		s_IsLerping = true;
	}

	return;
}

void Camera_UpdateAuto()
{
	// 死亡フォーカスモードの処理を最優先
	if (s_IsDeathFocus)
	{
		s_DeathFocusTimer += 1.0f / 60.0f;

		// フォーカス対象のプレイヤー位置を取得
		PLAYEROBJECT* player = GetPlayer(s_FocusPlayerIndex);

		if (player)
		{
			// プレイヤーの位置を注視点として設定
			s_TargetAt = player->position;

			// カメラを斜め上からフォーカス
			s_TargetPos = XMFLOAT3(
				player->position.x + 2.0f,
				player->position.y + 5.3f,
				player->position.z - 4.0f
			);

			// ズームイン
			s_TargetFov = 8.0f;
		}

		// フォーカス時間終了したら元のモードに戻る
		if (s_DeathFocusTimer >= s_DeathFocusDuration)
		{
			Camera_ReturnToNormal();
		}

		// 死亡フォーカス後はここで処理を終了
		return;
	}


	// すべてのプレイヤーの位置を平均して中心を計算
	XMFLOAT3 center = { 0.0f, 0.0f, 0.0f };

	// プレイヤーの位置の範囲を計算
	float minX = FLT_MAX, maxX = -FLT_MAX;
	float minZ = FLT_MAX, maxZ = -FLT_MAX;

	// プレイヤーの数をカウント
	int playerCount = 0;

	// 全プレイヤーの位置を取得して中心と範囲を計算
	for (int i = 0; i < 4; i++) 
	{
		PLAYEROBJECT* player = GetPlayer(i);
		if (!player) continue;

		// 死んだプレイヤーは中心計算の対象外にする
		bool isDead = (!player->active && player->stock <= 0);
		if (isDead) continue;

		// プレイヤーの位置を中心に加算
		center.x += player->position.x;
		center.z += player->position.z;

		// プレイヤーの位置の範囲を更新
		if (player->position.x < minX) minX = player->position.x;
		if (player->position.x > maxX) maxX = player->position.x;
		if (player->position.z < minZ) minZ = player->position.z;
		if (player->position.z > maxZ) maxZ = player->position.z;
		playerCount++;
	}

	// プレイヤーの平均位置を中心とする
	if (playerCount > 0) 
	{
		center.x /= (float)playerCount;
		center.z /= (float)playerCount;
	}

	// カメラオフセットの初期値
	static float camera_offset_x = 0.0f;
	static float camera_offset_y = 10.0f;
	static float camera_offset_z = -10.0f;

	//=================================================================
	// ImGuiでカメラオフセットの調整UI
	//=================================================================
	ImGui::Begin("CameraOffset");
	ImGui::SliderFloat("X", &camera_offset_x, -20.0f, 20.0f, "%.1f");
	ImGui::SliderFloat("Y", &camera_offset_y,  0.0f,  30.0f, "%.1f");
	ImGui::SliderFloat("Z", &camera_offset_z, -30.0f, 30.0f, "%.1f");

	// リセットボタン
	if (ImGui::Button("Reset"))
	{
		camera_offset_x = 0.0f;
		camera_offset_y = 10.0f;
		camera_offset_z = -10.0f;
	}

	// プリセットボタン
	if (ImGui::Button("Top"))
	{
		camera_offset_x = 0.0f;
		camera_offset_y = 20.0f;
		camera_offset_z = -0.001f;
	}
	ImGui::SameLine();
	if (ImGui::Button("Side"))
	{
		camera_offset_x = 15.0f;
		camera_offset_y = 5.0f;
		camera_offset_z = 0.0f;
	}

	ImGui::End();
	//=================================================================

	// 注視点は中心（目標に設定する）
	s_TargetAt = center;

	// カメラの位置を調整（平行投影の立体感）
	// 50度　(x,5 y,13.3 z,-10)
	s_TargetPos = XMFLOAT3(center.x + 5.0f, center.y + 13.3f, center.z + -10.0f);
	//s_TargetPos = XMFLOAT3(center.x + camera_offset_x, center.y + camera_offset_y, center.z + camera_offset_z);

	// 平行投影用の表示範囲計算
	float spreadX = maxX - minX;
	float spreadZ = maxZ - minZ;

	// プレイヤー間の最大距離（幅）
	float maxSpread = (spreadX > spreadZ) ? spreadX : spreadZ;

	// 平行投影での表示幅を計算して、この値が画面の横方向の幅になる
	float margin = 15.0f;
	float targetWidth = maxSpread + margin;

	// ズームの最小値
	if (targetWidth < 12.0f) targetWidth = 12.0f;

	// fovを平行投影の幅として利用（目標に設定）
	s_TargetFov = targetWidth;

	// カメラシェイクの更新
	if (s_IsShaking)
	{
		s_ShakeTimer += 1.0f / 60.0f; 

		if (s_ShakeTimer >= s_ShakeDuration)
		{
			// シェイク終了
			s_IsShaking = false;
			s_ShakeOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);
		}
		else
		{
			// 時間経過で強度を減らしていく
			float dec = 1.0f - (s_ShakeTimer / s_ShakeDuration);
			float currentIntensity = s_ShakeIntensity * dec;

			// ランダム生成
			s_ShakeOffset.x = RandomFloat() * currentIntensity;
			s_ShakeOffset.y = RandomFloat() * currentIntensity;
			s_ShakeOffset.z = RandomFloat() * currentIntensity;
		}
	}
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

	// ビュー行列作成(シェイク追加版！)
	XMVECTOR vpos = XMVectorSet(
		CameraObject.position.x + s_ShakeOffset.x,
		CameraObject.position.y + s_ShakeOffset.y,
		CameraObject.position.z + s_ShakeOffset.z, 0.0f);

	XMVECTOR vAt = XMVectorSet(
		CameraObject.atPosition.x + s_ShakeOffset.x,
		CameraObject.atPosition.y + s_ShakeOffset.y,
		CameraObject.atPosition.z + s_ShakeOffset.z, 0.0f);

	XMVECTOR vUp = XMVectorSet(CameraObject.upVector.x, CameraObject.upVector.y, CameraObject.upVector.z, 0.0f);

	CameraObject.view = XMMatrixLookAtLH(vpos, vAt, vUp);
}

// カメラシェイク関数
// intensity:シェイク強度（0.1-1.0）, duration:シェイクの時間
void Camera_StartShake(float intensity, float duration)
{
	s_IsShaking = true;
	s_ShakeIntensity = intensity;
	s_ShakeDuration = duration;
	s_ShakeTimer = 0.0f;
}


void SetCameraFov(float fov)         { CameraObject.fov = fov; s_TargetFov = fov; }
void SetCameraAspect(float asp)      { CameraObject.aspect = asp; }
void SetCameraClip(float n, float f) { CameraObject.nearClip = n; CameraObject.farClip = f; }

void SetCameraPosition(XMFLOAT3 pos)  { CameraObject.position = pos; s_TargetPos = pos; }
void SetCameraAtPosition(XMFLOAT3 at) { CameraObject.atPosition = at; s_TargetAt = at; }
void SetCameraUpVector(XMFLOAT3 up)	  { CameraObject.upVector = up; }

XMMATRIX GetViewMatrix()        { return	CameraObject.view; }
XMMATRIX GetProjectionMatrix()	{ return	CameraObject.projection; }

DirectX::XMFLOAT3 GetCameraPosition()
{
	return DirectX::XMFLOAT3();
}

// 死亡時のカメラフォーカス開始
void Camera_FocusOnPlayer(int playerIndex, float duration)
{
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	// 現在の状態を保存
	s_BeforeFocusPos = CameraObject.position;
	s_BeforeFocusAt = CameraObject.atPosition;
	s_BeforeFocusFov = CameraObject.fov;
	s_BeforeFocusMode = cameraMode;

	// フォーカスモードに切り替え（オートカメラのままのまま）
	s_IsDeathFocus = true;
	s_FocusPlayerIndex = playerIndex;
	s_DeathFocusDuration = duration;
	s_DeathFocusTimer = 0.0f;
}

// 通常モードに戻る
void Camera_ReturnToNormal()
{
	if (!s_IsDeathFocus) return;

	// 元の状態に戻す（通常のオートカメラ処理）
	s_IsDeathFocus = false;
	s_FocusPlayerIndex = -1;
}

// 死亡フォーカス中かどうか
bool Camera_IsInDeathFocus()
{
	return s_IsDeathFocus;
}

