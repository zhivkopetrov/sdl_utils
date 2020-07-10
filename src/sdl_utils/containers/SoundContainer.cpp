// Corresponding header
#include "sdl_utils/containers/SoundContainer.h"

// C system headers

// C++ system headers
#include <cstdlib>

// Other libraries headers

// Own components headers
#include "sdl_utils/drawing/LoadingScreen.h"
#include "sdl_utils/sound/SoundMixer.h"
#include "utils/data_type/EnumClassUtils.hpp"
#include "utils/Log.h"

typedef std::unordered_map<uint64_t, Mix_Music *>::iterator _musicMapIt;
typedef std::unordered_map<uint64_t, Mix_Music *>::const_iterator
    _musicMapConstIt;

typedef std::unordered_map<uint64_t, Mix_Chunk *>::iterator _chunkMapIt;
typedef std::unordered_map<uint64_t, Mix_Chunk *>::const_iterator
    _chunkMapConstIt;

typedef std::unordered_map<uint64_t, SoundData>::iterator _soundsDataMapIt;
typedef std::unordered_map<uint64_t, SoundData>::const_iterator
    _soundsDataMapConstIt;

int32_t SoundContainer::init(const uint64_t musicsCount,
                             const uint64_t chunksCount) {
  int32_t err = EXIT_SUCCESS;

  _soundsDataMap.reserve(musicsCount + chunksCount);

  _musicMap.reserve(musicsCount);

  _chunkMap.reserve(chunksCount);

  return err;
}

void SoundContainer::deinit() {
  // free Music sounds
  for (_musicMapIt it = _musicMap.begin(); it != _musicMap.end(); ++it) {
    SoundMixer::freeMusic(it->second);

    it->second = nullptr;
  }

  // clear Music unordered_map and shrink size
  _musicMap.clear();

  // free Chunk sounds
  for (_chunkMapIt it = _chunkMap.begin(); it != _chunkMap.end(); ++it) {
    SoundMixer::freeChunk(it->second);

    it->second = nullptr;
  }

  // clear Chunk unordered_map and shrink size
  _chunkMap.clear();

  // clear SoundData unordered_map and shrink size
  _soundsDataMap.clear();
}

void SoundContainer::loadAllStoredSounds() {
  Mix_Chunk *newChunk = nullptr;
  Mix_Music *newMusic = nullptr;

  _soundsDataMapConstIt it;

  for (it = _soundsDataMap.begin(); it != _soundsDataMap.end(); ++it) {
    if (SoundType::CHUNK == it->second.soundType) {
      if (EXIT_SUCCESS != loadChunk(it->second.header.path.c_str(),
                                    it->second.soundLevel, newChunk)) {
        LOGERR("Error in loadChunk() for soundId: %#16lX",
               it->second.header.hashValue);
      } else {
        // populate the chunk map
        _chunkMap[it->second.header.hashValue] = newChunk;

        // send message to loading screen for
        // successfully loaded resource
        LoadingScreen::onNewResourceLoaded(it->second.header.fileSize);

        // reset the variable so it can be reused
        newChunk = nullptr;
      }
    } else  // SoundType::MUSIC == it->second.soundType
    {
      if (EXIT_SUCCESS != loadMusic(it->second.header.path.c_str(),
                                    it->second.soundLevel, newMusic)) {
        LOGERR("Error in loadMusic() for soundId: %#16lX",
               it->second.header.hashValue);
      } else {
        // populate the _musicMap map
        _musicMap[it->second.header.hashValue] = newMusic;

        // send message to loading screen for
        // successfully loaded resource
        LoadingScreen::onNewResourceLoaded(it->second.header.fileSize);

        // reset the variable so it can be reused
        newMusic = nullptr;
      }
    }
  }
}

int32_t SoundContainer::getSoundData(const uint64_t soundId,
                                     const SoundData *&outData) {
  int32_t err = EXIT_SUCCESS;

  _soundsDataMapConstIt it = _soundsDataMap.find(soundId);

  // key not found
  if (it == _soundsDataMap.end()) {
    LOGERR("Error, soundData for rsrcId: %#16lX not found", soundId);

    err = EXIT_FAILURE;
  } else  // key found
  {
    outData = &it->second;
  }

  return err;
}

void SoundContainer::getMusicSound(const uint64_t rsrcId,
                                   Mix_Music *&outMusic) {
  _musicMapConstIt it = _musicMap.find(rsrcId);

  // key not found
  if (it == _musicMap.end()) {
    LOGERR("Error, Mix_Music for rsrcId: %#16lX not found", rsrcId);
  } else  // key found
  {
    outMusic = it->second;
  }
}

void SoundContainer::getChunkSound(const uint64_t rsrcId,
                                   Mix_Chunk *&outChunk) {
  _chunkMapConstIt it = _chunkMap.find(rsrcId);

  // key not found
  if (it == _chunkMap.end()) {
    LOGERR("Error, Mix_Chunk for rsrcId: %#16lX not found", rsrcId);
  } else  // key found
  {
    outChunk = it->second;
  }
}

int32_t SoundContainer::loadMusic(const char *path, const SoundLevel soundLevel,
                                  Mix_Music *&outMusic) {
  int32_t err = SoundMixer::loadMusicFromFile(path, outMusic);

  if (EXIT_SUCCESS != err) {
    LOGERR("Error in SoundMixer::loadMusicFromFile for filePath: %s", path);
  } else  // EXIT_SUCCESS == err
  {
    if (SoundLevel::UNKNOWN == soundLevel) {
      LOGERR("Error, UNKNOWN soundLevel value detected.");

      err = EXIT_FAILURE;
    } else  // it is valid SoundLevel value
    {
      SoundMixer::setMusicVolume(getEnumValue(soundLevel));
    }
  }

  return err;
}

int32_t SoundContainer::loadChunk(const char *path, const SoundLevel soundLevel,
                                  Mix_Chunk *&outChunk) {
  int32_t err = SoundMixer::loadChunkFromFile(path, outChunk);

  if (EXIT_SUCCESS != err) {
    LOGERR("Error in SoundMixer::loadChunkFromFile for filePath: %s", path);
  } else  // EXIT_SUCCESS == err
  {
    if (SoundLevel::UNKNOWN == soundLevel) {
      LOGERR("Error, UNKNOWN soundLevel value detected.");

      err = EXIT_FAILURE;
    } else  // it is valid SoundLevel value
    {
      SoundMixer::setChunkVolume(outChunk, getEnumValue(soundLevel));
    }
  }

  return err;
}
