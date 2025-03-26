#pragma once
#include "GLFW/glfw3.h"

namespace Lumiere {
	
class Window {
	
public:
	GLFWwindow* initWindow();
	
	void update();
	void cleanup();

	inline const GLFWwindow* getWindow() const { return m_Window; }
	inline const uint32_t getWidth() const { return m_Width; }
	inline const uint32_t getHeight() const { return m_Height; }
	
	inline const float getAspectRatio() const { return (float)m_Width / m_Height; }

private:
	GLFWwindow* m_Window;
	uint32_t m_Width = 1600;
	uint32_t m_Height = 900;
};

}