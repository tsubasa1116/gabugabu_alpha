// input.h

#pragma once

struct GamepadInput
{
	// ボタン
	bool A = false;
	bool B = false;
	bool X = false;
	bool Y = false;
	bool L = false;
	bool R = false;
	bool ZL = false;
	bool ZR = false;
	bool Minus = false;
	bool Plus = false;
	bool LStickPush = false;
	bool RStickPush = false;

	// スティック（正規化済み -1.0 ～ +1.0）
	float LStickX = 0.0f;
	float LStickY = 0.0f;/*
	float RStickX = 0.0f;
	float RStickY = 0.0f;*/

	// 十字キー
	bool Up = false;
	bool Down = false;
	bool Left = false;
	bool Right = false;
};

// 最大4人入力
extern GamepadInput g_Input[4];
