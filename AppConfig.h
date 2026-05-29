#pragma once

#include "raylib.h"

namespace cfg {
    constexpr int WINDOW_WIDTH = 1366;
    constexpr int WINDOW_HEIGHT = 768;

    constexpr int INITIAL_GRID_COLS = 100;
    constexpr int INITIAL_GRID_ROWS = 100;
    constexpr int MIN_GRID_COLS = 25;
    constexpr int MAX_GRID_COLS = 500;
    constexpr int MIN_GRID_ROWS = 25;
    constexpr int MAX_GRID_ROWS = 500;
    constexpr int CELL_SIZE = 10;

    constexpr int INITIAL_ALIVE_PERCENT = 42;
    constexpr int MIN_INITIAL_ALIVE_PERCENT = 0;
    constexpr int MAX_INITIAL_ALIVE_PERCENT = 80;

    constexpr int SIDE_PANEL_WIDTH = 260;
    constexpr int SIDE_PANEL_MARGIN = 16;

    constexpr float INITIAL_CAMERA_ZOOM = 0.7f;
    constexpr float MIN_CAMERA_ZOOM = 0.1f;
    constexpr float MAX_CAMERA_ZOOM = 10.0f;

    constexpr float GRID_VISIBILITY_THRESHOLD = 6.0f;

    constexpr int BASE_GENERATION_TPS = 4;
    constexpr int MIN_SPEED_MULTIPLIER = 1;
    constexpr int MAX_SPEED_MULTIPLIER = 8;
    constexpr int MAX_SIMULATION_STEPS_PER_FRAME = 2;

    constexpr Color BACKGROUND_COLOR = { 18, 18, 22, 255 };
    constexpr Color PANEL_COLOR = { 32, 32, 38, 230 };
    constexpr Color PANEL_BORDER_COLOR = { 74, 74, 84, 255 };
    constexpr Color GRID_LINE_COLOR = { 55, 55, 64, 255 };
    constexpr Color GRID_BORDER_COLOR = { 95, 95, 105, 255 };
    constexpr Color ALIVE_CELL_COLOR = { 90, 205, 130, 255 };
    constexpr Color DEAD_CELL_COLOR = { 25, 25, 30, 255 };
    constexpr Color TEXT_MAIN_COLOR = { 235, 235, 240, 255 };
    constexpr Color TEXT_MUTED_COLOR = { 165, 165, 175, 255 };
    constexpr Color ACCENT_COLOR = { 100, 170, 255, 255 };
    constexpr Color DANGER_COLOR = { 255, 105, 105, 255 };
    constexpr Color BUTTON_COLOR = { 62, 62, 70, 255 };
    constexpr Color BUTTON_HOVER_COLOR = { 82, 82, 92, 255 };
    constexpr Color SLIDER_TRACK_COLOR = { 80, 80, 88, 255 };
    constexpr Color SLIDER_KNOB_COLOR = { 170, 170, 180, 255 };
}

inline Rectangle GetSidePanelBounds() {
    return Rectangle{
        static_cast<float>(cfg::WINDOW_WIDTH - cfg::SIDE_PANEL_WIDTH),
        0.0f,
        static_cast<float>(cfg::SIDE_PANEL_WIDTH),
        static_cast<float>(cfg::WINDOW_HEIGHT)
    };
}

inline bool IsMouseOverSidePanel() {
    return CheckCollisionPointRec(GetMousePosition(), GetSidePanelBounds());
}
