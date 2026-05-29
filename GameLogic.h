#pragma once

#include "GameState.h"

void InitializeGame(GameState& game);
void ResizeGrid(GameState& game, int rows, int cols);
void ClearGrid(GameState& game);
void RandomizeGrid(GameState& game);

void StepSimulation(GameState& game);
void StepSimulationOpenMP(GameState& game);

void CountAliveCells(GameState& game);
void CountAliveCellsOpenMP(GameState& game);
