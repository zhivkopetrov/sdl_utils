// Corresponding header
#include "sdl_utils/sound/SoundMixer.h"

// C system headers

// C++ system headers
#include <cstdlib>

// Other libraries headers
#include <SDL_mixer.h>

// Own components headers
#include "sdl_utils/sound/defines/SoundMixerDefines.h"
#include "utils/Log.h"

int32_t SoundMixer::allocateSoundChannels(const int32_t requestedChannels) {
  if (requestedChannels != Mix_AllocateChannels(requestedChannels)) {
    LOGERR("Error in Mix_AllocateChannels() with requested channels: %d "
           "SDL_Mixer error: %s", requestedChannels, Mix_GetError());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int32_t SoundMixer::setCallbackOnChannelFinish(
    void (*cb)(const int32_t channel)) {
  if (nullptr == cb) {
    LOGERR("Warning, nullptr user defined callback detected.");
    return EXIT_FAILURE;
  }
  Mix_ChannelFinished(cb);

  return EXIT_SUCCESS;
}

void SoundMixer::setChannelVolume(const int32_t channel, const int32_t volume) {
  if (0 > volume || 128 < volume) {
    LOGERR(
        "Warning, invalid volume value provided %d for channel: %d. "
        "Volume must be in range 0-128",
        volume, channel);
    return;
  }

  Mix_Volume(channel, volume);
}

void SoundMixer::setAllChannelsVolume(const int32_t volume) {
  if (0 > volume || 128 < volume) {
    LOGERR(
        "Warning, invalid volume value provided %d. "
        "Volume must be in range 0-128",
        volume);
    return;
  }

  //-1 param for channel means all channels
  Mix_Volume(-1, volume);
}

int32_t SoundMixer::getChannelVolume(const int32_t channel) {
  //-1 param for volume means return the current channel volume
  return Mix_Volume(channel, -1);
}

void SoundMixer::pauseChannel(const int32_t channel) { Mix_Pause(channel); }

void SoundMixer::resumeChannel(const int32_t channel) { Mix_Resume(channel); }

void SoundMixer::stopChannel(const int32_t channel) {
  Mix_HaltChannel(channel);
}

void SoundMixer::stopAllChannels() { Mix_HaltChannel(-1); }

bool SoundMixer::isChannelPaused(const int32_t channel) {
  return Mix_Paused(channel);
}

bool SoundMixer::isChannelPlaying(const int32_t channel) {
  return Mix_Playing(channel);
}

int32_t SoundMixer::setChannelPanning(const int32_t channel,
                                      const uint8_t leftVolume,
                                      const uint8_t rightVolume) {
  if (0 == Mix_SetPanning(channel, leftVolume, rightVolume)) {
    LOGERR("Error in Mix_SetPanning() for leftVolume: %hhu, rightVolume: %hhu, "
           "SDL_Mixer error: %s", leftVolume, rightVolume, Mix_GetError());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int32_t SoundMixer::resetChannelPanning(const int32_t channel) {
  return setChannelPanning(channel, 255, 255);
}

int32_t SoundMixer::loadMusicFromFile(const char* path, Mix_Music*& outMusic) {
  // check for memory leaks
  if (nullptr != outMusic) {
    freeMusic(outMusic);
  }
  outMusic = Mix_LoadMUS(path);

  if (nullptr == outMusic) {
    LOGERR("Failed to load Mix_Music from path: %s. SDL_mixer Error: %s", path,
           Mix_GetError());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void SoundMixer::freeMusic(Mix_Music*& music) {
  // sanity check
  if (nullptr != music) {
    Mix_FreeMusic(music);
    music = nullptr;
  }
}

void SoundMixer::setMusicVolume(const int32_t volume) {
  if (0 > volume || 128 < volume) {
    LOGERR(
        "Warning, invalid volume value provided %d for music. "
        "Volume must be in range 0-128",
        volume);
    return;
  }

  Mix_VolumeMusic(volume);
}

int32_t SoundMixer::getMusicVolume() {
  //-1 param for volume means return the current music volume
  return Mix_VolumeMusic(-1);
}

int32_t SoundMixer::playMusic(Mix_Music* music, const int32_t loops) {
  return Mix_PlayMusic(music, loops);
}

void SoundMixer::pauseMusic() { Mix_PauseMusic(); }

void SoundMixer::resumeMusic() { Mix_ResumeMusic(); }

void SoundMixer::rewindMusic() { Mix_RewindMusic(); }

void SoundMixer::stopMusic() { Mix_HaltMusic(); }

bool SoundMixer::isMusicPlaying() { return Mix_PlayingMusic(); }

bool SoundMixer::isMusicPaused() { return Mix_PausedMusic(); }

int32_t SoundMixer::loadChunkFromFile(const char* path, Mix_Chunk*& outChunk) {
  // check for memory leaks
  if (nullptr != outChunk) {
    freeChunk(outChunk);
  }
  outChunk = Mix_LoadWAV(path);

  if (nullptr == outChunk) {
    LOGERR("Failed to load Mix_Chunk from path: %s. SDL_mixer Error: %s", path,
           Mix_GetError());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void SoundMixer::freeChunk(Mix_Chunk*& chunk) {
  // sanity check
  if (nullptr != chunk) {
    Mix_FreeChunk(chunk);
    chunk = nullptr;
  }
}

void SoundMixer::setChunkVolume(Mix_Chunk* chunk, const int32_t volume) {
  // sanity check
  if (nullptr == chunk) {
    LOGERR("Warning, nullptr chunk detected");
    return;
  }

  if (0 > volume || 128 < volume) {
    LOGERR(
        "Warning, invalid volume value provided %d for chunk address: %p"
        ". Volume must be in range 0-128",
        volume, chunk);
    return;
  }

  Mix_VolumeChunk(chunk, volume);
}

int32_t SoundMixer::getChunkVolume(Mix_Chunk* chunk) {
  // sanity check
  if (nullptr == chunk) {
    LOGERR("Warning, nullptr chunk detected. Returning 0 for volume");
    return 0;
  }

  //-1 param for volume means return the current chunk volume
  return Mix_VolumeChunk(chunk, -1);
}

int32_t SoundMixer::playChunk(Mix_Chunk* chunk, const int32_t channelId,
                              const int32_t loops) {
  return Mix_PlayChannel(channelId, chunk, loops);
}
