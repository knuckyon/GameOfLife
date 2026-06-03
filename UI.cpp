#include "UI.h"

#include <string>
#include <algorithm>
#include "raylib.h"
#include "AppConfig.h"
#include "CameraController.h"
#include "GameLogic.h"
#include "Utils.h"

namespace {
    constexpr int PANEL_X = cfg::WINDOW_WIDTH - cfg::SIDE_PANEL_WIDTH + cfg::SIDE_PANEL_MARGIN;
    constexpr int CONTROL_WIDTH = cfg::SIDE_PANEL_WIDTH - cfg::SIDE_PANEL_MARGIN * 2;
    constexpr int SLIDER_HITBOX_HEIGHT = 28;
    constexpr int SLIDER_HITBOX_OFFSET_Y = 12;
    constexpr int BUTTON_HEIGHT = 34;

    // Vertical layout for slider labels
    constexpr int SLIDER_ROWS_Y = 198;
    constexpr int SLIDER_COLS_Y = SLIDER_ROWS_Y + 58;
    constexpr int SLIDER_ALIVE_Y = SLIDER_COLS_Y + 58;
    constexpr int SLIDER_SPEED_Y = SLIDER_ALIVE_Y + 58;

    // Button row layout
    constexpr int BUTTON_ROW1_Y = SLIDER_SPEED_Y + 58;
    constexpr int BUTTON_ROW2_Y = BUTTON_ROW1_Y + BUTTON_HEIGHT + 10;
    constexpr int BUTTON_WIDTH = 104;
    constexpr int BUTTON_COL2_X = PANEL_X + 120;

    struct PanelButtons {
        Rectangle apply;
        Rectangle random;
        Rectangle clear;
        Rectangle center;
    };

    PanelButtons GetPanelButtons() {
        return {
            { static_cast<float>(PANEL_X),        static_cast<float>(BUTTON_ROW1_Y), static_cast<float>(BUTTON_WIDTH), static_cast<float>(BUTTON_HEIGHT) },
            { static_cast<float>(BUTTON_COL2_X),  static_cast<float>(BUTTON_ROW1_Y), static_cast<float>(BUTTON_WIDTH), static_cast<float>(BUTTON_HEIGHT) },
            { static_cast<float>(PANEL_X),        static_cast<float>(BUTTON_ROW2_Y), static_cast<float>(BUTTON_WIDTH), static_cast<float>(BUTTON_HEIGHT) },
            { static_cast<float>(BUTTON_COL2_X),  static_cast<float>(BUTTON_ROW2_Y), static_cast<float>(BUTTON_WIDTH), static_cast<float>(BUTTON_HEIGHT) },
        };
    }

    void DrawTextLine(const char* text, int x, int& y, int fontSize, Color color, int lineSpacing = 22) {
        DrawText(text, x, y, fontSize, color);
        y += lineSpacing;
    }

    float NormalizeValue(int value, int minValue, int maxValue) {
        return static_cast<float>(value - minValue) / static_cast<float>(maxValue - minValue);
    }

    int SliderInt(Rectangle rect, int value, int minValue, int maxValue) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), rect)) {
            const float t = std::clamp((GetMouseX() - rect.x) / rect.width, 0.0f, 1.0f);
            value = minValue + static_cast<int>(t * (maxValue - minValue) + 0.5f);
        }

        return ClampInt(value, minValue, maxValue);
    }

    bool ButtonPressed(Rectangle rect) {
        return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), rect);
    }

    void DrawSliderInt(const char* label, int value, int minValue, int maxValue, int x, int y) {
        DrawText(TextFormat("%s: %d", label, value), x, y, 16, cfg::TEXT_MAIN_COLOR);

        const Rectangle track{
            static_cast<float>(x),
            static_cast<float>(y + 24),
            static_cast<float>(CONTROL_WIDTH),
            4.0f
        };
        DrawRectangleRec(track, cfg::SLIDER_TRACK_COLOR);

        const float t = NormalizeValue(value, minValue, maxValue);
        const int knobX = static_cast<int>(track.x + t * track.width);
        DrawCircle(knobX, static_cast<int>(track.y + 2), 8.0f, cfg::SLIDER_KNOB_COLOR);
    }

    void DrawButton(Rectangle rect, const char* text) {
        const bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
        DrawRectangleRec(rect, hovered ? cfg::BUTTON_HOVER_COLOR : cfg::BUTTON_COLOR);
        DrawRectangleLinesEx(rect, 1.0f, cfg::PANEL_BORDER_COLOR);

        const int textWidth = MeasureText(text, 16);
        DrawText(
            text,
            static_cast<int>(rect.x + (rect.width - textWidth) / 2.0f),
            static_cast<int>(rect.y + 9),
            16,
            cfg::TEXT_MAIN_COLOR
        );
    }

    Rectangle SliderHitbox(int labelY) {
        return Rectangle{
            static_cast<float>(PANEL_X),
            static_cast<float>(labelY + SLIDER_HITBOX_OFFSET_Y),
            static_cast<float>(CONTROL_WIDTH),
            static_cast<float>(SLIDER_HITBOX_HEIGHT)
        };
    }
}

void UpdateSidePanelControls(GameState& game) {
    if (!IsMouseOverSidePanel()) {
        return;
    }

    game.pendingRows = SliderInt(SliderHitbox(SLIDER_ROWS_Y), game.pendingRows, cfg::MIN_GRID_ROWS, cfg::MAX_GRID_ROWS);
    game.pendingCols = SliderInt(SliderHitbox(SLIDER_COLS_Y), game.pendingCols, cfg::MIN_GRID_COLS, cfg::MAX_GRID_COLS);
    game.initialAlivePercent = SliderInt(SliderHitbox(SLIDER_ALIVE_Y), game.initialAlivePercent,
        cfg::MIN_INITIAL_ALIVE_PERCENT, cfg::MAX_INITIAL_ALIVE_PERCENT);
    game.speedMultiplier = SliderInt(SliderHitbox(SLIDER_SPEED_Y), game.speedMultiplier,
        cfg::MIN_SPEED_MULTIPLIER, cfg::MAX_SPEED_MULTIPLIER);

    const PanelButtons btns = GetPanelButtons();

    if (ButtonPressed(btns.apply)) {
        ResizeGrid(game, game.pendingRows, game.pendingCols);
        ResetCamera(game);
    }

    if (ButtonPressed(btns.random)) {
        RandomizeGrid(game);
    }

    if (ButtonPressed(btns.clear)) {
        ClearGrid(game);
    }

    if (ButtonPressed(btns.center)) {
        ResetCamera(game);
    }
}

void DrawSidePanel(const GameState& game) {
    const Rectangle panel = GetSidePanelBounds();
    DrawRectangleRec(panel, cfg::PANEL_COLOR);
    DrawRectangleLinesEx(panel, 1.0f, cfg::PANEL_BORDER_COLOR);

    int y = 24;

    DrawTextLine("GAME OF LIFE", PANEL_X, y, 24, cfg::TEXT_MAIN_COLOR, 34);
    DrawTextLine(game.isPaused ? "Status: PAUSED" : "Status: RUNNING", PANEL_X, y, 18,
        game.isPaused ? cfg::DANGER_COLOR : cfg::ALIVE_CELL_COLOR, 28);

    DrawTextLine(TextFormat("Generation: %d", game.generation), PANEL_X, y, 16, cfg::TEXT_MAIN_COLOR);
    DrawTextLine(TextFormat("Alive cells: %d", game.aliveCells), PANEL_X, y, 16, cfg::TEXT_MAIN_COLOR);
    DrawTextLine(TextFormat("Field: %d x %d", game.rows, game.cols), PANEL_X, y, 16, cfg::TEXT_MAIN_COLOR);
    DrawTextLine(TextFormat("Zoom: %d%%", static_cast<int>(game.camera.zoom * 100)), PANEL_X, y, 16, cfg::TEXT_MAIN_COLOR);
    DrawTextLine(TextFormat("Speed: x%d", game.speedMultiplier), PANEL_X, y, 16, cfg::TEXT_MAIN_COLOR);

    DrawSliderInt("Rows", game.pendingRows, cfg::MIN_GRID_ROWS, cfg::MAX_GRID_ROWS, PANEL_X, SLIDER_ROWS_Y);
    DrawSliderInt("Cols", game.pendingCols, cfg::MIN_GRID_COLS, cfg::MAX_GRID_COLS, PANEL_X, SLIDER_COLS_Y);
    DrawSliderInt("Initial alive", game.initialAlivePercent, cfg::MIN_INITIAL_ALIVE_PERCENT, cfg::MAX_INITIAL_ALIVE_PERCENT, PANEL_X, SLIDER_ALIVE_Y);
    DrawSliderInt("Speed", game.speedMultiplier, cfg::MIN_SPEED_MULTIPLIER, cfg::MAX_SPEED_MULTIPLIER, PANEL_X, SLIDER_SPEED_Y);

    const PanelButtons btns = GetPanelButtons();
    DrawButton(btns.apply, "Apply");
    DrawButton(btns.random, "Random");
    DrawButton(btns.clear, "Clear");
    DrawButton(btns.center, "Center");

    int gridY = BUTTON_ROW2_Y + BUTTON_HEIGHT + 18;
    DrawTextLine("Grid:", PANEL_X, gridY, 16, cfg::TEXT_MUTED_COLOR);
    DrawTextLine(game.showGrid ? "Enabled" : "Disabled", PANEL_X, gridY, 16,
        game.showGrid ? cfg::ALIVE_CELL_COLOR : cfg::TEXT_MUTED_COLOR);

    if (game.showGrid && !IsGridActuallyVisible(game)) {
        DrawTextLine("Auto-hidden at this zoom", PANEL_X, gridY, 14, cfg::TEXT_MUTED_COLOR);
    }

    DrawTextLine("[?] H / F1 - controls", PANEL_X, gridY += 16, 16, cfg::ACCENT_COLOR);
}

void DrawHelpPopup(const GameState& game) {
    if (!game.helpVisible) {
        return;
    }

    const int width = 430;
    const int height = 330;
    const int x = (cfg::WINDOW_WIDTH - cfg::SIDE_PANEL_WIDTH - width) / 2;
    const int y = (cfg::WINDOW_HEIGHT - height) / 2;

    const Rectangle popup = Rectangle{
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(width),
        static_cast<float>(height)
    };

    DrawRectangleRec(popup, Color{ 28, 28, 34, 245 });
    DrawRectangleLinesEx(popup, 2.0f, cfg::ACCENT_COLOR);

    int textY = y + 22;
    const int textX = x + 24;

    DrawTextLine("CONTROLS", textX, textY, 24, cfg::TEXT_MAIN_COLOR, 36);
    DrawTextLine("LMB drag  - draw alive cells", textX, textY, 18, cfg::TEXT_MAIN_COLOR, 28);
    DrawTextLine("RMB drag  - erase cells", textX, textY, 18, cfg::TEXT_MAIN_COLOR, 28);
    DrawTextLine("MMB drag  - move camera", textX, textY, 18, cfg::TEXT_MAIN_COLOR, 28);
    DrawTextLine("Wheel     - zoom", textX, textY, 18, cfg::TEXT_MAIN_COLOR, 28);
    DrawTextLine("Space     - pause / run", textX, textY, 18, cfg::TEXT_MAIN_COLOR, 28);
    DrawTextLine("N         - next generation", textX, textY, 18, cfg::TEXT_MAIN_COLOR, 28);
    DrawTextLine("R         - randomize", textX, textY, 18, cfg::TEXT_MAIN_COLOR, 28);
    DrawTextLine("C         - clear", textX, textY, 18, cfg::TEXT_MAIN_COLOR, 28);
    DrawTextLine("G         - toggle grid", textX, textY, 18, cfg::TEXT_MAIN_COLOR, 28);
    DrawTextLine("+ / -     - speed up / slow down", textX, textY, 18, cfg::TEXT_MAIN_COLOR, 28);
    DrawTextLine("F         - reset camera", textX, textY, 18, cfg::TEXT_MAIN_COLOR, 28);
}