#include "ui_manager.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

// Define the boundary detection threshold (in normalized coordinates)
const float UIManager::BOUNDARY_THRESHOLD = 0.015f; // Increased for easier selection

UIManager::UIManager(int screenWidth, int screenHeight)
    : screenWidth(screenWidth), screenHeight(screenHeight)
{
    // Start with default boundary positions
    verticalBoundaryPos = 0.5f;
    horizontalBoundaryPos = 0.5f;
}

void UIManager::addRegion(const std::string &name, float x, float y, float width, float height)
{
    // Check for invalid dimensions
    if (x < 0.0f || x > 1.0f || y < 0.0f || y > 1.0f ||
        width <= 0.0f || height <= 0.0f || x + width > 1.0f || y + height > 1.0f)
    {
        throw std::invalid_argument("Invalid region dimensions. All values must be normalized (0.0-1.0).");
    }

    // Check if region with this name already exists
    if (regionMap.find(name) != regionMap.end())
    {
        throw std::invalid_argument("Region with name '" + name + "' already exists.");
    }

    // Add the region
    regions.emplace_back(name, x, y, width, height);
    regionMap[name] = regions.size() - 1;

    // Update boundary positions when relevant regions are added
    if (name == "quad_tl" || name == "quad_tr" || name == "quad_bl" || name == "quad_br")
    {
        initBoundaryPositions();
    }
}

const std::vector<UIRegion> &UIManager::getRegions() const
{
    return regions;
}

const UIRegion *UIManager::getRegion(const std::string &name) const
{
    auto it = regionMap.find(name);
    if (it != regionMap.end())
    {
        return &regions[it->second];
    }
    return nullptr;
}

size_t UIManager::getRegionIndex(const std::string &name) const
{
    auto it = regionMap.find(name);
    if (it != regionMap.end())
    {
        return it->second;
    }
    throw std::invalid_argument("Region with name '" + name + "' does not exist.");
}

void UIManager::updateScreenSize(int width, int height)
{
    screenWidth = width;
    screenHeight = height;
    // No need to update regions as they use normalized coordinates
}

void UIManager::setWindow(GLFWwindow *win)
{
    window = win;
}

// Improved boundary detection for 2x2 grid structure
void UIManager::checkBoundaries(double mouseX, double mouseY)
{
    if (!window)
        return;

    // Convert mouse position to normalized coordinates
    double normalizedX = mouseX / screenWidth;
    double normalizedY = mouseY / screenHeight;

    // Make sure boundary positions are initialized
    if (verticalBoundaryPos <= 0.0f || horizontalBoundaryPos <= 0.0f)
    {
        initBoundaryPositions();
    }

    // Check for vertical boundary
    if (std::abs(normalizedX - verticalBoundaryPos) < BOUNDARY_THRESHOLD)
    {
        glfwSetCursor(window, glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));
        dragBoundaryType = BoundaryType::Vertical;
        dragBoundaryPos = verticalBoundaryPos;
        return;
    }

    // Check for horizontal boundary
    if (std::abs(normalizedY - horizontalBoundaryPos) < BOUNDARY_THRESHOLD)
    {
        glfwSetCursor(window, glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR));
        dragBoundaryType = BoundaryType::Horizontal;
        dragBoundaryPos = horizontalBoundaryPos;
        return;
    }

    // If we're here, we're not over a boundary
    glfwSetCursor(window, nullptr); // Reset cursor
    dragBoundaryType = BoundaryType::None;
}

bool UIManager::startDragging(double mouseX, double mouseY)
{
    // Check if we're over a boundary first
    checkBoundaries(mouseX, mouseY);

    // If we're over a boundary, start dragging
    if (dragBoundaryType != BoundaryType::None)
    {
        isDragging = true;

        // Store the initial positions and dimensions of all regions for proper resizing
        initialRegionStates.clear();
        initialRegionStates.reserve(regions.size());
        for (const auto &region : regions)
        {
            initialRegionStates.push_back({region.name, region.x, region.y, region.width, region.height});
        }

        // Also store the original boundary position
        originalBoundaryPos = dragBoundaryPos;

        return true;
    }

    return false;
}

// Completely revised dragging logic for 2x2 grid
void UIManager::updateDragging(double mouseX, double mouseY)
{
    if (!isDragging || initialRegionStates.empty())
        return;

    // Convert mouse position to normalized coordinates
    float normalizedX = static_cast<float>(mouseX / screenWidth);
    float normalizedY = static_cast<float>(mouseY / screenHeight);

    // Constrain to valid range (0.1-0.9) to prevent regions from getting too small
    const float MIN_EDGE = 0.1f;
    const float MAX_EDGE = 0.9f;
    normalizedX = std::max(MIN_EDGE, std::min(MAX_EDGE, normalizedX));
    normalizedY = std::max(MIN_EDGE, std::min(MAX_EDGE, normalizedY));

    if (dragBoundaryType == BoundaryType::Vertical)
    {
        // Find the indices of all regions
        size_t tlIndex = getRegionIndex("quad_tl");
        size_t trIndex = getRegionIndex("quad_tr");
        size_t blIndex = getRegionIndex("quad_bl");
        size_t brIndex = getRegionIndex("quad_br");

        // Use stored original boundary position
        float originalCenter = originalBoundaryPos;
        float newCenter = normalizedX;

        // Update top-left region (width)
        UIRegion &tlRegion = regions[tlIndex];
        tlRegion.width = initialRegionStates[tlIndex].width + (newCenter - originalCenter);

        // Update top-right region (x and width)
        UIRegion &trRegion = regions[trIndex];
        trRegion.x = newCenter;
        trRegion.width = 1.0f - newCenter - (1.0f - (initialRegionStates[trIndex].x + initialRegionStates[trIndex].width));

        // Update bottom-left region (width)
        UIRegion &blRegion = regions[blIndex];
        blRegion.width = initialRegionStates[blIndex].width + (newCenter - originalCenter);

        // Update bottom-right region (x and width)
        UIRegion &brRegion = regions[brIndex];
        brRegion.x = newCenter;
        brRegion.width = 1.0f - newCenter - (1.0f - (initialRegionStates[brIndex].x + initialRegionStates[brIndex].width));

        // Update drag position and vertical boundary position
        dragBoundaryPos = newCenter;
        verticalBoundaryPos = newCenter;
    }
    else if (dragBoundaryType == BoundaryType::Horizontal)
    {
        // Find the indices of all regions
        size_t tlIndex = getRegionIndex("quad_tl");
        size_t trIndex = getRegionIndex("quad_tr");
        size_t blIndex = getRegionIndex("quad_bl");
        size_t brIndex = getRegionIndex("quad_br");

        // Use stored original boundary position
        float originalCenter = originalBoundaryPos;
        float newCenter = normalizedY;

        // Update top-left region (height)
        UIRegion &tlRegion = regions[tlIndex];
        tlRegion.height = initialRegionStates[tlIndex].height + (newCenter - originalCenter);

        // Update top-right region (height)
        UIRegion &trRegion = regions[trIndex];
        trRegion.height = initialRegionStates[trIndex].height + (newCenter - originalCenter);

        // Update bottom-left region (y and height)
        UIRegion &blRegion = regions[blIndex];
        blRegion.y = newCenter;
        blRegion.height = 1.0f - newCenter - (1.0f - (initialRegionStates[blIndex].y + initialRegionStates[blIndex].height));

        // Update bottom-right region (y and height)
        UIRegion &brRegion = regions[brIndex];
        brRegion.y = newCenter;
        brRegion.height = 1.0f - newCenter - (1.0f - (initialRegionStates[brIndex].y + initialRegionStates[brIndex].height));

        // Update drag position and horizontal boundary position
        dragBoundaryPos = newCenter;
        horizontalBoundaryPos = newCenter;
    }

    // Debug prints for troubleshooting
    std::cout << "Dragging " << (dragBoundaryType == BoundaryType::Vertical ? "vertical" : "horizontal")
              << " boundary from " << originalBoundaryPos
              << " to " << (dragBoundaryType == BoundaryType::Vertical ? normalizedX : normalizedY) << std::endl;
}

void UIManager::endDragging()
{
    isDragging = false;
    dragBoundaryType = BoundaryType::None;
    initialRegionStates.clear();

    // Update boundary positions after dragging completes
    initBoundaryPositions();

    // Reset cursor
    if (window)
    {
        glfwSetCursor(window, nullptr);
    }
}

// Initialize boundary positions from quad regions
void UIManager::initBoundaryPositions()
{
    // Find the quad_tl region to determine boundary positions
    const UIRegion *tlRegion = getRegion("quad_tl");

    if (tlRegion)
    {
        // Right edge of top-left region is the vertical boundary
        verticalBoundaryPos = tlRegion->x + tlRegion->width;

        // Bottom edge of top-left region is the horizontal boundary
        horizontalBoundaryPos = tlRegion->y + tlRegion->height;

        std::cout << "Updated boundary positions: vertical=" << verticalBoundaryPos
                  << ", horizontal=" << horizontalBoundaryPos << std::endl;
    }
}

void UIManager::updateRegion(size_t index, float x, float y, float width, float height)
{
    if (index >= regions.size())
    {
        throw std::out_of_range("Region index out of range");
    }
    
    // Check for invalid dimensions
    if (x < 0.0f || x > 1.0f || y < 0.0f || y > 1.0f ||
        width <= 0.0f || height <= 0.0f || x + width > 1.0f || y + height > 1.0f)
    {
        throw std::invalid_argument("Invalid region dimensions. All values must be normalized (0.0-1.0).");
    }

    // Update the region
    regions[index].x = x;
    regions[index].y = y;
    regions[index].width = width;
    regions[index].height = height;
}