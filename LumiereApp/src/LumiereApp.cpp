#include "app/Application.h"
#include "app/EntryPoint.h"
//#include "scene/Scene.h"
#include "rendering/Renderer.h"

class BaseLayer : public Lumiere::Layer {
public:
	virtual void OnUIRender() override {
		ImGui::Begin("Inspector");
		if (ImGui::Button("Render"))
			OnRender();
		ImGui::End();

		ImGui::Begin("Viewport");
		ImGui::End();
	}

private:

	void OnRender() {
		m_Renderer.Render(m_Scene);
	}

private:
	Lumiere::Scene				m_Scene;
	Lumiere::Renderer			m_Renderer;
	
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