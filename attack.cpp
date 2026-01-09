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
#include "Building.h"
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
	for (int p = 0; p < PLAYER_MAX; p++)
	{
		Attack[p].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Attack[p].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Attack[p].scaling = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

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
	TexMetadata metadata;
	ScratchImage image;

	LoadFromWICFile(L"Asset\\Texture\\Red.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(),image.GetImageCount(), metadata, &g_Attack_Texture[0]);
	assert(g_Attack_Texture[0]);

	LoadFromWICFile(L"Asset\\Texture\\SkyBlue.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(),image.GetImageCount(), metadata, &g_Attack_Texture[1]);
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
	ATTACK_OBJECT& atttackObject = Attack[playerIndex];

	if (playerObject->isAttacking == true)
	{
		// がぶがぶの初期位置をプレイヤーの位置に設定
		atttackObject.position.x = playerObject->position.x;
		atttackObject.position.y = playerObject->position.y;
		atttackObject.position.z = playerObject->position.z;

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
		atttackObject.position.x = dir.x * playerObject->scaling.x + playerObject->position.x;
		atttackObject.position.y = playerObject->position.y;
		atttackObject.position.z = dir.z * playerObject->scaling.z + playerObject->position.z;

		// 攻撃タイマー更新
		playerObject->attackTimer += 1.0f / 60.0f;

		// プレイヤー毎の攻撃時間が経過したら攻撃終了
		if (playerObject->attackTimer >= ATTACKING_TIME)
		{
			playerObject->isAttacking = false;
			playerObject->attackTimer = 0.0f;
		}
	}

	// -------------------------------------------------------------
	// 当たり判定
	// -------------------------------------------------------------
	// AABBの更新
	CalculateAABB(atttackObject.boundingBox, atttackObject.position, XMFLOAT3(1.0f, 1.0f, 1.0f));

	int buildingCount = GetBuildingCount();			// 数を取得
	Building** buildingObjects = GetBuildings();	// リストを取得

	// 全てのフィールドオブジェクトと衝突判定を行う
	for (int i = 0; i < buildingCount; ++i)
	{
		// i番目のフィールドオブジェクトのAABBを取得
		// field.cppのInitializeで計算済みのため、そのまま参照
		AABB pStaticObjectAABB = buildingObjects[i]->boundingBox;

		// プレイヤーのAABBとフィールドオブジェクトのAABBでMTVを計算
		MTV collision = CalculateAABBMTV(atttackObject.boundingBox, pStaticObjectAABB);

		Keyboard_Keys_tag confirmKey[PLAYER_MAX] = { KK_SPACE , KK_ENTER/*, KK_, KK_ */};

		if (collision.isColliding)
		{
			// 建物（FIELD_BUILDING）に衝突していて、かつ各々のプレイヤーのがぶがぶキーが押されていたら
			{
				BuildingType type = buildingObjects[i]->Type;

				// 各建物タイプごとの処理
				if (Keyboard_IsKeyDown(confirmKey[playerIndex]))
				{
					switch (type)
					{
					case BuildingType::Glass:
					{
						buildingObjects[i]->isActive = false;									// 建物を非アクティブ化
						playerObject->breakCount_Glass += 1;									// ガラスを壊した数をプラス
						playerObject->evolutionGauge += 1 * playerObject->evolutionGaugeRate;	// 進化ゲージをプラス
						playerObject->brokenHistory.push_back(type);							// 最後に破壊した建物タイプを保存

						// 効果音やエフェクトを再生

						// ヒットでスキルを終了
						//playerObject->isAttacking = false;
						//playerObject->attackTimer = 0.0f;

						// 更新済みAABB
						CalculateAABB(atttackObject.boundingBox, atttackObject.position, atttackObject.scaling);
						break;
					}

					case BuildingType::Concrete:
					{
						buildingObjects[i]->isActive = false;									// 建物を非アクティブ化
						playerObject->breakCount_Concrete += 1;									// コンクリートを壊した数をプラス
						playerObject->evolutionGauge += 1 * playerObject->evolutionGaugeRate;	// 進化ゲージをプラス
						playerObject->brokenHistory.push_back(type);							// 最後に破壊した建物タイプを保存

						// 効果音やエフェクトを再生

						// ヒットでスキルを終了
						//playerObject->isAttacking = false;
						//playerObject->attackTimer = 0.0f;

						// 更新済みAABB
						CalculateAABB(atttackObject.boundingBox, atttackObject.position, atttackObject.scaling);
						break;
					}

					case BuildingType::Plant:
					{
						buildingObjects[i]->isActive = false;									// 建物を非アクティブ化
						playerObject->breakCount_Plant += 1;									// 植物を壊した数をプラス
						playerObject->evolutionGauge += 1 * playerObject->evolutionGaugeRate;	// 進化ゲージをプラス
						playerObject->brokenHistory.push_back(type);							// 最後に破壊した建物タイプを保存

						// 効果音やエフェクトを再生

						// ヒットでスキルを終了
						//playerObject->isAttacking = false;
						//playerObject->attackTimer = 0.0f;

						// 更新済みAABB
						CalculateAABB(atttackObject.boundingBox, atttackObject.position, atttackObject.scaling);
						break;
					}

					case BuildingType::Electric:
					{
						buildingObjects[i]->isActive = false;			// 建物を非アクティブ化
						playerObject->breakCount_Electric += 1;			// 電気を壊した数をプラス
						playerObject->evolutionGauge += 1;				// 進化ゲージをプラス
						playerObject->brokenHistory.push_back(type);	// 最後に破壊した建物タイプを保存

						// 効果音やエフェクトを再生

						// ヒットでスキルを終了
						//playerObject->isAttacking = false;
						//playerObject->attackTimer = 0.0f;

						// 更新済みAABB
						CalculateAABB(atttackObject.boundingBox, atttackObject.position, atttackObject.scaling);
						break;
					}

					default:
						break;
					}

					// --- 進化処理 ---
					if (playerObject->evolutionGauge >= EVOLUTIONGAUGE_MAX)
					{
						// プレイヤーを無敵にする
						playerObject->isInvincible = true;
						playerObject->invincibleTimer = 0.0f;

						// 現在のフォーム（進化前の状態）を保存
						Form currentForm = playerObject->form;

						// 1. 進化段階を1つ進める
						playerObject->form = static_cast<Form>(static_cast<int>(playerObject->form) + 1);

						// 2. 2進化までしか進化しないように制限
						if (playerObject->form >= Form::SecondEvolution)
						{
							playerObject->form = Form::SecondEvolution;
						}

						// 3. タイプ決定ロジック
						//    Typeの決定は、Normalから FirstEvolutionに進化する場合のみ実行
						if (currentForm == Form::Normal)
						{
							// 4種類の破壊した建物数を配列に格納
							const int counts[4] =
							{
								playerObject->breakCount_Glass,		// idx 0
								playerObject->breakCount_Concrete,	// idx 1
								playerObject->breakCount_Plant,		// idx 2
								playerObject->breakCount_Electric	// idx 3
							};

							// 対応するタイプ定義
							const BuildingType types[4] =
							{
								BuildingType::Glass,
								BuildingType::Concrete,
								BuildingType::Plant,
								BuildingType::Electric
							};

							// --- Step 1: 最大カウント数(maxCount)を求める ---
							int maxCount = 0;
							for (int i = 0; i < 4; i++)
							{
								if (counts[i] > maxCount)
								{
									maxCount = counts[i];
								}
							}

							// --- Step 2: 履歴を「最新」から「過去」へ遡って勝者を決める ---
							int maxIdx = 0;

							// vectorを後ろから回す
							for (int i = playerObject->brokenHistory.size() - 1; i >= 0; i--)
							{
								BuildingType historyType = playerObject->brokenHistory[i];
								int typeIdx = -1;

								// タイプをインデックス番号に変換
								for (int j = 0; j < 4; j++)
								{
									if (historyType == types[j])
									{
										typeIdx = j;
										break;
									}
								}

								// 「今見ている履歴のタイプ」が「最大カウント数を持つグループ」の一員か？
								if (typeIdx != -1 && counts[typeIdx] == maxCount)
								{
									maxIdx = typeIdx;
									break; // 見つかった時点で確定！
								}
							}

							// --- Step 3: 最終タイプ反映 ---
							switch (maxIdx)
							{
							case 0: playerObject->type = PlayerType::Glass;    break;
							case 1: playerObject->type = PlayerType::Concrete; break;
							case 2: playerObject->type = PlayerType::Plant;    break;
							case 3: playerObject->type = PlayerType::Electric; break;
							}
						}

						// 4. リセット処理
						playerObject->brokenHistory.clear(); // 履歴をクリア
						playerObject->evolutionGauge = 0;
						playerObject->breakCount_Glass = 0;
						playerObject->breakCount_Concrete = 0;
						playerObject->breakCount_Plant = 0;
						playerObject->breakCount_Electric = 0;

						continue;
					}
				}
			}

			// 衝突していたら、MTVの分だけ位置を戻す
			atttackObject.position.x += collision.translation.x;
			atttackObject.position.y += collision.translation.y;
			atttackObject.position.z += collision.translation.z;

			// 押し戻し後の新しいAABBを再計算
			// これにより、同じフレーム内で次のフィールドオブジェクトとの判定に備えます。
			CalculateAABB(atttackObject.boundingBox, atttackObject.position, atttackObject.scaling);

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
	XMMATRIX projection = GetProjectionMatrix();

	// ビュー行列作成
	XMMATRIX view = GetViewMatrix();

	// 最終的な変換行列を作成
	XMMATRIX WVP = WorldMatrix * view * projection;

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

void AttackPlayerCollisions()
{
	// 各プレイヤーの攻撃オブジェクトをループして、他プレイヤー全員に当たり判定を行う
	for (int atk = 0; atk < PLAYER_MAX; ++atk)
	{
		ATTACK_OBJECT* attack = GetAttack(atk + 1);
		PLAYEROBJECT* attacker = GetPlayer(atk + 1);

		if (attack == nullptr || attacker == nullptr) continue;
		if (!attacker->isAttacking) continue;	// 攻撃中のみ判定

		// 攻撃オブジェクトと攻撃者の AABB を更新
		CalculateAABB(attack->boundingBox, attack->position, attack->scaling);
		CalculateAABB(attacker->boundingBox, attacker->position, attacker->scaling);

		// 攻撃者の向きベクトルを更新（rotation.y から算出）
		{
			float rad = XMConvertToRadians(attacker->rotation.y);
			attacker->dir.x = sinf(rad);
			attacker->dir.z = cosf(rad);
		}

		// --- プレイヤー側で使っている描画スケール・ヒットボックス比率と合わせる ---
		const float RENDER_SCALE = 2.0f;
		const float HITBOX_WIDTH_SCALE = 0.6f;
		const float HITBOX_HEIGHT_SCALE = 1.0f;
		const float HITBOX_DEPTH_SCALE = 0.6f;

		// 攻撃が当たる対象として他プレイヤー全員をチェック
		for (int def = 0; def < PLAYER_MAX; ++def)
		{
			if (def == atk) continue; // 自分には当たらない

			PLAYEROBJECT* defender = GetPlayer(def + 1);
			if (defender == nullptr) continue;
			if (!defender->active) continue; // 非アクティブなプレイヤーは無視
			// 被弾中や無敵ならスキップ
			if (defender->isInvincible) continue;

			// defender 用のヒットボックススケールを Polygon3D と同じ方式で計算して AABB を作る
			XMFLOAT3 defenderHitboxScaling =
			{
				defender->scaling.x * RENDER_SCALE * HITBOX_WIDTH_SCALE,
				defender->scaling.y * RENDER_SCALE * HITBOX_HEIGHT_SCALE,
				defender->scaling.z * RENDER_SCALE * HITBOX_DEPTH_SCALE
			};
			CalculateAABB(defender->boundingBox, defender->position, defenderHitboxScaling);

			// 判定（defender AABB と 攻撃オブジェクト AABB）
			MTV col = CalculateAABBMTV(defender->boundingBox, attack->boundingBox);

			if (col.isColliding)
			{
				// ノックバック（攻撃者の向きと攻撃力を使用）
				defender->position.x += attacker->dir.x * attacker->power;
				// defender->position.y += attacker->power / 3.0f;
				defender->position.z += attacker->dir.z * attacker->power;

				// ダメージ（防御で軽減）
				defender->hp -= attacker->power * defender->defense;
				if (defender->hp < 0.0f) defender->hp = 0.0f;

				// スタンゲージ増加
				defender->stunGauge += 0.5f;

				// ダメージフラグ・タイマー（アニメ／UI 用）
				defender->isAttacked = true;
				defender->attackedTimer = 0.0f;

				// 再計算して状態を整える
				CalculateAABB(defender->boundingBox, defender->position, defenderHitboxScaling);
				CalculateAABB(attack->boundingBox, attack->position, attack->scaling);
			}
		}
	}
}

ATTACK_OBJECT* GetAttack(int playerIndex)
{
	if (playerIndex > PLAYER_MAX || playerIndex <= 0)
	{
		return nullptr;
	}

	return &Attack[playerIndex - 1];
}