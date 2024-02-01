#include <cstdio>
#include <glm/gtc/random.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Input.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "Application.h"
#include "Renderer/Camera.h"

using namespace Walnut;
using namespace glm;

namespace RTIAW::Render {
Camera::Camera(CameraOrientation orientation, float verticalFov,
               float aspectRatio, float aperture, float focusDist)
    : m_VerticalFOV(verticalFov) {

  spdlog::set_level(spdlog::level::debug);

  auto theta = Utils::DegToRad(m_VerticalFOV);
  auto h = std::tan(theta / 2);
  m_viewportHeight = 2.0f * h;
  m_viewportWidth = aspectRatio * m_viewportHeight;
  m_lensRadius = aperture / 2.0f;

  w = normalize(orientation.lookfrom -
                orientation.lookat);             // forwardDirection (negative)
  u = normalize(glm::cross(orientation.vup, w)); // rightDirection
  v = cross(w, u);                               // upDirection

  m_origin = orientation.lookfrom;
  // FIXME:
  w = normalize(orientation.lookat - orientation.lookfrom);

  m_horizontal = focusDist * m_viewportWidth * u;
  m_vertical = focusDist * m_viewportHeight * v;
  m_lowerLeftCorner =
      m_origin - m_horizontal / 2.0f - m_vertical / 2.0f - focusDist * w;
}

Ray Camera::NewRay(float s, float t) const {
  vec2 randVec = Random::diskRand(m_lensRadius);

  vec3 offset = u * randVec.x + v * randVec.y;

  return Ray(m_origin + offset, m_lowerLeftCorner + s * m_horizontal +
                                    t * m_vertical - m_origin - offset);
}

// FIXME: this is a hack, should be in ApplicationLayer
bool Camera::OnUpdate(float ts) {
  vec2 mousePos = Input::GetMousePosition();
  vec2 delta = (mousePos - m_LastMousePosition) * 0.002f;
  m_LastMousePosition = mousePos;

  if (!Input::IsMouseButtonDown(MouseButton::Right)) {
    Input::SetCursorMode(CursorMode::Normal);
    return false;
  }

  Input::SetCursorMode(CursorMode::Locked);

  bool moved = false;

  constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
  glm::vec3 rightDirection = cross(w, upDirection);

  float speed = 5.0f;

  // Movement
  if (Input::IsKeyDown(KeyCode::W)) {

    printf("W\n");
    m_origin += w * speed * ts;
    moved = true;
  } else if (Input::IsKeyDown(KeyCode::S)) {
    printf("S\n");
    m_origin -= w * speed * ts;
    moved = true;
  }
  if (Input::IsKeyDown(KeyCode::A)) {
    printf("A\n");

    m_origin -= rightDirection * speed * ts;
    moved = true;
  } else if (Input::IsKeyDown(KeyCode::D)) {
    printf("D\n");

    m_origin += rightDirection * speed * ts;
    moved = true;
  }
  if (Input::IsKeyDown(KeyCode::Q)) {
    printf("Q\n");
    m_origin -= upDirection * speed * ts;
    moved = true;
  } else if (Input::IsKeyDown(KeyCode::E)) {
    printf("E\n");
    m_origin += upDirection * speed * ts;
    moved = true;
  }

  // Rotation
  if (delta.x != 0.0f || delta.y != 0.0f) {
    float pitchDelta = delta.y * GetRotationSpeed();
    float yawDelta = delta.x * GetRotationSpeed();

    glm::quat q = normalize(cross(angleAxis(-pitchDelta, rightDirection),
                                  angleAxis(-yawDelta, upDirection)));
    w = rotate(q, w);

    moved = true;
  }

  if (moved) {
    RecalculateView();
    RecalculateRayDirections();
  }

  return moved;
}

void Camera::OnResize(uint32_t width, uint32_t height) {
  if (width == m_ViewportWidth && height == m_ViewportHeight)
    return;

  m_ViewportWidth = width;
  m_ViewportHeight = height;

  RecalculateProjection();
  RecalculateRayDirections();
}

float Camera::GetRotationSpeed() { return 0.3f; }

void Camera::RecalculateProjection() {
  m_Projection = perspectiveFov(radians(m_VerticalFOV), (float)m_ViewportWidth,
                                (float)m_ViewportHeight, m_NearClip, m_FarClip);
  m_InverseProjection = inverse(m_Projection);
}

void Camera::RecalculateView() {
  m_View = lookAt(m_origin, m_origin + w, vec3(0, 1, 0));
  m_InverseView = inverse(m_View);
}

void Camera::RecalculateRayDirections() {
  m_RayDirections.resize(m_ViewportWidth * m_ViewportHeight);

  for (uint32_t y = 0; y < m_ViewportHeight; y++) {
    for (uint32_t x = 0; x < m_ViewportWidth; x++) {
      glm::vec2 coord = {(float)x / (float)m_ViewportWidth,
                         (float)y / (float)m_ViewportHeight};
      coord = coord * 2.0f - 1.0f; // -1 -> 1

      vec4 target = m_InverseProjection * vec4(coord.x, coord.y, 1, 1);
      vec3 rayDirection =
          vec3(m_InverseView * vec4(normalize(vec3(target) / target.w),
                                    0)); // World space
      m_RayDirections[x + y * m_ViewportWidth] = rayDirection;
    }
  }
}
} // namespace RTIAW::Render
