#include "AppConfig.h"

#include "raylib.h"

bool IsMouseOverSidePanel() {
    return CheckCollisionPointRec(GetMousePosition(), GetSidePanelBounds());
}