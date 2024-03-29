#pragma once

#include "Renderer.h"

#include "Walnut/Random.h"

#include "stb/stb_image.h"
#include <cstdint>
#include <execution>
#include <glm/gtx/dual_quaternion.hpp>
#include <tuple>

#include <glm/fwd.hpp>
#include <glm/gtx/hash.hpp>

namespace Utils {

static uint32_t ConvertToRGBA(const glm::vec4 &color) {
  uint8_t r = (uint8_t)(color.r * 255.0f);
  uint8_t g = (uint8_t)(color.g * 255.0f);
  uint8_t b = (uint8_t)(color.b * 255.0f);
  uint8_t a = (uint8_t)(color.a * 255.0f);

  uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
  return result;
}

} // namespace Utils

void Renderer::OnResize(uint32_t width, uint32_t height) {
  if (m_FinalImage) {
    // No resize necessary
    if (m_ViewportWidth == width && m_ViewportHeight == height)
      return;

    m_FinalImage->Resize(width, height);
  } else {
    m_FinalImage = std::make_shared<Walnut::Image>(width, height,
                                                   Walnut::ImageFormat::RGBA);

    m_ViewportWidth = width;
    m_ViewportHeight = height;
  }

  delete[] m_ImageData;
  m_ImageData = new uint32_t[width * height];

  delete[] m_AccumulationData;
  m_AccumulationData = new glm::vec4[width * height];

  m_ImageHorizontalIter.resize(width);
  m_ImageVerticalIter.resize(height);
  for (uint32_t i = 0; i < width; i++)
    m_ImageHorizontalIter[i] = i;
  for (uint32_t i = 0; i < height; i++)
    m_ImageVerticalIter[i] = i;
}

Renderer::~Renderer() {}

Renderer::Renderer() {}

void Renderer::Render(const Scene &scene, const Camera &camera) {
  m_ActiveScene = &scene;
  m_ActiveCamera = &camera;

  m_ViewportWidth = m_FinalImage->GetWidth();
  m_ViewportHeight = m_FinalImage->GetHeight();

  if (m_FrameIndex == 1)
    memset(m_AccumulationData, 0,
           m_ViewportWidth * m_ViewportHeight * sizeof(glm::vec4));

#define MT 1
#ifdef MT
  std::for_each(
      std::execution::par, m_ImageVerticalIter.begin(),
      m_ImageVerticalIter.end(), [this](uint32_t y) {
        std::for_each(
            std::execution::par, m_ImageHorizontalIter.begin(),
            m_ImageHorizontalIter.end(), [this, y](uint32_t x) {
              uint32_t imageIndex = x + y * m_ViewportWidth;
              glm::vec4 color = PerPixel(x, y);
              m_AccumulationData[imageIndex] += (color);

              glm::vec4 accumulatedColor = m_AccumulationData[imageIndex];
              accumulatedColor /= (float)m_FrameIndex;
              accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f),
                                            glm::vec4(1.0f));

              m_ImageData[imageIndex] = Utils::ConvertToRGBA(accumulatedColor);
            });
      });

#else

  for (uint32_t y = 0; y < m_ViewportHeight; y++) {
    for (uint32_t x = 0; x < m_ViewportWidth; x++) {

      // PerPixel color from ray tracing

      glm::vec4 color = PerPixel(x, y);

      uint32_t imageIndex = x + y * m_ViewportWidth;
      m_AccumulationData[imageIndex] += color;

      glm::vec4 accumulatedColor = m_AccumulationData[imageIndex];
      accumulatedColor /= (float)m_FrameIndex;

      accumulatedColor =
          glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));

      m_ImageData[imageIndex] = Utils::ConvertToRGBA(color);
    }
  }
#endif

  m_FinalImage->SetData(m_ImageData);

  if (m_Settings.Accumulate)
    m_FrameIndex++;
  else
    m_FrameIndex = 1;
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y) {
  Ray ray;
  ray.Origin = m_ActiveCamera->GetPosition();
  ray.Direction =
      m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

  glm::vec3 light(0.0f);

  int bounces = 5;
  for (int i = 0; i < bounces; i++) {
    Renderer::HitPayload payload = TraceRay(ray);
    if (payload.HitDistance < 0.0f) {
      break;
    }

    const Sphere &sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
    const Material &material = m_ActiveScene->Materials[sphere.MaterialIndex];

    light += material.GetImage()->GetAlbedo(payload.u, payload.v) +
             material.GetEmission(); // get Emisson from material

    ray.Origin = payload.WorldPosition + payload.WorldNormal * 1e-4f;
    ray.Direction =
        glm::normalize(payload.WorldNormal + Walnut::Random::InUnitSphere());
  }

  return glm::vec4(light, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray &ray) {
  // (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
  // where
  // a = ray origin
  // b = ray direction
  // r = radius
  // t = hit distance

  int closestSphere = -1;
  float hitDistance = std::numeric_limits<float>::max();
  for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++) {
    const Sphere &sphere = m_ActiveScene->Spheres[i];
    glm::vec3 origin = ray.Origin - sphere.Center;

    float a = glm::dot(ray.Direction, ray.Direction);
    float b = 2.0f * glm::dot(origin, ray.Direction);
    float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

    // Quadratic forumula discriminant:
    // b^2 - 4ac

    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f)
      continue;

    // Quadratic formula:
    // (-b +- sqrt(discriminant)) / 2a

    // float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a); // Second hit
    // distance (currently unused)
    float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
    if (closestT > 0.0f && closestT < hitDistance) {
      hitDistance = closestT;
      closestSphere = (int)i;
    }
  }

  if (closestSphere < 0)
    return Miss(ray);

  return ClosestHit(ray, hitDistance, closestSphere);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray &ray, float hitDistance,
                                          int objectIndex) {
  Renderer::HitPayload payload;
  payload.HitDistance = hitDistance;
  payload.ObjectIndex = objectIndex;

  const Sphere &closestSphere = m_ActiveScene->Spheres[objectIndex];

  glm::vec3 origin =
      ray.Origin -
      closestSphere.Center; // new origin is bounce point position vector
  payload.WorldPosition =
      origin + ray.Direction * hitDistance; // bounce point normal vector
  payload.WorldNormal =
      glm::normalize(payload.WorldPosition); // bounce point unit normal vector
  payload.WorldPosition += closestSphere.Center;
  if (!payload.GetUVOnSphere(
          payload.WorldPosition)) // Get UV of bounce point on sphere
    std::perror("Failed to get UV coordinates");

  return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray &ray) {
  Renderer::HitPayload payload;
  payload.HitDistance = -1.0f;
  return payload;
}
