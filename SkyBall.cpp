// SkyBall.cpp

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "shader.h"
#include "Camera.h"
#include "SkyBall.h"

#define SKYBALL_ROTATION_SPEED	(2.0f / 60.0f)  // １フレーム当たりの回転角度
static SkyBallObject		g_SkyBall;	// BALLオブジェクト
static ID3D11Device* g_pDevice;
static ID3D11DeviceContext* g_pContext;

SkyBallObject* GetSkyBall()
{
	return &g_SkyBall;
}

void SkyBall_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// 3Dオブジェクト初期化
	g_SkyBall.Position = XMFLOAT3(0, 0, 0);
	g_SkyBall.Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_SkyBall.Speed = SKYBALL_ROTATION_SPEED;

	// 今回のデータはスケーリング設定で大きく表示するので大きさを適当に作る
	g_SkyBall.Scaling = XMFLOAT3(20.0f, 20.0f, 20.0f);
	g_SkyBall.Model = ModelLoad("asset\\model\\Skyv2.fbx");

	if (!g_SkyBall.Model)
	{
		OutputDebugStringA("SkyBall_Initialize: モデルロード失敗\n");
	}
	else
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "SkyBall_Initialize: モデルロード完了 meshes=%u textures=%u\n",
			g_SkyBall.Model->AiScene ? g_SkyBall.Model->AiScene->mNumMeshes : 0,
			g_SkyBall.Model->AiScene ? g_SkyBall.Model->AiScene->mNumTextures : 0);
		OutputDebugStringA(buf);
	}
}

void SkyBall_Finalize()
{
	ModelRelease(g_SkyBall.Model);
	g_SkyBall.Model = nullptr;
}

void SkyBall_Update()
{
	// カメラの位置を取得して座標を決める
	// 今回はカメラと同じ座標で表示しておく
	g_SkyBall.Position = GetCameraPosition();

	// 適当に回転させる
	g_SkyBall.Rotation.y += g_SkyBall.Speed;

	return;
}

void SkyBall_Draw()
{
	// 平行移動行列作成
	XMMATRIX	TranslationMatrix =
		XMMatrixTranslation(
			g_SkyBall.Position.x,
			g_SkyBall.Position.y,
			g_SkyBall.Position.z
		);
	// 回転行列作成
	XMMATRIX	 RotationMatrix =
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(g_SkyBall.Rotation.x),
			XMConvertToRadians(g_SkyBall.Rotation.y),
			XMConvertToRadians(g_SkyBall.Rotation.z)
		);
	// スケーリング行列作成
	XMMATRIX	 ScalingMatrix =
		XMMatrixScaling(
			g_SkyBall.Scaling.x,
			g_SkyBall.Scaling.y,
			g_SkyBall.Scaling.z
		);
	// ワールド行列作成
	XMMATRIX	world =
		ScalingMatrix *
		RotationMatrix *
		TranslationMatrix;

	CAMERA* camera = GetCamera();

	XMMATRIX oldProjection = camera->projection;

	// 透視投影
	camera->projection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(60.0f),
		camera->aspect,
		0.1f,
		1000.0f
	);

	XMMATRIX	view = GetViewMatrix();
	//XMMATRIX	projection = GetProjectionMatrix();
	XMMATRIX	wvp = world * view * camera->projection;

	// WVP行列をセット

	Shader_SetMatrix(wvp);

	// モデル表示

	ModelDraw(g_SkyBall.Model);

	camera->projection = oldProjection;
}

