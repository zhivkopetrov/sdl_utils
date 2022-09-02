# sdl_utils

**A static C++ SDL2, SDL2_ttf, SDL2_mixer, SDL2_image wrapper library**

It supports:
- Multithreaded asset loaded (GPU accelerated)
The load method utilizes the auto-generated asset definitions from the 'resource_builder' command line tool.
The tool can be found here: https://github.com/zhivkopetrov/tools
- Multithreaded rendering (GPU accelerated)
Rendeder must operate on the main thread to guarantee cross-platform behaviour.
- OS Window magement
- OS event handling - mouse, keyboard, touchscreen, signals, window resizing, etc ...
- 2D Audio - music streaming and simultaneous sound chunks playback

The library is optimized for fast compilation.


**Usage from plain CMake**
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


**Usage as part of ROS(catkin) / ROS2(colcon) meta-build systems**
- consume directly with find_package(sdl_utils) in the package CMakeLists.txt
- Link agains your target
- The library automatically exposes ans install it's artifacts following ROS/ROS2 structure
- Example usage project: https://github.com/zhivkopetrov/robotics_v1


**Dependencies**
- cmake_helpers - https://github.com/zhivkopetrov/cmake_helpers.git
- The provided library CMakeLists.txt assumes the helpers.cmake from cmake_helpers repo have already been included
- resource_utils - https://github.com/zhivkopetrov/resource_utils


**Non direct dependencies**
- tools/resouce_builder - https://github.com/zhivkopetrov/tools
Used to genera the asset descriptions, loaded by the sdl_utils library


**Platform support**
- Tested on Unix, Windows
- Never tested on Mac