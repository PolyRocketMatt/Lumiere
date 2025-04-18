#include "app/Application.h"
#include "app/EntryPoint.h"
#include "geometry/Primitives.h"
#include "scene/Camera.h"
#include "scene/Scene.h"
#include "rendering/Renderer.h"

#include "Timer.h"

#include <memory>
#include <optional>
#include <vector>

class BaseLayer : public Lumiere::Layer {
public:
	
	virtual void OnAttach() override {
		//	Initialize camera
		Lumiere::OrbitCameraMetadata md = {};

		md.position = glm::vec3(0.0f, 0.0f, 3.0f);
		md.forwardDirection = glm::vec3(0.0f, 0.0f, -1.0f);
		md.up = glm::vec3(0.0f, 1.0f, 0.0f);

		md.verticalFov = 45.0f;
		md.nearClip = 0.1f;
		md.farClip = 100.0f;
		
		m_Camera = Lumiere::OrbitCamera(md);

		//	Initialize (test) scene (for now)
		std::vector<std::shared_ptr<Lumiere::Primitive>> primitives;
		Lumiere::Sphere sphereA{};
		sphereA.color = glm::vec3(0.0f, 0.75f, 1.0f);
		sphereA.radius = 0.5f;
		primitives.push_back(std::make_shared<Lumiere::Sphere>(sphereA));

		Lumiere::Sphere sphereB{};
		sphereB.origin = glm::vec3(2.0f, 0.0f, -3.0f);
		sphereB.radius = 1.5f;
		sphereB.color = glm::vec3(1.0f, 0.0f, 1.0f);
		primitives.push_back(std::make_shared<Lumiere::Sphere>(sphereB));

		m_Scene = Lumiere::PrimitiveScene(primitives);
	}

	virtual void OnUpdate(float ts) override {
		if (m_Camera.has_value() && m_AllowCameraUpdate) {
			m_Camera.value().OnUpdate(ts);
		}
	}

	virtual void OnUIRender() override {
		ImGui::Begin("Inspector");
		
		if (ImGui::BeginTabBar("InspectorTabs", ImGuiTabBarFlags_None)) {
			if (ImGui::BeginTabItem("Inspector")) {
				DrawHeader("Inspector");

				ImGui::Text("Render Time: %.3fms", m_LastRenderTime);
				ImGui::Text("FPS: %.3f", 1000.0f / m_LastRenderTime);
				if (ImGui::Checkbox("Real-Time", &m_RealTime)) {
					
				}

				ImGui::Spacing();

				if (ImGui::Button("Render", ImVec2(-1, 0)) && !m_RealTime) {
					m_AllowCameraUpdate = false;
					OnRender();
					m_AllowCameraUpdate = true;
				}
					

				if (ImGui::Button("Reset Camera", ImVec2(-1, 0)) && m_Camera.has_value())
					m_Camera.value().SetPosition(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f));

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Render Settings")) {
				DrawHeader("Render Settings");

				ImGui::SliderInt("Samples Per Pixel", &m_SamplesPerPixel, 1, 512);
				ImGui::SliderInt("Max Bounces", &m_MaxBounces, 1, 64);

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
		ImGui::End();

		ImGui::Begin("Viewport");
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();
		if (image)
			ImGui::Image((ImTextureID) (uint64_t) image->GetDescriptorSet(), ImVec2{ (float)image->GetWidth(), (float)image->GetHeight() },
				ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();

		if (m_RealTime)
			OnRender();
	}

private:

	void DrawHeader(const char* label) {
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		
		ImGui::TextWrapped("%s", label);
		ImGui::Spacing();
	}

	void OnRender() {
		Lumiere::RenderMetadata metaData = {};
		metaData.samplesPerPixel = m_SamplesPerPixel;
		metaData.maxBounces = m_MaxBounces;

		if (m_Camera.has_value()) {
			Lumiere::Timer timer;
			m_Camera.value().OnResize(m_ViewportWidth, m_ViewportHeight);
			m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
			m_Renderer.Render(metaData, m_Camera.value(), m_Scene, m_Integrator);
			m_Renderer.PostProcess();
			m_LastRenderTime = timer.ElapsedMillis();
		}
	}

protected:
	std::optional<Lumiere::OrbitCamera>			m_Camera;
	Lumiere::PrimitiveScene					m_Scene;
	Lumiere::DebugIntegrator				m_Integrator;
	Lumiere::Renderer					m_Renderer;

	//	Inspector => Inspector
	float							m_LastRenderTime = 0.0f;
	bool							m_RealTime = true;
	bool							m_AllowCameraUpdate = true;

	//	Inspector => Render Settings
	int							m_SamplesPerPixel = 1;
	int							m_MaxBounces = 1;

	//	Viewport
	uint32_t						m_ViewportWidth;
	uint32_t						m_ViewportHeight;
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