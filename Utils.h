#pragma once

#include <algorithm>

// Convenience alias — prefer std::clamp<int> in new code.
inline int ClampInt(int value, int minValue, int maxValue) {
    return std::clamp(value, minValue, maxValue);
}