#pragma once

#ifndef RTIAW_shapes_box3
#define RTIAW_shapes_box3

#include "Renderer/HitRecord.h"
#include "Renderer/Ray.h"
#include "Renderer/Utils.h"
#include <optional>

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

    float tmin = _tmin, tmax = _tmax;
    float tymin, tymax, tzmin, tzmax;

    tmin = (bounds[r.sign[0]].x - r.origin.x) * r.inverseDirection.x;
    tmax = (bounds[1 - r.sign[0]].x - r.origin.x) * r.inverseDirection.x;

    tymin = (bounds[r.sign[1]].y - r.origin.y) * r.inverseDirection.y;
    tymax = (bounds[1 - r.sign[1]].y - r.origin.y) * r.inverseDirection.y;

    if ((tmin > tymax) || (tymin > tmax))
      return false;
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

    return tmin;
  };

  [[nodiscard]] HitRecord ComputeHitRecord(const Ray &r, const float t) const {
    HitRecord result{};
    result.t = t;
    result.p = r.At(result.t);
    vec3 outward_normal = glm::normalize(result.p - r.origin);
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
  }
  point3 bounds[2];
};
} // namespace RTIAW::Render::Shapes
#endif