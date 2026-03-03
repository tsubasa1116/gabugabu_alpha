#pragma once

// カットインの初期化・終了
void Cutin_Initialize();
void Cutin_Finalize();

// カットインの更新・描画
void Cutin_Update();
void Cutin_Draw();

// カットインを発動する関数（playerIndex: 0~3）
void Set_Cutin(int playerIndex, int typeIndex);