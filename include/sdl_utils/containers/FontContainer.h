#ifndef SDL_UTILS_FONTCONTAINER_H_
#define SDL_UTILS_FONTCONTAINER_H_

// System headers
#include <cstdint>
#include <unordered_map>

// Other libraries headers
#include "resource_utils/structs/FontData.h"
#include "utils/ErrorCode.h"

// Own components headers

// Forward declarations
typedef struct _TTF_Font TTF_Font;

class FontContainer {
 public:
  /** @brief used to initialise the Font container
   *
   *  @param const std::string & - absolute file path to resouces follder
   *  @param const uint64_t      - number of fonts to be loaded
   *
   *  @return ErrorCode          - error code
   * */
  ErrorCode init(const std::string &resourcesFolderLocation,
                 const uint64_t fontsCount);

  /** @brief used to deinitialize (free memory occupied by Font container)
   * */
  void deinit();

  /** @brief used to store the provided FontData in Font Container
   *
   *  @param const FontData & - populated structure with
   *                                                 Font specific data
   * */
   void storeFontData(const FontData &fontData) {
    _fontsDataMap[fontData.header.hashValue] = fontData;
  }

  /** @brief used to load all stored fonts from the _fontsDataMap
   *                                      as TTF_Font * in the _fontsMap
   * */
  void loadAllStoredFonts();

  /** @brief used acquire a previously stored TTF_Font *
   *                                                  from the _fontsMap
   *
   *         NOTE: this function does not return error code for
   *                                                  performance reasons
   *
   *  @param const uint64_t - unique font id
   *
   *  @return TTF_Font *    - requested TTF_Font * corresponding
   *                                      to the unique provided font id
   * */
   TTF_Font *getTtfFont(const uint64_t fontId) {
    return _fontsMap[fontId];
  }

  /** @brief used to acquire access to fonts map
   *
   *  @return std::unordered_map<uint64_t, TTF_Font *> * -
   *                                      reference to fontsMap container
   * */
   std::unordered_map<uint64_t, TTF_Font *> *getFontsMap() {
    return &_fontsMap;
  }

 private:
  /** @brief used to create TTF_Font for a given unique font location
   *                                                   on the file system
   *
   *  @param const char *   - unique text location on file system\
   *  @param const int32_t  - input font size
   *  @param SDL_Texture *& - created SDL_Texture
   *
   *  @returns ErrorCode    - error code
   * */
   ErrorCode loadTtfFont(const char *path, const int32_t fontSize,
                         TTF_Font *&outFont);

  //_fontsMap holds all fonts
  std::unordered_map<uint64_t, TTF_Font *> _fontsMap;

  //_rsrcDataMap holds font specific information for every font
  std::unordered_map<uint64_t, FontData> _fontsDataMap;

  std::string _resourcesFolderLocation;
};

#endif /* SDL_UTILS_FONTCONTAINER_H_ */
