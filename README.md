# OpenGL Cel-Shader

A real-time **cel shader (toon shader)** written in C++20 with OpenGL, GLEW and GLUT. It loads `.obj` models, renders them with banded toon lighting, and draws a configurable black outline around them.

This project started as a school assignment to learn OpenGL, built together with my teammate [Zarvork](https://github.com/Zarvork).

## Features

- Cel / toon shading with discrete light bands
- Switchable colour interpolation (linear or logarithmic)
- Adjustable black outline thickness
- `.obj` loading, with or without textures
- Free-fly camera and a movable light source

## Credits

The bundled test model is [12140 Skull v3 L2](https://sketchfab.com/3d-models/12140-skull-v3-l2-f1d38ebc067c4d66b31f310293d4be3b) by [jesusterradagomez](https://sketchfab.com/jesusterradagomez), taken from Sketchfab.

## Project structure

```
.
├── CMakeLists.txt      # top-level build file
├── flake.nix           # Nix dev shell with all dependencies
├── includes/           # C++ includes along external dependencies
├── objects/            # sample .obj models and textures
└── src/                # C++ sources and the executable target
```

## Dependencies

You need a C++20 compiler, CMake >= 3.21 and the following libraries:

- OpenGL (`gl`, `glu`)
- GLEW
- GLUT (freeglut)
- GLFW3
- pthread

### With Nix (recommended)

The provided flake sets up everything for you:

```sh
nix develop
```

### Without Nix

Install the packages above through your distribution. For example, on Debian/Ubuntu:

```sh
sudo apt install build-essential cmake libgl1-mesa-dev libglu1-mesa-dev \
                 libglew-dev freeglut3-dev libglfw3-dev
```

## Building

From the project root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The executable is generated at `build/CelShading`.

## Running

```sh
./build/CelShading <WITH_TEXTURE> <DIRECTORY> <FILE> <SCALE>
```

| Argument       | Description                                                                       |
| -------------- | --------------------------------------------------------------------------------- |
| `WITH_TEXTURE` | `0` if the `.obj` has no associated textures, `1` if it does                      |
| `DIRECTORY`    | Path to the directory containing the `.obj` file and its textures                 |
| `FILE`         | Name of the `.obj` file only (no path)                                            |
| `SCALE`        | Model scale: above `1` makes it bigger, below `1` makes it smaller                |

Paths are resolved relative to the directory you launch the program from, so run it from the project root.

### Example

```sh
./build/CelShading 0 objects skull.obj 2
```

## Controls

| Key              | Action                                                                     |
| ---------------- | -------------------------------------------------------------------------- |
| `W` `A` `S` `D`  | Move the camera                                                            |
| `E` / `Q`        | Move the camera up / down                                                  |
| `Space`          | Capture / release the mouse (press it twice when the scene first loads)    |
| `Tab`            | Switch colour interpolation between linear and logarithmic                 |
| `+` / `-`        | Increase / decrease the thickness of the black outline                     |
| `U` `H` `J` `K`  | Move the light                                                             |
| `Y` / `I`        | Move the light up / down                                                   |
| `Enter`          | Reset the view to its original position                                    |
| `T`              | Disable toon shading (models without textures only)                        |

## Troubleshooting

- **Black screen or nothing visible:** press `Space` twice, then `Enter` to reset the view. Also try a different `SCALE`.
- **Model or textures not found:** check that `DIRECTORY` is correct relative to where you run the binary.
- **Linker errors about GL/GLEW/GLUT:** make sure the development packages (not only the runtime libraries) are installed, or use `nix develop`.