#pragma once

#include "Renderer/Materials/Materials.h"
#include "Renderer/Shapes/Rectangle.h"
#include <vector>
#ifndef RTIAW_shapes_cube
#define RTIAW_shapes_cube
#include <array>
#include <optional>

#include "Renderer/HitRecord.h"
#include "Renderer/Shapes/Plane.h"
#include "Renderer/Utils.h"

static float CUBEVERTICES[] = {-0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,
                               0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,
                               -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f,
                               0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f};
static int CUBEINDEX[6][3] = {
    {0, 1, 3}, {1, 2, 5}, {2, 3, 6}, {0, 3, 4}, {4, 5, 7}, {4, 0, 5},
};
namespace RTIAW::Render::Shapes {
class Cube {
public:
public:
  Cube();
  explicit Cube(std::array<point3, 8> points);

  [[nodiscard]] std::array<point3, 8> GetVertices() const {
    return m_vertices;
  };
  [[nodiscard]] std::vector<Shapes::Rectangle> GetRectangles() const {
    return m_rectangles;
  };

  [[nodiscard]] float FastHit(const Ray &r, const float t_min,
                              const float t_max) const;
  [[nodiscard]] HitRecord ComputeHitRecord(const Ray &r, const float t) const;
  [[nodiscard]] std::optional<HitRecord> Hit(const Ray &r, const float t_min,
                                             const float t_max) const;

private:
  std::vector<Shapes::Rectangle> m_rectangles{};
  std::array<point3, 8> m_vertices{};
  Shapes::Plane m_plane{}; // The origin of this plane lies on one vertex of the
                           // parallelogram
  size_t m_materialIndex{};
  std::vector<Material> materials;
};
} // namespace RTIAW::Render::Shapes

#endif
