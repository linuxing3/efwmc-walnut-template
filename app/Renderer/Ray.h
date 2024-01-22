#ifndef RTIAW_ray
#define RTIAW_ray

#include "Renderer/Utils.h"
#include <glm/exponential.hpp>

namespace RTIAW::Render {
class Ray {
public:
  constexpr Ray() = default;
  Ray(const point3 origin, const vec3 direction) : origin{origin}, direction{glm::normalize(direction)} {
        inverseDirection = vec3(-direction.x, -direction.y,-direction.z); 
        sign[0] = (inverseDirection.x < 0); 
        sign[1] = (inverseDirection.y < 0); 
        sign[2] = (inverseDirection.z < 0); 
  }

  point3 origin{0, 0, 0};
  vec3 direction{0, 0, 0};
  vec3 inverseDirection{0, 0, 0};
  int sign[3];

  [[nodiscard]] constexpr point3 At(const float t) const { return origin + direction * t; }
};
} // namespace RTIAW::Render

#endif
