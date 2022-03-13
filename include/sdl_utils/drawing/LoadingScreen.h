#ifndef SDL_UTILS_LOADINGSCREEN_H_
#define SDL_UTILS_LOADINGSCREEN_H_

// System headers
#include <cstdint>

// Other libraries headers

// Own components headers
#include "utils/drawing/Rectangle.h"
#include "utils/ErrorCode.h"

// Forward declarations
struct SDL_Texture;
struct SDL_Renderer;
struct LoadingScreenConfig;

class LoadingScreen {
 public:
  LoadingScreen() = delete;

  /** @brief used to initialise background loading resources
   *
   *  @param const LoadingScreenConfig & - config for the LoadingScreen
   *
   *  @return ErrorCode      - error code
   * */
  static ErrorCode init(const LoadingScreenConfig &cfg,
                        const int32_t totalFileSize);

  /** @brief used to deinitialise background loading resources
   * */
  static void deinit();

  /** @brief used to notify LoadingScreen that a new resource was loaded.
   *         When enough loadedSize is reached to get to the next
   *         _lastLoadedPercent value (0 - 100) the LoadingScreen
   *         performs internal draw to the screen.
   *
   *         NOTE: this method should only be called from the main thread,
   *               because it is not thread safe.
   * */
  static void onNewResourceLoaded(const int32_t loadedSize);

  /** @brief used to acquire renderer pointer that will be performing
   *                                         the graphical render calls.
   *
   *  @param SDL_Renderer * - the actual hardware renderer
   * */
  static void setRenderer(SDL_Renderer* renderer);

  /** @brief used to set monitor rectangle -> so when renderer clipping
   *         is performed, the clip could be reset back to
   *                                  normal(monitor rectangle) boundary
   *
   *  @param const Rectangle & - the monitor window rectangle
   *  */
  static void setMonitorRect(const Rectangle& monitorRect);

 private:
  /** @brief used to visualise on the screen
   *                                    the loading screen + progress bar
   *
   *  @param const int32_t - currently loaded percentage (0-100)
   * */
  static void draw(const int32_t loadedSize);

  /** Since Renderer is not a static class -> when created Renderer will
   * set it's actual SDL Hardware renderer here so loading screen can be
   * drawn. After that point Loading screen will release it's hold on the
   * Hardware renderer.
   * */
  static SDL_Renderer* _renderer;

  static SDL_Texture* _loadingBackground;

  static SDL_Texture* _progressBarOn;

  static SDL_Texture* _progressBarOff;

  /* Holds total file size for resources that
   *                                 should be load during initialization
   * */
  static int32_t _totalFileSize;

  /* Holds total currently loaded file size for resources that are
   *                                 already loaded during initialization
   * */
  static int32_t _currLoadedFileSize;

  /* Holds last loaded file size for resources that are already
   *                         loaded during initialization (in percentage)
   * */
  static int32_t _lastLoadedPercent;

  static Rectangle _monitorRect;

  static bool _isUsed;
};

#endif /* SDL_UTILS_LOADINGSCREEN_H_ */
