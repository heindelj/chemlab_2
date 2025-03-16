cmake_minimum_required(VERSION 3.10)
project(MolecularViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# For Windows
if(WIN32)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
endif()

# Add dependencies
add_subdirectory(external/glfw)
add_subdirectory(external/glad)
add_subdirectory(external/glm)

# Add ImGui (as a git submodule, docking branch)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
# Make sure the submodule is properly initialized
if(NOT EXISTS "${IMGUI_DIR}/imgui.cpp")
    message(FATAL_ERROR "ImGui submodule not found. Please run 'git submodule update --init --recursive'")
endif()

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${IMGUI_DIR} 
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw glad)

# Add ImPlot (as a git submodule)
set(IMPLOT_DIR ${CMAKE_SOURCE_DIR}/external/implot)
# Make sure the submodule is properly initialized
if(NOT EXISTS "${IMPLOT_DIR}/implot.cpp")
    message(FATAL_ERROR "ImPlot submodule not found. Please run 'git submodule update --init --recursive'")
endif()

set(IMPLOT_SOURCES
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp
    ${IMPLOT_DIR}/implot_demo.cpp
)
add_library(implot STATIC ${IMPLOT_SOURCES})
target_include_directories(implot PUBLIC ${IMPLOT_DIR})
target_link_libraries(implot PUBLIC imgui)

# Source files
set(SOURCES
    src/main.cpp
    src/renderer.cpp
    src/ui_manager.cpp
    src/imgui_manager.cpp
)

# Header files
set(HEADERS
    include/renderer.h
    include/ui_manager.h
    include/ui_region.h
    include/imgui_manager.h
)

# Define the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add include directories for the executable target
target_include_directories(${PROJECT_NAME} PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMPLOT_DIR}
)

# Link libraries directly
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    glad
    glm
    imgui
    implot
)

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Add Julia support later
# option(USE_JULIA "Enable Julia embedding" OFF)
# if(USE_JULIA)
#     find_package(Julia REQUIRED)
#     include_directories(${Julia_INCLUDE_DIRS})
#     target_link_libraries(${PROJECT_NAME} ${Julia_LIBRARIES})
# endif()

# Make a nice folder structure in IDEs
source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})