#include "InputController.h"

#include "raylib.h"
#include "AppConfig.h"
#include "CameraController.h"
#include "GameLogic.h"

#include <algorithm>

void HandleKeyboardInput(GameState& game) {
    if (IsKeyPressed(KEY_SPACE)) {
        game.isPaused = !game.isPaused;
    }

    if (IsKeyPressed(KEY_N)) {
        StepSimulation(game);
    }

    if (IsKeyPressed(KEY_C)) {
        ClearGrid(game);
    }

    if (IsKeyPressed(KEY_R)) {
        RandomizeGrid(game);
    }

    if (IsKeyPressed(KEY_G)) {
        game.showGrid = !game.showGrid;
    }

    if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) {
        game.speedMultiplier = std::min(game.speedMultiplier + 1, cfg::MAX_SPEED_MULTIPLIER);
    }

    if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {
        game.speedMultiplier = std::max(game.speedMultiplier - 1, cfg::MIN_SPEED_MULTIPLIER);
    }

    if (IsKeyPressed(KEY_F)) {
        ResetCamera(game);
    }

    if (IsKeyPressed(KEY_H) || IsKeyPressed(KEY_F1)) {
        game.helpVisible = !game.helpVisible;
    }
}

void HandleCellDrawing(GameState& game) {
    if (IsMouseOverSidePanel()) {
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        game.isDrawing = true;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        game.isDrawing = false;
    }

    if (!game.isDrawing) {
        return;
    }

    // Update drawValue every frame so that switching from LMB to RMB mid-stroke
    // immediately switches between drawing and erasing.
    game.drawValue = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    const Vector2 worldPosition = GetScreenToWorld2D(GetMousePosition(), game.camera);
    const int cellX = static_cast<int>(worldPosition.x) / cfg::CELL_SIZE;
    const int cellY = static_cast<int>(worldPosition.y) / cfg::CELL_SIZE;

    if (cellX >= 0 && cellX < game.cols && cellY >= 0 && cellY < game.rows) {
        SetCellAlive(game, cellX, cellY, game.drawValue);
    }
}