#define TRACING_ENABLED 1

// Corresponding header
#include "sdl_utils/SDLLoader.h"

// C system headers
#ifdef __linux__
#include <X11/Xlib.h>
#endif /* __linux__ */

// C++ system headers

// Other libraries headers
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include "utils/debug/FunctionTracer.h"
#include "utils/ErrorCode.h"
#include "utils/Log.h"

// Own components headers
#include "sdl_utils/sound/defines/SoundMixerDefines.h"

int32_t SDLLoader::init() {
  TRACE_ENTRY_EXIT;

#ifdef __linux__
  // needed for X multhithread support
  if (!XInitThreads()) {
    LOGERR("Error in XInitThreads() -> Terminating ...");
    return FAILURE;
  }
#endif /* __linux__ */

  if (-1 == TTF_Init()) {
    LOGERR("SDL_ttf could not initialize! SDL_ttf Error: %s", TTF_GetError());
    return FAILURE;
  }

  if (0 > SDL_Init(SDL_INIT_VIDEO)) {
    LOGERR("SDL could not be initialised! SDL Error: %s", SDL_GetError());
    return FAILURE;
  }

  // Initialise PNG loading
  constexpr int32_t imgFlags = IMG_INIT_PNG;
  if (!(IMG_Init(imgFlags) & imgFlags)) {
    LOGERR("SDL_image could not be initialised! SDL_image Error: %s",
        IMG_GetError());
    return FAILURE;
  }

  if (0 > SDL_Init(SDL_INIT_AUDIO)) {
    LOGERR("SDL Audio could not be initialised! SDL Error: %s",
           SDL_GetError());
    return FAILURE;
  }

  if (0 > Mix_OpenAudio(FREQUENCY,           // sound frequency
                        MIX_DEFAULT_FORMAT,  // sample format
                        STEREO_CHANNELS,     // stereo hardware channels
                        SOUND_CHUNK_SIZE))   // chunk size
  {
    LOGERR("SDL_mixer could not initialised! SDL_mixer Error: %s",
           Mix_GetError());
    return FAILURE;
  }

  // fix a bug in SDL with version lower than 2.0.10
  // https://discourse.libsdl.org/t/sdl2-lag-with-sdl-getticks/25538/7
  if (!SDL_VERSION_ATLEAST(2, 0, 10)) {
    if (0 > SDL_Init(SDL_INIT_JOYSTICK)) {
      LOGERR("SDL Joystick could not be initialised! SDL Error: %s",
             SDL_GetError());
      return FAILURE;
    }
  }

  return SUCCESS;
}

void SDLLoader::deinit() {
  TRACE_ENTRY_EXIT;

  // Quit SDL subsystems
  IMG_Quit();
  TTF_Quit();
  Mix_Quit();
  SDL_Quit();
}
