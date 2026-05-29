#include "CameraController.h"

#include "raylib.h"
#include "raymath.h"
#include "AppConfig.h"
#include "Utils.h"

void UpdateCameraControls(GameState& game) {
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        const Vector2 delta = GetMouseDelta();
        game.camera.target = Vector2Subtract(
            game.camera.target,
            Vector2Scale(delta, 1.0f / game.camera.zoom)
        );
    }

    const float wheel = GetMouseWheelMove();
    if (wheel != 0.0f && !IsMouseOverSidePanel()) {
        const Vector2 mouseWorldBeforeZoom = GetScreenToWorld2D(GetMousePosition(), game.camera);

        game.camera.zoom += wheel * 0.1f * game.camera.zoom;
        game.camera.zoom = static_cast<float>(ClampInt(
            static_cast<int>(game.camera.zoom * 1000.0f),
            static_cast<int>(cfg::MIN_CAMERA_ZOOM * 1000.0f),
            static_cast<int>(cfg::MAX_CAMERA_ZOOM * 1000.0f)
        )) / 1000.0f;

        const Vector2 mouseWorldAfterZoom = GetScreenToWorld2D(GetMousePosition(), game.camera);
        game.camera.target = Vector2Add(
            game.camera.target,
            Vector2Subtract(mouseWorldBeforeZoom, mouseWorldAfterZoom)
        );
    }
}

void ResetCamera(GameState& game) {
    game.camera.target = Vector2{
        game.cols * cfg::CELL_SIZE / 2.0f,
        game.rows * cfg::CELL_SIZE / 2.0f
    };
    game.camera.offset = Vector2{
        (cfg::WINDOW_WIDTH - cfg::SIDE_PANEL_WIDTH) / 2.0f,
        cfg::WINDOW_HEIGHT / 2.0f
    };
    game.camera.rotation = 0.0f;
    game.camera.zoom = cfg::INITIAL_CAMERA_ZOOM;
}

bool IsGridActuallyVisible(const GameState& game) {
    return game.showGrid && (cfg::CELL_SIZE * game.camera.zoom >= cfg::GRID_VISIBILITY_THRESHOLD);
}
