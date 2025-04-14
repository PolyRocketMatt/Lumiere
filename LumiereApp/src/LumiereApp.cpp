#include "app/Application.h"
#include "app/EntryPoint.h"

class BaseLayer : public Lumiere::Layer {
public:
	virtual void OnUIRender() override {
		ImGui::Begin("Inspector");
		ImGui::End();

		ImGui::Begin("Viewport");
		ImGui::End();
	}
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