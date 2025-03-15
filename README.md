# Molecular Viewer

A modular OpenGL-based application for molecular visualization with customizable UI regions.

## Project Structure

```
MolecularViewer/
├── CMakeLists.txt         # Main build configuration
├── include/               # Header files
│   ├── renderer.h
│   ├── ui_manager.h
│   └── ui_region.h
├── src/                   # Source files
│   ├── main.cpp
│   ├── renderer.cpp
│   └── ui_manager.cpp
├── external/              # External dependencies
│   ├── glfw/              # Window management
│   ├── glad/              # OpenGL loading
│   └── glm/               # Math library
└── build/                 # Build directory
```

## Features

- OpenGL 3.3+ core profile
- Modular UI with customizable regions
- Extensible rendering system
- Future Julia integration for scientific computing

## Dependencies

- GLFW: Window creation and input handling
- GLAD: OpenGL function loading
- GLM: Mathematics library for OpenGL

## Building

1. Clone the repository:
```bash
git clone https://github.com/yourusername/molecular-viewer.git
cd molecular-viewer
```

2. Run the setup script to create directory structure and download dependencies:
```bash
chmod +x setup.sh
./setup.sh
```

3. Build the project:
```bash
cd build
cmake --build .
```

4. Run the application:
```bash
./MolecularViewer
```

## Future Plans

- Integration with Julia for computational chemistry
- Customizable plotting and visualization components
- Support for multiple molecular file formats
- Interactive UI elements

## License

[MIT License](LICENSE)