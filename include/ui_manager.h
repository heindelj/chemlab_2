#pragma once
#include <vector>
#include <map>
#include "ui_region.h"

// Forward declaration for GLFWwindow
struct GLFWwindow;

class UIManager
{
public:
    UIManager(int screenWidth, int screenHeight);

    // Add a UI region
    void addRegion(const std::string &name, float x, float y, float width, float height);

    // Get all regions
    const std::vector<UIRegion> &getRegions() const;

    // Get a specific region by name
    const UIRegion *getRegion(const std::string &name) const;

    // Handle window resize
    void updateScreenSize(int width, int height);

    // Boundary dragging support
    void setWindow(GLFWwindow *window);
    void checkBoundaries(double mouseX, double mouseY);
    bool startDragging(double mouseX, double mouseY);
    void updateDragging(double mouseX, double mouseY);
    void endDragging();

    // Get region index by name
    size_t getRegionIndex(const std::string &name) const;

    // Update a region's dimensions
    void updateRegion(size_t index, float x, float y, float width, float height);

    // Method to get the current boundary positions
    float getVerticalBoundaryPosition() const { return verticalBoundaryPos; }
    float getHorizontalBoundaryPosition() const { return horizontalBoundaryPos; }

    int screenWidth;
    int screenHeight;
    std::vector<UIRegion> regions;
    std::map<std::string, size_t> regionMap; // maps region name to index in vector

private:
    // Boundary detection and dragging
    static const float BOUNDARY_THRESHOLD;
    enum class BoundaryType {
        None,
        Vertical,
        Horizontal
    };

    // Structure to store the initial state of regions during drag operations
    struct RegionState {
        std::string name;
        float x;
        float y;
        float width;
        float height;
    };
    std::vector<RegionState> initialRegionStates;

    GLFWwindow *window = nullptr;
    bool isDragging = false;
    BoundaryType dragBoundaryType = BoundaryType::None;
    float dragBoundaryPos = 0.0f;
    float originalBoundaryPos = 0.0f;

    // Store the current positions of boundaries for consistent drawing
    float verticalBoundaryPos = 0.5f;   // X position of vertical boundary (normalized)
    float horizontalBoundaryPos = 0.5f; // Y position of horizontal boundary (normalized)

    // Method to initialize boundary positions based on regions
    void initBoundaryPositions();
};