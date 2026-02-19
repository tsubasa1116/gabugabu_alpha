#include "gamepad.h"

//======================================================
// グローバル変数
//======================================================
SDL_Gamepad* g_pGamepad[4] = { nullptr };
int g_GamepadCount = 0;  // 接続されたプロコンの数


//======================================================
// デッドゾーン処理を含むスティック正規化
//======================================================
float NormalizeStickWithDeadZone(Sint16 value)
{
	constexpr float DEAD_ZONE = 4000.0f;
	constexpr float MAX_VALUE = 32767.0f;

	// デッドゾーン
	if (value > -DEAD_ZONE && value < DEAD_ZONE)
		return 0.0f;

	// 正規化
	float v = static_cast<float>(value);

	if (v > 0.0f)
		return (v - DEAD_ZONE) / (MAX_VALUE - DEAD_ZONE);
	else
		return (v + DEAD_ZONE) / (MAX_VALUE - DEAD_ZONE);
}


//======================================================
// 初期化関数
//======================================================
void Gamepad_Initialize()
{
	if (!SDL_Init(SDL_INIT_GAMEPAD))
	{
		SDL_Log("SDL初期化失敗: %s", SDL_GetError());
		return;
	}

	// 接続されているゲームパッドを検出
	int num_joysticks = 0;
	SDL_JoystickID* joysticks = SDL_GetJoysticks(&num_joysticks);

	g_GamepadCount = 0;
	for (int i = 0; i < num_joysticks && g_GamepadCount < 4; i++)
	{
		if (SDL_IsGamepad(joysticks[i]))
		{
			g_pGamepad[g_GamepadCount] = SDL_OpenGamepad(joysticks[i]);
			if (g_pGamepad[g_GamepadCount])
			{
				SDL_Log("Gamepad %d 接続: %s", g_GamepadCount,
					SDL_GetGamepadName(g_pGamepad[g_GamepadCount]));
				g_GamepadCount++;
			}
		}
	}

	SDL_free(joysticks);
}


//======================================================
// 更新関数
//======================================================
void Gamepad_Update()
{
	SDL_UpdateGamepads();

	for (int p = 0; p < g_GamepadCount; p++)
	{
		if (!g_pGamepad[p]) continue;

		// スティック入力
		Sint16 lx = SDL_GetGamepadAxis(g_pGamepad[p], SDL_GAMEPAD_AXIS_LEFTX);
		Sint16 ly = SDL_GetGamepadAxis(g_pGamepad[p], SDL_GAMEPAD_AXIS_LEFTY);

		g_Input[p].LStickX = NormalizeStickWithDeadZone(lx);
		g_Input[p].LStickY = NormalizeStickWithDeadZone(ly);

		// ボタン入力
		g_Input[p].A = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_SOUTH);
		g_Input[p].B = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_EAST);
		g_Input[p].X = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_WEST);
		g_Input[p].Y = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_NORTH);

		g_Input[p].L = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
		g_Input[p].R = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);

		// トリガーをボタンとして扱う（閾値以上で押下判定）
		g_Input[p].ZL = SDL_GetGamepadAxis(g_pGamepad[p], SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > 16384;
		g_Input[p].ZR = SDL_GetGamepadAxis(g_pGamepad[p], SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 16384;

		g_Input[p].Minus = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_BACK);
		g_Input[p].Plus = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_START);

		g_Input[p].LStickPush = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_LEFT_STICK);
		g_Input[p].RStickPush = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_RIGHT_STICK);

		// 十字キー
		g_Input[p].Up = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_DPAD_UP);
		g_Input[p].Down = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_DPAD_DOWN);
		g_Input[p].Left = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_DPAD_LEFT);
		g_Input[p].Right = SDL_GetGamepadButton(g_pGamepad[p], SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
	}
}


//======================================================
// 終了処理関数
//======================================================
void Gamepad_Finalize()
{
	for (int i = 0; i < g_GamepadCount; i++)
	{
		if (g_pGamepad[i])
		{
			SDL_CloseGamepad(g_pGamepad[i]);
			g_pGamepad[i] = nullptr;
		}
	}

	SDL_Quit();
	g_GamepadCount = 0;
}


//======================================================
// 振動を開始する関数
//======================================================
void TriggerVibration(int playerIndex, float lowFreq, float highFreq, int ms)
{

	// プレイヤーインデックスの範囲チェック
	if (playerIndex < 0 || playerIndex >= g_GamepadCount)
		return;

	// ゲームパッドが存在しない場合は何もしない
	if (!g_pGamepad[playerIndex])
		return;

	// 振動の強さは0.0-1.0
	// SDL3では0-65535の範囲に変換する必要がある
	Uint16 low = static_cast<Uint16>(lowFreq * 65535.0f);
	Uint16 high = static_cast<Uint16>(highFreq * 65535.0f);

	// 振動を実行
	SDL_RumbleGamepad(g_pGamepad[playerIndex], low, high, ms);
}


//======================================================
// 振動を停止する関数
//======================================================
void StopVibration(int playerIndex)
{
	// プレイヤーインデックスの範囲チェック
	if (playerIndex < 0 || playerIndex >= g_GamepadCount)
		return;

	// ゲームパッドが存在しない場合は何もしない
	if (!g_pGamepad[playerIndex])
		return;

	// 振動を停止（両方のモーターを0に設定）
	SDL_RumbleGamepad(g_pGamepad[playerIndex], 0, 0, 0);
}


//======================================================
// ゲームパッド数の取得
//======================================================
int Gamepad_GetCount()
{
	return g_GamepadCount;
}


//======================================================
// ゲームパッド名の取得
//======================================================
const char* Gamepad_GetName(int playerIndex)
{
	// 範囲チェック
	if (playerIndex < 0 || playerIndex >= g_GamepadCount)
		return "Invalid Index";

	// ゲームパッドが存在しない場合
	if (!g_pGamepad[playerIndex])
		return "No Gamepad";

	// SDL_GetGamepadName() で名前を取得
	const char* name = SDL_GetGamepadName(g_pGamepad[playerIndex]);

	// 名前が取得できなかった場合
	if (!name)
		return "Unknown";

	return name;
}