// Corresponding header
#include "sdl_utils/containers/ResourceContainer.h"

// C system headers
#include <unistd.h>

// C++ system headers
#include <cstdlib>

// Other libraries headers
#include <SDL_surface.h>

// Own components headers
#include "sdl_utils/drawing/LoadingScreen.h"
#include "sdl_utils/drawing/Renderer.h"
#include "sdl_utils/drawing/Texture.h"

#include "resource_utils/defines/ResourceDefines.h"

#include "utils/concurrency/ThreadSafeQueue.hpp"
#include "utils/Log.h"

#define RGBA_BYTE_SIZE 4

#if USE_SOFTWARE_RENDERER
typedef std::unordered_map<uint64_t, SDL_Surface *>::iterator _rsrcMapIt;
typedef std::unordered_map<uint64_t, SDL_Surface *>::const_iterator
    _rsrcMapConstIt;
#else
typedef std::unordered_map<uint64_t, SDL_Texture *>::iterator _rsrcMapIt;
typedef std::unordered_map<uint64_t, SDL_Texture *>::const_iterator
    _rsrcMapConstIt;
#endif /* USE_SOFTWARE_RENDERER */
typedef std::unordered_map<uint64_t, ResourceData>::iterator _rsrcDataMapIt;
typedef std::unordered_map<uint64_t, ResourceData>::const_iterator
    _rsrcDataMapConstIt;

/** @brief used to load SDL_Surface's from file system async until a shutdown
 *         signal is provided
 *
 *  @param resQueue              - the resource Data queue (input)
 *  @param outSurfQueue          - the loaded surfaces queue (output)
 *  @param threadsLeftToComplete - number of threads still working on the async
 *                                 surface load from the file system
 *  */
static void loadSurfacesFromFileSystemAsyncUntilShutdown(
    ThreadSafeQueue<ResourceData> *resQueue,
    ThreadSafeQueue<std::pair<uint64_t, SDL_Surface *>> *outSurfQueue) {
  ResourceData resData;

  SDL_Surface *surface = nullptr;

  // NOTE: the while loop can be broken with inner ThreadSafeQueue signal
  //      for shutdown
  while (resQueue->waitAndPop(resData)) {
    if (EXIT_SUCCESS !=
        Texture::loadSurfaceFromFile(resData.header.path.c_str(), surface)) {
      LOGERR(
          "Warning, error in loadSurfaceFromFile() for file %s. "
          "Terminating other resourceLoading",
          resData.header.path.c_str());

      return;
    }

    // push the newly generated SDL_Surface to the ThreadSafe Surface Queue
    outSurfQueue->push(
        std::move(std::make_pair(resData.header.hashValue, surface)));

    // reset the variable so it can be reused
    surface = nullptr;
  }
}

/** @brief used to load SDL_Surface's from file system async
 *
 *  @param resQueue              - the resource Data queue (input)
 *  @param outSurfQueue          - the loaded surfaces queue (output)
 *  @param threadsLeftToComplete - number of threads still working on the async
 *                                 surface load from the file system
 *
 *  NOTE: this is the same method as ::loadSurfacesFromFileSystemAsync()
 *        but when resQueue is empty the function will exit
 *        so it's resources can be collected.
 *
 *        This is needed, because on ::init() we can use the
 *        secondary(update) thread to all work on loading images
 *                                          with Texture::loadSurfaceFromFile()
 *  */
static void loadSurfacesFromFileSystemAsync(
    ThreadSafeQueue<ResourceData> *resQueue,
    ThreadSafeQueue<std::pair<uint64_t, SDL_Surface *>> *outSurfQueue) {
  ResourceData resData;

  SDL_Surface *surface = nullptr;

  while (resQueue->tryPop(resData)) {
    if (EXIT_SUCCESS !=
        Texture::loadSurfaceFromFile(resData.header.path.c_str(), surface)) {
      LOGERR(
          "Warning, error in loadSurfaceFromFile() for file %s. "
          "Terminating other resourceLoading",
          resData.header.path.c_str());

      return;
    }

    // push the newly generated SDL_Surface to the ThreadSafe Surface Queue
    outSurfQueue->push(
        std::move(std::make_pair(resData.header.hashValue, surface)));

    // reset the variable so it can be reused
    surface = nullptr;
  }
}

ResourceContainer::ResourceContainer()
    : _renderer(nullptr),
      _resDataThreadQueue(nullptr),
      _loadedSurfacesThreadQueue(nullptr),
      _gpuMemoryUsage(0),
      _isMultithreadTextureLoadingEnabled(false) {}

int32_t ResourceContainer::init(const uint64_t staticWidgetsCount,
                                const uint64_t dynamicWidgetsCount) {
  /** IMPORTANT NOTE:
   *  Since _rsrcDataMap holds information about all possible resources
   *  (static + dynamic) we want to rehash the hashtable to it's actual used
   *  size at startup.
   * */
  _rsrcDataMap.reserve(staticWidgetsCount + dynamicWidgetsCount);

  /** IMPORTANT NOTE:
   *  _rsrcMap on the other hand contains only the static resources + dynamic
   *  resources ONLY for the current loaded game.
   *
   *  Here a place for future expansion is left, where couple of games could
   *  be opened simultaneously.
   * */
  _rsrcMap.reserve(staticWidgetsCount + dynamicWidgetsCount);

  _resDataThreadQueue = new ThreadSafeQueue<ResourceData>;

  if (nullptr == _resDataThreadQueue) {
    LOGERR("Error, bad alloc for ThreadSafeQueue<ResourceData>");
    return EXIT_FAILURE;
  }

  _loadedSurfacesThreadQueue =
      new ThreadSafeQueue<std::pair<uint64_t, SDL_Surface *>>;

  if (nullptr == _loadedSurfacesThreadQueue) {
    LOGERR("Error, bad alloc for ThreadSafeQueue<SDL_Surface *>");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void ResourceContainer::deinit() {
  // free Image/Sprite Textures
  for (_rsrcMapIt it = _rsrcMap.begin(); it != _rsrcMap.end(); ++it) {
#if USE_SOFTWARE_RENDERER
    Texture::freeSurface(it->second);
#else
    Texture::freeTexture(it->second);
#endif
  }

  // clear rsrcMap unordered_map and shrink size
  _rsrcMap.clear();

  // clear rsrcDataMap unordered_map and shrink size
  _rsrcDataMap.clear();

  if (_resDataThreadQueue && _loadedSurfacesThreadQueue)  // sanity checks
  {
    // send shutdown signals
    _resDataThreadQueue->shutdown();
    _loadedSurfacesThreadQueue->shutdown();

    // wait for some time to be sure the queues has completed shutdown
    usleep(2000);  // 2ms

    // and release memory for the queues
    delete _resDataThreadQueue;
    _resDataThreadQueue = nullptr;

    delete _loadedSurfacesThreadQueue;
    _loadedSurfacesThreadQueue = nullptr;
  }

  // all worker threads are done with their work - join them
  const size_t THREAD_POOL_SIZE = _workerThreadPool.size();
  for (size_t i = 0; i < THREAD_POOL_SIZE; ++i) {
    _workerThreadPool[i].join();
  }
  _workerThreadPool.clear();
}

void ResourceContainer::storeRsrcData(ResourceData &resourceData) {
  _rsrcDataMap[resourceData.header.hashValue] = resourceData;

  // initiate load only on 'static'(on_init) resource
  if (ResourceDefines::TextureLoadType::ON_INIT ==
      resourceData.textureLoadType) {
    // move the resource into the thread queue since it's no longer used
    _resDataThreadQueue->push(std::move(resourceData));
  }
}

void ResourceContainer::loadAllStoredResources(
    const uint32_t maxResourceLoadingThreads) {
  if (1 == maxResourceLoadingThreads) {
    LOGG("Starting Single Core resource loading ");
    loadAllStoredResourcesSingleCore();
    return;
  }

  /** Multi Threading strategy:
   *
   *  N - hardware supported number of cores.
   *
   *  Only the Hardware Renderer can perform GPU operations. With this said:
   *      > spawn N - 1 worker threads that will perform  only the
   *        CPU intensive work (reading files from disk, creating
   *        SDL_Surface's from them and storing those SDL_Surface's into a
   *        ThreadSafeQueue);
   *
   *      > use the main thread to pop generated SDL_Surface's from the
   *        ThreadSafeQueue that the worker threads are populating and start
   *        to create SDL_Texture's (GPU Accelerated Textures) out of those
   *        SDL_Surface's;
   *
   *  NOTE: in the case of Software Renderer usage:
   *        Since there will be no GPU operations and there is only CPU work
   *        to be executed -> spawn N - 1 worker threads for generating
   *        the SDL_Surface's and also use the main thread for the same job.
   * */

  /** Hardware_concurrency may return 0 if its not supported
   * if this happens -> run the resourceLoad in single core
   * */
  constexpr auto hardwareConcurrencyHint = 0;
  const uint32_t supportedHardwareThreads = std::thread::hardware_concurrency();

  uint32_t hardwareThreadNumber = 0;
  if (maxResourceLoadingThreads < supportedHardwareThreads) {
    hardwareThreadNumber =
        (hardwareConcurrencyHint == maxResourceLoadingThreads) ?
            supportedHardwareThreads : maxResourceLoadingThreads;
  } else {
    hardwareThreadNumber = supportedHardwareThreads;
    LOGR("maxResourceThreads requested: %d but hardware only supports up to: %d"
        " threads. Will use: %d resource loading threads",
        maxResourceLoadingThreads, supportedHardwareThreads,
        supportedHardwareThreads);
  }

  if (1 == hardwareThreadNumber) {
    LOGR("Multi Threading is not supported on this hardware. ");
    LOGG("Starting Single Core resource loading ");
    loadAllStoredResourcesSingleCore();
  }

  /** If threads are <= 1 no need to spawn second thread,
   *  because there will be a performance
   *  lost from the constant threads context switches
   * */
  /* Generate THREAD_NUM - 1 worker CPU threads and leave the main
   * thread to operate ot GPU + CPU operations
   * */
  const uint32_t WORKER_THREAD_NUM = hardwareThreadNumber - 1;

  LOGG("Starting Multi Core resource loading on %u threads",
       hardwareThreadNumber);

  if (WORKER_THREAD_NUM > 1)  // for the lifetime of the program
  {
    _isMultithreadTextureLoadingEnabled = true;

    _renderer->addRendererCmd_UT(
        RendererCmd::ENABLE_DISABLE_MULTITHREAD_TEXTURE_LOADING,
        reinterpret_cast<const uint8_t *>(
            &_isMultithreadTextureLoadingEnabled),
        sizeof(_isMultithreadTextureLoadingEnabled));
  }

  loadAllStoredResourcesMultiCore(WORKER_THREAD_NUM);
}

int32_t ResourceContainer::getRsrcData(const uint64_t rsrcId,
                                       const ResourceData *&outData) {
  _rsrcDataMapConstIt it = _rsrcDataMap.find(rsrcId);

  // key not found
  if (it == _rsrcDataMap.end()) {
    return EXIT_FAILURE;
  }
  outData = &it->second; // key found

  return EXIT_SUCCESS;
}

void ResourceContainer::loadResourceOnDemandSingle(const uint64_t rsrcId) {
  _rsrcDataMapIt it = _rsrcDataMap.find(rsrcId);

  if (_rsrcDataMap.end() == it)  // key not found
  {
    LOGERR("Error, rsrcData for rsrcId: %#16lX not found. "
        "will not create Image", rsrcId);
    return;
  }

  if (ResourceDefines::TextureLoadType::ON_INIT == it->second.textureLoadType) {
    LOGERR(
        "Warning, invoking dynamic load on a resource with ID: "
        "%#16lX that has TextureLoadType::ON_INIT. "
        "Will not load resource!",
        rsrcId);

    return;
  }

  // if refCount is bigger than zero -> resource is already loaded
  // increase the refCount and return
  if (0 < it->second.refCount) {
    ++it->second.refCount;
    return;
  }

  it->second.refCount = 1;

  if (_isMultithreadTextureLoadingEnabled) {
    // dispatch the resource data into the thread safe queue
    _resDataThreadQueue->pushWithCopy(it->second);
  }

  _renderer->addRendererCmd_UT(RendererCmd::LOAD_TEXTURE_SINGLE,
                               reinterpret_cast<const uint8_t *>(&rsrcId),
                               sizeof(rsrcId));
}

void ResourceContainer::loadResourceOnDemandMultiple(
    const std::vector<uint64_t> &rsrcIds, const int32_t batchId) {
  const uint32_t RSRC_SIZE = static_cast<uint32_t>(rsrcIds.size());

  // make another copy of the resources, because some of them might fail
  // the checks and their ID's should not be sent
  std::vector<uint64_t> rsrcIdsToSend;
  rsrcIdsToSend.reserve(RSRC_SIZE);

  _rsrcDataMapIt rsrcDataMapIt;
  uint32_t itemsToPop = 0;

  for (uint32_t i = 0; i < RSRC_SIZE; ++i) {
    rsrcDataMapIt = _rsrcDataMap.find(rsrcIds[i]);

    if (_rsrcDataMap.end() != rsrcDataMapIt) {
      if (ResourceDefines::TextureLoadType::ON_INIT !=
          rsrcDataMapIt->second.textureLoadType) {
        // if refCount is bigger than zero -> resource is already loaded
        // increase the refCount and continue
        if (0 < rsrcDataMapIt->second.refCount) {
          ++rsrcDataMapIt->second.refCount;

          continue;
        }

        rsrcDataMapIt->second.refCount = 1;

        ++itemsToPop;

        rsrcIdsToSend.emplace_back(rsrcIds[i]);

        if (_isMultithreadTextureLoadingEnabled) {
          // dispatch the resource data into the thread safe queue
          _resDataThreadQueue->pushWithCopy(rsrcDataMapIt->second);
        }
      } else  // initiate load on 'dynamic'(on_demand) resource
      {
        LOGERR(
            "Warning, invoking dynamic load on a resource with ID: "
            "%#16lX that has TextureLoadType::ON_INIT. "
            "Will not load resource!",
            rsrcIds[i]);

        continue;
      }
    } else  // key not found
    {
      LOGERR(
          "Error, rsrcData for rsrcId: %#16lX not found. "
          "will not create Image",
          rsrcIds[i]);

      continue;
    }
  }

  const uint64_t DATA_SIZE = sizeof(itemsToPop) + sizeof(batchId);
  uint8_t data[DATA_SIZE];
  uint64_t populatedBytes = 0;

  memcpy(data, &itemsToPop, sizeof(itemsToPop));
  populatedBytes += sizeof(itemsToPop);

  memcpy(data + populatedBytes, &batchId, sizeof(batchId));
  populatedBytes += sizeof(batchId);

  _renderer->addRendererCmd_UT(RendererCmd::LOAD_TEXTURE_MULTIPLE, data,
                               DATA_SIZE);

  _renderer->addRendererData_UT(
      reinterpret_cast<const uint8_t *>(rsrcIdsToSend.data()),
      (itemsToPop * sizeof(uint64_t)));
}

void ResourceContainer::unloadResourceOnDemandSingle(const uint64_t rsrcId) {
  _rsrcDataMapIt rsrcDataMapIt = _rsrcDataMap.find(rsrcId);
  if (rsrcDataMapIt == _rsrcDataMap.end()) {
    LOGERR(
        "Error, trying to unload rsrcId: %#16lX which is not existing", rsrcId);
    return;
  }

  if (0 == rsrcDataMapIt->second.refCount) {
    LOGERR("Error, trying to unload rsrcId: %#16lX that is not loaded", rsrcId);
    return;
  }

  // decrease the refCount
  --rsrcDataMapIt->second.refCount;

  // when refCount goes to zero -> resource should be unloaded
  if (0 == rsrcDataMapIt->second.refCount) {
    _renderer->addRendererCmd_UT(RendererCmd::DESTROY_TEXTURE,
                                 reinterpret_cast<const uint8_t *>(&rsrcId),
                                 sizeof(rsrcId));
  }
}

void ResourceContainer::unloadResourceOnDemandMultiple(
    const std::vector<uint64_t> &rsrcIds) {
  // NOTE: since this method is crucial for performance reasons
  //      the source code from ::unloadResourceOnDemandSingle()
  //      is copied and not invoked N times.

  _rsrcDataMapIt rsrcDataMapIt;
  const uint32_t SIZE = static_cast<uint32_t>(rsrcIds.size());

  for (uint32_t i = 0; i < SIZE; ++i) {
    rsrcDataMapIt = _rsrcDataMap.find(rsrcIds[i]);
    if (rsrcDataMapIt == _rsrcDataMap.end()) {
      LOGERR("Error, trying to unload rsrcId: %#16lX which is not existing",
          rsrcIds[i]);
      continue;
    }

    if (0 == rsrcDataMapIt->second.refCount) {
      LOGERR("Error, trying to unload rsrcId: %#16lX that is not loaded",
             rsrcIds[i]);

      continue;
    }

    // decrease the refCount
    --rsrcDataMapIt->second.refCount;

    // when refCount goes to zero -> resource should be unloaded
    if (0 == rsrcDataMapIt->second.refCount) {
      _renderer->addRendererCmd_UT(
          RendererCmd::DESTROY_TEXTURE,
          reinterpret_cast<const uint8_t *>(&rsrcIds[i]), sizeof(rsrcIds[i]));
    }
  }
}

#if USE_SOFTWARE_RENDERER
void ResourceContainer::attachRsrcTexture(const uint64_t rsrcId,
                                          const int32_t createdWidth,
                                          const int32_t createdHeight,
                                          SDL_Surface *createdTexture)
#else
void ResourceContainer::attachRsrcTexture(const uint64_t rsrcId,
                                          const int32_t createdWidth,
                                          const int32_t createdHeight,
                                          SDL_Texture *createdTexture)
#endif /* USE_SOFTWARE_RENDERER */
{
  // directly populate the rsrcMap with the newly created SDL_Texture
  _rsrcMap[rsrcId] = createdTexture;

  if (createdWidth || createdHeight) {
  }

#if !USE_SOFTWARE_RENDERER
  // increase the occupied GPU memory usage counter for the new texture
  _gpuMemoryUsage +=
      static_cast<uint64_t>((createdWidth * createdHeight * RGBA_BYTE_SIZE));
#endif /* !USE_SOFTWARE_RENDERER */
}

#if USE_SOFTWARE_RENDERER
void ResourceContainer::getRsrcTexture(const uint64_t rsrcId,
                                       SDL_Surface *&outTexture)
#else
void ResourceContainer::getRsrcTexture(const uint64_t rsrcId,
                                       SDL_Texture *&outTexture)
#endif /* USE_SOFTWARE_RENDERER */
{
  _rsrcMapConstIt it = _rsrcMap.find(rsrcId);

  // key found
  if (it != _rsrcMap.end()) {
    outTexture = it->second;
  } else  // key not found
  {
    LOGERR("Error, rsrcTexture for rsrcId: %#16lX not found", rsrcId);
  }
}

void ResourceContainer::detachRsrcTexture(const uint64_t rsrcId) {
  _rsrcMapConstIt rsrcMapIt = _rsrcMap.find(rsrcId);
  if (rsrcMapIt == _rsrcMap.end()) {
    LOGERR("Error, trying to detach rsrcId: %#16lX which is not existing",
        rsrcId);
    return;
  }

  _rsrcMap.erase(rsrcMapIt);

#if !USE_SOFTWARE_RENDERER
  _rsrcDataMapConstIt rsrcDataMapIt = _rsrcDataMap.find(rsrcId);
  if (rsrcDataMapIt == _rsrcDataMap.end()) {
    LOGERR("Error, trying to detach rsrcId: %#16lX which is not existing",
        rsrcId);
    return;
  }

  // decrease the occupied GPU memory usage counter for the
  // destroyed texture
  _gpuMemoryUsage -= static_cast<uint64_t>(
      (rsrcDataMapIt->second.imageRect.w * rsrcDataMapIt->second.imageRect.h *
       RGBA_BYTE_SIZE));
#endif /* !USE_SOFTWARE_RENDERER*/
}

int32_t ResourceContainer::loadSurface(const uint64_t rsrcId,
                                       SDL_Surface *&outSurface) {
  const ResourceData *resData = nullptr;

  if (EXIT_SUCCESS != getRsrcData(rsrcId, resData)) {
    LOGERR(
        "Error, ::getRsrcData() failed for rsrcId: %#16lX, "
        "will not load Surface",
        rsrcId);

    return EXIT_FAILURE;
  }

  if (EXIT_SUCCESS != loadSurfaceInternal(resData, outSurface)) {
    LOGERR(
        "Error, ::loadSurfaceInternal() failed for rsrcId: %#16lX, "
        "will not load Surface",
        rsrcId);

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int32_t ResourceContainer::loadSurfaceInternal(const ResourceData *rsrcData,
                                               SDL_Surface *&outSurface) {
  if (EXIT_SUCCESS !=
      Texture::loadSurfaceFromFile(rsrcData->header.path.c_str(), outSurface)) {
    LOGERR("Error in loadSurfaceFromFile() for rsrcId: %#16lX",
           rsrcData->header.hashValue);

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void ResourceContainer::loadAllStoredResourcesSingleCore() {
  // NOTE: some of the thread safe mechanism such as ThreadSafeQueue as reused.
  // the overhead is minimal and the source will be reused.

  ResourceData resData;

  SDL_Surface *newSurface = nullptr;

  while (_resDataThreadQueue->tryPop(resData)) {
    if (EXIT_SUCCESS !=
        Texture::loadSurfaceFromFile(resData.header.path.c_str(), newSurface)) {
      LOGERR(
          "Warning, error in loadSurfaceFromFile() for file %s. "
          "Terminating other resourceLoading",
          resData.header.path.c_str());

      return;
    }

    // NOTE: if HARDWARE_RENDERER is used -> divide the load time between:
    //          > creating the SDL_Surface;
    //          > creating the SDL_Texture from the SDL_Surface;
#if USE_SOFTWARE_RENDERER
    const int32_t fileSize = resData.header.fileSize;
#else
    const int32_t fileSize = resData.header.fileSize / 2;
#endif /* USE_SOFTWARE_RENDERER */

    // push the newly generated SDL_Surface to the ThreadSafe Surface Queue
    _loadedSurfacesThreadQueue->push(
        std::move(std::make_pair(resData.header.hashValue, newSurface)));

    // send message to loading screen for
    // successfully loaded resource
    LoadingScreen::onNewResourceLoaded(fileSize);

    // reset the variable so it can be reused
    newSurface = nullptr;
  }

  // temporary variables used for _loadedSurfacesThreadQueue::pop operation
  std::pair<uint64_t, SDL_Surface *> currResSurface(0, nullptr);

  // on main thread return
#if USE_SOFTWARE_RENDERER
  // store the generates SDL_Surfaces into the rsrcMap
  while (_loadedSurfacesThreadQueue->tryPop(currResSurface)) {
    _rsrcMap[currResSurface.first] = currResSurface.second;
  }
#else
  SDL_Texture *newTexture = nullptr;

  int32_t currSurfaceWidth = 0;
  int32_t currSurfaceHeight = 0;

  // generate GPU textures from the stored SDL_Surface's
  while (_loadedSurfacesThreadQueue->tryPop(currResSurface)) {
    currSurfaceWidth = currResSurface.second->w;
    currSurfaceHeight = currResSurface.second->h;

    if (EXIT_SUCCESS !=
        Texture::loadTextureFromSurface(currResSurface.second, newTexture)) {
      LOGERR(
          "Error in Texture::loadTextureFromSurface() for rsrcId: "
          "%#16lX",
          currResSurface.first);

      return;
    }

    // increase the occupied GPU memory usage counter for the new texture
    _gpuMemoryUsage += static_cast<uint64_t>(
        (currSurfaceWidth * currSurfaceHeight * RGBA_BYTE_SIZE));

    // store the generates SDL_Texture into the rsrcMap
    _rsrcMap[currResSurface.first] = newTexture;

    // NOTE: if HARDWARE_RENDERER is used -> divide the load time between:
    //          > creating the SDL_Surface;
    //          > creating the SDL_Texture from the SDL_Surface;

    const int32_t fileSize =
        _rsrcDataMap[currResSurface.first].header.fileSize / 2;

    // send message to loading screen for
    // successfully loaded resource
    LoadingScreen::onNewResourceLoaded(fileSize);

    // reset the variable so it can be reused
    newTexture = nullptr;
  }
#endif /* USE_SOFTWARE_RENDERER */
}

void ResourceContainer::loadAllStoredResourcesMultiCore(
    const uint32_t workerThreadsNum) {
  _workerThreadPool.reserve(workerThreadsNum);

  // temporary variables used for _loadedSurfacesThreadQueue::pop operation
  std::pair<uint64_t, SDL_Surface *> currResSurface(0, nullptr);

  // IMPORTANT: remember the size of the items that will be handled by the
  //           resource queue, before worker threads are spawned!!!
  uint32_t itemsToPop = static_cast<uint32_t>(_resDataThreadQueue->size());

  //-1, because we will used the secondary(update) thread only for
  //                                                           ::init() stage
  for (uint32_t i = 0; i < workerThreadsNum - 1; ++i) {
    // spawn the worker threads
    _workerThreadPool.emplace_back(loadSurfacesFromFileSystemAsyncUntilShutdown,
                                   _resDataThreadQueue,
                                   _loadedSurfacesThreadQueue);
  }

  // spawn the secondary(update) thread only while in ::init() stage
  _workerThreadPool.emplace_back(loadSurfacesFromFileSystemAsync,
                                 _resDataThreadQueue,
                                 _loadedSurfacesThreadQueue);

#if USE_SOFTWARE_RENDERER
  // use the main thread for the same operation as the worker threads
  loadSurfacesFromFileSystemAsync(_resDataThreadQueue,
                                  _loadedSurfacesThreadQueue);

  // store the generates SDL_Surfaces into the rsrcMap
  while (0 != itemsToPop) {
    if (!_loadedSurfacesThreadQueue->waitAndPop(currResSurface)) {
      LOGERR(
          "Warning, the pushed number of ResourceData structs does"
          " not match the outputed number of SDL_Surfaces!");

      break;
    }

    _rsrcMap[currResSurface.first] = currResSurface.second;

    --itemsToPop;
  }
#else
  // temporary variables used for calculations
  SDL_Texture *newTexture = nullptr;
  int32_t currSurfaceWidth = 0;
  int32_t currSurfaceHeight = 0;

  // start uploading on the GPU on the main thread
  while (0 != itemsToPop) {
    /** Block main thread and wait resources to be pushed
     * into the _loadedSurfacesThreadQueue
     * */
    if (!_loadedSurfacesThreadQueue->waitAndPop(currResSurface)) {
      LOGERR(
          "Warning, the pushed number of ResourceData structs does"
          " not match the outputed number of SDL_Surfaces!");

      break;
    }

    currSurfaceWidth = currResSurface.second->w;
    currSurfaceHeight = currResSurface.second->h;

    if (EXIT_SUCCESS !=
        Texture::loadTextureFromSurface(currResSurface.second, newTexture)) {
      LOGERR(
          "Error in Texture::loadTextureFromSurface() for rsrcId: "
          "%#16lX",
          currResSurface.first);

      return;
    } else {
      // increase the occupied GPU memory usage counter for the new texture
      _gpuMemoryUsage += static_cast<uint64_t>(
          (currSurfaceWidth * currSurfaceHeight * RGBA_BYTE_SIZE));

      // emplace GPU Texture to the rsrcMap
      _rsrcMap[currResSurface.first] = newTexture;

      // send message to loading screen for
      // successfully loaded resource
      LoadingScreen::onNewResourceLoaded(
          _rsrcDataMap[currResSurface.first].header.fileSize);

      // reset the variable so it can be reused
      newTexture = nullptr;
    }

    --itemsToPop;
  }
#endif /* USE_SOFTWARE_RENDERER */

  if (_workerThreadPool.back().joinable())  // sanity check
  {
    // collect thread resources
    _workerThreadPool.back().join();

    // remove the slot - it is no longer needed
    _workerThreadPool.pop_back();
  }
}
