include(CMakeFindDependencyMacro)

#find_dependency will correctly forward REQUIRED or QUIET args to the consumer
#find_package is only for internal use
find_dependency(cmake_helpers REQUIRED)
find_dependency(resource_utils REQUIRED)

if(NOT TARGET sdl_utils::sdl_utils)
  include(${CMAKE_CURRENT_LIST_DIR}/sdl_utilsTargets.cmake)
endif()

# This is for catkin compatibility.
set(sdl_utils_LIBRARIES sdl_utils::sdl_utils)

get_target_property(
    sdl_utils_INCLUDE_DIRS
    sdl_utils::sdl_utils
    INTERFACE_INCLUDE_DIRECTORIES
)

