#pragma once
#include <vector>
#include <map>
#include "ui_region.h"

class UIManager {
public:
    UIManager(int screenWidth, int screenHeight);
    
    // Add a UI region
    void addRegion(const std::string& name, float x, float y, float width, float height);
    
    // Get all regions
    const std::vector<UIRegion>& getRegions() const;
    
    // Get a specific region by name
    const UIRegion* getRegion(const std::string& name) const;
    
    // Handle window resize
    void updateScreenSize(int width, int height);

    int screenWidth;
    int screenHeight;
    std::vector<UIRegion> regions;
    std::map<std::string, size_t> regionMap; // maps region name to index in vector
};