// Corresponding header
#include "sdl_utils/containers/SoundContainer.h"

// System headers

// Other libraries headers
#include "utils/data_type/EnumClassUtils.h"
#include "utils/log/Log.h"

// Own components headers
#include "sdl_utils/drawing/LoadingScreen.h"
#include "sdl_utils/sound/SoundMixer.h"

ErrorCode SoundContainer::init(const std::string &resourcesFolderLocation,
                               const uint64_t musicsCount,
                               const uint64_t chunksCount) {
  _resourcesFolderLocation = resourcesFolderLocation;
  _soundsDataMap.reserve(musicsCount + chunksCount);
  _musicMap.reserve(musicsCount);
  _chunkMap.reserve(chunksCount);

  return ErrorCode::SUCCESS;
}

void SoundContainer::deinit() {
  // free Music sounds
  for (auto& musicWidgetPair : _musicMap) {
    SoundMixer::freeMusic(musicWidgetPair.second);
  }

  // clear Music unordered_map and shrink size
  _musicMap.clear();

  // free Chunk sounds
  for (auto& soundWidgetPair : _chunkMap) {
    SoundMixer::freeChunk(soundWidgetPair.second);
  }

  // clear Chunk unordered_map and shrink size
  _chunkMap.clear();

  // clear SoundData unordered_map and shrink size
  _soundsDataMap.clear();
}

void SoundContainer::loadAllStoredSounds() {
  Mix_Chunk *newChunk = nullptr;
  Mix_Music *newMusic = nullptr;
  std::string widgetPath;

  for (const auto& soundWidgetPair : _soundsDataMap) {
    const auto& soundWidget = soundWidgetPair.second;
    widgetPath = _resourcesFolderLocation;
    widgetPath.append(soundWidget.header.path);

    if (SoundType::CHUNK == soundWidget.soundType) {
      if (ErrorCode::SUCCESS !=
          loadChunk(widgetPath.c_str(), soundWidget.soundLevel, newChunk)) {
        LOGERR("Error in loadChunk() for soundId: %" PRIu64"",
                soundWidget.header.hashValue);
      } else {
        // populate the chunk map 
        _chunkMap[soundWidget.header.hashValue] = newChunk;

        // send message to loading screen for
        // successfully loaded resource
        LoadingScreen::onNewResourceLoaded(soundWidget.header.fileSize);

        // reset the variable so it can be reused
        newChunk = nullptr;
      }
    } else { // SoundType::MUSIC == soundWidget.soundType
      if (ErrorCode::SUCCESS !=
          loadMusic(widgetPath.c_str(), soundWidget.soundLevel, newMusic)) {
        LOGERR("Error in loadMusic() for soundId: %" PRIu64"",
               soundWidget.header.hashValue);
      } else {
        // populate the _musicMap map
        _musicMap[soundWidget.header.hashValue] = newMusic;

        // send message to loading screen for
        // successfully loaded resource
        LoadingScreen::onNewResourceLoaded(soundWidget.header.fileSize);

        // reset the variable so it can be reused
        newMusic = nullptr;
      }
    }
  }
}

ErrorCode SoundContainer::getSoundData(const uint64_t soundId,
                                       const SoundData *&outData) {
  auto it = _soundsDataMap.find(soundId);
  // key not found
  if (it == _soundsDataMap.end()) {
    LOGERR("Error, soundData for rsrcId: %" PRIu64" not found", soundId);
    return ErrorCode::FAILURE;
  }

  outData = &it->second;
  return ErrorCode::SUCCESS;
}

void SoundContainer::getMusicSound(const uint64_t rsrcId,
                                   Mix_Music *&outMusic) {
  auto it = _musicMap.find(rsrcId);
  // key not found
  if (it == _musicMap.end()) {
    LOGERR("Error, Mix_Music for rsrcId: %" PRIu64" not found", rsrcId);
  } else  // key found
  {
    outMusic = it->second;
  }
}

void SoundContainer::getChunkSound(const uint64_t rsrcId,
                                   Mix_Chunk *&outChunk) {
  auto it = _chunkMap.find(rsrcId);
  // key not found
  if (it == _chunkMap.end()) {
    LOGERR("Error, Mix_Chunk for rsrcId: %" PRIu64" not found", rsrcId);
  } else  // key found
  {
    outChunk = it->second;
  }
}

ErrorCode SoundContainer::loadMusic(const char *path,
                                    const SoundLevel soundLevel,
                                    Mix_Music *&outMusic) {
  if (ErrorCode::SUCCESS != SoundMixer::loadMusicFromFile(path, outMusic)) {
    LOGERR("Error in SoundMixer::loadMusicFromFile for filePath: %s", path);
    return ErrorCode::FAILURE;
  }

  if (SoundLevel::UNKNOWN == soundLevel) {
    LOGERR("Error, UNKNOWN soundLevel value detected.");
    return ErrorCode::FAILURE;
  }

  SoundMixer::setMusicVolume(getEnumValue(soundLevel));

  return ErrorCode::SUCCESS;
}

ErrorCode SoundContainer::loadChunk(const char *path,
                                    const SoundLevel soundLevel,
                                    Mix_Chunk *&outChunk) {
  if (ErrorCode::SUCCESS != SoundMixer::loadChunkFromFile(path, outChunk)) {
    LOGERR("Error in SoundMixer::loadChunkFromFile for filePath: %s", path);
    return ErrorCode::FAILURE;
  }

  if (SoundLevel::UNKNOWN == soundLevel) {
    LOGERR("Error, UNKNOWN soundLevel value detected.");
    return ErrorCode::FAILURE;
  }

  SoundMixer::setChunkVolume(outChunk, getEnumValue(soundLevel));

  return ErrorCode::SUCCESS;
}
