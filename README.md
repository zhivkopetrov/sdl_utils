# sdl_utils

## A static C++ SDL2, SDL2_ttf, SDL2_mixer, SDL2_image wrapper library
The library is optimized for fast compilation.

It supports:
- Multithreaded asset loaded (GPU accelerated)
The load method utilizes the auto-generated asset definitions from the 'resource_builder' command line tool.
The tool can be found here: https://github.com/zhivkopetrov/tools
- Multithreaded rendering (GPU accelerated)
Rendeder must operate on the main thread to guarantee cross-platform behaviour.
- OS Window magement
- OS event handling - mouse, keyboard, touchscreen, signals, window resizing, etc ...
- 2D Audio - music streaming and simultaneous sound chunks playback
- Dynamic containers for assets, which act like a backbone infrasturcture for flyweight texture handles
```
Supported asset containers:
-ResourceContainer (Images & Sprites)
-FboContainer (run-time Frame Buffer Object container)
-TextContainer (run-time font textures generated from TTF glyphs)
-SoundContainer (music streams & small sound chunks)
```

## Usage from plain CMake
- consume directly with find_package(sdl_utils) in a CMakeLists.txt
- Link against your target with suitable access modifier
```
target_link_libraries(
    ${PROJECT_NAME} 
    PUBLIC
        sdl_utils::sdl_utils
)
```
- Example usage project: https://github.com/zhivkopetrov/dev_battle.git

## Usage as part of ROS1(catkin) / ROS2(colcon) meta-build systems
- consume directly with find_package(sdl_utils) in the package CMakeLists.txt
- Link agains your target
- The library automatically exposes and install it's artifacts following ROS1/ROS2 structure
- Example usage project: https://github.com/zhivkopetrov/robotics_v1

## Dependencies
- cmake_helpers - https://github.com/zhivkopetrov/cmake_helpers.git
- utils - https://github.com/zhivkopetrov/utils
- resource_utils - https://github.com/zhivkopetrov/resource_utils

## Non-direct dependencies
- tools/resouce_builder - https://github.com/zhivkopetrov/tools
Used to generate the asset descriptions, loaded by the sdl_utils library

## Third party libs, which are not shipped with this repository
- SDL2
- SDL2-image
- SDL2-ttf
- SDL2-mixer

Installing them through apt is sufficient when native builds are targeted under Linux

```
sudo apt install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-mixer-dev
```

For windows builds download SDL2 packages from the official repo
https://github.com/libsdl-org/SDL/releases/latest

Extract under "C:/SDL2" or alongside the project binary build with the sdl_utils static lib

## Supported Platforms
Linux:
  - g++ (9.3 and above)
    - Tested up to g++ 12.1
  - clang++ (10 and above)
    - Tested up to clang++ 14.0
  - Emscripten (3.1.28 and above)
    - emcc/em++
  - Robot Operating System 2 (ROS2)
    - Through colcon meta-build system (CMake based)
  - Robot Operating System 1 (ROS1)
    - Through catkin meta-build system (CMake based)
      - Due to soon ROS1 end-of-life catkin builds are not actively supported

Windows:
  - MSVC++ (14.20 and above) Visual Studio 2019
    - Tested up to 17.30 Visual Studio 2022
  - Emscripten (3.1.28 and above)
    - emcc/em++
    - NOTE: non-MSVC CMake build generator is needed
      - For example: ninja
  - ROS1/ROS2
    - Although the code is ROS compatible, actual ROS functionalities have not been tested on Windows