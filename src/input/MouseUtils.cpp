// Corresponding header
#include "sdl_utils/input/MouseUtils.h"

// System headers

// Other libraries headers
#include <SDL_mouse.h>
#include <SDL_image.h>
#include "utils/ErrorCode.h"
#include "utils/log/Log.h"

// Own components headers

static SDL_Cursor *cursor = nullptr;

int32_t MouseUtils::toggleCursorStatus(const int32_t cursorStatus) {
  return SDL_ShowCursor(cursorStatus);
}

ErrorCode MouseUtils::createCursorFromImg(const char *imagePath,
                                          const int32_t cursorClickX,
                                          const int32_t cursorClickY) {
  SDL_Surface *cursorSurface = IMG_Load(imagePath);

  if (nullptr == cursorSurface) {
    LOGERR("Unable to load image %s! SDL_image Error: %s", imagePath,
        IMG_GetError());
    return ErrorCode::FAILURE;
  }

  cursor = SDL_CreateColorCursor(cursorSurface, cursorClickX, cursorClickY);

  if (nullptr == cursor) {
    LOGERR("Unable to create color cursor! SDL_image Error: %s",
        IMG_GetError());
    return ErrorCode::FAILURE;
  }

  SDL_SetCursor(cursor);
  SDL_FreeSurface(cursorSurface);

  return ErrorCode::SUCCESS;
}

void MouseUtils::freeCursor() {
  if (cursor) //sanity check
  {
    SDL_FreeCursor(cursor);
    cursor = nullptr;
  }
}
