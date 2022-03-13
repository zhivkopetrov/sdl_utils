// Corresponding header
#include "sdl_utils/drawing/RendererState.h"

// System headers

// Other libraries headers
#include "utils/Log.h"

// Own components headers
#include "sdl_utils/drawing/config/RendererConfig.h"

ErrorCode RendererState::init(const RendererConfig &cfg) {
  if (ErrorCode::SUCCESS !=
      renderData.init(cfg.maxRendererBackBufferDataSize)) {
    LOGERR("renderData.init() failed");
    return ErrorCode::FAILURE;
  }

  maxRuntimeRendererCmds = cfg.maxRuntimeRendererCommands;
  rendererCmd.resize(cfg.maxRuntimeRendererCommands, RendererCmd::UNDEFINED);

  maxRuntimeWidgets = cfg.maxRuntimeWidgets;
  widgets.resize(cfg.maxRuntimeWidgets);

  return ErrorCode::SUCCESS;
}
