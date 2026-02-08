#pragma once

#include <DirectXMath.h>
using namespace DirectX;

#define SIZE_DEFAULT { 1.0f, 1.0f ,1.0f }
#define COLL 0.9f

namespace color
{
    constexpr XMFLOAT4 white(1, 1, 1, 1);
    constexpr XMFLOAT4 black(0, 0, 0, 1);
    constexpr XMFLOAT4 red(1, 0, 0, 1);
    constexpr XMFLOAT4 green(0, 1, 0, 1);
    constexpr XMFLOAT4 blue(0, 0, 1, 1);
    constexpr XMFLOAT4 yellow(1, 1, 0, 1);
    constexpr XMFLOAT4 sky(0, 1, 1, 1);
    constexpr XMFLOAT4 gray(0.5f, 0.5f, 0.5f, 1);
    constexpr XMFLOAT4 purple(0.8f, 0.4f, 0.8f, 1.0f);
}
