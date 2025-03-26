#pragma once
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"
#include "vulkan/vulkan.h"

#include "Vulkan/VulkanUtils.h"

namespace Lumiere {

class VulkanProxy {

public:
	void initialize();

private:
	VkInstance m_Instance;

	void createVulkanInstance() {
		spdlog::info("Creating Vulkan instance...");

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Lumiere";
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_4;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		VkInstanceCreateInfo instanceInfo{};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledExtensionCount = glfwExtensionCount;
		instanceInfo.ppEnabledExtensionNames = glfwExtensions;
		instanceInfo.enabledLayerCount = 0;

		VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_Instance);
		checkTrueOrExcept(result, "Failed to create Vulkan instance!");
		spdlog::info("    Successfully created Vulkan instance!");
	}
};

}