
//transition.h

#pragma once
#include "direct3d.h"
#include "sprite.h"
#include "Manager.h"

enum TRANSITION_STATE
{
    TRANSITION_NONE = 0,
    TRANSITION_IN,
    TRANSITION_OUT,
};

struct TransitionObject
{
    TRANSITION_STATE state;
    float frame;
    float progress;     // 0.0 Å® 1.0
    XMFLOAT4 color;
    SCENE scene;

    bool sceneChanged;
};

void Transition_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Transition_Finalize();
void Transition_Update();
void Transition_Draw();

void SetTransition(float frame, XMFLOAT4 color, TRANSITION_STATE state, SCENE scene);
TRANSITION_STATE GetTransitionState();
