// Corresponding header
#include "sdl_utils/containers/FontContainer.h"

// System headers

// Other libraries headers
#include <SDL_ttf.h>
#include "utils/Log.h"

// Own components headers
#include "sdl_utils/drawing/LoadingScreen.h"


ErrorCode FontContainer::init(const std::string &resourcesFolderLocation,
                              const uint64_t fontsCount) {
  _resourcesFolderLocation = resourcesFolderLocation;
  _fontsDataMap.reserve(fontsCount);
  _fontsMap.reserve(fontsCount);

  return ErrorCode::SUCCESS;
}

void FontContainer::deinit() {
  // free Font Textures
  for (auto& fontsMapPair : _fontsMap) {
    TTF_CloseFont(fontsMapPair.second);
  }

  // clear TTF_Font unordered_map and shrink size
  _fontsMap.clear();

  // clear FontData unordered_map and shrink size
  _fontsDataMap.clear();
}

void FontContainer::loadAllStoredFonts() {
  TTF_Font* font = nullptr;
  std::string widgetPath;

  for (const auto& fontsWidgetPair : _fontsDataMap) {
    const auto& fontWidget = fontsWidgetPair.second;
    widgetPath = _resourcesFolderLocation;
    widgetPath.append(fontWidget.header.path);

    if (ErrorCode::SUCCESS !=
        loadTtfFont(widgetPath.c_str(), fontWidget.fontSize, font)) {
      LOGERR("Failed to load %s font! SDL_ttf Error: %s",
          widgetPath.c_str(), TTF_GetError());
    } else {
      // populate _fontsMap with the newly created font
      _fontsMap[fontWidget.header.hashValue] = font;

      // send message to loading screen for successfully loaded resource
      LoadingScreen::onNewResourceLoaded(fontWidget.header.fileSize);

      // reset font variable so it can be reused
      font = nullptr;
    }
  }
}

ErrorCode FontContainer::loadTtfFont(const char* path, const int32_t fontSize,
                                   TTF_Font*& outFont) {
  // Open the font
  outFont = TTF_OpenFont(path, fontSize);
  if (nullptr == outFont) {
    LOGERR("Failed to load %s font! SDL_ttf Error: %s", path, TTF_GetError());
    return ErrorCode::FAILURE;
  }

  return ErrorCode::SUCCESS;
}
