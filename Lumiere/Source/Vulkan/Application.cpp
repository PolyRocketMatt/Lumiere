#include "Application.h"

#include <iostream>

namespace Lumiere {
	
Application::Application() {
	GLFWwindow* window = m_Window.initWindow();
	
	m_VulkanProxy.initialize();
}

Application::~Application() {
	m_Window.cleanup();
}

void Application::run() {
	m_Window.update();
}

}