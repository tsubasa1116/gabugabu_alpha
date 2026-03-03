// Game.h
#pragma once

#include "direct3d.h"

// グローバル変数か、管理クラスのメンバ変数として用意
extern float g_hitStopTimer;

//// ヒットストップを開始する関数
//void StartHitStop(float duration) {
//	g_hitStopTimer = duration;
//}

void StartHitStop(float duration);

void Game_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Game_Finalize();
void Game_Update();
void Game_Draw();



