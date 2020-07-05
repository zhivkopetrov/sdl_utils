// Corresponding header
#include "sdl_utils/containers/SDLContainers.h"

// C system headers

// C++ system headers
#include <cstdlib>
#include <thread>

// Other libraries headers


// Own components headers
#include "sdl_utils/drawing/LoadingScreen.h"
#include "resource_utils/resource_loader/ResourceLoader.h"
#include "utils/debug/FunctionTracer.hpp"
#include "utils/Log.h"

SDLContainers::SDLContainers(Renderer* renderer, const std::string& projectName,
                             const bool isMultithreadResAllowed)
    :

      ResourceContainer(renderer),
      TextContainer(renderer),
      SpriteBufferContainer(renderer),
      _rsrcLoader(nullptr),
      _projectName(projectName),
      _isMultithreadResAllowed(isMultithreadResAllowed) {}

int32_t SDLContainers::init() {
  TRACE_ENTRY_EXIT;

  int32_t err = EXIT_SUCCESS;

  ResourceLoaderCfg resourceCfg;

  _rsrcLoader = new ResourceLoader(_projectName);
  if (nullptr == _rsrcLoader) {
    LOGERR("Error, bad alloc for _rsrcLoader() -> Terminating ...");

    err = EXIT_FAILURE;
  }

  if (EXIT_SUCCESS == err) {
    if (EXIT_SUCCESS != _rsrcLoader->init(&resourceCfg)) {
      LOGERR("Error in _rsrcLoader.init() -> Terminating ...");

      err = EXIT_FAILURE;
    }
  }

  if (EXIT_SUCCESS == err) {
    if (EXIT_SUCCESS != SoundContainer::init(resourceCfg.musicsCount,
                                             resourceCfg.chunksCount)) {
      LOGERR("Error in SoundContainer::init() -> Terminating ...");

      err = EXIT_FAILURE;
    }
  }

  if (EXIT_SUCCESS == err) {
    if (EXIT_SUCCESS != FontContainer::init(resourceCfg.fontsCount)) {
      LOGERR("Error in FontContainer::init() -> Terminating ...");

      err = EXIT_FAILURE;
    }
  }

  if (EXIT_SUCCESS == err) {
    if (EXIT_SUCCESS !=
        ResourceContainer::init(resourceCfg.staticWidgetsCount,
                                resourceCfg.dynamicWidgetsCount)) {
      LOGERR("Error in ResourceContainer::init() -> Terminating ...");

      err = EXIT_FAILURE;
    }
  }

  if (EXIT_SUCCESS == err) {
    if (EXIT_SUCCESS != TextContainer::init(FontContainer::getFontsMap())) {
      LOGERR("Error in TextContainer::init() -> Terminating ...");

      err = EXIT_FAILURE;
    }
  }

  if (EXIT_SUCCESS == err) {
    if (EXIT_SUCCESS != SpriteBufferContainer::init()) {
      LOGERR("Error in SpriteBufferContainer::init() -> Terminating ...");

      err = EXIT_FAILURE;
    }
  }

  if (EXIT_SUCCESS == err) {
    const int32_t TOTAL_FILE_SIZE = resourceCfg.widgetsFileSize +
                                    resourceCfg.fontsFileSize +
                                    resourceCfg.soundsFileSize;

    if (EXIT_SUCCESS != LoadingScreen::init(_projectName, TOTAL_FILE_SIZE)) {
      LOGERR("Error in LoadingScreen::init() -> Terminating ...)");

      err = EXIT_FAILURE;
    }
  }

  if (EXIT_SUCCESS == err) {
    if (EXIT_SUCCESS != populateSDLContainers()) {
      LOGERR("Error in populateSDLContainers() -> Terminating ...");

      err = EXIT_FAILURE;
    }
  }

  if (EXIT_SUCCESS == err) {
    // deinit loading screen resources, because we no longer need them
    LoadingScreen::deinit();

    // close the resource loader, because we no longer need it
    if (nullptr != _rsrcLoader) {
      _rsrcLoader->deinit();

      delete _rsrcLoader;
      _rsrcLoader = nullptr;
    }
  }

  // clear the string -> it is no longer needed
  _projectName.clear();

  return err;
}

void SDLContainers::deinit() {
  ResourceContainer::deinit();

  TextContainer::deinit();

  FontContainer::deinit();

  SoundContainer::deinit();

  SpriteBufferContainer::deinit();
}

int32_t SDLContainers::populateSDLContainers() {
  TRACE_ENTRY_EXIT;

  //=========== START SOUND POPULATE ==============
  SoundData soundData;

  while (_rsrcLoader->readSoundChunk(&soundData)) {
    SoundContainer::storeSoundData(soundData);

    soundData.reset();
  }

  SoundContainer::loadAllStoredSounds();
  //============ END SOUND POPULATE ===============

  //============ START FONT POPULATE ==============
  FontData fontData;

  while (_rsrcLoader->readFontChunk(&fontData)) {
    FontContainer::storeFontData(fontData);

    fontData.reset();
  }

  FontContainer::loadAllStoredFonts();
  //============= END FONT POPULATE ===============

  //========== START RESOURCE POPULATE ============
  ResourceData resData;

  while (_rsrcLoader->readResourceChunk(&resData)) {
    ResourceContainer::storeRsrcData(resData);

    resData.reset();
  }

  ResourceContainer::loadAllStoredResources(_isMultithreadResAllowed);
  //=========== END RESOURCE POPULATE =============

  return EXIT_SUCCESS;
}
