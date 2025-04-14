#include "Core/Application.h"
#include "Core/EntryPoint.h"

class BaseLayer : public Lumiere::Layer {
public:
	virtual void OnUIRender() override {
		ImGui::Begin("Hello");
		ImGui::Button("Button");
		ImGui::End();

		ImGui::ShowDemoWindow();
	}
};

Lumiere::Application* Lumiere::CreateApplication(int argc, char** argv) {
	Lumiere::ApplicationSpec spec;
	spec.name = "Lumiere Engine";

	Lumiere::Application* app = new Lumiere::Application(spec);
	app->PushLayer<BaseLayer>();
	app->SetMenubarCallback([app]() {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Exit")) {
				app->Close();
			}
			ImGui::EndMenu();
		}
		});
	return app;
}