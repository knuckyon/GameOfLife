#include "GameLogic.h"

#include <algorithm>
#include "raylib.h"
#include "AppConfig.h"
#include "CameraController.h"
#include "Utils.h"

#ifdef _OPENMP
#include <omp.h>
#endif

namespace {
    int CountNeighbors(const GameState& game, int x, int y) {
        int count = 0;

        const int rows = game.rows;
        const int cols = game.cols;
        const auto& cells = game.cells;

        for (int offsetY = -1; offsetY <= 1; ++offsetY) {
            const int neighborY = y + offsetY;

            if (neighborY < 0 || neighborY >= rows) {
                continue;
            }

            for (int offsetX = -1; offsetX <= 1; ++offsetX) {
                if (offsetX == 0 && offsetY == 0) {
                    continue;
                }

                const int neighborX = x + offsetX;

                if (neighborX < 0 || neighborX >= cols) {
                    continue;
                }

                const int neighborIndex = neighborY * cols + neighborX;

                if (cells[static_cast<std::size_t>(neighborIndex)] != 0) {
                    ++count;
                }
            }
        }

        return count;
    }
}

void InitializeGame(GameState& game) {
    ResizeGrid(game, cfg::INITIAL_GRID_ROWS, cfg::INITIAL_GRID_COLS);
    ResetCamera(game);
}

void ResizeGrid(GameState& game, int rows, int cols) {
    game.rows = ClampInt(rows, cfg::MIN_GRID_ROWS, cfg::MAX_GRID_ROWS);
    game.cols = ClampInt(cols, cfg::MIN_GRID_COLS, cfg::MAX_GRID_COLS);
    game.pendingRows = game.rows;
    game.pendingCols = game.cols;

    game.cells.assign(static_cast<std::size_t>(game.rows * game.cols), 0);
    game.nextCells.assign(static_cast<std::size_t>(game.rows * game.cols), 0);
    game.generation = 0;
    game.aliveCells = 0;
    game.isDrawing = false;
}

void ClearGrid(GameState& game) {
    std::fill(game.cells.begin(), game.cells.end(), 0);
    std::fill(game.nextCells.begin(), game.nextCells.end(), 0);
    game.generation = 0;
    game.aliveCells = 0;
}

void RandomizeGrid(GameState& game) {
    for (std::uint8_t& cell : game.cells) {
        cell = GetRandomValue(0, 100) < game.initialAlivePercent ? 1 : 0;
    }

    game.generation = 0;
    CountAliveCells(game);
}

void StepSimulation(GameState& game) {
    for (int y = 0; y < game.rows; ++y) {
        for (int x = 0; x < game.cols; ++x) {
            const int neighbors = CountNeighbors(game, x, y);
            const bool alive = IsCellAlive(game, x, y);

            game.nextCells[CellIndex(game, x, y)] =
                alive ? (neighbors == 2 || neighbors == 3) : (neighbors == 3);
        }
    }

    game.cells.swap(game.nextCells);
    ++game.generation;
    CountAliveCells(game);
}

void StepSimulationOpenMP(GameState& game) {
    int aliveTotal = 0;

#ifdef _OPENMP
#pragma omp parallel for reduction(+:aliveTotal) schedule(static)
#endif
    for (int y = 0; y < game.rows; ++y) {
        for (int x = 0; x < game.cols; ++x) {
            const int index = y * game.cols + x;

            const int neighbors = CountNeighbors(game, x, y);
            const bool alive = game.cells[static_cast<std::size_t>(index)] != 0;

            const std::uint8_t nextValue =
                alive
                ? static_cast<std::uint8_t>(neighbors == 2 || neighbors == 3)
                : static_cast<std::uint8_t>(neighbors == 3);

            game.nextCells[static_cast<std::size_t>(index)] = nextValue;

            if (nextValue != 0) {
                ++aliveTotal;
            }
        }
    }

    game.cells.swap(game.nextCells);
    ++game.generation;
    game.aliveCells = aliveTotal;
}

void CountAliveCells(GameState& game) {
    int total = 0;

    for (std::uint8_t cell : game.cells) {
        if (cell != 0) {
            ++total;
        }
    }

    game.aliveCells = total;
}

void CountAliveCellsOpenMP(GameState& game) {
    int total = 0;
    const int totalCells = game.rows * game.cols;

#ifdef _OPENMP
#pragma omp parallel for reduction(+:total) schedule(static)
#endif
    for (int i = 0; i < totalCells; ++i) {
        total += game.cells[static_cast<std::size_t>(i)] != 0 ? 1 : 0;
    }

    game.aliveCells = total;
}