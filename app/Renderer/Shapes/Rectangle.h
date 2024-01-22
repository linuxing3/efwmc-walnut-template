#ifndef RTIAW_shapes_rectangle
#define RTIAW_shapes_rectangle

#include <memory>
#include <optional>

#include "Renderer/HitRecord.h"
#include "Renderer/Utils.h"

#include "Renderer/Shapes/Parallelogram.h"

namespace RTIAW::Render::Shapes {
class Rectangle : public Parallelogram {
public:
  Rectangle() : Parallelogram() {}
  explicit Rectangle(std::array<point3, 3> points);

  //[[nodiscard]] float FastHit(const Ray &r, const float t_min, const float t_max) const;
  //[[nodiscard]] HitRecord ComputeHitRecord(const Ray &r, const float t) const;
  //[[nodiscard]] std::optional<HitRecord> Hit(const Ray &r, const float t_min, const float t_max) const;
};
} // namespace RTIAW::Render::Shapes

#endif
