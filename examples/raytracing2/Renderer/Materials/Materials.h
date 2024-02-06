#ifndef RTIAW_materials_materials
#define RTIAW_materials_materials

#include <variant>

#include "Renderer/Materials/Dielectric.h"
#include "Renderer/Materials/Lambertian.h"
#include "Renderer/Materials/Metal.h"

namespace RTIAW::Render {
using Material = std::variant<Materials::Lambertian, Materials::Dielectric, Materials::Metal>;
} // namespace RTIAW::Render

#endif
