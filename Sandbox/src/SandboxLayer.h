#pragma once

#include <Helios/Core/Layer.h>

#include <Helios/Scene/PerspectiveCamera.h>

#include <imgui.h>

#include "Helios/Renderer/Texture.h"
#include "Helios/Scene/CameraController.h"
#include "Helios/Scene/Entity.h"
#include "Helios/Scene/Scene.h"

class SandboxLayer : public Helios::Layer {
  public:
    SandboxLayer();
    ~SandboxLayer() override;

    void on_attach() override;
    void on_detach() override;

    void on_update(float ts) override;
    void on_event(Helios::Event& e) override;
    void on_imgui_render() override;

    void InstancingTest();

  private:
    void RenderAPITest();

  private:
    Helios::Scene m_scene;

    Helios::SharedPtr<Helios::Texture> m_texture;
    Helios::SharedPtr<Helios::Texture> m_texture_2;

    Helios::SharedPtr<Helios::Mesh> m_viking_mesh;
    Helios::SharedPtr<Helios::Texture> m_viking_texture;

    Helios::Transform m_mesh_transform;

    Helios::CameraController m_camera_controller;

    Helios::Entity m_camera;
    Helios::Entity m_viking_room;

    FT_Library m_ft_library;
    FT_Face m_face_arial;
    Helios::SharedPtr<Helios::Texture> m_text_texture;

  int m_frame_count = 0;
  float m_fps_time_count = 0;
  int m_current_fps = 0;
};
