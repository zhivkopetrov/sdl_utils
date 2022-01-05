// Corresponding header
#include "sdl_utils/containers/FontContainer.h"

// C system headers

// C++ system headers

// Other libraries headers
#include <SDL_ttf.h>

// Own components headers
#include "sdl_utils/drawing/LoadingScreen.h"
#include "utils/ErrorCode.h"
#include "utils/Log.h"

typedef std::unordered_map<uint64_t, TTF_Font*>::iterator _fontsMapIt;
typedef std::unordered_map<uint64_t, TTF_Font*>::const_iterator
    _fontsMapConstIt;

typedef std::unordered_map<uint64_t, FontData>::iterator _fontsDataMapIt;
typedef std::unordered_map<uint64_t, FontData>::const_iterator
    _fontsDataMapConstIt;

int32_t FontContainer::init(const uint64_t fontsCount) {
  _fontsDataMap.reserve(fontsCount);
  _fontsMap.reserve(fontsCount);

  return SUCCESS;
}

void FontContainer::deinit() {
  // free Font Textures
  for (_fontsMapIt it = _fontsMap.begin(); it != _fontsMap.end(); ++it) {
    TTF_CloseFont(it->second);

    it->second = nullptr;
  }

  // clear TTF_Font unordered_map and shrink size
  _fontsMap.clear();

  // clear FontData unordered_map and shrink size
  _fontsDataMap.clear();
}

void FontContainer::loadAllStoredFonts() {
  TTF_Font* font = nullptr;

  _fontsDataMapConstIt it;

  for (it = _fontsDataMap.begin(); it != _fontsDataMap.end(); ++it) {
    if (SUCCESS != loadTtfFont(it->second.header.path.c_str(),
                               it->second.fontSize, font)) {
      LOGERR("Failed to load %s font! SDL_ttf Error: %s",
             it->second.header.path.c_str(), TTF_GetError());
    } else {
      // populate _fontsMap with the newly created font
      _fontsMap[it->second.header.hashValue] = font;

      // send message to loading screen for successfully loaded resource
      LoadingScreen::onNewResourceLoaded(it->second.header.fileSize);

      // reset font variable so it can be reused
      font = nullptr;
    }
  }
}

int32_t FontContainer::loadTtfFont(const char* path, const int32_t fontSize,
                                   TTF_Font*& outFont) {
  // Open the font
  outFont = TTF_OpenFont(path, fontSize);
  if (nullptr == outFont) {
    LOGERR("Failed to load %s font! SDL_ttf Error: %s", path, TTF_GetError());
    return FAILURE;
  }

  return SUCCESS;
}
