#ifndef SDL_UTILS_SOUNDCONTAINER_H_
#define SDL_UTILS_SOUNDCONTAINER_H_

// System headers
#include <cstdint>
#include <unordered_map>

// Other libraries headers
#include "resource_utils/structs/SoundData.h"
#include "utils/ErrorCode.h"

// Own components headers

// Forward declarations
typedef struct _Mix_Music Mix_Music;
typedef struct Mix_Chunk Mix_Chunk;

class SoundContainer {
 public:
  /** @brief used to initialise the Sound container
   *
   *  @param const std::string & - absolute file path to resource folder
   *  @param const uint64_t      - number of musics to be loaded
   *  @param const uint64_t      - number of sound chunks to be loaded
   *
   *  @return ErrorCode         - error code
   * */
  ErrorCode init(const std::string &resourcesFolderLocation,
                 const uint64_t musicsCount,
                 const uint64_t chunksCount);

  /** @brief used to deinitialize
   *                           (free memory occupied by Sound container)
   * */
  void deinit();

  /** @brief used to store the provided SoundData in Sound Container
   *
   *  @param const SoundData & - populated structure with
   *                                                 Sound specific data
   * */
   void storeSoundData(const SoundData &soundData) {
    _soundsDataMap[soundData.header.hashValue] = soundData;
  }

  /** @brief used to load all stored sounds from the _soundsDataMap:
   *                                > as Mix_Chunk's in the _chunksMap;
   *                                > as Mix_Music's in the _soundsMap;
   * */
  void loadAllStoredSounds();

  /** @brief used to acquire previously stored sound data
   *                                       for a given unique sound ID
   *
   *  @param const uint64_t     - unique sound ID
   *  @param const SoundData *& - resource specific data
   *
   *  @returns ErrorCode        - error code
   * */
  ErrorCode getSoundData(const uint64_t soundId, const SoundData *&outData);

  /** @brief used to acquire previously stored pre-created Mix_Music
   *                                       for a given unique resource ID
   *  This function does not return error code for performance reasons
   *
   *  @param const uint64_t - unique resource ID
   *  @param Mix_Music *&   - pre-created Mix_Music
   * */
  void getMusicSound(const uint64_t rsrcId, Mix_Music *&outMusic);

  /** @brief used to acquire previously stored pre-created Mix_Chunk
   *                                       for a given unique resource ID
   *  This function does not return error code for performance reasons
   *
   *  @param const uint64_t - unique resource ID
   *  @param Mix_Chunk *&   - pre-created Mix_Chunk
   * */
  void getChunkSound(const uint64_t rsrcId, Mix_Chunk *&outChunk);

 private:
  /** @brief used to create Mix_Music for a given unique font location
   *                                                   on the file system
   *
   *  @param const char *     - unique text location on file system
   *  @param const SoundLevel - the input sound level
   *  @param Mix_Music *&     - created Mix_Music
   *
   *  @returns ErrorCode      - error code
   * */
  ErrorCode loadMusic(const char *path, const SoundLevel soundLevel,
                      Mix_Music *&outMusic);

  /** @brief used to create Mix_Chunk for a given unique font location
   *                                                   on the file system
   *
   *  @param const char *     - unique text location on file system.
   *  @param const SoundLevel - the input sound level
   *  @param Mix_Chunk *&     - created Mix_Chunk
   *
   *  @returns ErrorCode      - error code
   * */
  ErrorCode loadChunk(const char *path, const SoundLevel soundLevel,
                      Mix_Chunk *&outChunk);

  //_musicMap holds all music sounds
  std::unordered_map<uint64_t, Mix_Music *> _musicMap;

  //_chunkMap holds all chunk sounds
  std::unordered_map<uint64_t, Mix_Chunk *> _chunkMap;

  //_soundsDataMap holds font specific information for
  // every sound(music + chunk)
  std::unordered_map<uint64_t, SoundData> _soundsDataMap;

  std::string _resourcesFolderLocation;
};

#endif /* SDL_UTILS_SOUNDCONTAINER_H_ */
