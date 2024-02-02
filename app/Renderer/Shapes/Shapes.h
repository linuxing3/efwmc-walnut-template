#ifndef RTIAW_shapes_shapes
#define RTIAW_shapes_shapes

#include <variant>

#include "Renderer/Shapes/Cube.h"
#include "Renderer/Shapes/Parallelogram.h"
#include "Renderer/Shapes/Plane.h"
#include "Renderer/Shapes/Rectangle.h"
#include "Renderer/Shapes/Sphere.h"

namespace RTIAW::Render {
using Shape = std::variant<Shapes::Sphere, Shapes::Plane, Shapes::Parallelogram,
                           Shapes::Rectangle, Shapes::Cube>;
} // namespace RTIAW::Render

#endif
