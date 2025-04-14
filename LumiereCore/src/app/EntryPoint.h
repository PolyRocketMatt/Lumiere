#pragma once

extern Lumiere::Application* Lumiere::CreateApplication(int argc, char** argv);

int main(int argc, char** argv) {
	Lumiere::Application* app = Lumiere::CreateApplication(argc, argv);
	app->Run();
	delete app;

	return 0;
}