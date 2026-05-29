#pragma once

#include "GameState.h"

void UpdateCameraControls(GameState& game);
void ResetCamera(GameState& game);
bool IsGridActuallyVisible(const GameState& game);
