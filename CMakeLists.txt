#author Zhivko Petrov

cmake_minimum_required(VERSION 3.10.2)

project(sdl_utils)
  
set(_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/sdl_utils)         
set(_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(
    ${PROJECT_NAME} 
    STATIC
        ${_INC_DIR}/containers/config/SDLContainersConfig.hpp
        ${_INC_DIR}/containers/FontContainer.h
        ${_INC_DIR}/containers/ResourceContainer.h
        ${_INC_DIR}/containers/SDLContainers.h
        ${_INC_DIR}/containers/SoundContainer.h
        ${_INC_DIR}/containers/SpriteBufferContainer.h
        ${_INC_DIR}/containers/TextContainer.h
        ${_INC_DIR}/drawing/defines/DrawConstants.h
        ${_INC_DIR}/drawing/defines/MonitorDefines.h
        ${_INC_DIR}/drawing/config/LoadingScreenConfig.hpp
        ${_INC_DIR}/drawing/config/RendererConfig.hpp
        ${_INC_DIR}/drawing/DrawParams.h
        ${_INC_DIR}/drawing/GeometryUtils.h
        ${_INC_DIR}/drawing/LoadingScreen.h
        ${_INC_DIR}/drawing/MonitorWindow.h
        ${_INC_DIR}/drawing/Renderer.h
        ${_INC_DIR}/drawing/RendererState.h
        ${_INC_DIR}/drawing/Texture.h
        ${_INC_DIR}/input/defines/EventDefines.h
        ${_INC_DIR}/input/InputEvent.h
        ${_INC_DIR}/input/MouseUtils.h
        ${_INC_DIR}/input/TouchEntity.h
        ${_INC_DIR}/sound/defines/SoundMixerDefines.h
        ${_INC_DIR}/sound/SoundMixer.h
        ${_INC_DIR}/SDLLoader.h
        
        ${_SRC_DIR}/containers/FontContainer.cpp
        ${_SRC_DIR}/containers/ResourceContainer.cpp
        ${_SRC_DIR}/containers/SDLContainers.cpp
        ${_SRC_DIR}/containers/SoundContainer.cpp
        ${_SRC_DIR}/containers/SpriteBufferContainer.cpp
        ${_SRC_DIR}/containers/TextContainer.cpp
        ${_SRC_DIR}/drawing/defines/MonitorDefines.cpp
        ${_SRC_DIR}/drawing/DrawParams.cpp
        ${_SRC_DIR}/drawing/GeometryUtils.cpp
        ${_SRC_DIR}/drawing/LoadingScreen.cpp
        ${_SRC_DIR}/drawing/MonitorWindow.cpp
        ${_SRC_DIR}/drawing/Renderer.cpp
        ${_SRC_DIR}/drawing/RendererState.cpp
        ${_SRC_DIR}/drawing/Texture.cpp
        ${_SRC_DIR}/input/InputEvent.cpp
        ${_SRC_DIR}/input/MouseUtils.cpp
        ${_SRC_DIR}/input/TouchEntity.cpp
        ${_SRC_DIR}/sound/SoundMixer.cpp
        ${_SRC_DIR}/SDLLoader.cpp
)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

set_target_cpp_standard(${PROJECT_NAME} 20)
enable_target_warnings(${PROJECT_NAME})
set_target_visibility(${PROJECT_NAME})

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        VERSION 1.0.0
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE 
        ${SDL2_INCLUDE_DIR}
        ${SDL2_IMAGE_INCLUDE_DIR}
        ${SDL2_TTF_INCLUDE_DIR}
        ${SDL2_MIXER_INCLUDE_DIR}
)

if(UNIX)
    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
            -lX11
    )
endif()

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        resource_utils
    PRIVATE
        ${SDL2_LIBRARY}       # -lSDL2 flag
        ${SDL2_IMAGE_LIBRARY} # -lSDL2_image flag
        ${SDL2_TTF_LIBRARY}   # -lSDL2_ttf flag
        ${SDL2_MIXER_LIBRARY} # -lSDL2_mixer flag  
)

#invoke include-what-you-use (if it's enabled) for the current project
if(USE_IWYU)
    enable_target_include_what_you_use(${PROJECT_NAME})
endif(USE_IWYU)

if(BUILD_LIKE_POSITION_INDEPENDANT_CODE)
    enable_target_position_independent_code(${PROJECT_NAME})
endif()  

