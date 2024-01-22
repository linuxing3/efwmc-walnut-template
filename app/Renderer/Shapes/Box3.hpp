#pragma once

#ifndef RTIAW_shapes_box3
#define RTIAW_shapes_box3

#include "Renderer/HitRecord.h"
#include "Renderer/Ray.h"
#include "Renderer/Utils.h"
#include <optional>

template <typename T> int sign(T val) { return (T(0) < val) - (val < T(0)); }

static float CUBEVERTICES[] = {-0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,
                               0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,

                               -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f,
                               0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f};
// NOTE: like AABB
using namespace std;

/**
 *
BOX中只有两个对象，一个是盒子的左下角坐标，另一个是右上角坐标。
*/

namespace RTIAW::Render::Shapes {
class Box3 {
  /**
我们设B0x的值为bounds[0].x。则x=B0x代表了三维中的一个面。

我们射线公式 P=O+Dt，在x分量上可以用 Ox + tDx = B0x
来计算出射线击中面x=B0x时的t值：

t0x = (B0x - Ox) / Dx
t1x = (B1x - Ox) / Dx
t0y = (B0y - Oy) / Dy
t1y = (B1y - Oy) / Dy
t0z = (B0z - Oz) / Dz
t1z = (B1z - Oz) / Dz
  */
public:
  [[nodiscard]] float FastHit(const Ray &r, const float _tmin,
                              const float _tmax) const {

    float tmin, tmax;
    float tymin, tymax, tzmin, tzmax;

    tmin = (bounds[r.sign[0]].x - r.origin.x) * r.inverseDirection.x;
    tmax = (bounds[1 - r.sign[0]].x - r.origin.x) * r.inverseDirection.x;

    tymin = (bounds[r.sign[1]].y - r.origin.y) * r.inverseDirection.y;
    tymax = (bounds[1 - r.sign[1]].y - r.origin.y) * r.inverseDirection.y;

    if ((tmin > tymax) || (tymin > tmax))
      return numeric_limits<float>::max();
    if (tymin > tmin)
      tmin = tymin;
    if (tymax < tmax)
      tmax = tymax;

    tzmin = (bounds[r.sign[2]].z - r.origin.z) * r.inverseDirection.z;
    tzmax = (bounds[1 - r.sign[2]].z - r.origin.z) * r.inverseDirection.z;

    if ((tmin > tzmax) || (tzmin > tmax))
      return numeric_limits<float>::max();
    if (tzmin > tmin)
      tmin = tzmin;
    if (tzmax < tmax)
      tmax = tzmax;

    float root = tmin;

    return root;
  };

  [[nodiscard]] HitRecord ComputeHitRecord(const Ray &r, const float t) const {
    HitRecord result{};
    result.t = t;
    result.p = r.At(result.t);

    vec3 outward_normal = glm::normalize(result.p - center);
    float sign = glm::dot(outward_normal, result.p - center);
    outward_normal *= sign;

    result.SetFaceNormal(r, outward_normal);

    return result;
  };

  [[nodiscard]] optional<HitRecord> Hit(const Ray &r, const float tmin,
                                        const float tmax) const {
    static constexpr optional<HitRecord> empty_result{};
    if (const auto t = FastHit(r, tmin, tmax);
        t < numeric_limits<float>::max()) {
      return ComputeHitRecord(r, t);
    } else {
      return empty_result;
    }
  };

public:
  Box3(const point3 &vmin, const point3 &vmax) {
    bounds[0] = vmin;
    bounds[1] = vmax;
    center = point3((vmax.x - vmin.x) / 2, (vmax.y - vmin.y) / 2,
                    (vmax.z - vmax.z) / 2);

    // Loop through original data and copy to 2D array
    // for (int i = 0; i < 8; i++) {
    //   points[i][0] = CUBEVERTICES[i * 3];
    //   points[i][1] = CUBEVERTICES[i * 3 + 1];
    //   points[i][2] = CUBEVERTICES[i * 3 + 2];
    // }
  }
  point3 bounds[2];
  point3 center;
  // point3 points[8];
  // Original cube vertex data
};
} // namespace RTIAW::Render::Shapes
#endif
