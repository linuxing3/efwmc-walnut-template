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

const std::string MODEL_PATH = RESOURCE_DIR "/fourareen.obj";
const std::string TEXTURE_PATH = RESOURCE_DIR "/fourareen2k_albedo.png";
const std::string EARTHMAP_PATH = RESOURCE_DIR "/earthmap.jpeg";

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

static std::tuple<uint8_t *, int, int> LoadImage(std::string path) {

  int width, height, channels;
  uint8_t *data = nullptr;
  data = stbi_load(path.c_str(), &width, &height, &channels, 4);
  std::tuple<uint8_t *, int, int> result(data, width, height);
  return result;
}

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

Renderer::~Renderer() {
  // TODO: texture sample
}

Renderer::Renderer() {
  // TODO: texture sample
  m_TextureData = LoadImage(EARTHMAP_PATH);

  // Load model
  bool success = ResourceManager::loadGeometryFromObj(MODEL_PATH, m_VertexData);
  if (!success) {
    std::cerr << "Failed to load model" << std::endl;
  }
#ifdef DEBUG
  for (int i = 0; i < m_VertexData.size(); i++) {
    glm::vec3 textureColor = glm::vec3(m_VertexData[i].color[0] / 255.0f,
                                       m_VertexData[i].color[1] / 255.0f,
                                       m_VertexData[i].color[2] / 255.0f);
    glm::vec3 position = glm::normalize(m_VertexData[i].position) * 0.5f + 0.5f;
    printf("position: %f, %f, %f\n", position.x, position.y, position.z);
    printf("color: %f, %f, %f\n", textureColor.x, textureColor.y,
           textureColor.z);
  }
#endif
}

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
              m_AccumulationData[imageIndex] += color;

              glm::vec4 accumulatedColor = m_AccumulationData[imageIndex];
              accumulatedColor /= (float)m_FrameIndex;
              accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f),
                                            glm::vec4(1.0f));

              m_ImageData[imageIndex] = Utils::ConvertToRGBA(accumulatedColor);
            });
      });

#ifdef USE_MODEL
  // map m_VertexData
  for (int i = 0; i < m_VertexData.size(); i++) {
    glm::vec3 textureColor = glm::vec3(m_VertexData[i].color[0] / 255.0f,
                                       m_VertexData[i].color[2] / 255.0f,
                                       m_VertexData[i].color[1] / 255.0f);
    glm::vec3 position = glm::normalize(m_VertexData[i].position) * 0.5f + 0.5f;
    int x = position.x * m_ViewportWidth;
    int y = position.y * m_ViewportHeight;
    int imageIndex = x + y * m_ViewportWidth;
    m_ImageData[imageIndex] =
        Utils::ConvertToRGBA(glm::vec4(1.0f, 0.0, 1.0, 1.0f));
  }
#endif
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

  // map texture
  auto [texture, textureWidth, textureHeight] = m_TextureData;
  // texture sample
/* #define USE_UV */
#ifdef USE_UV
#else
  uint32_t textureIdx = x * (textureWidth / m_ViewportWidth) +
                        y * textureWidth * (textureHeight / m_ViewportHeight);
  glm::vec3 textureColor = glm::vec3(texture[4 * (textureIdx) + 0] / 255.0f,
                                     texture[4 * (textureIdx) + 1] / 255.0f,
                                     texture[4 * (textureIdx) + 2] / 255.0f);
#endif

  glm::vec3 light(0.0f);
  /* light += textureColor; // earth map as background */

  int bounces = 5;
  for (int i = 0; i < bounces; i++) {
    Renderer::HitPayload payload = TraceRay(ray);
    if (payload.HitDistance < 0.0f) {
      break;
    }

#ifdef USE_UV
    auto [u, v] = payload.GetUV();
    uint32_t textureIdx =
        u * (textureWidth) + v * textureWidth * (textureHeight);
    glm::vec3 textureColor = glm::vec3(texture[4 * (textureIdx) + 0] / 255.0f,
                                       texture[4 * (textureIdx) + 1] / 255.0f,
                                       texture[4 * (textureIdx) + 2] / 255.0f);
#endif

    const Sphere &sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
    const Material &material = m_ActiveScene->Materials[sphere.MaterialIndex];

    light +=
        textureColor + material.GetEmission(); // earth map as mipmap of sphere

    ray.Origin = payload.WorldPosition + payload.WorldNormal * 1e-4f;
    // ray.Direction = glm::reflect(ray.Direction,
    //	payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f,
    // 0.5f));
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
    glm::vec3 origin = ray.Origin - sphere.Position;

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

  glm::vec3 origin = ray.Origin - closestSphere.Position;
  payload.WorldPosition = origin + ray.Direction * hitDistance;
  payload.WorldNormal = glm::normalize(payload.WorldPosition);

  payload.WorldPosition += closestSphere.Position;

  return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray &ray) {
  Renderer::HitPayload payload;
  payload.HitDistance = -1.0f;
  return payload;
}
