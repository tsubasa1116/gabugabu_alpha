#pragma once
#include <SDL3/SDL.h>
#include "input.h"

void Gamepad_Initialize();
void Gamepad_Update();
void Gamepad_Finalize();

void TriggerVibration(int playerIndex, float lowFreq, float highFreq, int ms);
void StopVibration(int playerIndex);

int Gamepad_GetCount();

const char* Gamepad_GetName(int playerIndex);