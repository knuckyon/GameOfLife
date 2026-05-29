#include "Renderer.h"

#include "raylib.h"
#include "AppConfig.h"
#include "CameraController.h"
#include "UI.h"

namespace {
    void DrawCellField(const GameState& game) {
        for (int y = 0; y < game.rows; ++y) {
            for (int x = 0; x < game.cols; ++x) {
                const Rectangle cellRect = Rectangle{
                    static_cast<float>(x * cfg::CELL_SIZE),
                    static_cast<float>(y * cfg::CELL_SIZE),
                    static_cast<float>(cfg::CELL_SIZE),
                    static_cast<float>(cfg::CELL_SIZE)
                };

                DrawRectangleRec(cellRect, IsCellAlive(game, x, y) ? cfg::ALIVE_CELL_COLOR : cfg::DEAD_CELL_COLOR);
            }
        }
    }

    void DrawGridLines(const GameState& game) {
        if (!IsGridActuallyVisible(game)) {
            return;
        }

        for (int x = 0; x <= game.cols; ++x) {
            DrawLine(
                x * cfg::CELL_SIZE,
                0,
                x * cfg::CELL_SIZE,
                game.rows * cfg::CELL_SIZE,
                cfg::GRID_LINE_COLOR
            );
        }

        for (int y = 0; y <= game.rows; ++y) {
            DrawLine(
                0,
                y * cfg::CELL_SIZE,
                game.cols * cfg::CELL_SIZE,
                y * cfg::CELL_SIZE,
                cfg::GRID_LINE_COLOR
            );
        }
    }

    void DrawGridBorder(const GameState& game) {
        DrawRectangleLines(
            0,
            0,
            game.cols * cfg::CELL_SIZE,
            game.rows * cfg::CELL_SIZE,
            cfg::GRID_BORDER_COLOR
        );
    }
}

void DrawGame(const GameState& game) {
    BeginDrawing();
    ClearBackground(cfg::BACKGROUND_COLOR);

    BeginMode2D(game.camera);
    DrawCellField(game);
    DrawGridLines(game);
    DrawGridBorder(game);
    EndMode2D();

    DrawSidePanel(game);
    DrawHelpPopup(game);

    EndDrawing();
}
