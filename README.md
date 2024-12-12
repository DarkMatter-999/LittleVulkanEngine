# Little Vulkan Engine

A small Vulkan-based rendering engine, inspired by [blurrypiano's LittleVulkanEngine](https://github.com/blurrypiano/LittleVulkanEngine). This engine provides a simple and minimal setup for rendering graphics using Vulkan API, aimed at helping me learn the basics of Vulkan while building an engine from scratch.

https://github.com/user-attachments/assets/8cb96389-1139-40db-a642-f5d910647695

---

## Features

- **Vulkan API Integration**: Basic setup for Vulkan rendering, including initialization, pipeline creation, and rendering loop.
- **Minimal & Lightweight**: Designed to be a small, straightforward codebase to understand the Vulkan API without extra complexity.
- **Cross-Platform**: Should work on any platform Vulkan supports, including Linux, Windows, and macOS, with minimal modifications.
- **Camera & Transformation Support**: Basic camera setup and transformation matrices for rendering objects in 3D space.
- **Shader Support**: Example shaders for basic rendering with minimal setup.

---

## Getting Started

### Prerequisites
Before you begin, make sure you have the following installed:
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
- A C++ compiler (e.g., GCC, Clang, MSVC)
- [GLFW](https://www.glfw.org/) (for window management and input)
- [GLM](https://github.com/g-truc/glm) (for math operations like matrix transformations)

### Clone the Repository

```bash
git clone https://github.com/DarkMatter-999/LittleVulkanEngine.git
cd LittleVulkanEngine
```

### Build the Engine

1. Install the dependencies (Vulkan SDK, GLFW, GLM).

2. TinyObjLoader needs to be downloaded separately from [tiny_obj_loader.h](https://github.com/tinyobjloader/tinyobjloader/blob/release/tiny_obj_loader.h) and placed inside the `tinyobjloader` directory or the `src` directory.

3. Run Make to configure the project:

```bash
make -j4
```
4. After the build completes, run the engine:

```bash
make run
```

---

## Usage

The camera can be controlled using WASD along with Q & E for movement and arrow keys for rotation.

The engine currently supports:

- Loading shaders and compiling them with the Vulkan API.
- Creating and managing Vulkan buffers and pipelines.
- Basic 3D rendering with camera transformations.

More advanced features, such as adding texture support, lighting models, or more complex object handling are to be added in the future.

---

## Contributing

Contributions are welcome! If you'd like to help improve the engine, feel free to fork the repository, submit issues, or create pull requests.

---

## Acknowledgements

- Inspired by [blurrypiano's LittleVulkanEngine](https://github.com/blurrypiano/LittleVulkanEngine).
- Vulkan API documentation: [Khronos Vulkan](https://www.khronos.org/vulkan/).

