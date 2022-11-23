//Corresponding header
#include "sdl_utils/drawing/config/RendererConfig.h"

//System headers
#include <array>
#include <sstream>
#include <iomanip>

//Other libraries headers
#include <SDL_render.h>
#include "utils/data_type/EnumClassUtils.h"
#include "utils/Log.h"

//Own components headers

bool isRendererFlagEnabled(RendererFlagsMask mask, RendererFlag flag) {
  return mask & getEnumValue(flag);
}

void printRendererInfo(SDL_Renderer* renderer) {
  SDL_RendererInfo info;
  if (EXIT_SUCCESS != SDL_GetRendererInfo(renderer, &info)) {
    LOGERR("Error in, SDL_GetRendererInfo(), SDL Error: %s", SDL_GetError());
    return;
  }

  constexpr int32_t flagsSize = 4;
  const std::array<std::pair<const char*, bool>, flagsSize> rendererFlags = {
      std::make_pair("Software Renderer", (info.flags & SDL_RENDERER_SOFTWARE)),
      std::make_pair("Hardware Renderer",
          (info.flags & SDL_RENDERER_ACCELERATED)), std::make_pair(
          "vSync enabled", (info.flags & SDL_RENDERER_PRESENTVSYNC)),
      std::make_pair("FBO capability enabled",
          (info.flags & SDL_RENDERER_TARGETTEXTURE)) };

  std::ostringstream ostr;
  ostr << std::boolalpha
       << "=================================================================\n"
       << "Printing Renderer info:\n" << "Chosen Backend: ["
       << info.name << "]\n" << "Supported flags:\n";
  for (const auto& [name, value] : rendererFlags) {
    ostr << '\t' << name << " [" << value << "]\n";
  }
  ostr << "Supported texture formats:\n";
  for (uint32_t i = 0; i < info.num_texture_formats; ++i) {
    ostr << "\tformat[" << i << "]: "
         << SDL_GetPixelFormatName(info.texture_formats[i]) << '\n';
  }
  ostr
      << "Max Texture Width: [" << info.max_texture_width << "] px\n"
      << "Max Texture Height: [" << info.max_texture_height << "] px\n"
      << "=================================================================\n";

  LOG("%s", ostr.str().c_str());
}
