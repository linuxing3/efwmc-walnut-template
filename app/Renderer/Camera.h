#ifndef RTIAW_camera
#define RTIAW_camera

#include "Ray.h"
#include "Utils.h"
#include "spdlog/logger.h"

namespace RTIAW::Render {
class Camera {
public:
  struct CameraOrientation {
    point3 lookfrom; // m_position
    point3 lookat;
    vec3 vup;
  };

  Camera(CameraOrientation orientation, float verticalFov, float aspectRatio,
         float aperture, float focusDist);

  [[nodiscard]] Ray NewRay(float u, float v) const;

private:
  void RecalculateProjection();
  void RecalculateView();
  void RecalculateRayDirections();

private:
  // viewport properties
  float m_viewportHeight{2.0f};
  float m_viewportWidth{0.0f};
  float m_lensRadius{0.5f};

  point3 m_origin{0, 0, 0};
  point3 m_ForwardDirection{0.0f, 0.0f, 0.0f};

  vec3 m_horizontal;
  vec3 m_vertical;
  vec3 u, v, w;
  point3 m_lowerLeftCorner;

public:
  // Walnut Camera
  bool OnUpdate(float ts);
  void OnResize(uint32_t width, uint32_t height);

  [[nodiscard]] const glm::mat4 &GetProjection() const { return m_Projection; }
  [[nodiscard]] const glm::mat4 &GetInverseProjection() const {
    return m_InverseProjection;
  }
  [[nodiscard]] const glm::mat4 &GetView() const { return m_View; }
  [[nodiscard]] const glm::mat4 &GetInverseView() const {
    return m_InverseView;
  }

  float GetRotationSpeed();

private:
  glm::mat4 m_Projection{1.0f};
  glm::mat4 m_View{1.0f};
  glm::mat4 m_InverseProjection{1.0f};
  glm::mat4 m_InverseView{1.0f};

  float m_VerticalFOV = 45.0f;
  float m_NearClip = 0.1f;
  float m_FarClip = 100.0f;
  // Cached ray directions
  std::vector<glm::vec3> m_RayDirections;

  glm::vec2 m_LastMousePosition{0.0f, 0.0f};

  uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
};
} // namespace RTIAW::Render

#endif
