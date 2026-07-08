# Morpion-OpenGL

A 3D implementation of the classic Tic-Tac-Toe game (known as "Morpion" in French) built with C++20 and modern OpenGL.

## Features

* **3D Graphics:** Game board and pieces (crosses and circles) are 3D models (`.glb` formats) loaded using `tinygltf`.
* **Modern OpenGL:** Uses shaders (PBR), framebuffers for shadow mapping, and dynamic lighting.
* **Animations:** Smooth entry animations when placing pieces on the board.
* **Camera Controls:** Interactive 3D camera.

## Dependencies

The project relies on the following libraries:
* **GLFW 3:** For window creation and input handling.
* **GLM:** For vector and matrix mathematics.
* **GLAD:** OpenGL function loader.
* **TinyGLTF:** For loading 3D models (handled automatically via Meson/CMake subprojects).

You will also need **Meson** and a compatible C++20 compiler (like GCC, Clang, or MSVC) to build the project.

## Building and Running

This project uses the **Meson** build system, with a convenient `Makefile` provided to simplify the commands.

### Prerequisites
Make sure you have installed:
* A C++20 capable compiler.
* Meson and Ninja.
* GLFW and GLM (make sure they are available to `pkg-config` or installed system-wide depending on your OS).

### Build Instructions

You can use the provided `Makefile` to easily setup, build, and run the project:

1. **Setup the build directory:**
   ```bash
   make setup
   ```
   *(Optionally, use `make setupClang` if you prefer using Clang).*

2. **Compile the project:**
   ```bash
   make all
   ```

3. **Run the game:**
   ```bash
   make run
   ```

### Cleaning the build
To clean the build directory, simply run:
```bash
make clean
```
