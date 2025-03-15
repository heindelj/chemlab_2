cmake_minimum_required(VERSION 3.10)
project(MolecularViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# Set up libraries
set(LIBRARIES
    OpenGL::GL
)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

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

list(APPEND LIBRARIES glfw glad glm)

# Source files
set(SOURCES
    src/main.cpp
    src/renderer.cpp
    src/ui_manager.cpp
)

# Header files
set(HEADERS
    include/renderer.h
    include/ui_manager.h
    include/ui_region.h
)

# Define the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

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

# Copy shader files to build directory (if we add external shaders later)
# file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
