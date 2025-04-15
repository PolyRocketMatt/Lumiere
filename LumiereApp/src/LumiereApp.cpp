#include "app/Application.h"
#include "app/EntryPoint.h"
#include "scene/Camera.h"
#include "scene/Scene.h"
#include "rendering/Renderer.h"

#include "Timer.h"

#include <optional>

class BaseLayer : public Lumiere::Layer {
public:
	
	virtual void OnAttach() override {
		Lumiere::OrbitCameraMetadata md = {};

		md.position = glm::vec3(0.0f, 0.0f, 3.0f);
		md.forwardDirection = glm::vec3(0.0f, 0.0f, -1.0f);
		md.up = glm::vec3(0.0f, 1.0f, 0.0f);

		md.verticalFov = 45.0f;
		md.nearClip = 0.1f;
		md.farClip = 100.0f;
		
		m_Camera = Lumiere::OrbitCamera(md);
	}

	virtual void OnUpdate(float ts) override {
		if (m_Camera.has_value() && m_Camera.value().OnUpdate(ts)) {
			
		}
	}

	virtual void OnUIRender() override {
		ImGui::Begin("Inspector");
		ImGui::Text("Render Time: %.3fms", m_LastRenderTime);
		ImGui::Text("FPS: %.3f", 1000.0f / m_LastRenderTime);
		

		if (ImGui::Button("Render", ImVec2(-1, 0)))
			OnRender();
		if (ImGui::Button("Reset Camera", ImVec2(-1, 0)) && m_Camera.has_value())
			m_Camera.value().SetPosition(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f));
				
		ImGui::End();

		ImGui::Begin("Viewport");
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();
		if (image)
			ImGui::Image((ImTextureID) (uint64_t) image->GetDescriptorSet(), ImVec2{ (float)image->GetWidth(), (float)image->GetHeight() },
				ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();

		OnRender();
	}

private:

	void OnRender() {
		if (m_Camera.has_value()) {
			Lumiere::Timer timer;
			m_Camera.value().OnResize(m_ViewportWidth, m_ViewportHeight);
			m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
			m_Renderer.Render(m_Camera.value(), m_Scene);
			m_LastRenderTime = timer.ElapsedMillis();
		}
	}

protected:
	std::optional<Lumiere::OrbitCamera>			m_Camera;
	Lumiere::Scene								m_Scene;
	Lumiere::Renderer							m_Renderer;
	
	uint32_t									m_ViewportWidth;
	uint32_t									m_ViewportHeight;
	float										m_LastRenderTime = 0.0f;
};

Lumiere::Application* Lumiere::CreateApplication(int argc, char** argv) {
	Lumiere::ApplicationSpec spec;
	spec.name = "Lumiere Engine";

	Lumiere::Application* app = new Lumiere::Application(spec);
	app->PushLayer<BaseLayer>();
	app->SetMenubarCallback([app]() {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Exit"))
				app->Close();
			ImGui::EndMenu();
		}
		});
	return app;
}