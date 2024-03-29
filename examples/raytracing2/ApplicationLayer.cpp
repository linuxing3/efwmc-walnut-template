#include <functional>
#include <stdexcept>

// #include <tchar.h>

#include <imgui_internal.h>
#include <magic_enum.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <Walnut/Image.h>
#include <glm/gtc/type_ptr.hpp>
#include <stdint.h>

#include "ApplicationLayer.h"
#include "Renderer/Shapes/Rectangle.h"
#include "imgui.h"

using namespace RTIAW::Render::Shapes;
namespace RTIAW {
ApplicationLayer::ApplicationLayer()
    : m_logger{spdlog::stdout_color_st("ApplicationLayer")} {
  spdlog::set_level(spdlog::level::debug);

  m_renderer.SetScene(RTIAW::Render::Renderer::Scenes::OneSphereScene);
}

void ApplicationLayer::OnUpdate(float ts) { m_renderer.OnUpdate(ts); }

void ApplicationLayer::OnUIRender() {
  //   #define R 1

  // my stuff here?
  // ImGuiViewport *viewport = ImGui::GetMainViewport();
  // ImGui::SetNextWindowPos(viewport->Pos);
  // ImGui::SetNextWindowSize(viewport->Size);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

  // BUG:
  //   ImGui::Begin("Rendered Image", nullptr, ImGuiWindowFlags_NoInputs);
  ImGui::Begin("Rendered Image");

  uint32_t imx = 400;
  uint32_t imy = 400;

  auto sizeAvailable = ImGui::GetContentRegionAvail();
  if (sizeAvailable.x != 0 && sizeAvailable.y != 0) {
    imx = sizeAvailable.x;
    imy = sizeAvailable.y;
  }

  if (!m_image) {
    m_image = std::make_unique<Walnut::Image>(
        imx, imy, Walnut::ImageFormat::RGBA, nullptr);
  } else if (imx != m_image->GetWidth() || imy != m_image->GetHeight()) {
    m_image->Resize(imx, imy);
  }

  const void *imagebuffer = m_renderer.ImageBuffer();
  if (imagebuffer) {
    m_image->SetData(imagebuffer);
    ImGui::Image(m_image->GetDescriptorSet(),
                 {(float)m_image->GetWidth(), (float)m_image->GetHeight()},
                 ImVec2(0, 1), ImVec2(1, 0));
  }
#ifdef R
#endif // R
  ImGui::End();
  ImGui::PopStyleVar(2);

  // --------------------------------------------------------------
  ImGui::Begin("Controls");
  // TODO: change this if/when we'll support multiple scenes
  // ImGui::Text(fmt::format("Scene: {}",
  // magic_enum::enum_name(m_renderer.Scene())).data()); static
  // std::underlying_type_t<Render::Renderer::Scenes> selected_scene_index = 0;
  if (ImGui::BeginCombo("Scene",
                        magic_enum::enum_name(m_selectedScene).data())) {
    for (auto scene : magic_enum::enum_values<Render::Renderer::Scenes>()) {
      const bool is_selected = (m_selectedScene == scene);
      if (ImGui::Selectable(magic_enum::enum_name(scene).data(), is_selected))
        m_selectedScene = scene;

      // Set the initial focus when opening the combo (scrolling + keyboard
      // navigation focus)
      if (is_selected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  ImGui::Text(magic_enum::enum_name(m_renderer.State()).data());
  const bool startDisable =
      m_renderer.State() == Render::Renderer::RenderState::Running;
  if (startDisable) {
    ImGui::BeginDisabled();
  } else {
    // TODO: find out how to disable window resizing
    // EnableWindowResize();
  }
  if (ImGui::Button("Start!")) {
    // (re-)initialize buffer
    m_renderer.SetScene(m_selectedScene);
    m_renderer.SetImageSize(imx, imy);
    m_renderer.StartRender();
  }
  if (startDisable) {
    ImGui::EndDisabled();
    if (ImGui::Button("Abort")) {
      m_renderer.StopRender();
    }
  }

  // TODO: mvp matrix to update vertex coordinates
  ImGui::DragFloat3("translate x", value_ptr(m_renderer.mvp.model[0]), 0.1f,
                    -1.0f, 1.0f);
  ImGui::DragFloat3("translate y", value_ptr(m_renderer.mvp.model[1]), 0.1f,
                    -1.0f, 1.0f);
  ImGui::DragFloat3("translate z", value_ptr(m_renderer.mvp.model[2]), 0.1f,
                    -1.0f, 1.0f);

  ImGui::DragFloat3("view x", value_ptr(m_renderer.mvp.view[0]), 0.1f, -1.0f,
                    1.0f);
  ImGui::DragFloat3("view y", value_ptr(m_renderer.mvp.view[1]), 0.1f, -1.0f,
                    1.0f);
  ImGui::DragFloat3("view z", value_ptr(m_renderer.mvp.view[2]), 0.1f, -1.0f,
                    1.0f);

  // TODO: mvp matrix to update vertex coordinates

  // camera position
  ImGui::DragFloat3("Camera Position", glm::value_ptr(m_renderer.lookfrom),
                    0.1f, -30.f, 30.0f);
  ImGui::DragFloat3("Camera LookAt", glm::value_ptr(m_renderer.lookat), 0.0f,
                    -100.0f, 100.0f);

  ImGui::DragFloat3("Base Color", glm::value_ptr(m_renderer.material_color),
                    0.1f, 0.0f, 1.0f);

  ImGui::DragFloat("Scale", &m_renderer.scale, 0.1f, 0.0f, 100.0f);
  ImGui::End();

  // --------------------------------------------------------------

  ImGui::Begin("Render Settings");
  ImGui::DragInt("Samples", (int *)&m_renderer.samplesPerPixel, 1, 1, 10);
  ImGui::DragInt("Bounces", (int *)&m_renderer.maxRayDepth, 1, 1, 10);
  //   ImGui::Text("Last render time: %d ms", m_renderer.lastRenderTimeMS);
  ImGui::Text("Last render: %.3fms", m_renderer.lastRenderTime);
  ImGui::Separator();

  auto objects = m_renderer.getScene().GetObjects();
  for (uint32_t i = 0; i < objects.size(); i++) {
    ImGui::PushID(i);
    /* Sphere sphere = std::get<Sphere>(objects[i].GetShape()); */
    /* ImGui::DragFloat3("Position", glm::value_ptr(sphere.m_center), 0.1f); */
    /* ImGui::DragFloat("Radius", &sphere.m_radius, 0.1f); */
    /* Rectangle rect = std::get<Rectangle>(objects[i].GetShape()); */
    /* ImGui::DragFloat3("Position", glm::value_ptr(rect.center), 0.1f); */
    ImGui::PopID();
  }

  ImGui::End();
  // --------------------------------------------------------------
}
} // namespace RTIAW
