#include "Renderer/Shapes/Cube.h"
#include "Renderer/HittableObject.h"
#include "Renderer/Shapes/Plane.h"
#include <array>
#include <optional>

#include "Renderer/HitRecord.h"
#include "Renderer/Shapes/Rectangle.h"
#include "Renderer/Utils.h"

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
using RTIAW::Render::point3;

namespace RTIAW::Render::Shapes {

Cube::Cube() {
  for (int i = 0; i < 8; i++) {
    m_vertices[i].x = CUBEVERTICES[i * 3];
    m_vertices[i].y = CUBEVERTICES[i * 3 + 1];
    m_vertices[i].z = CUBEVERTICES[i * 3 + 2];
  }

  for (int i = 0; i < 6; i++) {
    m_rectangles.push_back(Shapes::Rectangle({
        m_vertices[CUBEINDEX[i][0]],
        m_vertices[CUBEINDEX[i][1]],
        m_vertices[CUBEINDEX[i][2]],
    }));
  }
};

Cube::Cube(std::array<point3, 8> points) {

  for (int i = 0; i < 6; i++) {
    if (glm::dot(points[CUBEINDEX[i][0]], points[CUBEINDEX[i][1]]) == 0 &&
        glm::dot(points[CUBEINDEX[i][0]], points[CUBEINDEX[i][2]]) == 0) {
      m_rectangles.push_back(Shapes::Rectangle({
          points[CUBEINDEX[i][0]],
          points[CUBEINDEX[i][1]],
          points[CUBEINDEX[i][2]],
      }));
    } else {
      throw std::runtime_error(
          "Cube::Cube: the given points do not define a cube plane.");
    }
  }
};

float Cube::FastHit(const Ray &r, const float t_min, const float t_max) const {

};

HitRecord Cube::ComputeHitRecord(const Ray &r, const float t) const {

};

std::optional<HitRecord> Cube::Hit(const Ray &r, const float t_min,
                                   const float t_max) const {};
} // namespace RTIAW::Render::Shapes
