#pragma once
#include <string>

struct UIRegion {
    std::string name;
    float x;      // x position (0.0-1.0, normalized)
    float y;      // y position (0.0-1.0, normalized)
    float width;  // width (0.0-1.0, normalized)
    float height; // height (0.0-1.0, normalized)
    
    // Constructor
    UIRegion(const std::string& n, float x, float y, float w, float h)
        : name(n), x(x), y(y), width(w), height(h) {}
};