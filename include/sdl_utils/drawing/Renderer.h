#ifndef SDL_UTILS_RENDERER_H_
#define SDL_UTILS_RENDERER_H_

// System headers
#include <condition_variable>
#include <cstdint>
#include <mutex>

// Other libraries headers
#include "utils/class/NonCopyable.h"
#include "utils/class/NonMoveable.h"
#include "utils/ErrorCode.h"

// Own components headers
#include "sdl_utils/drawing/defines/RendererDefines.h"
#include "sdl_utils/drawing/RendererState.h"

// Forward declarations
class SDLContainers;
class Color;
struct SDL_Window;
struct SDL_Surface;
struct SDL_Renderer;

class Renderer : public NonCopyable, public NonMoveable {
 public:
  Renderer();

  /** @brief used to initialise the actual SDL_Renderer in the
   *                                          renderer(secondary) thread.
   *
   *  @param SDL_Window *  - window created by the OS (handled by SDL)
   *
   *  @return int32_t - error code
   *
   *   WARNING: When using multithreading approach there are severe
   *            limitations by the underlying graphics API (OpenGL).
   *
   *            All OpenGL calls should happen only from 1 thread!
   *            That is the thread that is currently owning
   *                                                  the OpenGL context.
   *
   *            Multithreading draw could be performed only with one of
   *            the following approaches:
   *
   *            > Approach A) (SDL/OpenGL)
   *             1) Create window in main thread;
   *             2) Create OpenGL context(or SDL_Renderer)
   *                                            in renderer(main) thread;
   *             3) Spawn a secondary(update) thread for the
   *                                        update thread(business logic)
   *
   *            > Approach B) (OpenGL)
   *             1) Create window in main thread;
   *             2) Create OpenGL context(or SDL_Renderer) in main thread;
   *             3) Pass the OpenGL context and created window
   *                                        to renderer(secondary) thread;
   *             4) Release current OpenGL context in main thread
   *                                  SDL_GL_MakeCurrent(window, nullptr);
   *             5) Acquire OpenGL context in renderer(secondary) thread
   *                                  SDL_GL_MakeCurrent(window, context);
   *
   *            > Approach C) (OpenGL)
   *             1) Create window in main thread;
   *             2) Pass the created window to renderer(secondary) thread;
   *             3) Create OpenGL context(or SDL_Renderer)
   *                                        in renderer(secondary) thread;
   *
   *   NOTE: Approach A) is chosen - since the program uses SDL_Renderer
   *
   *   NOTE2: ::init() is executed before the secondary thread is still
   *          spawned (it is being executed on the main thread)
   * */
  ErrorCode init(const RendererConfig& cfg);

  /** @brief used to destroy renderer and window
   *
   *   NOTE: ::deinit() is executed after the threads are joined
   *                            (it is being executed on the main thread)
   * */
  void deinit();

  /** @brief used to provide the renderer with an API to comunicate with
   *         containers that are responsible for holding
   *                              the loaded SDL_Surface's/SDL_Texture's
   *
   *  @param SDLContainers * - address of actual containers
   *
   *         NOTE: this step can NOT be done as part of the ::init()
   *               method, because when ::init() method is being invoked
   *               there is no way the SDLCointainers object is already
   *               constructed
   * */
  void setSDLContainers(SDLContainers* containers);

  //================= START UPDATE THREAD INTERFACE =====================
  // NOTE: @_UT - update thread interface

  //=====================================================================
  /** @brief Every frame should start with this function call
   * */
  void clearScreen_UT();

  /** All the drawing functions calls should be encapsulated
   *   between clearScreen() and finishFrame() functions
   * */

  /** @brief Every frame should end with this function call
   *
   *  @param const bool - developer option to override the the check
   *                      for locked/unlocked status of the renderer.
   *
   *         NOTE: this can be used for example when the developer is
   *               sure he want's to "steal" the draw call from default
   *               renderer target to other provided one
   *               (usually a SpriteBuffer).
   *
   *         WARNING: set the flag to true only if you are certain what
   *                  you are doing
   * */
  void finishFrame_UT(const bool overrideRendererLockCheck);
  //=====================================================================

  /** @brief transfer draw specific data from Widgets to renderer
   *         NOTE: drawParams are being copied here.
   *         The reason is those commands are sent
   *         via a thread-safe mechanism on a different thread that
   *         will perform all the draw calls
   *
   *  @param const DrawParams & - draw specific data for a single Widget
   * */
  void addDrawCmd_UT(const DrawParams& drawParams) const;

  /* @brief used to store draw specific rendering commands populated by
   *                                              the main(update) thread
   *
   * @param const RendererCmd - render specific command
   * @param const uint8_t *   - inData buffer
   * @param const uint64_t    - bytes count to write
   * */
  void addRendererCmd_UT(const RendererCmd rendererCmd,
                         const uint8_t* data = nullptr,
                         const uint64_t bytes = 0);

  /* @brief used to store draw specific data populated by
   *                                              the main(update) thread
   *
   * @param const uint8_t *   - inData buffer
   * @param const uint64_t    - bytes count to write
   *
   *       NOTE: this method is intended to be use separately of
   *             ::addRendererCmd_UT() only for performance reasons.
   *
   *       Example: SpriteBuffer what has 30 stored widgets;
   *                const uint32_t SIZE = 30;
   *                ::addRendererData(
   *                             reinterpret_cast<const uint8_t *(&SIZE),
   *                             sizeof(SIZE));
   *
   *                After which ::addRendererCmd_UT(
   *                             RendererCmd::UPDATE_RENDERER_TARGET,
   *                             reinterpret_cast<const uint8_t *(
   *                                              _storedWidgets.data()),
   *                             sizeof(DrawParams) * SIZE)
   * */
  void addRendererData_UT(const uint8_t* data, const uint64_t bytes);

  /** @brief used to sending the main(thread) a message to exit it's
   *         render loop
   * */
  void shutdownRenderer_UT();

  /** @brief used to swap the command back buffers
   *                       (swap the update and rendering thread targets)
   *
   *         WARNING: do NOT invoke this method if you don't know what
   *                  you are doing
   * */
  void swapBackBuffers_UT();

  /** @brief used to acquire count for last frame total drawn widgets
   *
   *  @returns uint32_t - total widget count currently being drawn
   * */
  uint32_t getTotalWidgetCount_UT() const {
    return _rendererState[_updateStateIdx].lastTotalWidgetCounter;
  }

  /** @brief used to unlock the currently used renderer.
   *         When the renderer is unlock - the default renderer target
   *         could be changed to some other Surface/Texture
   *                                      (usually owned by SpriteBuffer)
   *
   *         WARNING: after you finish your work with the renderer
   *                  a call to ::lockRenderer_UT() must be made in the
   *                  same draw cycle (before a call to
   *                                                 ::finishFrame_UT()).
   *                  If this is not done - the renderer will not it's
   *                  default target and no drawing
   *                                          will be performed at all.
   *
   *  @return ErrorCode - error code
   * */
  ErrorCode unlockRenderer_UT();

  /** @brief used to lock the currently used renderer.
   *         When the renderer is locked - the default
   *                                              renderer target is used
   *
   *         NOTE: when this method is invoked - the currently used
   *                          renderer sets back it's default draw target.
   *
   *  @return ErrorCode - error code
   * */
  ErrorCode lockRenderer_UT();

  /** @brief takes a snapshot of current renderer pixels
   *
   *  @param const char*               - file path
   *  @param const ScreenshotContainer - type of container [PNG, JPG, ...]
   *  @param const int32_t             - quality (applied only for JPG)
   *                                     range: [0, 100], worst(0) - best(100)
   *
   *  WARNING: the renderer operation that would be performed from this call
   *           is quite slow and should not be used in performance
   *           critical parts of the code
   * */
  void takeScreenshot_UT(const char *file, const ScreenshotContainer container,
                         const int32_t quality);

  /** @brief used to set global renderer clear color with which default
   *         renderer target is cleared once ::clearScreen() is invoked.
   *
   *  @param const Color & - clear color
   * */
  void setRendererClearColor_UT(const Color& clearColor);

  /** @brief resets absolute global renderer axis movement
   * */
  void resetAbsoluteGlobalMovement_UT();

  /** @brief set absolute global renderer X and Y axis movement
   *         NOTE: this is applied to all stored widgets
   *                                               for the current frame
   *
   *  @param const int32_t - X axis movement
   *  @param const int32_t - Y axis movement
   * */
  void setAbsoluteGlobalMovement_UT(const int32_t x, const int32_t y);

  /** @brief moves global renderer X coordinate
   *
   *  @param const int32_t - X axis movement
   * */
  void moveGlobalX_UT(const int32_t x);

  /** @brief moves global renderer Y coordinate
   *
   *  @param const int32_t - Y axis movement
   * */
  void moveGlobalY_UT(const int32_t y);

  // NOTE: @_UT - update thread interface
  //================== END UPDATE THREAD INTERFACE ======================

  //================= START RENDER THREAD INTERFACE =====================
  // NOTE: @_RT - render thread interface

  /** @brief used as a entry level for the rendering(main) thread to
   *         enter it's drawing loop
   * */
  void executeRenderCommands_RT();

 private:
  /** @brief clear the window/screen with current selected clear color
   * */
  void clearScreenExecution_RT();

  /** @brief makes the actual draw calls and uploads the final texture
   *         from the GPU to the window/display
   * */
  void finishFrameExecution_RT();

  /** @brief changes clear color for the actual hardware renderer
   * */
  void changeClearColor_RT();

  /** @brief loads a single texture (uploads the vertex data to the GPU)
   *         and free's the surface
   * */
  void loadTextureSingle_RT();

  /** @brief loads multiple textures (uploads the vertex data to the GPU)
   *         and free's their surfaces.
   *
   *         NOTE: implementation uses ThreadSafeQueue.
   *               the render thread is waiting on a condition variable
   *               and uploading vertex data to the GPU while waiting
   *               for new Surfaces to be generated (if there are not any)
   * */
  void loadTextureMultiple_RT();

  /** @brief destroys a single texture (releases memory on the GPU)
   * */
  void destroyTexture_RT();

  /** @brief created a single FBO(frame buffer object).
   *         A FBO is a special texture which is marked with
   *         TEXTURE_STREAMING by OpenGL.
   *
   *         It's idea is to be constantly changed so the GPU can know
   *         to locate it in a place where it can be accessed faster.
   * */
  void createFBO_RT();

  /** @brief destroys a FBO(frame buffer object) (releases memory on the GPU)
   * */
  void destroyFBO_RT();

  /** @brief used to change the renderer target to a specific VBO
   * */
  void changeRendererTarget_RT();

  /** @brief used to change the renderer target back to the
   *                                         monitor/display back buffer.
   * */
  void resetRendererTarget_RT();

  /** @brief used to clear the current renderer target(some VBO) with
   *         selected clear color.
   * */
  void clearRendererTarget_RT();

  /** @brief used to update the current renderer target(some VBO).
   *         The VBO update process is identical to normal draw calls on
   *         the monitor/display but they are placed on top of the VBO.
   *
   *         NOTE: relativity of the coordinates is still in place here.
   *         (local monitor coordinates are not transformed to global
   *                                                  window coordinates)
   * */
  void updateRendererTarget_RT();

  /** @brief used to update the blending of the selected texture
   * */
  void changeTextureBlending_RT();

  /** @brief used to update the opacity of the selected texture
   * */
  void changeTextureOpacity_RT();

  /** @brief loads a single text texture (makes TTF calls and uploads
   *         the vertex data to the GPU)
   *
   *  @param const bool - is text being reloaded or created for first time
   * */
  void createTTFText_RT(const bool isTextBeingReloaded);

  /** @brief destroys a single texture (releases memory on the GPU)
   *
   *  @return int32_t - unique TextContainer text ID
   * */
  int32_t destroyTTFText_RT();

  /** @brief actual rendering of stored widgets on the current back buffer
   *
   *  @param const std::vector<DrawParams> -
   *                                  draw specific data for a Texture array
   * */
  void drawWidgetsToBackBuffer_RT(const DrawParams drawParamsArr[],
                                  const uint32_t size);

  /** @brief used to take a screenshot from the current renderer pixels
   * */
  void takeScreenshot_RT();

  /** @brief used to update the state of the multithreading texture
   *                                                     loading strategy
   * */
  void enableDisableMultithreadTextureLoading_RT();

  /** @brief used to apply global offset to the currently stored for the
   *         frame draw params of the widgets
   *
   *  @param const uint32_t - widgets size
   * */
  void applyGlobalOffsets_RT(const uint32_t widgetsSize);

  // NOTE: @_RT - render thread interface
  //================== END RENDER THREAD INTERFACE ======================

  enum InternalDefines { SUPPORTED_BACK_BUFFERS = 2 };

  // The window we'll be rendering to
  SDL_Window* _window;

  // The Hardware Accelerated Renderer
  SDL_Renderer* _sdlRenderer;

  SDLContainers* _containers;

  /** Holds the current _rendererState 'pointers'.
   * _updateStateIdx 'points' to update(main) thread _rendererState data
   * _renderStateIdx 'points' to rendering(secondary) thread
   *                                                 _rendererState data
   **/
  int32_t _updateStateIdx;
  int32_t _renderStateIdx;

  /** used for synchronisation mechanism between update(secondary) and
   *                                              renderer(main) thread
   *
   *      Direction: update thread --> renderer thread
   **/
  std::mutex _updateMutex;
  std::condition_variable _updateCondVar;

  /** used for synchronisation mechanism between update(secondary) and
   *                                              renderer(main) thread
   *
   *      Direction: renderer thread --> update thread
   **/
  std::mutex _renderMutex;
  std::condition_variable _renderCondVar;

  /* Used to store RendererState for the current state.
   *
   * In order to achieve near zero-overhead thread syncronization the
   * whole RendererState is copied 2 or more times.
   *
   * The update(main) thread works on frame N,
   * while the renderer(secondary) threads works on frame N - 1.
   *
   * The lock time will be as much as swapping the two
   * RendererState pointers (double-buffering like approach).
   *
   * After the swap the update(main) thread will continue to populate
   * the new fresh frame, while the renderer(secondary) thread will draw
   * the previous frame.
   *
   * NOTE: For now 2 RendererStates are kept simultaneously. If later on
   * triple buffering is needed -> this number can be increased here
   * from 2 to 3.
   */
  mutable RendererState _rendererState[SUPPORTED_BACK_BUFFERS];

  /** a synchronization flag used to determine whether the renderer
   *  thread is busy.
   *
   *  Direction: renderer thread --> update thread
   **/
  bool _isRendererBusy;

  /* used to mark the state of the renderer
   * This is needed to prevent deadlocks from the update thread
   * when renderer has already been shut downed*/
  bool _isShutdowned;

  /** a flag to determine whether multithreading texture loading is used.
   *     > If TRUE -> other threads are responsible for loading the
   *       images from harddrive to raw pixel data (SDL_Surface) and
   *       the renderer thread is only responsible for uploading the
   *                      vertex data to the GPU (creating SDL_Texture).
   *
   *     > If FALSE -> the renderer thread is responsible for both
   *       loading the images from harddrive to raw pixel data
   *       (SDL_Surface) and uploading the vertex data to the GPU
   *                                             (creating SDL_Texture).
   **/
  bool _isMultithreadTextureLoadingEnabled;
};

#endif /* SDL_UTILS_RENDERER_H_ */
