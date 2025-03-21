#include "ui_manager.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

// Define the boundary detection threshold (in normalized coordinates)
const float UIManager::BOUNDARY_THRESHOLD = 0.01f;

UIManager::UIManager(int screenWidth, int screenHeight)
    : screenWidth(screenWidth), screenHeight(screenHeight)
{
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

void UIManager::checkBoundaries(double mouseX, double mouseY)
{
    if (!window)
        return;

    // Convert mouse position to normalized coordinates
    double normalizedX = mouseX / screenWidth;
    double normalizedY = mouseY / screenHeight;

    // Check for vertical boundaries
    for (size_t i = 0; i < regions.size(); ++i)
    {
        const UIRegion &region = regions[i];

        // Right edge
        float rightEdge = region.x + region.width;
        if (std::abs(normalizedX - rightEdge) < BOUNDARY_THRESHOLD)
        {
            // Found a potential vertical boundary, check if there's a region to the right
            if (findAdjacentRegions(rightEdge, BoundaryType::Vertical))
            {
                // Set the cursor to horizontal resize
                glfwSetCursor(window, glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));
                dragBoundaryType = BoundaryType::Vertical;
                dragBoundaryPos = rightEdge;
                return;
            }
        }

        // Bottom edge
        float bottomEdge = region.y + region.height;
        if (std::abs(normalizedY - bottomEdge) < BOUNDARY_THRESHOLD)
        {
            // Found a potential horizontal boundary, check if there's a region below
            if (findAdjacentRegions(bottomEdge, BoundaryType::Horizontal))
            {
                // Set the cursor to vertical resize
                glfwSetCursor(window, glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR));
                dragBoundaryType = BoundaryType::Horizontal;
                dragBoundaryPos = bottomEdge;
                return;
            }
        }
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
        return true;
    }

    return false;
}

void UIManager::updateDragging(double mouseX, double mouseY)
{
    if (!isDragging)
        return;

    // Convert mouse position to normalized coordinates
    float normalizedX = static_cast<float>(mouseX / screenWidth);
    float normalizedY = static_cast<float>(mouseY / screenHeight);

    // Constrain to valid range (0-1)
    normalizedX = std::max(0.0f, std::min(1.0f, normalizedX));
    normalizedY = std::max(0.0f, std::min(1.0f, normalizedY));

    // Define minimum size for regions (10% of screen)
    const float MIN_SIZE = 0.1f;

    if (dragBoundaryType == BoundaryType::Vertical)
    {
        // Don't allow resizing if it would make a region too small
        if (normalizedX < regions[leftRegionIndex].x + MIN_SIZE ||
            normalizedX > regions[rightRegionIndex].x + regions[rightRegionIndex].width - MIN_SIZE)
        {
            return;
        }

        // Calculate size changes
        float oldBoundary = dragBoundaryPos;
        float newBoundary = normalizedX;
        float deltaWidth = newBoundary - oldBoundary;

        // Resize left region (increase width)
        UIRegion &leftRegion = regions[leftRegionIndex];
        leftRegion.width += deltaWidth;

        // Resize right region (decrease width and move)
        UIRegion &rightRegion = regions[rightRegionIndex];
        rightRegion.x += deltaWidth;
        rightRegion.width -= deltaWidth;

        // Update the boundary position
        dragBoundaryPos = newBoundary;
    }
    else if (dragBoundaryType == BoundaryType::Horizontal)
    {
        // Don't allow resizing if it would make a region too small
        if (normalizedY < regions[topRegionIndex].y + MIN_SIZE ||
            normalizedY > regions[bottomRegionIndex].y + regions[bottomRegionIndex].height - MIN_SIZE)
        {
            return;
        }

        // Calculate size changes
        float oldBoundary = dragBoundaryPos;
        float newBoundary = normalizedY;
        float deltaHeight = newBoundary - oldBoundary;

        // Resize top region (increase height)
        UIRegion &topRegion = regions[topRegionIndex];
        topRegion.height += deltaHeight;

        // Resize bottom region (decrease height and move)
        UIRegion &bottomRegion = regions[bottomRegionIndex];
        bottomRegion.y += deltaHeight;
        bottomRegion.height -= deltaHeight;

        // Update the boundary position
        dragBoundaryPos = newBoundary;
    }
}

void UIManager::endDragging()
{
    isDragging = false;
    dragBoundaryType = BoundaryType::None;

    // Reset cursor
    if (window)
    {
        glfwSetCursor(window, nullptr);
    }
}

bool UIManager::findAdjacentRegions(float boundaryPos, BoundaryType type)
{
    if (type == BoundaryType::Vertical)
    {
        // Find regions on the left and right of the vertical boundary
        leftRegionIndex = -1;
        rightRegionIndex = -1;

        for (size_t i = 0; i < regions.size(); ++i)
        {
            const UIRegion &region = regions[i];

            // Region has right edge at the boundary
            if (std::abs((region.x + region.width) - boundaryPos) < BOUNDARY_THRESHOLD)
            {
                leftRegionIndex = i;
            }

            // Region has left edge at the boundary
            if (std::abs(region.x - boundaryPos) < BOUNDARY_THRESHOLD)
            {
                rightRegionIndex = i;
            }
        }

        return (leftRegionIndex != -1 && rightRegionIndex != -1);
    }
    else if (type == BoundaryType::Horizontal)
    {
        // Find regions above and below the horizontal boundary
        topRegionIndex = -1;
        bottomRegionIndex = -1;

        for (size_t i = 0; i < regions.size(); ++i)
        {
            const UIRegion &region = regions[i];

            // Region has bottom edge at the boundary
            if (std::abs((region.y + region.height) - boundaryPos) < BOUNDARY_THRESHOLD)
            {
                topRegionIndex = i;
            }

            // Region has top edge at the boundary
            if (std::abs(region.y - boundaryPos) < BOUNDARY_THRESHOLD)
            {
                bottomRegionIndex = i;
            }
        }

        return (topRegionIndex != -1 && bottomRegionIndex != -1);
    }

    return false;
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