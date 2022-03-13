// Corresponding header
#include "sdl_utils/drawing/Renderer.h"

// System headers
#include <algorithm>

// Other libraries headers
#include <SDL_hints.h>
#include <SDL_render.h>
#include "utils/concurrency/ThreadSafeQueue.h"
#include "utils/data_type/EnumClassUtils.h"
#include "utils/drawing/Color.h"
#include "utils/Log.h"

// Own components headers
#include "sdl_utils/containers/SDLContainers.h"
#include "sdl_utils/drawing/LoadingScreen.h"
#include "sdl_utils/drawing/Texture.h"
#include "sdl_utils/drawing/config/RendererConfig.h"

#define LOCAL_DEBUG 0

#if LOCAL_DEBUG
namespace {
constexpr const char* RENDERER_CMD_NAMES[]{
    "CLEAR_SCREEN",
    "FINISH_FRAME",
    "CHANGE_CLEAR_COLOR",
    "LOAD_TEXTURE_SINGLE",
    "LOAD_TEXTURE_MULTIPLE",
    "DESTROY_TEXTURE",
    "CREATE_FBO",
    "DESTROY_FBO",
    "CHANGE_RENDERER_TARGET",
    "RESET_RENDERER_TARGET",
    "CLEAR_RENDERER_TARGET",
    "UPDATE_RENDERER_TARGET",
    "CHANGE_TEXTURE_BLENDMODE",
    "CHANGE_TEXTURE_OPACITY",
    "CREATE_TTF_TEXT",
    "RELOAD_TTF_TEXT",
    "DESTROY_TTF_TEXT",
    "ENABLE_DISABLE_MULTITHREAD_TEXTURE_LOADING"
    "EXIT_RENDERING_LOOP",
};
}
#endif /* LOCAL_DEBUG */

Renderer::Renderer()
    : _window(nullptr),
      _sdlRenderer(nullptr),
      _containers(nullptr),
      _updateStateIdx(0),
      _renderStateIdx(1),
      _isRendererBusy(false),
      _isMultithreadTextureLoadingEnabled(false) {}

ErrorCode Renderer::init(const RendererConfig& cfg) {
  _window = cfg.window;

  for (int32_t i = 0; i < SUPPORTED_BACK_BUFFERS; ++i) {
    if (ErrorCode::SUCCESS != _rendererState[i].init(cfg)) {
      LOGERR("_rendererState[%d].init() failed", i);
      return ErrorCode::FAILURE;
    }
  }

  /** Set texture filtering to linear
   *                     (used for image scaling /pixel interpolation/ )
   * */
  if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
    LOGERR("Warning: Linear texture filtering not enabled! "
           "SDL_SetHint() failed. SDL Error: %s", SDL_GetError());
    return ErrorCode::FAILURE;
  }

  // Create the actual hardware renderer for window
  _sdlRenderer =
      SDL_CreateRenderer(_window, -1,
                         SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE
#if ENABLE_VSYNC
                             | SDL_RENDERER_PRESENTVSYNC
#endif /* ENABLE_VSYNC */
      );
  if (nullptr == _sdlRenderer) {
    LOGERR("Renderer could not be created! SDL Error: %s", SDL_GetError());
    return ErrorCode::FAILURE;
  }

  // Initialize renderer color
  if (EXIT_SUCCESS !=
      SDL_SetRenderDrawColor(_sdlRenderer, Colors::BLACK.rgba.r,
                             Colors::BLACK.rgba.g, Colors::BLACK.rgba.b,
                             Colors::BLACK.rgba.a)) {
    LOGERR("Error in, SDL_SetRenderDrawColor(), SDL Error: %s",
           SDL_GetError());
    return ErrorCode::FAILURE;
  }

#if ENABLE_VSYNC && DISABLE_DOUBLE_BUFFERING_SWAP_INTERVAL
  if (SUCCESS != SDL_GL_SetSwapInterval(0)) {
    LOGERR("SDL_GL_SetSwapInterval(0) failed, SDL Error: %s",
           SDL_GetError());
    return FAILURE;
  }
#endif /* ENABLE_VSYNC && DISABLE_DOUBLE_BUFFERING_SWAP_INTERVAL */

  Texture::setRenderer(_sdlRenderer);
  LoadingScreen::setRenderer(_sdlRenderer);

  if (!SDL_RenderTargetSupported(_sdlRenderer)) {
    LOGERR("Warning, Render Target change is not supported on this "
           "platform. This will result in non-working FBOs.");
  }

  return ErrorCode::SUCCESS;
}

void Renderer::deinit() {
  if (_sdlRenderer)  // sanity check
  {
    SDL_DestroyRenderer(_sdlRenderer);
    _sdlRenderer = nullptr;
  }
}

void Renderer::setSDLContainers(SDLContainers *containers) {
  _containers = containers;
}

void Renderer::clearScreen_UT() {
  addRendererCmd_UT(RendererCmd::CLEAR_SCREEN);
}

void Renderer::finishFrame_UT(const bool overrideRendererLockCheck) {
  addRendererCmd_UT(
      RendererCmd::FINISH_FRAME,
      reinterpret_cast<const uint8_t *>(&overrideRendererLockCheck),
      sizeof(overrideRendererLockCheck));

  swapBackBuffers_UT();
}

void Renderer::addDrawCmd_UT(const DrawParams& drawParams) const {
  const int32_t IDX = _updateStateIdx;

#ifndef NDEBUG
  if (_rendererState[IDX].currWidgetCounter >=
      _rendererState[IDX].maxRuntimeWidgets) {
    LOGERR("Critical Problem: maxRunTimeWidgets value: %d is reached! "
           "Increase it's value from the configuration! or reduce the number of"
           " active widgets. Widgets will not be drawn in order to save the "
           "system from crashing", _rendererState[IDX].maxRuntimeWidgets);
    return;
  }
#endif //!NDEBUG

  _rendererState[IDX].widgets[_rendererState[IDX].currWidgetCounter] =
      drawParams;

  // increment the total widget count for this frame
  ++_rendererState[IDX].currWidgetCounter;
}

void Renderer::addRendererCmd_UT(const RendererCmd rendererCmd,
                                 const uint8_t *data, const uint64_t bytes) {
  const int32_t IDX = _updateStateIdx;

#ifndef NDEBUG
  if (_rendererState[IDX].currRendererCmdsCounter >=
      _rendererState[IDX].maxRuntimeRendererCmds) {
    LOGERR("Critical Problem: maxRunTimeRendererCommands value: %d is reached! "
           "Increase it's value from the configuration! or reduce the number of"
           " renderer calls. Renderer command: %hhu will not be execution in "
           "order to save the system from crashing",
           _rendererState[IDX].maxRuntimeRendererCmds,
           getEnumValue(rendererCmd));
    return;
  }
#endif //NDEBUG

  _rendererState[IDX].rendererCmd[_rendererState[IDX].currRendererCmdsCounter] =
      rendererCmd;
  ++_rendererState[IDX].currRendererCmdsCounter;

  if (bytes) {
    if (bytes != _rendererState[IDX].renderData.write(data, bytes)) {
      LOGERR(
          "Warning, Circular buffer overflow for %lu bytes"
          "(write data size is bigger than buffer capacity)!",
          bytes);
    }
  }

#if LOCAL_DEBUG
  LOGC("%s command pushed with %lu bytes of data, commands counter: %u",
       RENDERER_CMD_NAMES[getEnumValue(rendererCmd)], bytes,
       _rendererState[IDX].currRendererCmdsCounter);
#endif /* LOCAL_DEBUG */
}

void Renderer::addRendererData_UT(const uint8_t *data, const uint64_t bytes) {
  if (bytes != _rendererState[_updateStateIdx].renderData.write(data, bytes)) {
    LOGERR(
        "Warning, Circular buffer overflow for %lu bytes"
        "(write data size is bigger than buffer capacity)!",
        bytes);
  }

#if LOCAL_DEBUG
  LOGC("%lu bytes of data is being pushed, value: %u", bytes,
       *(reinterpret_cast<const uint32_t *>(data)));
#endif /* LOCAL_DEBUG */
}

void Renderer::shutdownRenderer_UT() {
  addRendererCmd_UT(RendererCmd::EXIT_RENDERING_LOOP);

  swapBackBuffers_UT();
}

void Renderer::swapBackBuffers_UT() {
  // acquire lock for render mutex
  std::unique_lock<std::mutex> renderLock(_renderMutex);

  /** Condition variables can be subject to spurious wake-ups,
   *  so it is important to check the actual condition
   *  being waited for when the call to wait returns
   * */
  while (_isRendererBusy) {
    _renderCondVar.wait(renderLock);
#if LOCAL_DEBUG
    LOGC("sleeping on _renderCondVar");
#endif /* LOCAL_DEBUG */
  }

#if LOCAL_DEBUG
  LOGC("_renderCondVar informed.");
#endif /* LOCAL_DEBUG */

  // acquire lock for update mutex
  std::unique_lock<std::mutex> updateLock(_updateMutex);

  // acquire isLocked from the update index, before the swap, because
  // after the swap we should operate on the real isLocked variable
  _rendererState[_renderStateIdx].isLocked =
      _rendererState[_updateStateIdx].isLocked;

  // swap containers
  std::swap(_updateStateIdx, _renderStateIdx);

  // prepare the renderer thread 'done' flag
  _isRendererBusy = true;

  // manually unlock the mutex before notifying the renderer thread
  updateLock.unlock();

  // wake the renderer thread that is sleeping on the condition variable
  _updateCondVar.notify_one();
}

ErrorCode Renderer::unlockRenderer_UT() {
  if (_rendererState[_updateStateIdx].isLocked) {
    _rendererState[_updateStateIdx].isLocked = false;
  } else {
    LOGERR("Error, trying to unlock the main renderer, "
          "when it's already unlocked");
    return ErrorCode::FAILURE;
  }

  return ErrorCode::SUCCESS;
}

ErrorCode Renderer::lockRenderer_UT() {
  if (!_rendererState[_updateStateIdx].isLocked) {
    _rendererState[_updateStateIdx].isLocked = true;
    addRendererCmd_UT(RendererCmd::RESET_RENDERER_TARGET);
  } else {
    LOGERR("Error, trying to lock the main renderer, "
           "when it's already locked");
    return ErrorCode::FAILURE;
  }

  return ErrorCode::SUCCESS;
}

void Renderer::setRendererClearColor_UT(const Color &clearColor) {
  addRendererCmd_UT(RendererCmd::CHANGE_CLEAR_COLOR,
                    reinterpret_cast<const uint8_t *>(&clearColor),
                    sizeof(clearColor));
}

void Renderer::resetAbsoluteGlobalMovement_UT() {
  setAbsoluteGlobalMovement_UT(0, 0);
}

void Renderer::setAbsoluteGlobalMovement_UT(const int32_t x, const int32_t y) {
  _rendererState[_updateStateIdx].globalOffsetX = x;
  _rendererState[_updateStateIdx].globalOffsetY = y;
}

void Renderer::moveGlobalX_UT(const int32_t x) {
  _rendererState[_updateStateIdx].globalOffsetX += x;
}

void Renderer::moveGlobalY_UT(const int32_t y) {
  _rendererState[_updateStateIdx].globalOffsetY += y;
}

void Renderer::executeRenderCommands_RT() {
  /** IMPORTANT NOTE: use directly the variable _renderStateIdx, because it
   *                 will constantly be changed in the course of the program
   **/

  // acquire lock on the update update mutex
  std::unique_lock<std::mutex> updateLock(_updateMutex);

  // construct the unique_lock object without locking the _renderMutex
  std::unique_lock<std::mutex> renderLock(_renderMutex, std::defer_lock);

  while (true) {
    /** Condition variables can be subject to spurious wake-ups,
     *  so it is important to check the actual condition
     *  being waited for when the call to wait returns
     * */
    while (0 == _rendererState[_renderStateIdx].currRendererCmdsCounter) {
#if LOCAL_DEBUG
      LOGY("Waiting on _updateCondVar");
#endif /* LOCAL_DEBUG */

      _updateCondVar.wait(updateLock);
    }

#if LOCAL_DEBUG
    LOGY("_updateCondVar informed");
#endif /* LOCAL_DEBUG */

    // lock render mutex -> this prevents the update thread for pushing
    // several updates before this draw is finished
    renderLock.lock();

    const uint32_t RENDER_OP_COUNT =
        _rendererState[_renderStateIdx].currRendererCmdsCounter;

    for (uint32_t idx = 0; idx < RENDER_OP_COUNT; ++idx) {
#if LOCAL_DEBUG
      LOGY_ON_SAME_LINE("command counter: %u. ", idx);
#endif /* LOCAL_DEBUG */

      switch (_rendererState[_renderStateIdx].rendererCmd[idx]) {
        case RendererCmd::CLEAR_SCREEN:
          clearScreenExecution_RT();
          break;

        case RendererCmd::FINISH_FRAME:
          finishFrameExecution_RT();
          break;

        case RendererCmd::CHANGE_CLEAR_COLOR:
          changeClearColor_RT();
          break;

        case RendererCmd::LOAD_TEXTURE_SINGLE:
          loadTextureSingle_RT();
          break;

        case RendererCmd::LOAD_TEXTURE_MULTIPLE:
          loadTextureMultiple_RT();
          break;

        case RendererCmd::DESTROY_TEXTURE:
          destroyTexture_RT();
          break;

        case RendererCmd::CREATE_FBO:
          createFBO_RT();
          break;

        case RendererCmd::DESTROY_FBO:
          destroyFBO_RT();
          break;

        case RendererCmd::CHANGE_RENDERER_TARGET:
          changeRendererTarget_RT();
          break;

        case RendererCmd::RESET_RENDERER_TARGET:
          resetRendererTarget_RT();
          break;

        case RendererCmd::CLEAR_RENDERER_TARGET:
          clearRendererTarget_RT();
          break;

        case RendererCmd::UPDATE_RENDERER_TARGET:
          updateRendererTarget_RT();
          break;

        case RendererCmd::CHANGE_TEXTURE_BLENDMODE:
          changeTextureBlending_RT();
          break;

        case RendererCmd::CHANGE_TEXTURE_OPACITY:
          changeTextureOpacity_RT();
          break;

        case RendererCmd::CREATE_TTF_TEXT:
          createTTFText_RT(false);
          break;

        case RendererCmd::RELOAD_TTF_TEXT:
          createTTFText_RT(true);
          break;

        case RendererCmd::DESTROY_TTF_TEXT:
          destroyTTFText_RT();
          break;

        case RendererCmd::ENABLE_DISABLE_MULTITHREAD_TEXTURE_LOADING:
          enableDisableMultithreadTextureLoading_RT();
          break;

        case RendererCmd::EXIT_RENDERING_LOOP:
          return;

        default:
          LOGERR("Error, received unknown RendererOp: %hhu at index: %u",
              getEnumValue(
                  _rendererState[_renderStateIdx].rendererCmd[idx]), idx);
          break;
      }
    }

    // all renderer commands were executed -> zero out the counter
    _rendererState[_renderStateIdx].currRendererCmdsCounter = 0;

    // manually unlock render mutex -> and let the update thread swap buffers
    //(if its ready with it)
    _isRendererBusy = false;
    renderLock.unlock();

    // wake the update thread that is sleeping on the condition variable
    _renderCondVar.notify_one();
  }
}

void Renderer::clearScreenExecution_RT() {
#if LOCAL_DEBUG
  LOGY("Executing clearScreenExecution_RT() (with 0 bytes of data)");
#endif /* LOCAL_DEBUG */

  // clear screen
  if (EXIT_SUCCESS != SDL_RenderClear(_sdlRenderer)) {
    LOGERR("Error in, SDL_RenderClear(), SDL Error: %s", SDL_GetError());
    return;
  }
}

void Renderer::finishFrameExecution_RT() {
  const int32_t IDX = _renderStateIdx;

  bool overrideRendererLockCheck = false;
  _rendererState[IDX].renderData >> overrideRendererLockCheck;

#if LOCAL_DEBUG
  LOGY("Executing finishFrameExecution_RT(), overrideRendererLockCheck: %d "
       "(with %lu bytes of data)",
       overrideRendererLockCheck, sizeof(overrideRendererLockCheck));
#endif /* LOCAL_DEBUG */

  if (!overrideRendererLockCheck && !_rendererState[IDX].isLocked) {
    LOGERR("WARNING, WARNING, WARNING, Renderer is left unlocked! Consider "
           "locking back the renderer in the same draw cycle after you are "
           "done with your work.");

    LOGC("Developer hint: Maybe you left some FBO unlocked?");

    LOGR("In order for the system to recover from this logical FatalError "
         "main System Renderer will lock itself (probably leaving the "
         "entity that unlocked it in the first place in broken state "
         "/usually this a FBO/ )");

    _rendererState[IDX].isLocked = true;
    resetRendererTarget_RT();
  }

  // store in a local variable for better cache performance
  const uint32_t USED_SIZE = _rendererState[IDX].currWidgetCounter;

#ifndef NDEBUG
  if (0 == USED_SIZE) {
    LOGERR("Critical Error, queued widgets for drawing is 0! This usually "
           "means that there is error internal logic of threads "
           "synchronisation or someone might have called ::finishFrame() "
           "with 0 draw calls made (which is not allowed)");
    return;
  }
#endif /* NDEBUG */

  // apply global offset (if they are turned on)
  applyGlobalOffsets_RT(USED_SIZE);

  // do the actual drawing of all stored images for THIS FRAME
  drawWidgetsToBackBuffer_RT(_rendererState[IDX].widgets.data(), USED_SIZE);

  //------------- UPDATE SCREEN----------------
  SDL_RenderPresent(_sdlRenderer);

  // copy the total widget counter since we are in the end of a frame
  _rendererState[IDX].lastTotalWidgetCounter =
      _rendererState[IDX].currWidgetCounter;

  // reset the widget count
  _rendererState[IDX].currWidgetCounter = 0;
}

void Renderer::changeClearColor_RT() {
  // note: there is no default constructor for color, the Colors::BLACK
  // is just for initialisation
  Color clearColor = Colors::BLACK;

  _rendererState[_renderStateIdx].renderData >> clearColor;

#if LOCAL_DEBUG
  LOGY("Executing changeClearColor_RT(), clerColor32BitRGBA: %u (with %lu "
       "bytes of data", clearColor.get32BitRGBA(), sizeof(clearColor));
#endif /* LOCAL_DEBUG */

  // set renderer drawing color
  if (EXIT_SUCCESS !=
      SDL_SetRenderDrawColor(_sdlRenderer, clearColor.rgba.r, clearColor.rgba.g,
                             clearColor.rgba.b, clearColor.rgba.a)) {
    LOGERR("Error in, SDL_SetRenderDrawColor(), SDL Error: %s", SDL_GetError());
  }
}

void Renderer::loadTextureSingle_RT() {
  uint64_t rsrcId = 0;

  _rendererState[_renderStateIdx].renderData >> rsrcId;

#if LOCAL_DEBUG
  LOGY("Executing loadTextureSingle_RT(), rsrcId: %#16lX with (%lu bytes of "
       "data)",rsrcId, sizeof(rsrcId));
#endif /* LOCAL_DEBUG */

  SDL_Surface *surface = nullptr;

  if (_isMultithreadTextureLoadingEnabled) {
    // temporary variables used for _loadedSurfacesThreadQueue::pop operation
    std::pair<uint64_t, SDL_Surface *> currResSurface(0, nullptr);

    ThreadSafeQueue<std::pair<uint64_t, SDL_Surface *>> *surfaceQueue =
        _containers->getLoadedSurfacesQueue();

    while (true) {
      /** Block rendering thread and wait resources to be pushed
       * into the _loadedSurfacesThreadQueue
       * */
      const auto [isShutdowned, hasTimedOut] =
          surfaceQueue->waitAndPop(currResSurface);
      if (isShutdowned) {
        LOG("surfaceQueue shutdowned");
        return;
      }
      if (hasTimedOut) {
        continue;
      }

      if (rsrcId == currResSurface.first) {
        surface = currResSurface.second;
        break; // correct rsrcId found -> stop the search
      } else {
        /** The popped rsrcId does not follow the request order of the
         * resource -> return it back to the surfaceQueue and extract
         * the next one.
         *
         * This can happen due to the multithreading nature of
         * surfaceQueue. It is not keeping it's elements in the
         * same order as they were inserted.
         *
         * For example a loadTextureSingle_RT() can be requested with
         * rsrcIds 1 2.
         *
         * 2 can be ready at some time and are stored into the
         * surfaceQueue, but 1 is still not.
         *
         * In this method it is required to keep the original order of
         * requested rsrcIds 1 and 2 so -> 2 is returned back to the
         * queue until finally 1 arrives.
         * */
        surfaceQueue->push(std::move(currResSurface));
      }
    }
  } else  // single thread approach
  {
    if (ErrorCode::SUCCESS != _containers->loadSurface(rsrcId, surface)) {
      LOGERR("Error, gRsrcMgrBase->loadSurface() failed for rsrcId: "
             "%#16lX", rsrcId);
      return;
    }
  }

  // remember surface width and height before surface is free()-ed
  const int32_t surfaceWidth = surface->w;
  const int32_t surfaceHeight = surface->h;

  SDL_Texture *texture = nullptr;
  if (ErrorCode::SUCCESS != Texture::loadTextureFromSurface(surface, texture)) {
    LOGERR("Error in Texture::loadTextureFromSurface() for rsrcId: %#16lX",
           rsrcId);
    return;
  }

  // attach newly created SDL_Surface/SDL_Texture
  _containers->attachRsrcTexture(rsrcId, surfaceWidth, surfaceHeight, texture);
}

void Renderer::loadTextureMultiple_RT() {
  uint32_t itemsToPop = 0;
  int32_t batchId = 0;

  _rendererState[_renderStateIdx].renderData >> itemsToPop >> batchId;
  std::vector<uint64_t> rsrcIds(itemsToPop);

#if LOCAL_DEBUG
  LOGY("Executing loadTextureMultiple_RT(), itemsTopPop: %u, batchId: %d "
       "(with %lu bytes of data)", itemsToPop, batchId,
      (sizeof(itemsToPop) + sizeof(batchId) + (itemsToPop * sizeof(uint64_t))));
#endif /* LOCAL_DEBUG */

  for (uint32_t i = 0; i < itemsToPop; ++i) {
    _rendererState[_renderStateIdx].renderData >> rsrcIds[i];

#if LOCAL_DEBUG
    LOGY("Extracting rsrcIds[%u]: %#16lX", i, rsrcIds[i]);
#endif /* LOCAL_DEBUG */
  }

  int32_t currSurfaceWidth = 0;
  int32_t currSurfaceHeight = 0;

  SDL_Texture *texture = nullptr;

  if (_isMultithreadTextureLoadingEnabled) {
    // temporary variables used for _loadedSurfacesThreadQueue::pop operation
    std::pair<uint64_t, SDL_Surface *> currResSurface(0, nullptr);

    ThreadSafeQueue<std::pair<uint64_t, SDL_Surface *>> *surfaceQueue =
        _containers->getLoadedSurfacesQueue();

    // start uploading on the GPU on the rendering thread
    while (0 != itemsToPop) {
      /** Block rendering thread and wait resources to be pushed
       * into the _loadedSurfacesThreadQueue
       * */
      const auto [isShutdowned, hasTimedOut] =
          surfaceQueue->waitAndPop(currResSurface);
      if (isShutdowned) {
        LOG("surfaceQueue shutdowned");
        return;
      }
      if (hasTimedOut) {
        continue;
      }

      auto it = std::find(rsrcIds.begin(), rsrcIds.end(), currResSurface.first);

      if (rsrcIds.end() == it) {
        /** The popped rsrcId does not belong to the requested 'patch'
         * of resources -> return it back to the surfaceQueue and
         * extract the next one.
         *
         * This can happen due to the multithreading nature of
         * surfaceQueue. It is not keeping it's elements in the
         * same order as they were inserted.
         *
         * For example a loadTextureMultiple_RT() can be requested
         * with rsrcIds 1 2 3.
         *
         * 2 and 3 are ready at some time and are stored into the
         * surfaceQueue, but 1 is still not.
         *
         * In that time another call to loadTextureMultiple_RT() with
         * different rsrcIds - lets say 4 5.
         *
         * 4 and 5 are finished and are stored to the surfaceQueue.
         * At this moment 1 is ready and is stored last
         * in the surfaceQueue
         *
         * In this method it is required to process rsrcIds 1 2 and 3.
         * We process 2 and 3, because they are first in the queue and
         * when we pop 4 and 5 -> they are returned back to queue ...
         * multiple times, until finally 1 arrives.
         * */
        surfaceQueue->push(std::move(currResSurface));

        continue;
      }

      // elements successfully found -> remove it
      rsrcIds.erase(it);

      currSurfaceWidth = currResSurface.second->w;
      currSurfaceHeight = currResSurface.second->h;

      if (ErrorCode::SUCCESS !=
          Texture::loadTextureFromSurface(currResSurface.second, texture)) {
        LOGERR("Error in Texture::loadTextureFromSurface() for rsrcId: "
               "%#16lX", currResSurface.first);

        return;
      }

      _containers->attachRsrcTexture(currResSurface.first, currSurfaceWidth,
                                     currSurfaceHeight, texture);

      // reset the variable so it can be reused
      texture = nullptr;

      --itemsToPop;
    }
  } else  // single thread approach
  {
    SDL_Surface *surface = nullptr;
    int32_t currIndex = 0;

    // start uploading on the GPU on the rendering thread
    while (0 != itemsToPop) {
      if (ErrorCode::SUCCESS !=
          _containers->loadSurface(rsrcIds[currIndex], surface)) {
        LOGERR("Error, gRsrcMgrBase->loadSurface() failed for rsrcId: %#16lX",
               rsrcIds[currIndex]);
        return;
      }

      currSurfaceWidth = surface->w;
      currSurfaceHeight = surface->h;

      if (ErrorCode::SUCCESS !=
          Texture::loadTextureFromSurface(surface, texture)) {
        LOGERR("Error in Texture::loadTextureFromSurface() for rsrcId: %#16lX",
               rsrcIds[currIndex]);

        return;
      }

      _containers->attachRsrcTexture(rsrcIds[currIndex], currSurfaceWidth,
                                     currSurfaceHeight, texture);

      // reset the variables so it can be reused
      // it is important to clear both of the variables
      surface = nullptr;
      texture = nullptr;

      --itemsToPop;
      ++currIndex;
    }
  }

  _containers->onLoadTextureMultipleCompleted(batchId);
}

void Renderer::destroyTexture_RT() {
  uint64_t rsrcId = 0;

  _rendererState[_renderStateIdx].renderData >> rsrcId;

#if LOCAL_DEBUG
  LOGY("Executing destroyTexture_RT(), rsrcId: %#16lX (with %lu bytes of "
       "data)",
      rsrcId, sizeof(rsrcId));
#endif /* LOCAL_DEBUG */

  SDL_Texture *texture = nullptr;
  _containers->getRsrcTexture(rsrcId, texture);

  Texture::freeTexture(texture);
  _containers->detachRsrcTexture(rsrcId);
}

void Renderer::createFBO_RT() {
  int32_t width = 0;
  int32_t height = 0;
  int32_t containerId = 0;

  _rendererState[_renderStateIdx].renderData >> width >> height >> containerId;

#if LOCAL_DEBUG
  LOGY("Executing createFBO_RT(), width: %d, height: %d, containerId: %d "
       "(with %lu bytes of data)", width, height, containerId,
       (sizeof(width) + sizeof(height) + sizeof(containerId)));
#endif /* LOCAL_DEBUG */

  SDL_Texture *texture = nullptr;

  // allocate new empty Texture
  if (ErrorCode::SUCCESS !=
      Texture::createEmptyTexture(width, height, texture)) {
    LOGERR("Texture::createEmptyTexture() failed");
    return;
  }

  _containers->attachFbo(containerId, width, height, texture);
}

void Renderer::destroyFBO_RT() {
  int32_t containerId = 0;
  _rendererState[_renderStateIdx].renderData >> containerId;

#if LOCAL_DEBUG
  LOGY("Executing destroyFBO_RT(), containerId: %d (with %lu bytes of data)",
       containerId, sizeof(containerId));
#endif /* LOCAL_DEBUG */

  SDL_Texture *texture = nullptr;
  _containers->getFboTexture(containerId, texture);

  Texture::freeTexture(texture);
  _containers->detachFbo(containerId);
}

void Renderer::changeRendererTarget_RT() {
  SDL_Texture *texture = nullptr;

  int32_t containerId = 0;
  _rendererState[_renderStateIdx].renderData >> containerId;

#if LOCAL_DEBUG
  LOGY("Executing changeRendererTarget_RT(), containerId: %d (with %lu bytes "
       "of data)", containerId, sizeof(containerId));
#endif /* LOCAL_DEBUG */

  _containers->getFboTexture(containerId, texture);

  // set SpriteBuffer texture as renderer target
  if (ErrorCode::SUCCESS != Texture::setRendererTarget(texture)) {
    LOGERR("Error, Texture::setRendererTarget() failed");
  }
}

void Renderer::resetRendererTarget_RT() {
#if LOCAL_DEBUG
  LOGY("Executing resetRendererTarget_RT() (with 0 bytes of data)");
#endif /* LOCAL_DEBUG */

  if (ErrorCode::SUCCESS != Texture::setRendererTarget(nullptr))
  {
    LOGERR("Error, default renderer target could not be set. "
           "SDL_SetRenderTarget() failed, SDL Error: %s", SDL_GetError());
  }
}

void Renderer::clearRendererTarget_RT() {
  // note: there is no default constructor for color, the Colors::BLACK
  // is just for initialisation
  Color clearColor = Colors::BLACK;

  _rendererState[_renderStateIdx].renderData >> clearColor;

#if LOCAL_DEBUG
  LOGY(
      "Executing clearRendererTarget_RT(), clerColor32BitRGBA: %u (with %lu "
      "bytes of data",
      clearColor.get32BitRGBA(), sizeof(clearColor));
#endif /* LOCAL_DEBUG */

  if (ErrorCode::SUCCESS != Texture::clearCurrentRendererTarget(clearColor)) {
    LOGERR("Error in Texture::clearCurrentRendererTarget()");
  }
}

void Renderer::updateRendererTarget_RT() {
  uint32_t itemsSize = 0;
  _rendererState[_renderStateIdx].renderData >> itemsSize;
  const uint64_t DATA_TO_READ = itemsSize * sizeof(DrawParams);

#if LOCAL_DEBUG
  LOGY("Executing updateRendererTarget_RT(), itemsSize: %u (with %lu bytes of"
       " data)", itemsSize, sizeof(DATA_TO_READ) + sizeof(itemsSize));
#endif /* LOCAL_DEBUG */

  DrawParams *storedItems = new DrawParams[itemsSize];
  if (nullptr == storedItems) {
    LOGERR("Error, bad alloc for DrawParams()");
    return;
  }

  if (DATA_TO_READ !=
      _rendererState[_renderStateIdx].renderData.read(
          reinterpret_cast<uint8_t *>(storedItems), DATA_TO_READ)) {
    LOGERR(
        "Warning, Circular buffer overflow(read data requested is "
        "bigger than buffer capacity)! ");

    delete[] storedItems;
    storedItems = nullptr;

    return;
  }

  drawWidgetsToBackBuffer_RT(storedItems, itemsSize);

  delete[] storedItems;
  storedItems = nullptr;
}

void Renderer::changeTextureBlending_RT() {
  WidgetType widgetType = WidgetType::UNKNOWN;
  BlendMode blendmode = BlendMode::NONE;
  uint64_t parsedBytes = 0;

  _rendererState[_renderStateIdx].renderData >> widgetType >> blendmode;
  parsedBytes += (sizeof(widgetType) + sizeof(blendmode));

  SDL_Texture *texture = nullptr;

  if (WidgetType::IMAGE == widgetType) {
    uint64_t rsrcId = 0;
    _rendererState[_renderStateIdx].renderData >> rsrcId;
    parsedBytes += sizeof(rsrcId);

    _containers->getRsrcTexture(rsrcId, texture);
  } else if (WidgetType::TEXT == widgetType) {
    int32_t containerId = 0;
    _rendererState[_renderStateIdx].renderData >> containerId;
    parsedBytes += sizeof(containerId);

    _containers->getTextTexture(containerId, texture);
  } else { // WidgetType::SPRITE_BUFFER == widgetType
    int32_t containerId = 0;
    _rendererState[_renderStateIdx].renderData >> containerId;
    parsedBytes += sizeof(containerId);

    _containers->getFboTexture(containerId, texture);
  }

#if LOCAL_DEBUG
  LOGY("Executing changeTextureBlending_RT(), widgetType: %hhu, blendmode: %hhu"
       " (with %lu bytes of data)",
       getEnumValue(widgetType), getEnumValue(blendmode), parsedBytes);
#endif /* LOCAL_DEBUG */

  if (ErrorCode::SUCCESS != Texture::setBlendMode(texture, blendmode)) {
    LOGERR("Error in Texture::setBlendMode() for  blendMode: %hhu",
        getEnumValue(blendmode));
  }
}

void Renderer::changeTextureOpacity_RT() {
  WidgetType widgetType = WidgetType::UNKNOWN;
  int32_t opacity = 0;
  int32_t containerId = 0;

  _rendererState[_renderStateIdx].renderData >> widgetType >> opacity >>
      containerId;

#if LOCAL_DEBUG
  const uint64_t parsedBytes =
      sizeof(widgetType) + sizeof(opacity) + sizeof(containerId);

  LOGY(
      "Executing changeTextureOpacity_RT(), widgetType: %hhu, opacity: %d "
      "(with %lu bytes of data)",
      getEnumValue(widgetType), opacity, parsedBytes);
#endif /* LOCAL_DEBUG */

  SDL_Texture *texture = nullptr;

  /** changeTextureOpacity_RT is only expected if widget is of type
   * WidgetType::TEXT or WidgetType::SPRITE_BUFFER.
   *
   * WidgetType::IMAGE actually change their alpha together with the
   * actual draw call.
   *
   * The reason for this is because texts and sprite buffers are unique
   * and their alpha could be changed immediately.
   * Images on the other hand are not unique. Several Image objects
   * could reuse the same graphical resource thus changing the alpha
   * for one Image object would result changing the alpha for
   * all of them.
   *
   * Images address this issue by simply storing their opacity as a
   * number and in the moment of actual draw call -> if the opacity is
   * different that FULL_OPACITY:
   *     - a local change in opacity is made;
   *     - the draw call is made;
   *     - restore to FULL_OPACITY is made;
   * */
  if (WidgetType::TEXT == widgetType) {
    _containers->getTextTexture(containerId, texture);
  } else if (WidgetType::SPRITE_BUFFER == widgetType) {
    _containers->getFboTexture(containerId, texture);
  } else { // WidgetType::IMAGE == widgetType
    LOGERR("Error, changeTextureBlending_RT() on WidgetType::IMAGE invoked."
           " Change in Alpha should only be made for WidgetType::TEXT or "
           "WidgetType::SPRITE_BUFFER");
    return;
  }

  Texture::setAlpha(texture, opacity);
}

void Renderer::createTTFText_RT(const bool isTextBeingReloaded) {
  uint64_t fontId = 0;
  uint64_t textLength = 0;
  int32_t containerId = 0;
  int32_t createdWidth = 0;
  int32_t createdHeight = 0;

  // note: there is no default constructor for color, the Colors::BLACK
  // is just for initialisation
  Color textColor = Colors::BLACK;
  SDL_Texture *texture = nullptr;

  uint64_t parsedBytes = sizeof(fontId) + sizeof(textColor) +
                         sizeof(textLength) + textLength;

  if (isTextBeingReloaded) {
    // the containerId remains the same when text is reloaded
    containerId = destroyTTFText_RT();
  } else {
    // fresh new containerId -> read it from renderData
    _rendererState[_renderStateIdx].renderData >> containerId;
    parsedBytes += sizeof(containerId);
  }

  _rendererState[_renderStateIdx].renderData >> fontId >> textColor >>
    textLength;

  //+1 to leave space for manually populating the terminating null character
  char *textContent = new char[textLength + 1];
  if (nullptr == textContent) {
    LOGERR("Error, bad alloc for textContent");
    return;
  }

  textContent[textLength] = '\0';

  if (textLength != _rendererState[_renderStateIdx].renderData.read(
                        reinterpret_cast<uint8_t *>(textContent), textLength)) {
    LOGERR(
        "Warning, Circular buffer overflow(read data requested is "
        "bigger than buffer capacity)!");

    delete[] textContent;
    textContent = nullptr;

    return;
  }

#if LOCAL_DEBUG
  LOGY("Executing cteateTTFText_RT(), contaierID: %d, fontId: %#16lX, "
       "textColor.32bitRGBA: %u, textLenght: %lu, textContent: %s (with %lu "
       "bytes of data)", containerId, fontId, textColor.get32BitRGBA(),
       textLength, textContent, parsedBytes);
#endif /* LOCAL_DEBUG */

  if (ErrorCode::SUCCESS !=
      Texture::loadFromText(textContent, (*_containers->getFontsMap())[fontId],
                           textColor, texture, createdWidth, createdHeight)) {
    LOGERR("Error in loadFromText() for fontId: %#16lX", fontId);

    delete[] textContent;
    textContent = nullptr;

    return;
  }

  _containers->attachText(containerId, createdWidth, createdHeight, texture);

  delete[] textContent;
  textContent = nullptr;
}

int32_t Renderer::destroyTTFText_RT() {
  int32_t containerId = 0;
  _rendererState[_renderStateIdx].renderData >> containerId;

#if LOCAL_DEBUG
  LOGY("Executing destroyTTFText_RT(), containerId: %d (with %lu bytes of "
       "data)", containerId, sizeof(containerId));
#endif /* LOCAL_DEBUG */

  SDL_Texture *texture = nullptr;

  _containers->getTextTexture(containerId, texture);
  Texture::freeTexture(texture);
  _containers->detachText(containerId);

  return containerId;
}

void Renderer::drawWidgetsToBackBuffer_RT(const DrawParams drawParamsArr[],
                                          const uint32_t size) {
  SDL_Texture *texture = nullptr;

  for (uint32_t i = 0; i < size; ++i) {
    if (WidgetType::IMAGE == drawParamsArr[i].widgetType) {
      // for performance reasons look-up is not checked whether an
      // element is found or not. An error should be
      // caught already on init()/create()
      _containers->getRsrcTexture(drawParamsArr[i].rsrcId, texture);

      if (FULL_OPACITY == drawParamsArr[i].opacity) {
        Texture::draw(texture, drawParamsArr[i]);
      } else  // FULL_OPACITY != _widgets[i]
      {
        /** Since several Widgets could be sharing the same graphical
         * resource loaded into memory - the resource opacity could
         * not be left changed by a single widget. This will result
         * in abnormal behavior for the other widget that are using
         * the same graphical resource.
         * */

        // change opacity, perform draw and restore full opacity
        Texture::setAlpha(texture, drawParamsArr[i].opacity);
        Texture::draw(texture, drawParamsArr[i]);
        Texture::setAlpha(texture, FULL_OPACITY);
      }
    } else if (WidgetType::TEXT == drawParamsArr[i].widgetType) {
      // for performance reasons look-up is not checked whether an
      // element is found or not. An error should be
      // caught already on init()/create()
      _containers->getTextTexture(drawParamsArr[i].spriteBufferId, texture);

      Texture::draw(texture, drawParamsArr[i]);
    } else  // WidgetType::SPRITE_BUFFER ==
            //         _rendererState[_renderStateIdx].widgets[i].widgetType)
    {
      _containers->getFboTexture(
          drawParamsArr[i].spriteBufferId, texture);

      Texture::draw(texture, drawParamsArr[i]);
    }
  }
}

void Renderer::enableDisableMultithreadTextureLoading_RT() {
  _rendererState[_renderStateIdx].renderData >>
      _isMultithreadTextureLoadingEnabled;

#if LOCAL_DEBUG
  LOGY("Executing enableDisableMultithreadTextureLoading_RT(), "
       "_isMultithreadTextureLoadingEnabled: %d (with %lu bytes of data)",
       _isMultithreadTextureLoadingEnabled,
       sizeof(_isMultithreadTextureLoadingEnabled));
#endif /* LOCAL_DEBUG */
}

void Renderer::applyGlobalOffsets_RT(const uint32_t widgetsSize) {
  const int32_t IDX = _renderStateIdx;

  if (0 != _rendererState[IDX].globalOffsetX) {
    // copy the variable because of cache reuse
    const int32_t OFFSET_X = _rendererState[IDX].globalOffsetX;

    for (uint32_t i = 0; i < widgetsSize; ++i) {
      if (!_rendererState[IDX].widgets[i].hasCrop) {
        _rendererState[IDX].widgets[i].pos.x += OFFSET_X;
      } else {
        _rendererState[IDX].widgets[i].frameCropRect.x += OFFSET_X;
      }
    }
  }

  if (0 != _rendererState[IDX].globalOffsetY) {
    // copy the variable because of cache reuse
    const int32_t OFFSET_Y = _rendererState[IDX].globalOffsetY;

    for (uint32_t i = 0; i < widgetsSize; ++i) {
      if (!_rendererState[IDX].widgets[i].hasCrop) {
        _rendererState[IDX].widgets[i].pos.y += OFFSET_Y;
      } else {
        _rendererState[IDX].widgets[i].frameCropRect.y += OFFSET_Y;
      }
    }
  }
}
