#pragma once

#include "stb/stb_image.h"
#include <cstdio>
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
#ifdef DEBUG_LOG
    printf("Width: %2f, Height: %2f\n", Width, Height);
    for (int i = 0; i < Width * Height; i++) {
      printf("[");
      printf("%hhu ", Pixels[4 * i]);
      printf("%hhu ", Pixels[4 * i + 1]);
      printf("%hhu ", Pixels[4 * i + 2]);
      printf("%hhu ", Pixels[4 * i + 3]);
      printf("]\n");
    }
#endif
    Channels = channels;
  }

  glm::vec3 GetAlbedo(float u, float v) {
    // uu is the horizontal axis
    float uu = u * Width;
    // vv is the vertical axis
    float vv = (1.0 - v) * Height;
    // textureIdx is the index of the pixel in the texture
    uint32_t textureIdx = 4 * (floor(uu) + floor(vv) * Width);
    // Albeto is the color of the pixel in the texture
    Albedo =
        glm::vec3(Pixels[textureIdx] / 255.0f, Pixels[textureIdx + 1] / 255.0f,
                  Pixels[textureIdx + 2] / 255.0f);
#ifdef DEBUG_LOG
    printf("[");
    printf("%f ", Albedo.x);
    printf("%f ", Albedo.y);
    printf("%f ", Albedo.z);
    printf("]\r");
#endif
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
