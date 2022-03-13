#ifndef SDL_UTILS_RESOURCECONTAINER_H_
#define SDL_UTILS_RESOURCECONTAINER_H_

// System headers
#include <cstdint>
#include <thread>
#include <unordered_map>
#include <vector>

// Other libraries headers
#include "resource_utils/structs/ResourceData.h"
#include "utils/ErrorCode.h"

// Own components headers
#include "sdl_utils/drawing/defines/RendererDefines.h"

// Forward declarations

// Since ThreadSafeQueue is a very heavy include -> use forward declaration to it
template <typename T>
class ThreadSafeQueue;
class Renderer;

struct SDL_Surface;
struct SDL_Texture;

class ResourceContainer {
 public:
  ResourceContainer();

  virtual ~ResourceContainer() = default;

  /** @brief used to initialise the Resource container
   *
   *  @param const std::string & - absolute file path to resource folder
   *  @param const uint64_t      - number of static widgets to be loaded
   *  @param const uint64_t      - number of dynamic widgets
   *                                    (they are not loaded at ::init())
   *
   *  @return ErrorCode          - error code
   * */
  ErrorCode init(const std::string &resourcesFolderLocation,
                 const uint64_t staticWidgetsCount,
                 const uint64_t dynamicWidgetsCount);

  /** @brief used to deinitialize
   *                          (free memory occupied by Resource container)
   * */
  void deinit();

  /** @brief used to acquire the global renderer (for pushing draw commands)
   *
   *  @return int32_t - error code
   * */
   void setRenderer(Renderer * renderer) {
    _renderer = renderer;
  }

  /** @brief used to store the provided ResourceData in Resource Container
   *
   *  @param ResourceData & - populated structure with
   *                                               Resource specific data
   * */
  void storeRsrcData(ResourceData &resourceData);

  /** @brief used to load all stored resources from the _rsrsDataMap
   *       > as SDL_Surface * in the _rsrsMap (for Software Renderer);
   *       > as SDL_Textute * in the _rsrsMap (for Hardware Renderer);
   *
   *  @param const uint32_t - number of multithreading resource loading
   *                         if '0' is provided - hardware_concurrency is used
   *
   *         NOTE: if the hardware that runs the compiled source is
   *               capable of supporting multithreading - such is used.
   *               If multithreading is not supported - resources are
   *               loaded only on one thread.
   * */
  void loadAllStoredResources(const uint32_t maxResourceLoadingThreads);

  /** @brief used to load resource on demand
   *         IMPORTANT
   *         NOTE: invoking this method will use the MAIN thread only
   *               (the one that created the SDL_Renderer) to perform
   *               ALL the work, which is raw CPU loading of the pixels
   *               off the hard-drive + uploading the pixels on the GPU.
   *
   *         NOTE2: in order to load resource dynamically you must already
   *               have it's absolute path loaded in the _rsrcDataMap;
   *
   *         NOTE3: this function does not return error code
   *                                              for performance reasons
   *
   *         NOTE4: you if have the following case:
   *                loadResourceOnDemandSingle(someRsrcId1);
   *                loadResourceOnDemandSingle(someRsrcId2);
   *                loadResourceOnDemandSingle(someRsrcId3);
   *                loadResourceOnDemandSingle(someRsrcId4);
   *
   *                This will massacre the performance and the engine will
   *                experience a heavy FPS drop.
   *                To use the multithreading approach and extract the
   *                maximum power out of the platform do the following:
   *
   *                const std::vector RSRC_IDS {someRsrcId1, someRsrcId2,
   *                                            someRsrcId3, someRsrcId4};
   *
   *                loadResourceOnDemandMultiple(RSRC_IDS);
   *
   *                This will do is dispatch the CPU specific work
   *                to other worker threads that are already waiting on
   *                a condition variable.
   *
   *                The main thread will consume their work and start to
   *                upload to the GPU simultaneously as the worker threads
   *                are doing their CPU work!
   *
   *         NOTE5: If loadResourceOnDemandMultiple() is used it is up to
   *                the developer to call unloadResourceOnDemandMultiple()
   *                on the same resourceIds;
   *
   *  @param const uint64_t - unique resource ID
   * */
  void loadResourceOnDemandSingle(const uint64_t rsrcId);

  /** @brief used to load resource on demand
   *         NOTE: in order to load resource dynamically you must already
   *         have it's absolute path loaded in the _rsrcDataMap;
   *
   *         NOTE2: this function does not return error code
   *                                              for performance reasons
   *
   *         NOTE3: This will do is dispatch the CPU specific work
   *                to other worker threads that are already waiting on
   *                a condition variable.
   *
   *                The main thread will consume their work and start to
   *                upload to the GPU simultaneously as the worker threads
   *                are doing their CPU work!
   *

   *  @param const std::vector<uint64_t> & - unique resource IDs
   * */
  void loadResourceOnDemandMultiple(const std::vector<uint64_t> &rsrcIds,
                                    const int32_t batchId = 0);

  /** @brief used to unload resource on demand
   *
   *  @param const uint64_t - unique resource ID
   * */
  void unloadResourceOnDemandSingle(const uint64_t rsrcId);

  /** @brief used to unload resource on demand
   *
   *  @param const std::vector<uint64_t> & - unique resource IDs
   *
   *         NOTE: all invocation to memory free() are called from the
   *               main thread since there is not much benefit in calling
   *               free() from multiple threads.
   *               Another this is that free() is relatively cheap to call
   * */
  void unloadResourceOnDemandMultiple(const std::vector<uint64_t> &rsrcIds);

  /** @brief used to acquire previously stored resource data
   *                                       for a given unique rsrc ID
   *
   *  @param const uint64_t        - unique sound ID
   *  @param const ResourceData *& - resource specific data
   *
   *  @returns ErrorCode           - error code
   * */
  ErrorCode getRsrcData(const uint64_t rsrcId, const ResourceData *&outData);

  /** @brief used to attach a newly generated SDL_Texture by the renderer
   *         to the ResourceContainer and increase the used GPU VRAM
   *
   *  @param const uint64_t - unique resource ID
   *  @param const int32_t  - created width of the SDL_Surface
   *  @param const int32_t  - created height of the SDL_Surface
   *  @param SDL_Texture *  - pointer to memory of the created SDL_Surface
   **/
  void attachRsrcTexture(const uint64_t rsrcId, const int32_t createdWidth,
                         const int32_t createdHeight,
                         SDL_Texture *createdTexture);

  /** @brief used to acquire previously stored pre-created SDL_Texture
   *                                       for a given unique resource ID
   *  This function does not return error code for performance reasons
   *
   *  @param const uint64_t - unique resource ID
   *  @param SDL_Texture *& - pre-created SDL_Texture
   * */
  void getRsrcTexture(const uint64_t rsrcId, SDL_Texture *&outTexture);

  /** @brief used to detach(free the slot in the container) for
   *         successfully destroyed SDL_Surface/SDL_Texture by the
   *                              renderer and decrease the used GPU VRAM
   *
   *  @param const uint64_t - unique resource ID
   **/
  void detachRsrcTexture(const uint64_t rsrcId);

  /** @brief used to load a single Surface
   *
   *  @param const ResourceData & - populated structure with
   *                                                 Widget specific data
   *  @param SDL_Surface *&       - created SDL_Surface
   *
   *  @returns ErrorCode          - error code
   * */
  ErrorCode loadSurface(const uint64_t rsrcId, SDL_Surface *&outSurface);

  /** @brief uses to expose the stored surfaces/into a thread safe queue/
   *         to the renderer API.
   *
   *         WARNING: do not invoke this method outside of
   *                  the Renderer API!!!
   * */
   ThreadSafeQueue<std::pair<uint64_t, SDL_Surface *>>
      *getLoadedSurfacesQueue() const {
    return _loadedSurfacesThreadQueue;
  }

  /** @brief used to acquire the occupied GPU VRAM from
   *                                              the ResourceContainer
   *
   *  @return uint64_t - occupied VRAM in bytes
   * */
   uint64_t getGPUMemoryUsage() const { return _gpuMemoryUsage; }

  /** @brief a callback fired when a successful
   *         RendererCmd::LOAD_TEXTURE_MULTIPLE batch has been completed
   *         by the renderer.
   *
   *  @param const int32_t - unique ID of the batch that was loaded
   *
   *         NOTE: every project individual RsrcMgr should override
   *               this method if wants to attach a callback and be
   *               informed when multiple texture batch upload to the GPU
   *               is finished.
   *
   *         Example: when game dynamic resources are finished loading.
   *
   *         WARNING: this method will be called from the main(drawing)
   *                  thread. Do not forget to lock your data!
   **/
  virtual void onLoadTextureMultipleCompleted(const int32_t batchId) = 0;

 private:
  /** @brief used to load a single Surface
   *
   *  @param const ResourceData * - populated structure with
   *                                                 Widget specific data
   *  @param SDL_Surface *&       - created SDL_Surface
   *
   *  @returns ErrorCode          - error code
   * */
  ErrorCode loadSurfaceInternal(const ResourceData *rsrcData,
                                SDL_Surface *&outSurface);

  /** @brief used to internally load all stored resources
   *                                          only using the main thread
   * */
  void loadAllStoredResourcesSingleCore();

  /** @brief used to internally load all stored resources
   *     only using all possible hardware threads (if such are supported)
   *
   *  @param const uint32_t - number of worker threads to spawn
   *                                        (main thread is not included)
   * */
  void loadAllStoredResourcesMultiCore(const uint32_t workerThreadsNum);

  // holds pointer to hardware render in order
  // to be able to push RendererCmd's
  Renderer *_renderer;

  //_rsrcMap holds all Images
  std::unordered_map<uint64_t, SDL_Texture *> _rsrcMap;

  //_rsrcDataMap holds resource specific information for every Image
  std::unordered_map<uint64_t, ResourceData> _rsrcDataMap;

  /** A copy of the resourceData's (used for
   *                                     multithread loading of resources)
   *  */
  ThreadSafeQueue<ResourceData> *_resDataThreadQueue;

  /** Holds all loaded SDL_Surface's during initiliazation (used for
   *                                     multithread loading of resources)
   *
   *  std::pair first : unique rsrcId for currently processed ResourceData
   *  std::pair second: SDL_Surface * that will be created from the
   *                                              processed ResourceData
   *  */
  ThreadSafeQueue<std::pair<uint64_t, SDL_Surface *>>
      *_loadedSurfacesThreadQueue;

  /* Spawn threads responsible for doing the CPU work (loading resources
   * out of hard-drive to SDL_Surface * 'pixel representation')
   *
   * The threads will be shutdown and join-ed at deinit();
   * */
  std::vector<std::thread> _workerThreadPool;

  std::string _resourcesFolderLocation;

  // holds the currently occupied GPU VRAM in bytes
  uint64_t _gpuMemoryUsage;

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

#endif /* SDL_UTILS_RESOURCECONTAINER_H_ */
