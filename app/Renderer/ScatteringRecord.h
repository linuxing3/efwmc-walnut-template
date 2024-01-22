#ifndef RTIAW_scatteringrecord
#define RTIAW_scatteringrecord

#include "Renderer/Ray.h"
#include "Renderer/Utils.h"

namespace RTIAW::Render {
struct ScatteringRecord {
  color attenuation{0, 0, 0};
  Ray ray{};
};
} // namespace RTIAW::Render

#endif
