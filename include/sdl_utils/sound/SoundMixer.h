#ifndef SDL_UTILS_SOUNDMIXER_H_
#define SDL_UTILS_SOUNDMIXER_H_

/*
 * SoundMixer.h
 *
 *  @brief A class for playing sounds.
 *
 *         Definitions:
 *              > Music:
 *                       - Only 1 music can be played/paused/rewind at a time;
 *                       - Music is usually a bigger piece of sound;
 *                       - Music is not loaded into memory - it is
 *                                                         constantly buffered;
 *              > Chunks/Sounds:
 *                       - Big numbers of sounds can be played simultaneously;
 *                       - Chunks/Sounds are loaded into memory;
 */

// System headers
#include <cstdint>

// Other libraries headers
#include "utils/ErrorCode.h"

// Own components headers

// Forward declarations
enum class SoundLevel : uint8_t;
typedef struct _Mix_Music Mix_Music;
typedef struct Mix_Chunk Mix_Chunk;

class SoundMixer {
 public:
  SoundMixer() = delete;

  //=================== START Common Sound functions =====================

  /** @brief used to request dynamically change of supported
   *                                                      sound channels.
   *
   *  @param const int32_t - the number of requested Channels
   *
   *  @return ErrorCode    - error code
   * */
  static ErrorCode allocateSoundChannels(const int32_t requestedChannels);

  /** @brief used to set user defined callback for when a sound (music or
   *                chunk) is finished playing on it's associated channel
   *
   *  @param void (*cb)(int32_t) - user defined callback to be attached
   *
   *  @return ErrorCode          - error code
   * */
  static ErrorCode setCallbackOnChannelFinish(
      void (*cb)(const int32_t channel));

  //==================== END Common Sound functions ======================

  //=================== START Channel Sound functions ====================

  /** @brief used to set channel volume
   *         NOTE: This function does not return error code for
   *                                                  performance reasons
   *
   *  @param const int32_t - specific channel Id
   *  @param const int32_t - volume in range [0-128]
   * */
  static void setChannelVolume(const int32_t channel, const int32_t volume);

  /** @brief used to set global(all channels) volume
   *         NOTE: This function does not return error code for
   *                                                  performance reasons
   *
   *  @param const int32_t - volume in range [0-128]
   * */
  static void setAllChannelsVolume(const int32_t volume);

  /** @brief used to get the selected channel volume
   *         NOTE: This function does not return error code for
   *                                                  performance reasons
   *
   *  @param const int32_t - specific channel Id
   *
   *  @return int32_t - selected channel volume
   * */
  static int32_t getChannelVolume(const int32_t channel);

  /** @brief used to pause currently playing channel
   *         NOTE: This function does not return error code for
   *                                                  performance reasons
   *
   *  @param const int32_t - specific channel Id. If -1 is passes the
   *                                    sound for all channels is changed
   * */
  static void pauseChannel(const int32_t channel);

  /** @brief used to resume currently paused channel
   *         NOTE: This function does not return error code for
   *                                                  performance reasons
   *
   *  @param const int32_t - specific channel Id. If -1 is passes the
   *                                    sound for all channels is changed
   * */
  static void resumeChannel(const int32_t channel);

  /** @brief used to stop currently playing channel
   *         NOTE: This function does not return error code for
   *                                                  performance reasons
   *
   *  @param const int32_t - specific channel
   * */
  static void stopChannel(const int32_t channel);

  /** @brief used to stop all playing channels
   * */
  static void stopAllChannels();

  /** @brief used to determine whether a specific channel is currently
   *                                                        paused or not
   *         NOTE: This function does not return error code for
   *                                                  performance reasons
   *
   *  @param const int32_t - specific channel Id.
   *
   *  @return bool - is channel paused or not
   * */
  static bool isChannelPaused(const int32_t channel);

  /** @brief used to determine whether a specific channel is currently
   *                                                       playing or not
   *         NOTE: This function does not return error code for
   *                                                  performance reasons
   *
   *  @param const int32_t - specific channel Id.
   *
   *  @return bool - is channel playing or not
   * */
  static bool isChannelPlaying(const int32_t channel);

  /** @brief used to set the panning of a channel. The left volume and
   *         right volume are specified as integers between 0 and 255,
   *                                    quietest to loudest, respectively.
   *
   *         NOTE: for real panning effect consider using
   *                      setChannelPanning(channel, left, 255 - left);
   *
   *  @param const int32_t - specific channel Id
   *  @param const uint8_t - left volume value
   *  @param const uint8_t - right volume value
   *
   *  @return ErrorCode    - error code
   * */
  static ErrorCode setChannelPanning(const int32_t channel,
                                     const uint8_t leftVolume,
                                     const uint8_t rightVolume);

  /** @brief used to reset the panning of a channel.
   *
   *  @param const int32_t - specific channel Id
   *
   *  @return ErrorCode    - error code
   * */
  static ErrorCode resetChannelPanning(const int32_t channel);

  //==================== END Channel Sound functions =====================

  //=================== START Music related functions ====================

  /** @brief used to load SDL_Texture from file on the hard drive
   *
   *  @param const char * - absolute path to file
   *  @param Mix_Music *& - dynamically created Mix_Music
   *
   *  @returns ErrorCode  - error code
   * */
  static ErrorCode loadMusicFromFile(const char* path, Mix_Music*& outMusic);

  /** @brief used to free Mix_Music
   *
   *  @param Mix_Music*& the surface to be freed
   * */
  static void freeMusic(Mix_Music*& music);

  /** @brief used to set music volume
   *         NOTE: This function does not return error code for
   *                                                  performance reasons
   *
   *  @param const int32_t - volume in range [0-128]
   * */
  static void setMusicVolume(const int32_t volume);

  /** @brief used to get the selected channel volume
   *
   *  @return int32_t - currently loaded music volume
   * */
  static int32_t getMusicVolume();

  /** @brief used to play a music stream
   *         NOTE: This function does not return error code for
   *                                                  performance reasons
   *
   *  @param Mix_Music *   - the music to be played
   *  @param const int32_t - number of loops for the music (-1 for
   *                                              endless loop /~65000/ )
   *
   *  @return int32_t      - the associated sound channel for the
   *                                                      provided chunk
   * */
  static int32_t playMusic(Mix_Music* music, const int32_t loops = -1);

  /** @brief used to pause currently playing music stream
   * */
  static void pauseMusic();

  /** @brief used to resume currently paused music stream
   * */
  static void resumeMusic();

  /** @brief used to rewind(start from the begging) a currently playing
   *                                                         music stream
   *
   *         NOTE: the stream must first be paused
   * */
  static void rewindMusic();

  /** @brief used to stop a currently playing music stream
   * */
  static void stopMusic();

  /** @brief used to determine whether a music is currently playing or not
   *
   *  @return bool - is music playing or not
   * */
  static bool isMusicPlaying();

  /** @brief used to determine whether a music is currently paused or not
   *
   *  @return bool - is music paused or not
   * */
  static bool isMusicPaused();

  //==================== END Music related functions =====================

  //=================== START Chunk related functions ====================

  /** @brief used to load SDL_Texture from file on the hard drive
   *
   *  @param const char * - absolute path to file
   *  @param Mix_Chunk *& - dynamically created Mix_Chunk
   *
   *  @returns ErrorCode    - error code
   * */
  static ErrorCode loadChunkFromFile(const char* path, Mix_Chunk*& outChunk);

  /** @brief used to free Mix_Chunk
   *
   *  @param Mix_Chunk *& the surface to be freed
   * */
  static void freeChunk(Mix_Chunk*& chunk);

  /** @brief used to set chunk volume
   *         NOTE: This function does not return error code for
   *                                                  performance reasons
   *
   *  @param Mix_Chunk *   - chunk, which volume level will be changed
   *  @param const int32_t - volume in range [0-128]
   * */
  static void setChunkVolume(Mix_Chunk* chunk, const int32_t volume);

  /** @brief used to get the selected channel volume
   *         NOTE: This function does not return error code for
   *                                                  performance reasons
   *
   *  @param Mix_Chunk * - selected Mix_Chunk
   *
   *  @return int32_t - selected Mix_Chunk volume
   * */
  static int32_t getChunkVolume(Mix_Chunk* chunk);

  /** @brief used to play a music stream
   *         NOTE: This function does not return error code for
   *                                                  performance reasons
   *
   *  @param Mix_Chunk*    - the sound chunk to be played
   *  @param const int32_t - specific used requested sound channel
   *  @param const int32_t - number of loops for the music (-1 for
   *                                              endless loop /~65000/ )
   *
   *  @return int32_t      - the associated sound channel for the
   *                                                      provided chunk
   * */
  static int32_t playChunk(Mix_Chunk* chunk, const int32_t channelId,
                           const int32_t loops = 0);

  //==================== END Chunk related functions =====================
};

#endif /* SDL_UTILS_SOUNDMIXER_H_ */
