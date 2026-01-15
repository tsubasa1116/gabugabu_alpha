
//swipe.h

#pragma once
#include "direct3d.h"
#include "sprite.h"
#include "Manager.h"

enum SWIPE_STATE
{
    SWIPE_NONE = 0,
    SWIPE_IN,
    SWIPE_OUT,
};

struct SwipeObject
{
    SWIPE_STATE state;
    float frame;
    float progress;     // 0.0 Å® 1.0
    XMFLOAT4 color;
    SCENE scene;

    bool sceneChanged;
};

void Swipe_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Swipe_Finalize();
void Swipe_Update();
void Swipe_Draw();

void SetSwipe(int frame, XMFLOAT4 color, SWIPE_STATE state, SCENE scene);
SWIPE_STATE GetSwipeState();