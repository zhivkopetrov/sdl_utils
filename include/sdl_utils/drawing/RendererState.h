#ifndef SDL_UTILS_RENDERERSTATE_H_
#define SDL_UTILS_RENDERERSTATE_H_

// System headers
#include <cstdint>
#include <vector>

// Other libraries headers
#include "utils/common/CircularBuffer.h"
#include "utils/ErrorCode.h"

// Own components headers
#include "sdl_utils/drawing/defines/RendererDefines.h"
#include "sdl_utils/drawing/DrawParams.h"

// Forward declarations
struct RendererConfig;

struct RendererState {
  ErrorCode init(const RendererConfig &cfg);

  /** Holds stored number of widgets for the current frame
   * */
  uint32_t currWidgetCounter = 0;

  /** Holds stored number or renderer commands for the current frame
   * */
  uint32_t currRendererCmdsCounter = 0;

  /** Store total widget in order to monitor their number in the
   *  graphical debug console
   * */
  uint32_t lastTotalWidgetCounter = 0;

  /** Holds widgets.size() and rendererCmd.size()
   * */
  uint32_t maxRuntimeWidgets = 0;
  uint32_t maxRuntimeRendererCmds = 0;

  /* Holds global moves offsets (by default they are not used) */
  int32_t globalOffsetX = 0;
  int32_t globalOffsetY = 0;

  /* A container for all draw specific widget data.
   * Since rendering will be the most heavy operation we need to prepare
   * draw specific data in the most comfortable way.
   *
   * An array is chosen here, so we can guarantee that we will have
   * 0(1) lookup time and constant cache hits, because of the
   * sequential memory layout
   * */
  std::vector<DrawParams> widgets;

  /* Used to store draw specific rendering commands populated by
   * the main(update) thread
   * */
  std::vector<RendererCmd> rendererCmd;

  /** A buffer that holds all draw specific data populated by
   *  the main(update) thread
   **/
  CircularBuffer renderData;

  /** Used to determine whether the render is locked or not.
   *     > If Renderer is locked - the default renderer target is used;
   *     > if Renderer is unlocked - probably the renderer target
   *          has been changed to some other SDL_Texture
   *                                     (usually owned by SpriteBuffer);
   * */
  bool isLocked = true;
};

#endif //SDL_UTILS_RENDERERSTATE_H_
