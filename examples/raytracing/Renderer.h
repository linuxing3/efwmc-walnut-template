#pragma once

#include "ResourceManager.h"
#include "Walnut/Image.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>

using VertexAttributes = ResourceManager::VertexAttributes;

class Renderer {
public:
  struct Settings {
    bool Accumulate = true;
  };

public:
  Renderer();
  ~Renderer();

  void OnResize(uint32_t width, uint32_t height);
  void Render(const Scene &scene, const Camera &camera);

  std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

  void ResetFrameIndex() { m_FrameIndex = 1; }
  Settings &GetSettings() { return m_Settings; }

private:
  struct HitPayload {
    float HitDistance;
    glm::vec3 WorldPosition;
    glm::vec3 WorldNormal;
    float u;
    float v;

    int ObjectIndex;

    bool GetUVOnSphere(glm::vec3 hitPointWorldPosition) {
      glm::vec3 n = glm::normalize(hitPointWorldPosition);
      float x = n.x;
      float y = n.y;
      float z = n.z;
      u = atan2(x, z) / (2.0 * 3.1415926) + 0.5;
      v = y * 0.5 + 0.5;

      if (u <= 1.0 && u >= 0.0 && v <= 1.0 && v >= 0.0) {
        return true;
      }
      return false;
    }
  };

  glm::vec4 PerPixel(uint32_t x, uint32_t y); // RayGen

  HitPayload TraceRay(const Ray &ray);
  HitPayload ClosestHit(const Ray &ray, float hitDistance, int objectIndex);
  HitPayload Miss(const Ray &ray);

private:
  std::shared_ptr<Walnut::Image> m_FinalImage;
  Settings m_Settings;

  // texture stuff
  std::tuple<uint8_t *, int, int> m_TextureData;
  std::vector<VertexAttributes> m_VertexData;

  // Walnut stuff
  std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter;

  const Scene *m_ActiveScene = nullptr;
  const Camera *m_ActiveCamera = nullptr;

  uint32_t *m_ImageData = nullptr;
  glm::vec4 *m_AccumulationData = nullptr;

  uint32_t m_FrameIndex = 1;
  uint32_t m_ViewportWidth, m_ViewportHeight = 0;
};
