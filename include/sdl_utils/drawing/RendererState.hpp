#ifndef SDL_UTILS_RENDERERSTATE_HPP_
#define SDL_UTILS_RENDERERSTATE_HPP_

// C system headers

// C++ system headers

// Other libraries headers
#include "RendererDefines.h"

// Own components headers
#include "sdl_utils/drawing/DrawParams.h"
#include "utils/common/AutoCircularBuffer.hpp"

#if USE_SOFTWARE_RENDERER
#include "utils/drawing/Color.h"
#endif /* USE_SOFTWARE_RENDERER*/

namespace RendererStateDefines {
enum Defines {
  // keep buffer size a multiple of 2 -> (2^16)
  RENDER_DATA_BUFFER_SIZE = UINT16_MAX + 1
};
}

/** WARNING: Since RendererState is crucial for performance -> strictly keep
 *          any method outside of the RendererState struct definition.
 *          Placing it inside will automatically exclude big number of
 *                                                 compile time optimisations.
 **/
struct RendererState {
  RendererState()
#if USE_SOFTWARE_RENDERER
      : clearColor(Colors::BLACK)
#endif /* USE_SOFTWARE_RENDERER*/
  {
    currWidgetCounter = 0;
    currRendererCmdsCounter = 0;
    lastTotalWidgetCounter = 0;
    globalOffsetX = 0;
    globalOffsetY = 0;
    enabledMonitorsCount = 0;
    isLocked = true;

    for (int32_t i = 0; i < RendererDefines::RENDERER_MAX_COMMANDS; ++i) {
      rendererCmd[i] = RendererCmd::UNDEFINED;
    }
  }

  /** Holds stored number of widgets for the current frame
   * */
  uint32_t currWidgetCounter;

  /** Holds stored number or renderer commands for the current frame
   * */
  uint32_t currRendererCmdsCounter;

  /** Store total widget in order to monitor their number in the
   *  graphical debug console
   * */
  uint32_t lastTotalWidgetCounter;

  /* Holds global moves offsets (by default they are not used) */
  int32_t globalOffsetX;
  int32_t globalOffsetY;

  // Holds the number of enabled monitors
  int32_t enabledMonitorsCount;

  /* A container for all draw specific widget data.
   * Since rendering will be the most heavy operation we need to prepare
   * draw specific data in the most comfortable way.
   *
   * An array is chosen here, so we can guarantee that we will have
   * 0(1) lookup time and constant cache hits, because of the
   * sequential memory layout
   * */
  DrawParams widgets[RendererDefines::MAX_REAL_TIME_WIDGETS_COUNT];

  /* Used to store draw specific rendering commands populated by
   * the main(update) thread
   * */
  RendererCmd rendererCmd[RendererDefines::RENDERER_MAX_COMMANDS];

  /** A buffer that holds all draw specific data populated by
   *  the main(update) thread
   **/
  AutoCircularBuffer<RendererStateDefines::RENDER_DATA_BUFFER_SIZE> renderData;

  /** Used to determine whether the render is locked or not.
   *     > If Renderer is locked - the default renderer target is used;
   *     > if Renderer is unlocked - probably the renderer target
   *          has been changed to some other SDL_Texture
   *                                     (usually owned by SpriteBuffer);
   * */
  bool isLocked;

#if USE_SOFTWARE_RENDERER
  Color clearColor;
#endif /* USE_SOFTWARE_RENDERER*/
};

#endif //SDL_UTILS_RENDERERSTATE_HPP_
