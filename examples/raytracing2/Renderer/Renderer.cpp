#include <cstdint>
#include <optional>

#include "Walnut/Timer.h"
#include "fmt/chrono.h"
#include <glm/gtc/random.hpp>

#include "Renderer/Ray.h"
#include "Renderer/Renderer.h"

// #include "Materials/Material.h"
#include "Renderer/HittableObjectList.h"

#include <numeric>

#define RENDER_PERQUAD

namespace RTIAW::Render {

static uint32_t ConvertToRGBA(const glm::vec4 &color) {
  auto r = (uint8_t)(color.r * 255.0f);
  auto g = (uint8_t)(color.g * 255.0f);
  auto b = (uint8_t)(color.b * 255.0f);
  auto a = (uint8_t)(color.a * 255.0f);

  uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
  return result;
}

static color ConvertColor(color pixel_color) {

  auto r = static_cast<uint8_t>(255.0f * pixel_color.r);
  auto g = static_cast<uint8_t>(255.0f * pixel_color.g);
  auto b = static_cast<uint8_t>(255.0f * pixel_color.b);
  color result{r, g, b};
  return result;
}

Renderer::~Renderer() {
  switch (m_state) {
  case RenderState::Finished:
  case RenderState::Stopped:
    m_renderingThread.join();
    break;
  default:
    break;
  }
}

void Renderer::SetImageSize(unsigned int x, unsigned int y) {
  m_imageSize = glm::uvec2{x, y};
  m_renderBuffer.clear();
  m_renderBuffer.resize(x * y * 4);
}

void Renderer::StartRender() {
  switch (m_state) {
  case RenderState::Finished:
  case RenderState::Stopped:
    m_renderingThread.join();
    break;
  default:
    break;
  }

  LoadScene();
  m_renderingThread = std::thread{&Renderer::Render, this};
}

void Renderer::StopRender() { m_state = RenderState::Stopped; }

std::vector<Renderer::Quad> Renderer::SplitImage(unsigned int quadSize) const {
  std::vector<Quad> result;
  const auto nX = static_cast<unsigned int>(
      std::ceil(m_imageSize.x / static_cast<float>(quadSize)));
  const auto nY = static_cast<unsigned int>(
      std::ceil(m_imageSize.y / static_cast<float>(quadSize)));
  for (unsigned int j = nY; j > 0; --j) {
    for (unsigned int i = 0; i < nX; ++i) {
      result.emplace_back(
          glm::uvec2{i * quadSize, (j - 1) * quadSize},
          glm::min(glm::uvec2{(i + 1) * quadSize, j * quadSize}, m_imageSize));
    }
  }

  return result;
}

void Renderer::Render() {
  m_logger->debug("Start rendering!!!");
  timer = new Walnut::Timer();
  m_state = RenderState::Running;

#ifdef RENDER_PERLINE
  auto renderLine = [this](const unsigned int lineCoord) {
    if (m_state == RenderState::Stopped) {
      return;
    }

    for (unsigned int i = 0; i < m_imageSize.x; ++i) {
      const auto pixelCoord = glm::uvec2{i, lineCoord};
      color pixel_color{0, 0, 0};
      for (unsigned int i_sample = 0; i_sample < samplesPerPixel; ++i_sample) {
        const auto u = (static_cast<float>(pixelCoord.x) +
                        m_unifDistribution(m_rnGenerator)) /
                       (m_imageSize.x - 1);
        const auto v = (static_cast<float>(pixelCoord.y) +
                        m_unifDistribution(m_rnGenerator)) /
                       (m_imageSize.y - 1);
        Ray r = m_camera->NewRay(u, v);
        pixel_color += ShootRay(r, maxRayDepth);
      }
      WritePixelToBuffer(pixelCoord.x, pixelCoord.y, samplesPerPixel,
                         pixel_color);
    }
  };
#endif

  auto renderQuad = [this](const glm::uvec2 minCoo, const glm::uvec2 maxCoo) {
    if (m_state == RenderState::Stopped) {
      return;
    }

    std::mt19937 generator{std::random_device{}()};

    for (unsigned int j = maxCoo.y; j > minCoo.y; --j) {
      for (unsigned int i = minCoo.x; i < maxCoo.x; ++i) {
        color pixel_color{0, 0, 0};
        const auto pixelCoord = glm::uvec2{i, j - 1};
        for (unsigned int i_sample = 0; i_sample < samplesPerPixel;
             ++i_sample) {
          const auto u = (static_cast<float>(pixelCoord.x) +
                          m_unifDistribution(generator)) /
                         (m_imageSize.x - 1);
          const auto v = (static_cast<float>(pixelCoord.y) +
                          m_unifDistribution(generator)) /
                         (m_imageSize.y - 1);
          Ray r = m_camera->NewRay(u, v);
          pixel_color += ShootRay(r, maxRayDepth);
        }
        WritePixelToBuffer(pixelCoord.x, pixelCoord.y, samplesPerPixel,
                           pixel_color);
      }
    }
  };

  std::vector<std::future<void>> futures;

#ifdef RENDER_PERLINE
  // Render per-line
  for (int j = m_imageSize.y - 1; j >= 0; --j) {
    futures.push_back(m_threadPool.AddTask(renderLine, j));
  }
#else
  // Render per-quad
  for (const auto &[minCoo, maxCoo] : SplitImage()) {
    futures.push_back(m_threadPool.AddTask(renderQuad, minCoo, maxCoo));
  }

#endif

  // wait until all tasks are done...
  std::for_each(begin(futures), end(futures),
                [](auto &future) { future.wait(); });

  m_state = RenderState::Finished;
}

color Renderer::ShootRay(const Ray &ray, unsigned int depth) {
  // If we've exceeded the ray bounce limit, no more light is gathered.
  if (depth == 0)
    return {0, 0, 0};

  if (const auto &[o_hitRecord, o_scatterResult] =
          m_scene.Hit(ray, 0.001f, RTIAW::Utils::infinity);
      o_hitRecord) {
    const auto &[t, p, normal, front_face] = o_hitRecord.value();

    if (o_scatterResult) {
      const auto &[attenuation, scattered] = o_scatterResult.value();
      return attenuation * ShootRay(scattered, depth - 1);
    }

    return {0, 0, 0};
  }

  constexpr color white{1.0, 1.0, 1.0};
  constexpr color azure{0.5, 0.7, 1.0};

  float t = 0.5f * (ray.direction.y + 1.0f);
  return (1.0f - t) * white + t * azure;
}

void Renderer::WritePixelToBuffer(unsigned int ix, unsigned int iy,
                                  unsigned int samples_per_pixel,
                                  color pixel_color) {
  // flip the vertical coordinate because the display backend follow the
  // opposite convention iy = m_imageSize.y - 1 - iy;

  pixel_color /= samples_per_pixel;
  pixel_color = glm::sqrt(pixel_color);
  pixel_color = glm::clamp(pixel_color, 0.0f, 1.0f);

  const unsigned int idx = 4 * (ix + iy * m_imageSize.x);
  const auto new_color = ConvertColor(pixel_color);

  m_renderBuffer[idx] = new_color.r;
  m_renderBuffer[idx + 1] = new_color.g;
  m_renderBuffer[idx + 2] = new_color.b;
  m_renderBuffer[idx + 3] = 255;
};

} // namespace RTIAW::Render
