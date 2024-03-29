#ifndef RTIAW_hittableobject
#define RTIAW_hittableobject

#include <memory>
#include <optional>
#include <variant>

#include "Renderer/Materials/Materials.h"
#include "Renderer/Ray.h"
#include "Renderer/Shapes/Shapes.h"

namespace RTIAW::Render {
using HitResult =
    std::pair<std::optional<HitRecord>, std::optional<ScatteringRecord>>;
class HittableObject {

 public:
  HittableObject(Shape shape, const size_t materialIndex)
      : m_shape{shape}, m_materialIndex{materialIndex} {}
  virtual ~HittableObject() = default;

  [[nodiscard]] size_t MaterialIndex() const { return m_materialIndex; };
  [[nodiscard]] float FastHit(const Ray &r, float t_min, float t_max) const;
  [[nodiscard]] HitRecord ComputeHitRecord(const Ray &r, float t) const;
  [[nodiscard]] std::optional<HitRecord> Hit(const Ray &r, float t_min,
                                             float t_max) const;

  Shape GetShape() { return m_shape; };

  size_t getMaterialIndex() { return m_materialIndex; };

 private:
  Shape m_shape;
  size_t m_materialIndex{std::numeric_limits<size_t>::max()};
};
}  // namespace RTIAW::Render

#endif
