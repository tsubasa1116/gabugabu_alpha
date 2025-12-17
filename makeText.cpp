#include "makeText.h"
#include "direct3d.h"
#include "debug_ostream.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

extern ID3D11Device* Direct3D_GetDevice();
extern IDXGISwapChain* Direct3D_GetSwapChain();

static ComPtr<ID2D1Factory1>      g_d2dFactory;
static ComPtr<IDWriteFactory>     g_dwriteFactory;
static ComPtr<ID2D1Device>        g_d2dDevice;
static ComPtr<ID2D1DeviceContext> g_d2dContext;
static ComPtr<ID2D1Bitmap1>       g_d2dTarget;

static ComPtr<IDWriteTextFormat> g_textFormat;

bool Initialize_MakeText()
{
	// D2Dファクトリの作成
	D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		__uuidof(ID2D1Factory1),
		nullptr,
		(void**)&g_d2dFactory
	);

	// DWriteファクトリの作成
	DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		(IUnknown**)&g_dwriteFactory
	);
	
	// DXGIデバイスの取得
	ComPtr<IDXGIDevice> dxgiDevice;
	Direct3D_GetDevice()->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice);
	
	// D2Dデバイスの作成
	g_d2dFactory->CreateDevice(dxgiDevice.Get(), &g_d2dDevice);
	
	// D2Dデバイスコンテキストの作成
	g_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &g_d2dContext);
	
	return true;
}

bool CreateRenderTarget_MakeText()
{
	ComPtr<IDXGISurface> backBuffer;
	Direct3D_GetSwapChain()->GetBuffer(0, __uuidof(IDXGISurface), &backBuffer);

	D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
	);

	g_d2dContext->CreateBitmapFromDxgiSurface(backBuffer.Get(), &props, &g_d2dTarget);

	g_d2dContext->SetTarget(g_d2dTarget.Get());

	return true;
}

void BeginDraw_MakeText()
{
	g_d2dContext->BeginDraw();
}

void EndDraw_MakeText()
{
	g_d2dContext->EndDraw();
}

D2D1_COLOR_F D2DColor(TextColor color)
{
	switch (color)
	{
	case TextColor::White:  return D2D1::ColorF(D2D1::ColorF::White);
	case TextColor::Black:  return D2D1::ColorF(D2D1::ColorF::Black);
	case TextColor::Red:    return D2D1::ColorF(D2D1::ColorF::Red);
	case TextColor::Green:  return D2D1::ColorF(D2D1::ColorF::Green);
	case TextColor::Blue:   return D2D1::ColorF(D2D1::ColorF::Blue);
	case TextColor::Yellow: return D2D1::ColorF(D2D1::ColorF::Yellow);
	}
	return D2D1::ColorF(D2D1::ColorF::White);
}

void DrawTextEx(
	const wchar_t* text,
	float x, float y,
	float fontSize,
	const wchar_t* fontName,
	TextColor color
)
{
	// フォント作成
	ComPtr<IDWriteTextFormat> format;
	g_dwriteFactory->CreateTextFormat(
		fontName,
		nullptr,
		DWRITE_FONT_WEIGHT_THIN,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		fontSize,
		L"ja-JP",
		&format
	);

	BeginDraw_MakeText();

	ComPtr<ID2D1SolidColorBrush> brush;
	g_d2dContext->CreateSolidColorBrush(D2DColor(color), &brush);

	D2D1_RECT_F rect = D2D1::RectF(x, y, x + 2000, y + 300);

	g_d2dContext->DrawText(
		text,
		(UINT32)wcslen(text),
		format.Get(),
		rect,
		brush.Get()
	);

	EndDraw_MakeText();
}
