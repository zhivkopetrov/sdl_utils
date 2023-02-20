#author Zhivko Petrov

cmake_minimum_required(VERSION 3.10.2)

project(
    sdl_utils
    LANGUAGES CXX
    VERSION 1.0.0
)

find_package(cmake_helpers REQUIRED)
if(NOT DISABLE_ROS_TOOLING)
    # CMAKE_MODULE_PATH is populated inside in order to access the find_modules folder.
    # This is required for the Find_SDL*.cmake files
    
    # Ament uses non-monolith build (separate build and install steps).
    # The helpers.cmake has to be included manually.
    # This allows access to the helpers.cmake user defined macros
    # In the case of monolith build - the file is automatically included
    # from the cmake_helpersConfig.cmake when the package is consumed by find_package()
	include(${cmake_helpers_DIR}/helpers.cmake)
endif()

find_package(resource_utils REQUIRED)
# emscripten uses it's own SDL2* family ports
# looking for the platform ones will yield an error
if(NOT EMSCRIPTEN)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(SDL2_mixer REQUIRED)
endif()


set(_INC_FOLDER_NAME include)
set(_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${_INC_FOLDER_NAME}/${PROJECT_NAME})         
set(_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(
    ${PROJECT_NAME} 
    STATIC
        ${_INC_DIR}/containers/config/SDLContainersConfig.h
        ${_INC_DIR}/containers/FontContainer.h
        ${_INC_DIR}/containers/ResourceContainer.h
        ${_INC_DIR}/containers/SDLContainers.h
        ${_INC_DIR}/containers/SoundContainer.h
        ${_INC_DIR}/containers/FboContainer.h
        ${_INC_DIR}/containers/TextContainer.h
        ${_INC_DIR}/drawing/defines/DrawConstants.h
        ${_INC_DIR}/drawing/defines/MonitorDefines.h
        ${_INC_DIR}/drawing/defines/RendererDefines.h
        ${_INC_DIR}/drawing/defines/DrawUtilityDefines.h
        ${_INC_DIR}/drawing/config/LoadingScreenConfig.h
        ${_INC_DIR}/drawing/config/RendererConfig.h
        ${_INC_DIR}/drawing/config/MonitorWindowConfig.h
        ${_INC_DIR}/drawing/DrawParams.h
        ${_INC_DIR}/drawing/GeometryUtils.h
        ${_INC_DIR}/drawing/LoadingScreen.h
        ${_INC_DIR}/drawing/MonitorWindow.h
        ${_INC_DIR}/drawing/Renderer.h
        ${_INC_DIR}/drawing/RendererState.h
        ${_INC_DIR}/drawing/Texture.h
        ${_INC_DIR}/input/InputEventGenerator.h
        ${_INC_DIR}/input/MouseUtils.h
        ${_INC_DIR}/sound/defines/SoundMixerDefines.h
        ${_INC_DIR}/sound/SoundMixer.h
        ${_INC_DIR}/SDLLoader.h
        
        ${_SRC_DIR}/containers/FontContainer.cpp
        ${_SRC_DIR}/containers/ResourceContainer.cpp
        ${_SRC_DIR}/containers/SDLContainers.cpp
        ${_SRC_DIR}/containers/SoundContainer.cpp
        ${_SRC_DIR}/containers/FboContainer.cpp
        ${_SRC_DIR}/containers/TextContainer.cpp
        ${_SRC_DIR}/drawing/config/RendererConfig.cpp
        ${_SRC_DIR}/drawing/defines/MonitorDefines.cpp
        ${_SRC_DIR}/drawing/defines/RendererDefines.cpp
        ${_SRC_DIR}/drawing/DrawParams.cpp
        ${_SRC_DIR}/drawing/GeometryUtils.cpp
        ${_SRC_DIR}/drawing/LoadingScreen.cpp
        ${_SRC_DIR}/drawing/MonitorWindow.cpp
        ${_SRC_DIR}/drawing/Renderer.cpp
        ${_SRC_DIR}/drawing/RendererState.cpp
        ${_SRC_DIR}/drawing/Texture.cpp
        ${_SRC_DIR}/input/InputEventGenerator.cpp
        ${_SRC_DIR}/input/MouseUtils.cpp
        ${_SRC_DIR}/sound/SoundMixer.cpp
        ${_SRC_DIR}/SDLLoader.cpp
)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME} 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${_INC_FOLDER_NAME}>
        $<INSTALL_INTERFACE:${_INC_FOLDER_NAME}>
)

if(EMSCRIPTEN)
    target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
            # emscripten bundles all SDL2* ports includes here
            ${SDL2_INCLUDE_DIRS}
    )
else()
    target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
            ${SDL2_INCLUDE_DIR}
            ${SDL2_IMAGE_INCLUDE_DIR}
            ${SDL2_TTF_INCLUDE_DIR}
            ${SDL2_MIXER_INCLUDE_DIR}
    )
endif()

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        resource_utils::resource_utils
)

if(EMSCRIPTEN)
    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
            # emscripten bundles all SDL2* ports libs here
            ${SDL2_LIBRARIES}
    )
else()
    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
            ${SDL2_LIBRARY}       # -lSDL2 flag
            ${SDL2_IMAGE_LIBRARY} # -lSDL2_image flag
            ${SDL2_TTF_LIBRARY}   # -lSDL2_ttf flag
            ${SDL2_MIXER_LIBRARY} # -lSDL2_mixer flag 
    )
endif()

if(UNIX)
    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
            -lX11
    )
endif()

if(NOT DISABLE_ROS_TOOLING)
    enable_ros_tooling_for_target(
        ${PROJECT_NAME}
        ${CMAKE_CURRENT_SOURCE_DIR}/package.xml
    )
endif()

set_target_cpp_standard(${PROJECT_NAME} 20)
enable_target_warnings(${PROJECT_NAME})
set_target_visibility(${PROJECT_NAME})
install_and_export_target(${PROJECT_NAME} ${_INC_FOLDER_NAME})

#invoke include-what-you-use (if it's enabled) for the current project
if(USE_IWYU)
    enable_target_include_what_you_use(${PROJECT_NAME})
endif(USE_IWYU)

if(BUILD_LIKE_POSITION_INDEPENDANT_CODE)
    enable_target_position_independent_code(${PROJECT_NAME})
endif()  

