#pragma once
#include <iostream>

namespace ConsoleColor {
    constexpr const char* RED = "\033[31;1m";    // Bright Red
    constexpr const char* GREEN = "\033[32;1m";  // Bright Green
    constexpr const char* YELLOW = "\033[33;1m"; // Bright Yellow
    constexpr const char* BLUE = "\033[34;1m";   // Bright Blue
    constexpr const char* RESET = "\033[0m";     // Reset to default
}
