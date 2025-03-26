#include "Window.h"

#include <stdexcept>

namespace Lumiere {

GLFWwindow* Window::initWindow() {
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW!");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_Window = glfwCreateWindow(m_Width, m_Height, "Lumiere", nullptr, nullptr);
	return m_Window;
}

void Window::update() {
	while (!glfwWindowShouldClose(m_Window)) {
		glfwPollEvents();
	}
}

void Window::cleanup() {
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

}