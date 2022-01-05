// Corresponding header
#include "sdl_utils/drawing/RendererState.h"

// C system headers

// C++ system headers

// Other libraries headers

// Own components headers
#include "sdl_utils/drawing/config/RendererConfig.hpp"
#include "utils/ErrorCode.h"
#include "utils/Log.h"

int32_t RendererState::init(const RendererConfig &cfg) {
  if (SUCCESS != renderData.init(cfg.maxRendererBackBufferDataSize)) {
    LOGERR("renderData.init() failed");
    return FAILURE;
  }

  maxRuntimeRendererCmds = cfg.maxRuntimeRendererCommands;
  rendererCmd.resize(cfg.maxRuntimeRendererCommands, RendererCmd::UNDEFINED);

  maxRuntimeWidgets = cfg.maxRuntimeWidgets;
  widgets.resize(cfg.maxRuntimeWidgets);

  return SUCCESS;
}
