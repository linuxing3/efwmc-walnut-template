#pragma once

#include "stb/stb_image.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

#include <vector>

namespace EFWMC {
struct Texture {
  glm::vec3 Albedo{1.0f};
  uint8_t *Pixels = nullptr;
  float Width;
  float Height;
  float Channels;
  float HOffset{0.0};

  Texture(){};
  ~Texture(){};

  Texture(const std::string path) { LoadImage(path); }

  std::tuple<uint8_t *, int, int> GetPixels() {
    std::tuple<uint8_t *, int, int> result(Pixels, Width, Height);
    return result;
  };

  void LoadImage(std::string path) {
    int width, height, channels;
    Pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
    Width = width;
    Height = height;
    Channels = channels;
  }

  glm::vec3 GetAlbedo(float u, float v) {
    float rot = u + HOffset;
    if (rot > 1.0)
      rot -= 1.0;
    float uu = rot * Width;
    float vv = (1.0 - v) * Height;
    uint32_t textureIdx = 4 * (uu + floor(vv) * Width);
    Albedo =
        glm::vec3(Pixels[textureIdx] / 255.0f, Pixels[textureIdx + 1] / 255.0f,
                  Pixels[textureIdx + 2] / 255.0f);
    return Albedo;
  }
};
} // namespace EFWMC

struct Material {
  EFWMC::Texture *Image;
  glm::vec3 Albedo{1.0f};
  float Roughness = 1.0f;
  float Metallic = 0.0f;
  glm::vec3 EmissionColor{0.0f};
  float EmissionPower = 0.0f;

  Material(EFWMC::Texture *_image) : Image(_image){};

  glm::vec3 GetEmission() const { return EmissionColor * EmissionPower; }
  EFWMC::Texture *GetImage() const { return Image; }
};

struct Sphere {
  glm::vec3 Center{0.0f};
  float Radius = 0.5f;

  int MaterialIndex = 0;
};

struct Scene {
  std::vector<Sphere> Spheres;
  std::vector<Material> Materials;
};
