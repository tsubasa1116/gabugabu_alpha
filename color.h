#pragma once

#include <DirectXMath.h>
using namespace DirectX;

#include <thread>
#include <atomic>

#define SIZE_DEFAULT { 1.0f, 1.0f ,1.0f }
#define COLL 0.9f
#define	SCREEN_WIDTH ((float)Direct3D_GetBackBufferWidth())
#define	SCREEN_HEIGHT ((float)Direct3D_GetBackBufferHeight())
#define	SCREEN_ADJUST_X (SCREEN_WIDTH / 1280.0f)
#define	SCREEN_ADJUST_Y (SCREEN_HEIGHT / 720.0f)
namespace color
{
    constexpr XMFLOAT4 white(1, 1, 1, 1);
    constexpr XMFLOAT4 black(0, 0, 0, 1);
    constexpr XMFLOAT4 red(1, 0, 0, 1);
    constexpr XMFLOAT4 green(0, 1, 0, 1);
    constexpr XMFLOAT4 blue(0, 0, 1, 1);
    constexpr XMFLOAT4 yellow(1, 1, 0, 1);
    constexpr XMFLOAT4 sky(0, 1, 1, 1);
    constexpr XMFLOAT4 gray(0.2f, 0.2f, 0.2f, 1);
    constexpr XMFLOAT4 purple(0.73f, 0.06f, 0.95f, 1.0f);
}
