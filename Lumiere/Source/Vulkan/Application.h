#pragma once
#include "VulkanProxy.h"
#include "Window.h"

namespace Lumiere {

class Application {

public:
	Application();
	~Application();

	void run();

private:
	Window m_Window;
	VulkanProxy m_VulkanProxy;
};

}