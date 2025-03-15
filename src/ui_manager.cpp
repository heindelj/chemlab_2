#include "ui_manager.h"
#include <stdexcept>

UIManager::UIManager(int screenWidth, int screenHeight)
    : screenWidth(screenWidth), screenHeight(screenHeight) {
}

void UIManager::addRegion(const std::string& name, float x, float y, float width, float height) {
    // Check for invalid dimensions
    if (x < 0.0f || x > 1.0f || y < 0.0f || y > 1.0f ||
        width <= 0.0f || height <= 0.0f || x + width > 1.0f || y + height > 1.0f) {
        throw std::invalid_argument("Invalid region dimensions. All values must be normalized (0.0-1.0).");
    }
    
    // Check if region with this name already exists
    if (regionMap.find(name) != regionMap.end()) {
        throw std::invalid_argument("Region with name '" + name + "' already exists.");
    }
    
    // Add the region
    regions.emplace_back(name, x, y, width, height);
    regionMap[name] = regions.size() - 1;
}

const std::vector<UIRegion>& UIManager::getRegions() const {
    return regions;
}

const UIRegion* UIManager::getRegion(const std::string& name) const {
    auto it = regionMap.find(name);
    if (it != regionMap.end()) {
        return &regions[it->second];
    }
    return nullptr;
}

void UIManager::updateScreenSize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    // No need to update regions as they use normalized coordinates
}