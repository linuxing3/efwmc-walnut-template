#ifndef RTIAW_renderer
#define RTIAW_renderer

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "Renderer/Camera.h"
#include "Renderer/HittableObjectList.h"
#include "Renderer/ThreadPool.h"
#include "Renderer/Utils.h"
#include "Walnut/Timer.h"

#include <random>

namespace RTIAW::Render {

class Renderer {
public:
  point3 lookfrom{10.0f, 10.0f, 10.0f};
  point3 lookat{0.0f, 0.0f, 0.0f};
  point3 material_color{0.8f, 0.2f, 0.1f};
  float aperture = 0.1f;
  float scale = 2.0f;
  enum class RenderState { Ready, Running, Finished, Stopped };
  enum class Scenes {
    DefaultScene,
    ThreeSpheres,
    TestScene,
    OneSphereScene,
    RectangleScene,
    Cube
  };

  std::tuple<uint8_t *, int, int> m_TextureData;

  Walnut::Timer *timer;
  // define a mvp struct holds all the mvp matrices
  struct MVP {
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
  };

  MVP mvp;

  Renderer() : m_logger{spdlog::stdout_color_st("Renderer")} {}
  Renderer(const Renderer &) = delete;
  ~Renderer();

  HittableObjectList getScene() { return m_scene; };
  void SetImageSize(unsigned int x, unsigned int y);
  void SetScene(Scenes scene = Scenes::DefaultScene) { m_sceneType = scene; };

  void SetSamplesPerPixel(unsigned int nSamples) { samplesPerPixel = nSamples; }
  void SetMaxRayBounces(unsigned int nBounces) { maxRayDepth = nBounces; }

  void StartRender();
  void StopRender();
  void OnUpdate(float ts){
      // m_camera->OnUpdate(ts);
  };

  [[nodiscard]] Scenes Scene() const { return m_sceneType; }
  [[nodiscard]] RenderState State() const { return m_state; }
  [[nodiscard]] const void *ImageBuffer() const {
    return m_renderBuffer.empty() ? nullptr : m_renderBuffer.data();
  }

  unsigned int samplesPerPixel = 10;
  unsigned int maxRayDepth = 10;
  unsigned int lastRenderTimeMS = 0;
  float lastRenderTime = 0.0f;

private:
  std::shared_ptr<spdlog::logger> m_logger;

  glm::uvec2 m_imageSize{0, 0};

  Scenes m_sceneType{Scenes::DefaultScene};
  HittableObjectList m_scene;
  void LoadScene();

  RenderState m_state = RenderState::Ready;

  // render buffer
  std::vector<uint8_t> m_renderBuffer{};

  // main rendering thread
  std::thread m_renderingThread;
  Utils::Pool m_threadPool{};

  // our camera :)
  std::unique_ptr<Camera> m_camera;

  struct Quad {
    Quad(glm::uvec2 min, glm::uvec2 max) : minCoo{min}, maxCoo{max} {};
    glm::uvec2 minCoo;
    glm::uvec2 maxCoo;
  };
  std::vector<Quad> SplitImage(unsigned int quadSize = 100) const;
  // actual internal implementation
  void Render();
  color ShootRay(const Ray &ray, unsigned int depth);
  void WritePixelToBuffer(unsigned int ix, unsigned int iy,
                          unsigned int samples_per_pixel, color pixel_color);

  // rng stuff
  std::mt19937 m_rnGenerator{};
  std::uniform_real_distribution<float> m_unifDistribution{0.0f, 1.0f};

  [[nodiscard]] float AspectRatio() const {
    return static_cast<float>(m_imageSize.x) /
           static_cast<float>(m_imageSize.y);
  }
};
} // namespace RTIAW::Render

#endif
