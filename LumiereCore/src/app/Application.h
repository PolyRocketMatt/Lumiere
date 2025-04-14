#pragma once

#include "Layer.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "imgui.h"
#include "vulkan/vulkan.h"

void check_vk_result(VkResult err);

struct GLFWwindow;

namespace Lumiere {

struct ApplicationSpec {
	std::string name = "Lumiere App";
	uint32_t width = 1600;
	uint32_t height = 900;
};

class Application {
public:
	Application(const ApplicationSpec& spec = ApplicationSpec());
	~Application();
	
	static Application& Get();

	void Run();
	void Close();

	void SetMenubarCallback(const std::function<void()>& callback) { m_MenubarCallback = callback; }

	template<typename T>
	void PushLayer() {
		static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not a subclass of Layer!");
		m_LayerStack.emplace_back(std::make_shared<T>())->OnAttach();
	}
	
	void PushLayer(const std::shared_ptr<Layer>& layer) { 
		m_LayerStack.emplace_back(layer); 
		layer->OnAttach();
	}

	float GetTime();
	
	GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }
	
	static VkInstance GetInstance();
	static VkPhysicalDevice GetPhysicalDevice();
	static VkDevice GetDevice();

	static VkCommandBuffer GetCommandBuffer(bool begin);
	static void FlushCommandBuffer(VkCommandBuffer buffer);
	
	static void SubmitResourceFree(std::function<void()>&& func);
private:
	void Init();
	void Shutdown();

private:
	ApplicationSpec		m_Spec;
	GLFWwindow*			m_WindowHandle = nullptr;
	bool				m_Running = false;

	float				m_TimeStep = 0.0f;
	float				m_FrameTime = 0.0f;
	float				m_LastFrameTime = 0.0f;

	std::vector<std::shared_ptr<Layer>> m_LayerStack;
	std::function<void()> m_MenubarCallback;
};

//	Implemented by Lumiere Application (CLIENT)
Application* CreateApplication(int argc, char** argv);

}