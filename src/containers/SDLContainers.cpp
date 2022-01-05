// Corresponding header
#include "sdl_utils/containers/SDLContainers.h"

// C system headers

// C++ system headers
#include <thread>

// Other libraries headers

// Own components headers
#include "sdl_utils/drawing/LoadingScreen.h"
#include "resource_utils/resource_loader/ResourceLoader.h"
#include "utils/debug/FunctionTracer.hpp"
#include "utils/ErrorCode.h"
#include "utils/Log.h"

SDLContainers::SDLContainers(const SDLContainersConfig &cfg) : _config(cfg) {
}

int32_t SDLContainers::init() {
  ResourceLoader rsrcLoader;
  if (SUCCESS != rsrcLoader.init(_config.resourcesBinLocation)) {
    LOGERR("Error in _rsrcLoader.init() -> Terminating ...");
    return FAILURE;
  }

  EgnineBinHeadersData binHeaderData;
  if (SUCCESS != rsrcLoader.readEngineBinHeaders(binHeaderData)) {
    LOGERR("Error in readEngineBinHeaders() -> Terminating ...");
    return FAILURE;
  }

  if (SUCCESS != SoundContainer::init(binHeaderData.musicsCount,
                                           binHeaderData.chunksCount)) {
    LOGERR("Error in SoundContainer::init() -> Terminating ...");
    return FAILURE;
  }

  if (SUCCESS != FontContainer::init(binHeaderData.fontsCount)) {
    LOGERR("Error in FontContainer::init() -> Terminating ...");
    return FAILURE;
  }

  if (SUCCESS !=
      ResourceContainer::init(binHeaderData.staticWidgetsCount,
                              binHeaderData.dynamicWidgetsCount)) {
    LOGERR("Error in ResourceContainer::init() -> Terminating ...");
    return FAILURE;
  }

  if (SUCCESS != TextContainer::init(
      FontContainer::getFontsMap(), _config.maxRuntimeTexts)) {
    LOGERR("Error in TextContainer::init() -> Terminating ...");
    return FAILURE;
  }

  if (SUCCESS !=
      SpriteBufferContainer::init(_config.maxRuntimeSpriteBuffers)) {
    LOGERR("Error in SpriteBufferContainer::init() -> Terminating ...");
    return FAILURE;
  }

  const int32_t TOTAL_FILE_SIZE = binHeaderData.widgetsFileSize +
      binHeaderData.fontsFileSize + binHeaderData.soundsFileSize;
  if (SUCCESS !=
      LoadingScreen::init(_config.loadingScreenCfg, TOTAL_FILE_SIZE)) {
    LOGERR("Error in LoadingScreen::init() -> Terminating ...)");
    return FAILURE;
  }

  if (SUCCESS != populateSDLContainers(rsrcLoader)) {
    LOGERR("Error in populateSDLContainers() -> Terminating ...");
    return FAILURE;
  }

  // deinit loading screen resources, because we no longer need them
  LoadingScreen::deinit();

  return SUCCESS;
}

void SDLContainers::deinit() {
  ResourceContainer::deinit();

  TextContainer::deinit();

  FontContainer::deinit();

  SoundContainer::deinit();

  SpriteBufferContainer::deinit();
}

void SDLContainers::setRenderer(Renderer * renderer) {
  ResourceContainer::setRenderer(renderer);
  TextContainer::setRenderer(renderer);
  SpriteBufferContainer::setRenderer(renderer);
}

int32_t SDLContainers::populateSDLContainers(ResourceLoader &rsrcLoader) {
  TRACE_ENTRY_EXIT;

  //=========== START SOUND POPULATE ==============
  SoundData soundData;

  while (rsrcLoader.readSoundChunk(soundData)) {
    SoundContainer::storeSoundData(soundData);
    soundData.reset();
  }

  SoundContainer::loadAllStoredSounds();
  //============ END SOUND POPULATE ===============

  //============ START FONT POPULATE ==============
  FontData fontData;

  while (rsrcLoader.readFontChunk(fontData)) {
    FontContainer::storeFontData(fontData);
    fontData.reset();
  }

  FontContainer::loadAllStoredFonts();
  //============= END FONT POPULATE ===============

  //========== START RESOURCE POPULATE ============
  ResourceData resData;

  while (rsrcLoader.readResourceChunk(resData)) {
    ResourceContainer::storeRsrcData(resData);
    resData.reset();
  }

  ResourceContainer::loadAllStoredResources(_config.maxResourceLoadingThreads);
  //=========== END RESOURCE POPULATE =============

  return SUCCESS;
}