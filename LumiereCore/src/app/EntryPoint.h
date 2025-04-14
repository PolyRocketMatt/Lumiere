#pragma once

extern Lumiere::Application* Lumiere::CreateApplication(int argc, char** argv);

namespace Lumiere {

int Main(int argc, char** argv) {
	Lumiere::Application* app = CreateApplication(argc, argv);
	app->Run();
	delete app;

	return 0;
}

}

int main(int argc, char** argv) {
	return Lumiere::Main(argc, argv);
}