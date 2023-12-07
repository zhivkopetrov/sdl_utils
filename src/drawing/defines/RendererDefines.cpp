//Corresponding header
#include "sdl_utils/drawing/defines/RendererDefines.h"

//System headers

//Other libraries headers
#include "utils/data_type/EnumClassUtils.h"
#include "utils/log/Log.h"

//Own components headers

RendererFlagsMask valiteRendererFlagsMask(const RendererFlagsMask mask) {
  constexpr auto maxRendererFlagsMaskValue =
      getEnumValue(RendererFlag::SOFTARE_RENDERER) |
      getEnumValue(RendererFlag::HARDWARE_RENDERER) |
      getEnumValue(RendererFlag::VSYNC_ENABLE) |
      getEnumValue(RendererFlag::FBO_ENABLE);
  if (maxRendererFlagsMaskValue < mask) {
    LOGERR("Error, received unsupported RendererFlagsMask: [%u]. Defaulting to "
           "RendererFlag::HARDWARE_RENDERER | RendererFlag::FBO_ENABLE", mask);
    constexpr auto defaultMask =
        getEnumValue(RendererFlag::HARDWARE_RENDERER) |
        getEnumValue(RendererFlag::FBO_ENABLE);
    return defaultMask;
  }

  return mask;
}

RendererPolicy valiteRendererExecutionPolicy(const RendererPolicy policy) {
  switch (policy) {
  case RendererPolicy::SINGLE_THREADED:
    return policy;

  case RendererPolicy::MULTI_THREADED:
    return policy;

  default:
    LOGERR("Error, received unsupported RendererPolicy: [%d]. Defaulting to "
           "SINGLE_THREADED execution policy", getEnumValue(policy));
    return RendererPolicy::SINGLE_THREADED;
  }
}

std::string getRendererPolicyName(RendererPolicy policy) {
  switch (policy) {
  case RendererPolicy::SINGLE_THREADED:
    return "SINGLE_THREADED";

  case RendererPolicy::MULTI_THREADED:
    return "MULTI_THREADED";

  default:
    LOGERR("Error, received unsupported RendererPolicy: [%d]",
        getEnumValue(policy));
    return "UNSUPPORTED_RENDERER_POLICY";
  }
}
