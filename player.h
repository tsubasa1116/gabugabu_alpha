// player.h

#pragma once

#include <vector>
#include <d3d11.h>
#include "collider.h"
#include "Building.h"
#include "special.h"

// マクロ定義
#define	PLAYER_MAX				(4)	// プレイヤー最大数
#define	DELTA_TIME	 (1.0f / 60.0f)	// デルタタイム（秒）

#define	PLAYER_MAX_HP				(500.0f)// プレイヤー 最大HP
#define	PLAYER_MAX_SATIETY			(7.0f)	// プレイヤー 最大満腹度
#define	PLAYER_EVOLUTION_GAUGE_RATE	(0.5f)	// プレイヤー 進化ゲージ増加率

#define	EVOLUTIONGAUGE_MAX	(1.0f)	// 進化ゲージ最大値
#define	ATTACKING_TIME		(0.2f)	// 攻撃持続時間
#define	ATTACKED_TIME		(0.5f)	// ダメージ持続時間
#define	HEALING_TIME		(2.0f)	// 回復持続時間
#define	EVOLVING_TIME		(4.0f)	// 進化時間
#define	STUNGAUGE_MAX		(10)	// スタンゲージ最大値
#define	STUN_TIME			(5.0f)	// スタン持続時間
#define	DOWN_TIME			(3.0f)	// ダウン持続時間
#define	POISON_TIME			(5.0f)	// 毒持続時間
#define	EGG_BREAKING_TIME	(0.3f)	// 卵エフェクトが割れる再生時間

#define	PLAYER_VERTEX	(6)		// 一面のみの頂点数
#define COORDINATE		(0.5f)	// デフォルト (0.5f)
#define TEXCOORD		(1.0f)	// デフォルト (1.0f)

#define PLAYER_SE_COUNT		(4)	// プレイヤー SEの数

enum class PlayerDir
{
	Down = 0,
	Down_Left,
	Left,
	Up_Left,
	Up,
	Up_Right,
	Right,
	Down_Right,
	Max
};

enum class Form
{
	First = 0,	// 第1形態
	Second,		// 第2形態
	Third		// 第3形態
};

enum class PlayerType
{
	None,		// 未設定
	Glass,		// ガラス
	Concrete,	// コンクリ
	Plant,		// 植物
	Electricity,// 電気
	Max
};

// プレイヤーオブジェクト専用の構造体
struct PLAYEROBJECT
{
	XMFLOAT3 position;		// 座標
	XMFLOAT3 velocity;		// 
	XMFLOAT3 oldPosition;	// 過去の座標
	XMFLOAT3 rotation;		// 回転角度
	XMFLOAT3 scaling;		// 拡大率
	AABB boundingBox;		// 当たり判定
	float hp;				// 体力
	float attack;			// 攻撃力
	float power;			// ふっとばしのパワー
	float weight;			// 重さ （重いほどふっとばされにくい）
	float speed;			// スピード
	float defense;			// 防御率
	XMFLOAT3 dir;			// 向き
	int stock;				// 残機
	int rank;				// 順位
	bool active;			// 生存フラグ
	float satiety;			// 満腹度

	bool isAttacking;		// 攻撃中かどうか
	float attackTimer;		// 攻撃中の経過時間

	bool isAttacked;		// 被弾中かどうか
	float attackedTimer;	// 被弾中の経過時間

	bool isHealing;			// 回復中かどうか
	float healingTimer;		// 回復中の経過時間

	bool isEvolving;		// 進化中かどうか
	float evolvingTimer;	// 進化中の経過時間

	bool useSkill;			// スキル中かどうか
	float skillTimer;		// スキル中の経過時間
	float skillCoolTimer;	// スキルクール中の経過時間
	bool skillAnimation;	// スキルアニメーション中かどうか

	bool useSpecial;		// スペシャル中かどうか
	float specialTimer;		// スペシャル中の経過時間

	bool isInvincible;		// 無敵中かどうか
	float invincibleTimer;	// 無敵中の経過時間

	float stunGauge;		// スタンゲージ
	bool isStunning;		// スタン中かどうか
	float stunTimer;		// スタン中の経過時間

	bool isDown;			// ダウン中かどうか
	float downTimer;		// ダウン中の経過時間

	bool isPoisoned;		// 毒状態かどうか
	float poisonTimer;		// 毒の経過時間

	bool duringRespawn;		// リスポーン中かどうか
	float respawnTimer;		// リスポーン中の経過時間

	bool isEggBreaking;		// 卵エフェクトが割れ始める瞬間
	float eggBreakingTimer;	// 卵エフェクトのタイマー

	float moveAngle = 0.0f;	// プレイヤー固有の回転補間用角度
	XMFLOAT3 moveDir = { 0.0f, 0.0f, 0.0f };	// 移動ベクトル
	PlayerDir lastDir;							// 待機時の向き
	bool isMoving = false;						// 移動中かどうか
	bool isShadowEnabled = false;					// 地上にいるかどうか

	Form form;								// 変身形態
	PlayerType type;						// プレイヤーの属性タイプ
	bool isTypeFixed		;				// 進化タイプが固定されたかどうか
	float evolutionGauge;					// 進化ゲージ
	float evolutionGaugeRate;				// 進化ゲージ 倍率
	int breakCount_Glass;					// 破壊した数 ガラス
	int breakCount_Concrete;				// 破壊した数 コンクリート
	int breakCount_Plant;					// 破壊した数 植物
	int breakCount_Electricity;				// 破壊した数 電気
	std::vector<BuildingType> brokenHistory;// 破壊した建物のリスト

	XMFLOAT3 knockback_velocity = { 0.0f, 0.0f, 0.0f };	// 吹き飛ばし用の速度ベクトル
	bool is_knocked_back = false;						// 吹き飛ばし中かどうか
	float knockback_duration = 0.0f;					// 吹き飛ばしの残り時間（フレーム数

	XMFLOAT2 screenPos;	// テキスト描画用スクリーン座標
	bool isOnScreen;

	Circle electricityCircles[SPECIAL_ELECTRICITY_QUANTITY]; // スペシャル 電気の円
	std::vector<GLASS_BOX> glassBoxes; // スペシャル ガラスのミサイルリスト

	XMFLOAT2 moveInput2D;

	int electricityTileIndices[SPECIAL_ELECTRICITY_QUANTITY] = { 0 }; // 電気スペシャル用：選択されたタイルのインデックス
};

void Player_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
static void LoadTextureList(ID3D11Device* pDevice);
void Player_Finalize();
void Player_Update();
void Player_Draw(bool s_IsKonamiCodeEntered);
void Player_DrawHP();

// アニメーション関数
inline void LoopRange(int& animFrame, int start, int count, int advance = 1);

void Ranking(int playerIndex);

void Player_Respawn(int playerIndex);

void Player_DrawStock(int i);
PLAYEROBJECT* GetPlayer(int playerIndex);

void Player_DrawText();

void TriggerbyHPShake(int playerIndex, float amplitude, float duration, float speed);

bool Player_CanUseSpecial(int playerIndex);


