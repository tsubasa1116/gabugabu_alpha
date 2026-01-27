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
#include "DamageText.h"
#include "Effect.h"
#include "input.h"

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
#define NUM_VERTEX (24)

static Vertex2 Attack_vdata[NUM_VERTEX] =
{
	// -Z面 (法線: 0,0,-1)
	{// 頂点0 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),		// 座標
		XMFLOAT3(0.0f, 0.0f, -1.0f),		// 法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),	// カラー
		XMFLOAT2(0.0f,0.0f)					// テクスチャ座標
	},
	{// 頂点1 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// 頂点2 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// 頂点3 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// +X面 (法線: 1,0,0)
	{// 頂点4 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// 頂点5 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// 頂点6 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// 頂点7 RIGHT-BOTTM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// +Z面 (法線: 0,0,1)
	{// 頂点8 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// 頂点9 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// 頂点10 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// 頂点11 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// -X面 (法線: -1,0,0)
	{// 頂点12 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// 頂点13 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// 頂点14 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// 頂点15 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// +Y面 (法線: 0,1,0)
	{// 頂点16 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// 頂点17 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// 頂点18 LEFT-BOTTOM
		XMFLOAT3(-0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// 頂点19 RIGHT-BOTTOM
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

	// -Y面 (法線: 0,-1,0)
	{// 頂点20 LEFT-TOP
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{// 頂点21 RIGHT-TOP
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{// 頂点22 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{// 頂点23 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
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
	bd.ByteWidth = sizeof(Vertex2) * NUM_VERTEX; // 格納できる頂点数 * 頂点サイズ
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

	PLAYEROBJECT* playerObject = GetPlayer(playerIndex);
	if (playerObject == nullptr) return;
	PLAYEROBJECT& player = *playerObject;

	ATTACK_OBJECT& atttackObject = Attack[playerIndex];

	if (player.isAttacking == true)
	{
		float Player_RotationY = player.rotation.y;
		float rad = XMConvertToRadians(Player_RotationY);

		// 進行方向を計算
		XMFLOAT3 dir =
		{
			sinf(rad),	// X方向
			0.0f,		// Y方向（水平）
			cosf(rad)	// Z方向
		};

		// プレイヤーの前方にがぶがぶを配置
		atttackObject.position.x = dir.x * player.scaling.x + player.position.x;
		atttackObject.position.y = player.position.y;
		atttackObject.position.z = dir.z * player.scaling.z + player.position.z;

		// 攻撃タイマー更新
		player.attackTimer += DELTA_TIME;

		// プレイヤー毎の攻撃時間が経過したら攻撃終了
		if (player.attackTimer >= ATTACKING_TIME)
		{
			player.isAttacking = false;
			player.attackTimer = 0.0f;
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
		// 非アクティブなオブジェクトをスキップ（二重でゲージが加算されることを防ぐため）
		if (!buildingObjects[i]->isActive) continue;

		// i番目のフィールドオブジェクトのAABBを取得
		// field.cppのInitializeで計算済みのため、そのまま参照
		AABB pStaticObjectAABB = buildingObjects[i]->boundingBox;

		// プレイヤーのAABBとフィールドオブジェクトのAABBでMTVを計算
		MTV collision = CalculateAABBMTV(atttackObject.boundingBox, pStaticObjectAABB);

		Keyboard_Keys_tag confirmKey[PLAYER_MAX] = { KK_SPACE , KK_ENTER, KK_V, /*KK_ */};

		// 建物（FIELD_BUILDING）に衝突していて、かつ各々のプレイヤーのがぶがぶキーが押されていたら
		if (collision.isColliding)
		{
			BuildingType type = buildingObjects[i]->Type;

			if (g_Input[playerIndex].A || Keyboard_IsKeyDown(confirmKey[playerIndex]))
			{
				// 各建物タイプごとの処理
				switch (type)
				{
				case BuildingType::Glass:
					buildingObjects[i]->isActive = false;				// 建物を非アクティブ化
					player.breakCount_Glass += 1;						// ガラスを壊した数をプラス
					player.evolutionGauge += player.evolutionGaugeRate * 10;	// 進化ゲージをプラス
					player.brokenHistory.push_back(type);				// 最後に破壊した建物タイプを保存

					// 効果音やエフェクトを再生

					// ヒットでスキルを終了
					//player.isAttacking = false;
					//player.attackTimer = 0.0f;

					// 更新済みAABB
					CalculateAABB(atttackObject.boundingBox, atttackObject.position, atttackObject.scaling);
					break;

				case BuildingType::Concrete:
					buildingObjects[i]->isActive = false;				// 建物を非アクティブ化
					player.breakCount_Concrete += 1;					// コンクリートを壊した数をプラス
					player.evolutionGauge += player.evolutionGaugeRate * 10;	// 進化ゲージをプラス
					player.brokenHistory.push_back(type);				// 最後に破壊した建物タイプを保存

					// 効果音やエフェクトを再生

					// ヒットでスキルを終了
					//player.isAttacking = false;
					//player.attackTimer = 0.0f;

					// 更新済みAABB
					CalculateAABB(atttackObject.boundingBox, atttackObject.position, atttackObject.scaling);
					break;

				case BuildingType::Plant:					
					buildingObjects[i]->isActive = false;				// 建物を非アクティブ化
					player.breakCount_Plant += 1;						// 植物を壊した数をプラス
					player.evolutionGauge += player.evolutionGaugeRate * 10;	// 進化ゲージをプラス
					player.brokenHistory.push_back(type);				// 最後に破壊した建物タイプを保存

					// 効果音やエフェクトを再生

					// ヒットでスキルを終了
					//player.isAttacking = false;
					//player.attackTimer = 0.0f;

					// 更新済みAABB
					CalculateAABB(atttackObject.boundingBox, atttackObject.position, atttackObject.scaling);
					break;

				case BuildingType::Electric:
					buildingObjects[i]->isActive = false;				// 建物を非アクティブ化
					player.breakCount_Electric += 1;					// 電気を壊した数をプラス
					player.evolutionGauge += player.evolutionGaugeRate * 10;	// 進化ゲージをプラス
					player.brokenHistory.push_back(type);				// 最後に破壊した建物タイプを保存

					// 効果音やエフェクトを再生

					// ヒットでスキルを終了
					//player.isAttacking = false;
					//player.attackTimer = 0.0f;

					// 更新済みAABB
					CalculateAABB(atttackObject.boundingBox, atttackObject.position, atttackObject.scaling);
					break;

				default:
					break;
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
			//hal::dout << "衝突！押し戻し量: " << collision.overlap << " @ " << (collision.translation.x != 0 ? "X軸" : (collision.translation.y != 0 ? "Y軸" : "Z軸")) << std::endl;

			// ↑↑↑　#include "debug_ostream.h"　のインクルードでデバッグ確認
		}
	}

	// --- 進化処理 ---
	if (player.evolutionGauge >= EVOLUTIONGAUGE_MAX)
	{
		// プレイヤーを無敵にする
		player.isInvincible = true;
		player.invincibleTimer = 0.0f;

		// 現在のフォーム（進化前の状態）を保存
		Form currentForm = player.form;

		// 1. 進化段階を1つ進める
		player.form = static_cast<Form>(static_cast<int>(player.form) + 1);

		// 2. 2進化までしか進化しないように制限
		if (player.form >= Form::SecondEvolution)
		{
			player.form = Form::SecondEvolution;
		}

		// 3. タイプ決定ロジック
		//    Typeの決定は、Normalから FirstEvolutionに進化する場合のみ実行
		if (currentForm == Form::Normal)
		{
			// 4種類の破壊した建物数を配列に格納
			const int counts[4] =
			{
				player.breakCount_Glass,	// idx 0
				player.breakCount_Concrete,	// idx 1
				player.breakCount_Plant,	// idx 2
				player.breakCount_Electric	// idx 3
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
			for (int i = player.brokenHistory.size() - 1; i >= 0; i--)
			{
				BuildingType historyType = player.brokenHistory[i];
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

		// 4. リセット処理 (毎回実行)
		//    タイプ決定の if ブロックの外に出すことで、どのフォーム段階からの進化でもリセットされる


		// 2進化に到達した直後ならエフェクトをセット
		/*if (playerObject->form == Form::SecondEvolution && currentForm != Form::SecondEvolution)
		{
			XMFLOAT2 pos = { 170.0f, 600.0f };
			XMFLOAT2 size = { 300.0f, 300.0f };
			Effect_Set(0, pos, size);
		}*/

		// 2進化に到達した直後ならエフェクトをセット（プレイヤー番号別位置・タイプ別テクスチャ）
		if (playerObject->form == Form::SecondEvolution && currentForm != Form::SecondEvolution)
		{
			// プレイヤーごとの画面上のエフェクト位置
			const XMFLOAT2 playerEffectPos[PLAYER_MAX] =
			{
				{ 160.0f, 620.0f }, // プレイヤー1
				{ 470.0f, 620.0f }, // プレイヤー2
				{ 780.0f, 620.0f }, // プレイヤー3
				{ 1090.0f, 620.0f }  // プレイヤー4
			};

			// 進化タイプ別のテクスチャ番号（Effect のテクスチャ配列と合わせること）
			int effectTexNo = 0; // デフォルト
			switch (playerObject->type)
			{
			case PlayerType::Glass:		effectTexNo = 0; break;
			case PlayerType::Concrete:	effectTexNo = 1; break;
			case PlayerType::Plant:		effectTexNo = 2; break;
			case PlayerType::Electric:	effectTexNo = 3; break;
			default:					effectTexNo = 0; break;
			}

			// プレイヤー番号は playerIndex（0ベース）
			XMFLOAT2 pos = playerEffectPos[playerIndex];
			XMFLOAT2 size = { 300.0f, 300.0f };

			Effect_Set(effectTexNo, pos, size);
		}

		player.brokenHistory.clear(); // 履歴もクリアする
		player.evolutionGauge = 0;
		player.breakCount_Glass = 0;
		player.breakCount_Concrete = 0;
		player.breakCount_Plant = 0;
		player.breakCount_Electric = 0;
	}
}


void Attack_Draw(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX) return;

	// 参照を取る
	ATTACK_OBJECT& attackObject = Attack[playerIndex];
	ID3D11ShaderResourceView* tex = g_Attack_Texture[playerIndex];

	// =====================
	// ワールド行列の作成
	// =====================

	// スケーリング行列の作成
	XMMATRIX ScalingMatrix = XMMatrixScaling
	(
		attackObject.scaling.x,
		attackObject.scaling.y,
		attackObject.scaling.z
	);

	// 回転行列の作成
	XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw
	(
		XMConvertToRadians(attackObject.rotation.x),
		XMConvertToRadians(attackObject.rotation.y),
		XMConvertToRadians(attackObject.rotation.z)
	);

	// 平行移動行列の作成
	XMMATRIX TranslationMatrix = XMMatrixTranslation
	(
		attackObject.position.x,
		attackObject.position.y,
		attackObject.position.z
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

	LIGHT light{};
	light.Enable = TRUE;
	// 光の向き（ワールド空間）シェーダー側で単位化して使っている想定
	light.Direction = XMFLOAT4(-0.5f, -1.0f, 0.2f, 0.0f);
	// 拡散光と環境光
	light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	Shader_SetLight(light);

	// シェーダーを描画パイプラインへ設定
	Shader_Begin();

	// 不透明で描画するためブレンドを無効化し、描画カラーのアルファを1に固定する
	SetBlendState(BLENDSTATE_NONE);
	Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	// 頂点シェーダーを描画パイプラインへ設定
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex2* vertex = (Vertex2*)msr.pData;

	// 頂点データを頂点バッファへコピーする
	CopyMemory(&vertex[0], &Attack_vdata[0], sizeof(Vertex2) * NUM_VERTEX);

	// コピー完了
	g_pContext->Unmap(g_VertexBuffer, 0);

	// テクスチャをセット
	g_pContext->PSSetShaderResources(0, 1, &tex);

	// 頂点バッファをセット
	UINT stride = sizeof(Vertex2);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// インデックスバッファをセット
	g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 描画するポリゴンの種類をセット 3頂点でポリゴン1枚として表示
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_pContext->DrawIndexed(6 * 6, 0, 0);

	SetBlendState(BLENDSTATE_ALPHA);
}

void AttackPlayerCollisions()
{
	// 各プレイヤーの攻撃オブジェクトをループして、他プレイヤー全員に当たり判定を行う
	for (int atk = 0; atk < PLAYER_MAX; ++atk)
	{
		if (atk < 0 || atk >= PLAYER_MAX) continue; // 追加の安全チェック
		ATTACK_OBJECT& attackObject = Attack[atk];

		// player は既存の GetPlayer を使ってヌルチェック
		PLAYEROBJECT* attackerPtr = GetPlayer(atk);
		if (attackerPtr == nullptr) continue;
		PLAYEROBJECT& attacker = *attackerPtr;

		if (!attacker.isAttacking) continue;	// 攻撃中のみ判定

		// 攻撃オブジェクトと攻撃者の AABB を更新
		CalculateAABB(attackObject.boundingBox, attackObject.position, attackObject.scaling);
		CalculateAABB(attacker.boundingBox, attacker.position, attacker.scaling);

		// 攻撃者の向きベクトルを更新（rotation.y から算出）
		{
			float rad = XMConvertToRadians(attacker.rotation.y);
			attacker.dir.x = sinf(rad);
			attacker.dir.z = cosf(rad);
		}

		// --- プレイヤー側で使っている描画スケール・ヒットボックス比率と合わせる ---
		const float RENDER_SCALE = 2.0f;
		const float HITBOX_HEIGHT_SCALE = 1.0f;
		// Polygon3D と同じ短辺/長辺定義を使う
		const float HITBOX_SHORT = 0.35f;
		const float HITBOX_LONG  = 0.65f;

		// 攻撃が当たる対象として他プレイヤー全員をチェック
		for (int def = 0; def < PLAYER_MAX; ++def)
		{
			if (def == atk) continue; // 自分には当たらない

			PLAYEROBJECT* defender = GetPlayer(def);
			if (defender == nullptr) continue;
			if (!defender->active) continue;
			// 被弾中や無敵ならスキップ
			if (defender->isInvincible) continue;

			// --- defender の向きから短辺/長辺を決める（Polygon3D と同様） ---
			float radDef = XMConvertToRadians(defender->rotation.y);
			float defFacingX = sinf(radDef);
			float defFacingZ = cosf(radDef);
			bool defFacingZDominant = fabsf(defFacingZ) >= fabsf(defFacingX);

			float widthScale  = defFacingZDominant ? HITBOX_SHORT : HITBOX_LONG;	// X方向スケール
			float depthScale  = defFacingZDominant ? HITBOX_LONG  : HITBOX_SHORT;	// Z方向スケール

			// 第2形態 第3形態はXとZ同じにする
			if (defender->form == Form::FirstEvolution || defender->form == Form::SecondEvolution)
			{
				widthScale = 0.25f;
				depthScale = 0.25f;
			}

			// defender 用のヒットボックススケールを計算して AABB を作る
			XMFLOAT3 defenderHitboxScaling =
			{
				defender->scaling.x * RENDER_SCALE * widthScale,
				defender->scaling.y * RENDER_SCALE * HITBOX_HEIGHT_SCALE,
				defender->scaling.z * RENDER_SCALE * depthScale
			};
			CalculateAABB(defender->boundingBox, defender->position, defenderHitboxScaling);

			// 判定（defender AABB と 攻撃オブジェクト AABB）
			MTV col = CalculateAABBMTV(defender->boundingBox, attackObject.boundingBox);

			if (col.isColliding)
			{
				// ノックバック（攻撃者の向きと攻撃力を使用）
				defender->position.x += attacker.dir.x * attacker.power;
				// defender->position.y += attacker->power / 3.0f;
				defender->position.z += attacker.dir.z * attacker.power;

				// ダメージ用変数
				float rawDamage = attacker.attack * defender->defense;

				// ダメージ（防御で軽減）
				defender->hp -= rawDamage;
				if (defender->hp < 0.0f) defender->hp = 0.0f;

				// スタンゲージ増加
				defender->stunGauge += 0.5f;

				// ダメージ数字を表示（頭上にオフセット）
				int dmgInt = static_cast<int>(rawDamage + 0.5f);
				XMFLOAT3 hitPos = defender->position;
				hitPos.y += defender->scaling.y + 0.3f;
				SetDamageText(hitPos, dmgInt, TextColor::Blue);

				// ダメージフラグ・タイマー（アニメ／UI 用）
				defender->isAttacked = true;
				defender->attackedTimer = 0.0f;

				// 再計算
				CalculateAABB(defender->boundingBox, defender->position, defenderHitboxScaling);
				CalculateAABB(attackObject.boundingBox, attackObject.position, attackObject.scaling);
			}
		}
	}
}

ATTACK_OBJECT* GetAttack(int playerIndex)
{
	// 範囲チェック 0未満 または 4以上なら nullptr を返す
	if (playerIndex < 0 || playerIndex >= PLAYER_MAX)
	{
		return nullptr;
	}

	return &Attack[playerIndex];
}