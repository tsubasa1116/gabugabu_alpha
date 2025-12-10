// attack.cpp

#include "DirectXMath.h"
#include "d3d11.h"
using namespace DirectX;

#include "attack.h"
#include "sprite.h"
#include "shader.h"
#include "Camera.h"
#include "collider.h"
#include "field.h"
#include "building.h"
#include "debug_ostream.h"
#include "Polygon3D.h"
#include "keyboard.h"

// グローバル変数
static ID3D11Device* g_pDevice = NULL;
static ID3D11DeviceContext* g_pContext = NULL;

// 頂点バッファ
static ID3D11Buffer* g_VertexBuffer;

// インデックスバッファ
static ID3D11Buffer* g_IndexBuffer;

// テクスチャ変数
static ID3D11ShaderResourceView* g_Attack_Texture[PLAYER_MAX];

// オブジェクト
static ATTACK_OBJECT Attack[PLAYER_MAX];

// 攻撃タイマー
float PlayerAttackTimer[PLAYER_MAX];

// プレイヤー 攻撃フラグ
bool PlayerIsAttacking[PLAYER_MAX] = { false, false/*, false, false*/ };

// マクロ定義
#define NUM_VERTEX (36)

static Vertex Attack_vdata[NUM_VERTEX] =
{
	// -Z面
	{
		// 頂点0 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点1 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点2 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),    // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 1.0f),             // テクスチャ座標
	},

	{
		// 頂点3 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 1.0f),             // テクスチャ座標
	},

	// +X面
	{
		// 頂点4 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点5 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),       // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点6 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 1.0f),             // テクスチャ座標
	},

	{
		// 頂点7 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 1.0f),             // テクスチャ座標
	},

	// +Z面
	{
		// 頂点8 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),       // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点9 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点10 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 1.0f),             // テクスチャ座標
	},

	{
		// 頂点11 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 1.0f),             // テクスチャ座標
	},

	// -X面
	{
		// 頂点12 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点13 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点14 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 1.0f),             // テクスチャ座標
	},

	{
		// 頂点15 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),    // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 1.0f),             // テクスチャ座標
	},

	// +Y面
	{
		// 頂点16 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点17 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),       // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点18 LEFT-BOTTOM
		XMFLOAT3(-0.5f, 0.5f, -0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 1.0f),             // テクスチャ座標
	},

	{
		// 頂点19 RIGHT-BOTTOM
		XMFLOAT3(0.5f, 0.5f, -0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 1.0f),             // テクスチャ座標
	},

	// -Y面
	{
		// 頂点20 LEFT-TOP
		XMFLOAT3(-0.5f, -0.5f, -0.5f),    // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点21 RIGHT-TOP
		XMFLOAT3(0.5f, -0.5f, -0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 0.0f),             // テクスチャ座標
	},
	{
		// 頂点22 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),     // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(0.0f, 1.0f),             // テクスチャ座標
	},

	{
		// 頂点23 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),      // 座標
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), // 色
		XMFLOAT2(1.0f, 1.0f),             // テクスチャ座標
	},
};

// インデックス配列
static UINT Attack_idxdata[6 * 6]
{
	 0,  1,  2,  2,  1,  3, // -Z面
	 4,  5,  6,  6,  5,  7, // +X面
	 8,  9, 10, 10,  9, 11, // +Z面
	12, 13, 14, 14, 13, 15, // -X面
	16, 17, 18, 18, 17, 19, // +Y面
	20, 21, 22, 22, 21, 23, // -Y面
};

void Attack_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	Attack[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Attack[0].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Attack[0].scaling = XMFLOAT3(0.2f, 0.2f, 0.2f);

	Attack[1].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Attack[1].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Attack[1].scaling = XMFLOAT3(0.3f, 0.3f, 0.3f);

	// 頂点バッファ作成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * NUM_VERTEX; // 格納できる頂点数 * 頂点サイズ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	g_pDevice = pDevice;
	g_pContext = pContext;

	// テクスチャ読み込み
	TexMetadata metadata1;
	ScratchImage image1;

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata1, image1);
	CreateShaderResourceView(pDevice, image1.GetImages(),
		image1.GetImageCount(), metadata1, &g_Attack_Texture[0]);
	assert(g_Attack_Texture[0]);

	// テクスチャ読み込み
	TexMetadata metadata2;
	ScratchImage image2;
	LoadFromWICFile(L"Asset\\Texture\\SkyBlue.jpg", WIC_FLAGS_NONE, &metadata2, image2);
	CreateShaderResourceView(pDevice, image2.GetImages(),
		image2.GetImageCount(), metadata2, &g_Attack_Texture[1]);
	assert(g_Attack_Texture[1]);

	// インデックスバッファ作成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(UINT) * 6 * 6; // 格納できる頂点数 * 頂点サイズ

		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		pDevice->CreateBuffer(&bd, NULL, &g_IndexBuffer);

		// インデックスバッファへ書き込み
		D3D11_MAPPED_SUBRESOURCE msr;
		pContext->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		UINT* index = (UINT*)msr.pData;

		// インデックスデータをバッファへコピー
		CopyMemory(&index[0], &Attack_idxdata[0], sizeof(UINT) * 6 * 6);
		pContext->Unmap(g_IndexBuffer, 0);
	}
}

void Attack_Finalize()
{
	if (g_VertexBuffer != NULL)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
	if (g_IndexBuffer != NULL)
	{
		g_IndexBuffer->Release();
		g_IndexBuffer = NULL;
	}

	for (int i = 0; i < PLAYER_MAX; i++)
	{
		if (g_Attack_Texture[i])
		{
			g_Attack_Texture[i]->Release();
			g_Attack_Texture[i] = NULL;
		}
	}
}

void Attack_Update(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex + 1);
	ATTACK_OBJECT& sk = Attack[playerIndex];

	if (playerObject->isAttacking == true)
	{
		// がぶがぶの初期位置をプレイヤーの位置に設定
		sk.position.x = playerObject->position.x;
		sk.position.y = playerObject->position.y;
		sk.position.z = playerObject->position.z;

		float Player_RotationY = playerObject->rotation.y;
		float rad = XMConvertToRadians(Player_RotationY);

		// 進行方向を計算
		XMFLOAT3 dir =
		{
			sinf(rad),  // X方向
			0.0f,       // Y方向（水平）
			cosf(rad)   // Z方向
		};

		// プレイヤーの前方にがぶがぶを配置
		sk.position.x = dir.x * playerObject->scaling.x + playerObject->position.x;
		sk.position.y = playerObject->position.y;
		sk.position.z = dir.z * playerObject->scaling.z + playerObject->position.z;

		// 攻撃タイマーを進める
		playerObject->attackTimer += 1.0f / 60.0f;

		// プレイヤー毎の攻撃時間が経過したら攻撃終了
		if (playerObject->attackTimer > playerObject->attackDuration)
		{
			playerObject->isAttacking = false;
			playerObject->attackTimer = 0.0f;
		}
	}

	// -------------------------------------------------------------
	// 当たり判定
	// -------------------------------------------------------------
	// AABBの更新
	CalculateAABB(sk.boundingBox, sk.position, XMFLOAT3(1.0f, 1.0f, 1.0f));

	int buildingCount = GetBuildingCount();			// 数を取得
	Building** buildingObjects = GetBuildings();	// リストを取得

	// 全てのフィールドオブジェクトと衝突判定を行う
	for (int i = 0; i < buildingCount; ++i)
	{
		// i番目のフィールドオブジェクトのAABBを取得
		// field.cppのInitializeで計算済みのため、そのまま参照
		AABB pStaticObjectAABB = buildingObjects[i]->boundingBox;

		// プレイヤーのAABBとフィールドオブジェクトのAABBでMTVを計算
		MTV collision = CalculateAABBMTV(sk.boundingBox, pStaticObjectAABB);

		Keyboard_Keys_tag confirmKey[PLAYER_MAX] = { KK_SPACE , KK_ENTER/*, KK_, KK_ */};

		if (collision.isColliding)
		{
			// 建物（FIELD_BUILDING）に衝突していて、かつ各々のプレイヤーのがぶがぶキーが押されていたら
			{
				BuildingType type = buildingObjects[i]->Type;

				// 各建物タイプごとの処理（Glass を参考に拡張）
				if (Keyboard_IsKeyDown(confirmKey[playerIndex]))
				{
					switch (type)
					{
					case BuildingType::Glass:
					{
						buildingObjects[i]->isActive = false;
						playerObject->breakCount_Glass += 1;

						// 効果音やエフェクトを再生

						// ヒットでスキルを終了
						playerObject->isAttacking = false;
						playerObject->attackTimer = 0.0f;

						// 更新済みAABB
						CalculateAABB(sk.boundingBox, sk.position, sk.scaling);

						// 変身
						if (playerObject->breakCount_Glass >= 3)
						{
							playerObject->form = (Form)((int)playerObject->form + 1);
							playerObject->breakCount_Glass = 0;
						}
						break;
					}

					case BuildingType::Concrete:
					{
						buildingObjects[i]->isActive = false;
						playerObject->breakCount_Concrete += 1;

						// 効果音やエフェクトを再生

						// ヒットでスキルを終了
						playerObject->isAttacking = false;
						playerObject->attackTimer = 0.0f;

						// 更新済みAABB
						CalculateAABB(sk.boundingBox, sk.position, sk.scaling);

						// 変身
						if (playerObject->breakCount_Concrete >= 3)
						{
							playerObject->form = (Form)((int)playerObject->form + 1);
							playerObject->breakCount_Concrete = 0;
						}
						break;
					}

					case BuildingType::Plant:
					{
						buildingObjects[i]->isActive = false;
						playerObject->breakCount_Plant += 1;

						// 効果音やエフェクトを再生

						// ヒットでスキルを終了
						playerObject->isAttacking = false;
						playerObject->attackTimer = 0.0f;

						// 更新済みAABB
						CalculateAABB(sk.boundingBox, sk.position, sk.scaling);

						// 変身
						if (playerObject->breakCount_Plant >= 3)
						{
							playerObject->form = (Form)((int)playerObject->form + 1);
							playerObject->breakCount_Plant = 0;
						}
						break;
					}

					case BuildingType::Electric:
					{
						buildingObjects[i]->isActive = false;
						playerObject->breakCount_Electric += 1;

						// 効果音やエフェクトを再生

						// ヒットでスキルを終了
						playerObject->isAttacking = false;
						playerObject->attackTimer = 0.0f;

						// 更新済みAABB
						CalculateAABB(sk.boundingBox, sk.position, sk.scaling);

						// 変身
						if (playerObject->breakCount_Electric >= 3)
						{
							playerObject->form = (Form)((int)playerObject->form + 1);
							playerObject->breakCount_Electric = 0;
						}
						break;
					}

					default:
						break;
					}

					// 建物種別処理を行ったら次の建物へ
					continue;
				}
			}

			// 衝突していたら、MTVの分だけ位置を戻す
			sk.position.x += collision.translation.x;
			sk.position.y += collision.translation.y;
			sk.position.z += collision.translation.z;

			// 押し戻し後の新しいAABBを再計算
			// これにより、同じフレーム内で次のフィールドオブジェクトとの判定に備えます。
			CalculateAABB(sk.boundingBox, sk.position, sk.scaling);

			// デバッグ出力
			hal::dout << "衝突！押し戻し量: " << collision.overlap << " @ " << (collision.translation.x != 0 ? "X軸" : (collision.translation.y != 0 ? "Y軸" : "Z軸")) << std::endl;

			// ↑↑↑　#include "debug_ostream.h"　のインクルードでデバッグ確認
		}
	}
}

void Attack_Draw(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	// 参照を取る
	ATTACK_OBJECT& sk = Attack[playerIndex];
	ID3D11ShaderResourceView* tex = g_Attack_Texture[playerIndex];

	// =====================
	// ワールド行列の作成
	// =====================

	// スケーリング行列の作成
	XMMATRIX ScalingMatrix = XMMatrixScaling
	(
		sk.scaling.x,
		sk.scaling.y,
		sk.scaling.z
	);

	// 回転行列の作成
	XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw
	(
		XMConvertToRadians(sk.rotation.x),
		XMConvertToRadians(sk.rotation.y),
		XMConvertToRadians(sk.rotation.z)
	);

	// 平行移動行列の作成
	XMMATRIX TranslationMatrix = XMMatrixTranslation
	(
		sk.position.x,
		sk.position.y,
		sk.position.z
	);

	XMMATRIX WorldMatrix = ScalingMatrix * RotationMatrix * TranslationMatrix;

	// プロジェクション行列作成
	XMMATRIX Projection = GetProjectionMatrix();

	// ビュー行列作成
	XMMATRIX View = GetViewMatrix();

	// 最終的な変換行列を作成
	XMMATRIX WVP = WorldMatrix * View * Projection;

	// 変換行列を頂点シェーダーへセット
	Shader_SetMatrix(WVP);

	// シェーダーを描画パイプラインへ設定
	Shader_Begin();

	// 不透明で描画するためブレンドを無効化し、描画カラーのアルファを1に固定する
	SetBlendState(BLENDSTATE_NONE);
	Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	// 頂点シェーダーを描画パイプラインへ設定
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex* vertex = (Vertex*)msr.pData;

	// 頂点データを頂点バッファへコピーする
	CopyMemory(&vertex[0], &Attack_vdata[0], sizeof(Vertex) * NUM_VERTEX);

	// コピー完了
	g_pContext->Unmap(g_VertexBuffer, 0);

	// テクスチャをセット
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// 頂点バッファをセット
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// インデックスバッファをセット
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 描画するポリゴンの種類をセット 3頂点でポリゴン1枚として表示
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//// 描画リクエスト
	//g_pContext->Draw(NUM_VERTEX, 0);

	g_pContext->DrawIndexed(6 * 6, 0, 0);

	SetBlendState(BLENDSTATE_ALPHA);

}

ATTACK_OBJECT* GetAttack(int playerIndex)
{
	if (playerIndex > PLAYER_MAX || playerIndex <= 0)
	{
		return nullptr;
	}

	return &Attack[playerIndex - 1];
}