#pragma once
#include "Application.h"
#include "Layer.h"
#include "Walnut/Timer.h"

#include "Camera.h"
#include "Renderer.h"

#include "stb/stb_image.h"
#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

const std::string MODEL_PATH = RESOURCE_DIR "/fourareen.obj";
const std::string TEXTURE_PATH = RESOURCE_DIR "/fourareen2k_albedo.png";
const std::string EARTHMAP_PATH = RESOURCE_DIR "/earthmap.jpeg";
const std::string MOON_PATH = RESOURCE_DIR "/moon.jpeg";

class RayTracerLayer : public Walnut::Layer {

public:
  RayTracerLayer() : m_Camera(45.0f, 0.1f, 100.0f) {

    auto *earthTexture = new EFWMC::Texture(EARTHMAP_PATH);
    auto *moonTexture = new EFWMC::Texture(MOON_PATH);

    // material 0
    Material &pinkSphere = m_Scene.Materials.emplace_back(earthTexture);
    pinkSphere.Roughness = 0.0f;

    // material 1
    Material &blueSphere = m_Scene.Materials.emplace_back(moonTexture);
    blueSphere.Roughness = 0.1f;

    // material 2
    Material &orangeSphere = m_Scene.Materials.emplace_back(moonTexture);
    orangeSphere.Roughness = 0.1f;
    orangeSphere.EmissionColor = {1.0f, 0.5f, 0.0f};
    orangeSphere.EmissionPower = 0.1f;

    {
      Sphere sphere;
      sphere.Center = {0.0f, 0.0f, 0.0f};
      sphere.Radius = 1.5f;
      sphere.MaterialIndex = 0;
      m_Scene.Spheres.push_back(sphere);
    }

    {
      Sphere sphere;
      sphere.Center = {0.0f, -101.0f, 0.0f};
      sphere.Radius = 100.0f;
      sphere.MaterialIndex = 1;
      m_Scene.Spheres.push_back(sphere);
    }

    {
      Sphere sphere;
      sphere.Center = {2.0f, 0.0f, 0.0f};
      sphere.Radius = 0.5f;
      sphere.MaterialIndex = 2;
      m_Scene.Spheres.push_back(sphere);
    }
    Application::Get()->QueueEvent([]() { Image::InitModel(945, 1028); });
  }

  virtual void OnUpdate(float ts) override {
    if (m_Camera.OnUpdate(ts))
      m_Renderer.ResetFrameIndex();
  }

  virtual void OnUIRender() override {
    ImGui::Begin("Settings");
    ImGui::Text("Last render: %.3fms", m_LastRenderTime);
    if (ImGui::Button("Render")) {
      Render();
    }

    ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);

    if (ImGui::Button("Reset"))
      m_Renderer.ResetFrameIndex();

    ImGui::End();

    ImGui::Begin("Scene");
    ImGui::DragFloat3("Camera Position", glm::value_ptr(m_Camera.m_Position),
                      0.1f);
    ImGui::DragFloat3("Camera LookAt",
                      glm::value_ptr(m_Camera.m_ForwardDirection), 0.1f);

    // spheres control
    for (size_t i = 0; i < m_Scene.Spheres.size(); i++) {
      ImGui::PushID(i);

      ImGui::Text("Sphere %zu", i);

      Sphere &sphere = m_Scene.Spheres[i];
      ImGui::DragFloat3("Position", glm::value_ptr(sphere.Center), 0.1f);
      ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
      ImGui::DragInt("Material", &sphere.MaterialIndex, 1.0f, 0,
                     (int)m_Scene.Materials.size() - 1);

      ImGui::Separator();

      ImGui::PopID();
    }

    // materials control
    for (size_t i = 0; i < m_Scene.Materials.size(); i++) {
      ImGui::PushID(i);

      ImGui::Text("Material %zu", i);

      Material &material = m_Scene.Materials[i];
      ImGui::DragFloat("Roughness", &material.Roughness, 0.05f, 0.0f, 1.0f);
      ImGui::DragFloat("Metallic", &material.Metallic, 0.05f, 0.0f, 1.0f);
      ImGui::ColorEdit3("Emission Color",
                        glm::value_ptr(material.EmissionColor));
      ImGui::DragFloat("Emission Power", &material.EmissionPower, 0.05f, 0.0f,
                       FLT_MAX);

      ImGui::Separator();

      ImGui::PopID();
    }

    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport");

    m_ViewportWidth = ImGui::GetContentRegionAvail().x;
    m_ViewportHeight = ImGui::GetContentRegionAvail().y;

    if (m_ViewportWidth > 0 && m_ViewportHeight > 0) {
      auto image = m_Renderer.GetFinalImage();
      if (image)
        ImGui::Image(image->GetDescriptorSet(),
                     {(float)image->GetWidth(), (float)image->GetHeight()},
                     ImVec2(0, 1), ImVec2(1, 0));
    }

    ImGui::End();
    ImGui::PopStyleVar();

    Render();
  }

  void Render() {

    Timer timer;

    if (m_ViewportWidth > 0 && m_ViewportHeight > 0) {
      m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
      m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
      m_Renderer.Render(m_Scene, m_Camera);
    }

    m_LastRenderTime = timer.ElapsedMillis();
  }

private:
  Renderer m_Renderer;
  Camera m_Camera;
  Scene m_Scene;
  uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

  float m_LastRenderTime = 0.0f;
};
