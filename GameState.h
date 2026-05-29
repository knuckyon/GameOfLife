#pragma once

#include <vector>
#include <cstdint>
#include "raylib.h"
#include "AppConfig.h"

struct GameState {
    int rows = cfg::INITIAL_GRID_ROWS;
    int cols = cfg::INITIAL_GRID_COLS;
    int pendingRows = cfg::INITIAL_GRID_ROWS;
    int pendingCols = cfg::INITIAL_GRID_COLS;
    int initialAlivePercent = cfg::INITIAL_ALIVE_PERCENT;

    std::vector<std::uint8_t> cells;
    std::vector<std::uint8_t> nextCells;

    bool isPaused = true;
    bool showGrid = true;
    bool helpVisible = false;

    int generation = 0;
    int aliveCells = 0;
    int speedMultiplier = 1;

    bool isDrawing = false;
    bool drawValue = true;

    Camera2D camera{};
};

inline int CellIndex(const GameState& game, int x, int y) {
    return y * game.cols + x;
}

inline bool IsCellAlive(const GameState& game, int x, int y) {
    return game.cells[CellIndex(game, x, y)] != 0;
}

inline void SetCellAlive(GameState& game, int x, int y, bool value) {
    game.cells[CellIndex(game, x, y)] = value ? 1 : 0;
}
