// Corresponding header
#include "sdl_utils/drawing/RendererState.h"

// C system headers

// C++ system headers
#include <cstdlib>

// Other libraries headers

// Own components headers
#include "sdl_utils/drawing/config/RendererConfig.hpp"
#include "utils/Log.h"

int32_t RendererState::init(const RendererConfig &cfg) {
  if (EXIT_SUCCESS != renderData.init(cfg.maxRendererBackBufferDataSize)) {
    LOGERR("renderData.init() failed");
    return EXIT_FAILURE;
  }

  maxRuntimeRendererCmds = cfg.maxRuntimeRendererCommands;
  rendererCmd.resize(cfg.maxRuntimeRendererCommands, RendererCmd::UNDEFINED);

  maxRuntimeWidgets = cfg.maxRuntimeWidgets;
  widgets.resize(cfg.maxRuntimeWidgets);

  return EXIT_SUCCESS;
}
