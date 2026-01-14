#pragma once

#include <d2d1_1.h>
#include <dwrite.h>
#include <dxgi.h>
#include <wrl.h>

enum TextFont
{
	Meiryo,
	MeiryoBold
};

enum TextColor
{
	White,
	Black,
	Red,
	Green,
	Blue,
	Yellow
};

//namespace FontList
//{
//	const std::wstring FontPath[] =
//	{
//		L"asset\\font\\misaki_gothic.ttf",
//	};
//}

bool Initialize_MakeText();
bool CreateRenderTarget_MakeText();
void BeginDraw_MakeText();
void EndDraw_MakeText();
void DrawTextEx(
	const wchar_t* text,
	float x, float y,
	float fontSize,
	const wchar_t* fontName,
	TextColor color
);
D2D1_COLOR_F D2DColor(TextColor color);