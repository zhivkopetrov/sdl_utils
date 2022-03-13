// Corresponding header
#include "sdl_utils/containers/SDLContainers.h"

// System headers
#include <thread>

// Other libraries headers
#include "resource_utils/resource_loader/ResourceLoader.h"
#include "utils/debug/FunctionTracer.h"
#include "utils/Log.h"

// Own components headers
#include "sdl_utils/drawing/LoadingScreen.h"

SDLContainers::SDLContainers(const SDLContainersConfig &cfg) : _config(cfg) {
}

ErrorCode SDLContainers::init() {
  ResourceLoader rsrcLoader;
  if (ErrorCode::SUCCESS != rsrcLoader.init(_config.resourcesFolderLocation)) {
    LOGERR("Error in _rsrcLoader.init() -> Terminating ...");
    return ErrorCode::FAILURE;
  }

  EgnineBinHeadersData binHeaderData;
  if (ErrorCode::SUCCESS != rsrcLoader.readEngineBinHeaders(binHeaderData)) {
    LOGERR("Error in readEngineBinHeaders() -> Terminating ...");
    return ErrorCode::FAILURE;
  }

  if (ErrorCode::SUCCESS !=
      SoundContainer::init(_config.resourcesFolderLocation,
                           binHeaderData.musicsCount,
                           binHeaderData.chunksCount)) {
    LOGERR("Error in SoundContainer::init() -> Terminating ...");
    return ErrorCode::FAILURE;
  }

  if (ErrorCode::SUCCESS !=
      FontContainer::init(_config.resourcesFolderLocation,
                          binHeaderData.fontsCount)) {
    LOGERR("Error in FontContainer::init() -> Terminating ...");
    return ErrorCode::FAILURE;
  }

  if (ErrorCode::SUCCESS !=
      ResourceContainer::init(_config.resourcesFolderLocation,
                              binHeaderData.staticWidgetsCount,
                              binHeaderData.dynamicWidgetsCount)) {
    LOGERR("Error in ResourceContainer::init() -> Terminating ...");
    return ErrorCode::FAILURE;
  }

  if (ErrorCode::SUCCESS != TextContainer::init(
      FontContainer::getFontsMap(), _config.maxRuntimeTexts)) {
    LOGERR("Error in TextContainer::init() -> Terminating ...");
    return ErrorCode::FAILURE;
  }

  if (ErrorCode::SUCCESS !=
      FboContainer::init(_config.maxRuntimeSpriteBuffers)) {
    LOGERR("Error in FboContainer::init() -> Terminating ...");
    return ErrorCode::FAILURE;
  }

  const int32_t TOTAL_FILE_SIZE = binHeaderData.widgetsFileSize +
      binHeaderData.fontsFileSize + binHeaderData.soundsFileSize;
  if (ErrorCode::SUCCESS !=
      LoadingScreen::init(_config.loadingScreenCfg, TOTAL_FILE_SIZE)) {
    LOGERR("Error in LoadingScreen::init() -> Terminating ...)");
    return ErrorCode::FAILURE;
  }

  if (ErrorCode::SUCCESS != populateSDLContainers(rsrcLoader)) {
    LOGERR("Error in populateSDLContainers() -> Terminating ...");
    return ErrorCode::FAILURE;
  }

  // deinit loading screen resources, because we no longer need them
  LoadingScreen::deinit();

  return ErrorCode::SUCCESS;
}

void SDLContainers::deinit() {
  ResourceContainer::deinit();
  TextContainer::deinit();
  FontContainer::deinit();
  SoundContainer::deinit();
  FboContainer::deinit();
}

void SDLContainers::setRenderer(Renderer * renderer) {
  ResourceContainer::setRenderer(renderer);
  TextContainer::setRenderer(renderer);
  FboContainer::setRenderer(renderer);
}

ErrorCode SDLContainers::populateSDLContainers(ResourceLoader &rsrcLoader) {
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

  return ErrorCode::SUCCESS;
}
