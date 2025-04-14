#include "Input.h"

#include "app/Application.h"

#include <GLFW/glfw3.h>

namespace Lumiere {

static glm::vec2 s_ScrollDelta = { 0.0f, 0.0f };

bool Input::IsKeyDown(KeyCode keycode) {
	GLFWwindow* windowHandle = Application::Get().GetWindowHandle();
	int state = glfwGetKey(windowHandle, (int)keycode);
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonDown(MouseButton button) {
	GLFWwindow* windowHandle = Application::Get().GetWindowHandle();
	int state = glfwGetMouseButton(windowHandle, (int)button);
	return state == GLFW_PRESS;
}

glm::vec2 Input::GetMousePosition() {
	GLFWwindow* windowHandle = Application::Get().GetWindowHandle();

	double x, y;
	glfwGetCursorPos(windowHandle, &x, &y);
	return { (float)x, (float)y };
}

glm::vec2 Input::GetMouseScrollDelta() {
	glm::vec2 delta = s_ScrollDelta;
	s_ScrollDelta = { 0.0f, 0.0f };		//	Reset after each access
	return delta;
}

bool Input::HasScrollDelta() {
	return s_ScrollDelta.y != 0.0f;
}

void Input::SetCursorMode(CursorMode mode) {
	GLFWwindow* windowHandle = Application::Get().GetWindowHandle();
	glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
}

void Input::SetScrollDelta(float xOffset, float yOffset) {
	s_ScrollDelta = { xOffset, yOffset };
}

}