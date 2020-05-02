#ifndef SDL_UTILS_RENDERERDEFINES_H_
#define SDL_UTILS_RENDERERDEFINES_H_

// C system headers

// C++ system headers
#include <cstdint>

// Other libraries headers

// Own components headers

/* Values:
 * USE_SOFTWARE_RENDERER 0 - use Hardware renderer
 * USE_SOFTWARE_RENDERER 1 - use Software renderer
 *
 * Software renderer means Image/Text Surface's are processed via the
 * Central Processor Unit (CPU).
 *
 * Benefits:
 *      - GPU is not needed to perform draw calls;
 *      - The Graphical User Interface (GUI) will work in a virtual machine;
 * Virtual machines can not use the GPU since the host machine is using it.
 *
 * Drawbacks:
 *      - Performance is painfully slow;
 *
 *
 *
 * Hardware renderer means hardware acceleration is used.
 *
 * Benefits:
 *      - Surfaces are not longer used. Instead Textures are created from them,
 * which are processed by the GPU.
 *      - Performance is much faster;
 *
 * Drawbacks:
 *      - GUI could not be displayed when working on virtual machines
 *                  (at least on older VMs that does not use the physical GPU);
 * */

#define USE_SOFTWARE_RENDERER 0

/* Values:
 *
 * USE_FAST_QUALITY_TEXT 0
 * Text quality is better, because Anti-Aliasing is applied. What this means is
 * that alpha blending to dither the font with the given color is applied.
 * This created a way smoother text, but is around 20% - 40% slower that the
 * fast low quality version.
 *
 *
 * USE_FAST_QUALITY_TEXT 1
 * The quality is poor, because the text is pixel drawn resulting in a "edge"
 * shaped text. While the text is not so good looking it is way more faster
 * than it's Anti-Aliasing counterpart.
 * */
#define USE_FAST_QUALITY_TEXT 0

/* Values:
 *
 * DISABLE_DOUBLE_BUFFERING_SWAP_INTERVAL 0
 * Does not interfere with swap interval. This will result in underlying
 * graphics API "swap_buffers" command to block and assure the frame capping
 * set by the monitor max refresh rate.
 *
 * DISABLE_DOUBLE_BUFFERING_SWAP_INTERVAL 1
 * Make the double buffering swap interval to 0. What this means is that the
 * underlying graphics API, will not block "swap_buffers" command and will
 * leave control /frame capping/ to the application side.
 *
 * IMPORTANT NOTE: this feature will work only when VSync is enabled.
 * */
#define DISABLE_DOUBLE_BUFFERING_SWAP_INTERVAL 0

/* Values:
 * ENABLE_VSYNC 1
 * Enabled VSync, which caps the application frame rate to the maximum monitor
 * refresh rate.
 *
 * ENABLE_VSYNC 0
 * Disables VSync, which leaves the frame rate control to be handled by the
 * application.
 * */
#define ENABLE_VSYNC 0

enum class RendererCmd : uint8_t {
  CLEAR_SCREEN = 0,
  FINISH_FRAME,
  CHANGE_CLEAR_COLOR,
  LOAD_TEXTURE_SINGLE,
  LOAD_TEXTURE_MULTIPLE,
  DESTROY_TEXTURE,
  CREATE_VBO,   // VBO (vertex buffer object)
  DESTROY_VBO,  // VBO (vertex buffer object)
  CHANGE_RENDERER_TARGET,
  RESET_RENDERER_TARGET,
  CLEAR_RENDERER_TARGET,
  UPDATE_RENDERER_TARGET,
  CHANGE_TEXTURE_BLENDMODE,
  CHANGE_TEXTURE_OPACITY,
  CREATE_TTF_TEXT,
  RELOAD_TTF_TEXT,
  DESTROY_TTF_TEXT,
  ENABLE_DISABLE_MULTITHREAD_TEXTURE_LOADING,
  EXIT_RENDERING_LOOP,

  UNDEFINED = 255
};

namespace RendererDefines {
enum Defines {
  ENGINE_REAL_TIME_IMAGES = 20,
  ENGINE_REAL_TIME_TEXTS = 90,
  ENGINE_REAL_TIME_VBO = 20,

  GAME_AVERAGE_REAL_TIME_IMAGES = 60,
  GAME_AVERAGE_REAL_TIME_TEXTS = 110,
  GAME_AVERAGE_REAL_TIME_VBO = 30,

  MAX_SIMULTANEOUSLY_SUPPORTED_GAMES = 1,

  MAX_REAL_TIME_GAME_WIDGETS = GAME_AVERAGE_REAL_TIME_IMAGES +
                               GAME_AVERAGE_REAL_TIME_TEXTS +
                               GAME_AVERAGE_REAL_TIME_VBO,

  MAX_REAL_TIME_IMAGES_COUNT =
      ENGINE_REAL_TIME_IMAGES +
      (MAX_SIMULTANEOUSLY_SUPPORTED_GAMES * GAME_AVERAGE_REAL_TIME_IMAGES),

  MAX_REAL_TIME_TEXT_COUNT =
      ENGINE_REAL_TIME_TEXTS +
      (MAX_SIMULTANEOUSLY_SUPPORTED_GAMES * GAME_AVERAGE_REAL_TIME_TEXTS),

  MAX_REAL_TIME_VBO_COUNT =
      ENGINE_REAL_TIME_VBO +
      (MAX_SIMULTANEOUSLY_SUPPORTED_GAMES * GAME_AVERAGE_REAL_TIME_VBO),

  MAX_REAL_TIME_WIDGETS_COUNT =
      MAX_REAL_TIME_IMAGES_COUNT + MAX_REAL_TIME_TEXT_COUNT +
      MAX_REAL_TIME_VBO_COUNT + GAME_AVERAGE_REAL_TIME_VBO,

  RENDERER_MAX_COMMANDS = MAX_REAL_TIME_WIDGETS_COUNT * 2
};
}

#endif /* SDL_UTILS_RENDERERDEFINES_H_ */
