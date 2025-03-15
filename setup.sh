#!/bin/bash
# Setup script for MolecularViewer project

# Create directory structure
mkdir -p src include external build

# Clone dependencies
cd external
echo "Cloning GLFW..."
git clone https://github.com/glfw/glfw.git
echo "Cloning GLM..."
git clone https://github.com/g-truc/glm.git

# Setup GLAD
mkdir -p glad
cd glad

# Create glad CMakeLists.txt
cat > CMakeLists.txt << 'EOL'
cmake_minimum_required(VERSION 3.10)
project(glad C)

add_library(glad src/glad.c)
target_include_directories(glad PUBLIC include)
EOL

cd ../..

# Generate initial build files
echo "Generating build files..."
cd build
cmake ..
cd ..

echo "Project setup complete! You can now build with:"
echo "cd build"
echo "cmake --build ."
