#include "Application.h"
#include "input/Input.h"

//
// Adapted from Dear ImGui Vulkan example
//

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <iostream>

// Emedded font
#include "imgui/Roboto-Regular.embed"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

//#define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

static VkAllocationCallbacks*	g_Allocator = NULL;
static VkInstance               g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice                 g_Device = VK_NULL_HANDLE;
static uint32_t                 g_QueueFamily = (uint32_t)-1;
static VkQueue                  g_Queue = VK_NULL_HANDLE;
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
static VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

static ImGui_ImplVulkanH_Window g_MainWindowData;
static int                      g_MinImageCount = 2;
static bool                     g_SwapChainRebuild = false;

// Per-frame-in-flight
static std::vector<std::vector<VkCommandBuffer>> s_AllocatedCommandBuffers;
static std::vector<std::vector<std::function<void()>>> s_ResourceFreeQueue;

// Unlike g_MainWindowData.FrameIndex, this is not the the swapchain image index
// and is always guaranteed to increase (eg. 0, 1, 2, 0, 1, 2)
static uint32_t s_CurrentFrameIndex = 0;

static Lumiere::Application* s_Instance = nullptr;

void check_vk_result(VkResult err) {
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

#ifdef IMGUI_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
	(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
	fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
	return VK_FALSE;
}
#endif // IMGUI_VULKAN_DEBUG_REPORT

static void SetupVulkan(const char** extensions, uint32_t extensions_count) {
	VkResult err;

	// Create Vulkan Instance
	{
		VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		createInfo.enabledExtensionCount = extensions_count;
		createInfo.ppEnabledExtensionNames = extensions;
#ifdef IMGUI_VULKAN_DEBUG_REPORT
		// Enabling validation layers
		const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames = layers;

		// Enable debug report extension (we need additional storage, so we duplicate the user array to add our new extension to it)
		const char** extensions_ext = (const char**)malloc(sizeof(const char*) * (extensions_count + 1));
		memcpy(extensions_ext, extensions, extensions_count * sizeof(const char*));
		extensions_ext[extensions_count] = "VK_EXT_debug_report";
		createInfo.enabledExtensionCount = extensions_count + 1;
		createInfo.ppEnabledExtensionNames = extensions_ext;

		// Create Vulkan Instance
		err = vkCreateInstance(&createInfo, g_Allocator, &g_Instance);
		check_vk_result(err);
		free(extensions_ext);

		// Get the function pointer (required for any extensions)
		auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkCreateDebugReportCallbackEXT");
		IM_ASSERT(vkCreateDebugReportCallbackEXT != NULL);

		// Setup the debug report callback
		VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
		debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		debug_report_ci.pfnCallback = debug_report;
		debug_report_ci.pUserData = NULL;
		err = vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci, g_Allocator, &g_DebugReport);
		check_vk_result(err);
#else
		// Create Vulkan Instance without any debug feature
		err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
		check_vk_result(err);
		IM_UNUSED(g_DebugReport);
#endif
	}

	// Select GPU
	{
		uint32_t gpuCount;
		err = vkEnumeratePhysicalDevices(g_Instance, &gpuCount, NULL);
		check_vk_result(err);
		IM_ASSERT(gpuCount > 0);

		VkPhysicalDevice* gpus = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpuCount);
		err = vkEnumeratePhysicalDevices(g_Instance, &gpuCount, gpus);
		check_vk_result(err);

		// If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
		// most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
		// dedicated GPUs) is out of scope of this sample.
		int useGpu = 0;
		for (int i = 0; i < (int)gpuCount; i++) {
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(gpus[i], &properties);
			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				useGpu = i;
				break;
			}
		}

		g_PhysicalDevice = gpus[useGpu];
		free(gpus);
	}

	// Select graphics queue family
	{
		uint32_t count;
		vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, NULL);
		VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
		vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
		for (uint32_t i = 0; i < count; i++)
			if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				g_QueueFamily = i;
				break;
			}
		free(queues);
		IM_ASSERT(g_QueueFamily != (uint32_t)-1);
	}

	// Create Logical Device (with 1 queue)
	{
		int deviceExtensionCount = 1;
		const char* deviceExtensions[] = { "VK_KHR_swapchain" };
		const float queuePriority[] = { 1.0f };
		VkDeviceQueueCreateInfo queueInfo[1] = { };
		queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo[0].queueFamilyIndex = g_QueueFamily;
		queueInfo[0].queueCount = 1;
		queueInfo[0].pQueuePriorities = queuePriority;
		
		VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		createInfo.queueCreateInfoCount = sizeof(queueInfo) / sizeof(queueInfo[0]);
		createInfo.pQueueCreateInfos = queueInfo;
		createInfo.enabledExtensionCount = deviceExtensionCount;
		createInfo.ppEnabledExtensionNames = deviceExtensions;
		
		err = vkCreateDevice(g_PhysicalDevice, &createInfo, g_Allocator, &g_Device);
		check_vk_result(err);
		vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
	}

	// Create Descriptor Pool
	{
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
		poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
		poolInfo.pPoolSizes = poolSizes;
		
		err = vkCreateDescriptorPool(g_Device, &poolInfo, g_Allocator, &g_DescriptorPool);
		check_vk_result(err);
	}
}

static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height) {
	wd->Surface = surface;

	// Check for WSI support
	VkBool32 res;
	vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily, wd->Surface, &res);
	if (res != VK_TRUE) {
		fprintf(stderr, "Error no WSI support on physical device 0\n");
		exit(-1);
	}

	// Select Surface Format
	const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

	// Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
	VkPresentModeKHR presentModes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
	VkPresentModeKHR presentModes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
	wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(g_PhysicalDevice, wd->Surface, &presentModes[0], IM_ARRAYSIZE(presentModes));
	//printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

	// Create SwapChain, RenderPass, Framebuffer, etc.
	IM_ASSERT(g_MinImageCount >= 2);
	ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
}

static void CleanupVulkan() {
	vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

#ifdef IMGUI_VULKAN_DEBUG_REPORT
	// Remove the debug report callback
	auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkDestroyDebugReportCallbackEXT");
	vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif // IMGUI_VULKAN_DEBUG_REPORT

	vkDestroyDevice(g_Device, g_Allocator);
	vkDestroyInstance(g_Instance, g_Allocator);
}

static void CleanupVulkanWindow() {
	ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData, g_Allocator);
}

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data) {
	VkResult err;

	VkSemaphore imageAcquiredSemaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
	VkSemaphore renderCompleteSemaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
	
	err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE, &wd->FrameIndex);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
		g_SwapChainRebuild = true;
		return;
	}
	check_vk_result(err);

	s_CurrentFrameIndex = (s_CurrentFrameIndex + 1) % g_MainWindowData.ImageCount;

	ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
	{
		err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
		check_vk_result(err);

		err = vkResetFences(g_Device, 1, &fd->Fence);
		check_vk_result(err);
	}

	{
		// Free resources in queue
		for (auto& func : s_ResourceFreeQueue[s_CurrentFrameIndex])
			func();
		s_ResourceFreeQueue[s_CurrentFrameIndex].clear();
	}
	{
		// Free command buffers allocated by Application::GetCommandBuffer
		// These use g_MainWindowData.FrameIndex and not s_CurrentFrameIndex because they're tied to the swapchain image index
		auto& allocatedCommandBuffers = s_AllocatedCommandBuffers[wd->FrameIndex];
		if (allocatedCommandBuffers.size() > 0) {
			vkFreeCommandBuffers(g_Device, fd->CommandPool, (uint32_t)allocatedCommandBuffers.size(), allocatedCommandBuffers.data());
			allocatedCommandBuffers.clear();
		}

		err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
		check_vk_result(err);
		
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
		err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
		check_vk_result(err);
	}
	{
		VkRenderPassBeginInfo info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		info.renderPass = wd->RenderPass;
		info.framebuffer = fd->Framebuffer;
		info.renderArea.extent.width = wd->Width;
		info.renderArea.extent.height = wd->Height;
		info.clearValueCount = 1;
		info.pClearValues = &wd->ClearValue;
		vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

	// Submit command buffer
	vkCmdEndRenderPass(fd->CommandBuffer);
	{
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &imageAcquiredSemaphore;
		info.pWaitDstStageMask = &waitStage;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &fd->CommandBuffer;
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &renderCompleteSemaphore;

		err = vkEndCommandBuffer(fd->CommandBuffer);
		check_vk_result(err);
		err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
		check_vk_result(err);
	}
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd) {
	if (g_SwapChainRebuild)
		return;
	
	VkSemaphore renderCompleteSemaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = &renderCompleteSemaphore;
	info.swapchainCount = 1;
	info.pSwapchains = &wd->Swapchain;
	info.pImageIndices = &wd->FrameIndex;
	
	VkResult err = vkQueuePresentKHR(g_Queue, &info);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
		g_SwapChainRebuild = true;
		return;
	}
	check_vk_result(err);
	wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
}

static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

namespace Lumiere {

Application::Application(const ApplicationSpec& spec)
	: m_Spec(spec) {
	s_Instance = this;
	
	Init();
}

Application::~Application() {
	Shutdown();
	
	s_Instance = nullptr;
}

Application& Application::Get() {
	return *s_Instance;
}

void Application::Init() {
	//	Setup GLFW Window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		std::cerr << "[glfw] Could not initialize GLFW!\n";
		return;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_WindowHandle = glfwCreateWindow(m_Spec.width, m_Spec.height, m_Spec.name.c_str(), NULL, NULL);
	
	//	Scroll callback
	glfwSetScrollCallback(m_WindowHandle, [](GLFWwindow* window, double xOffset, double yOffset) {
		Input::SetScrollDelta((float)xOffset, (float)yOffset);
	});

	//	Setup Vulkan
	if (!glfwVulkanSupported()) {
		std::cerr << "[glfw] Vulkan is not supported!\n";
		return;
	}

	uint32_t extensions_count = 0;
	const char** extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
	SetupVulkan(extensions, extensions_count);

	//	Create Window Surface
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(g_Instance, m_WindowHandle, g_Allocator, &surface);
	check_vk_result(err);

	//	Create Framebuffers
	int w, h;
	glfwGetFramebufferSize(m_WindowHandle, &w, &h);
	ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
	SetupVulkanWindow(wd, surface, w, h);

	s_AllocatedCommandBuffers.resize(wd->ImageCount);
	s_ResourceFreeQueue.resize(wd->ImageCount);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(m_WindowHandle, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	//init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
	init_info.Instance = g_Instance;
	init_info.PhysicalDevice = g_PhysicalDevice;
	init_info.Device = g_Device;
	init_info.QueueFamily = g_QueueFamily;
	init_info.Queue = g_Queue;
	init_info.PipelineCache = g_PipelineCache;
	init_info.DescriptorPool = g_DescriptorPool;
	init_info.RenderPass = wd->RenderPass;
	init_info.Subpass = 0;
	init_info.MinImageCount = g_MinImageCount;
	init_info.ImageCount = wd->ImageCount;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = g_Allocator;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info);

	// Load default font
	ImFontConfig fontConfig;
	fontConfig.FontDataOwnedByAtlas = false;
	ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
	io.FontDefault = robotoFont;

	// Upload Fonts
	{
		// Use any command queue
		VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
		VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

		err = vkResetCommandPool(g_Device, command_pool, 0);
		check_vk_result(err);
		VkCommandBufferBeginInfo begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(command_buffer, &begin_info);
		check_vk_result(err);

		ImGui_ImplVulkan_CreateFontsTexture();

		VkSubmitInfo end_info = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		end_info.commandBufferCount = 1;
		end_info.pCommandBuffers = &command_buffer;
		err = vkEndCommandBuffer(command_buffer);
		check_vk_result(err);
		err = vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);
		check_vk_result(err);

		err = vkDeviceWaitIdle(g_Device);
		check_vk_result(err);
		ImGui_ImplVulkan_DestroyFontsTexture();
	}
}

void Application::Shutdown() {
	for (auto& layer : m_LayerStack)
		layer->OnDetach();

	m_LayerStack.clear();

	// Cleanup
	VkResult err = vkDeviceWaitIdle(g_Device);
	check_vk_result(err);

	// Free resources in queue
	for (auto& queue : s_ResourceFreeQueue) {
		for (auto& func : queue)
			func();
	}
	s_ResourceFreeQueue.clear();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	CleanupVulkanWindow();
	CleanupVulkan();

	glfwDestroyWindow(m_WindowHandle);
	glfwTerminate();
}

void Application::Run() {
	m_Running = true;

	ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
	ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	ImGuiIO& io = ImGui::GetIO();

	// Main loop
	while (!glfwWindowShouldClose(m_WindowHandle) && m_Running) {
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		for (auto& layer : m_LayerStack)
			layer->OnUpdate(m_TimeStep);

		// Resize swap chain?
		if (g_SwapChainRebuild) {
			int width, height;
			glfwGetFramebufferSize(m_WindowHandle, &width, &height);
			if (width > 0 && height > 0) {
				ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
				ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
				g_MainWindowData.FrameIndex = 0;

				// Clear allocated command buffers from here since entire pool is destroyed
				s_AllocatedCommandBuffers.clear();
				s_AllocatedCommandBuffers.resize(g_MainWindowData.ImageCount);

				g_SwapChainRebuild = false;
			}
		}

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

			// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
			// because it would be confusing to have two docking targets within each others.
			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;
			if (m_MenubarCallback)
				windowFlags |= ImGuiWindowFlags_MenuBar;

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
			// and handle the pass-thru hole, so we ask Begin() to not render a background.
			if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
				windowFlags |= ImGuiWindowFlags_NoBackground;

			// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
			// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
			// all active windows docked into it will lose their parent and become undocked.
			// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
			// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("DockSpace", nullptr, windowFlags);
			ImGui::PopStyleVar();

			ImGui::PopStyleVar(2);

			// Submit the DockSpace
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
				ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);
			}

			if (m_MenubarCallback) {
				if (ImGui::BeginMenuBar()) {
					m_MenubarCallback();
					ImGui::EndMenuBar();
				}
			}

			for (auto& layer : m_LayerStack)
				layer->OnUIRender();

			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		ImDrawData* mainDrawData = ImGui::GetDrawData();
		const bool mainIsMinimized = (mainDrawData->DisplaySize.x <= 0.0f || mainDrawData->DisplaySize.y <= 0.0f);
		wd->ClearValue.color.float32[0] = clearColor.x * clearColor.w;
		wd->ClearValue.color.float32[1] = clearColor.y * clearColor.w;
		wd->ClearValue.color.float32[2] = clearColor.z * clearColor.w;
		wd->ClearValue.color.float32[3] = clearColor.w;
		if (!mainIsMinimized)
			FrameRender(wd, mainDrawData);

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		// Present Main Platform Window
		if (!mainIsMinimized)
			FramePresent(wd);

		float time = GetTime();
		m_FrameTime = time - m_LastFrameTime;
		m_TimeStep = glm::min<float>(m_FrameTime, 0.0333f);
		m_LastFrameTime = time;
	}
}

void Application::Close() {
	m_Running = false;
}

float Application::GetTime() {
	return (float)glfwGetTime();
}

VkInstance Application::GetInstance() {
	return g_Instance;
}

VkPhysicalDevice Application::GetPhysicalDevice() {
	return g_PhysicalDevice;
}

VkDevice Application::GetDevice() {
	return g_Device;
}

VkCommandBuffer Application::GetCommandBuffer(bool begin) {
	ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;

	// Use any command queue
	VkCommandPool commandPool = wd->Frames[wd->FrameIndex].CommandPool;

	VkCommandBufferAllocateInfo cmdBufAllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	cmdBufAllocateInfo.commandPool = commandPool;
	cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer& commandBuffer = s_AllocatedCommandBuffers[wd->FrameIndex].emplace_back();
	auto err = vkAllocateCommandBuffers(g_Device, &cmdBufAllocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	err = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	check_vk_result(err);

	return commandBuffer;
}

void Application::FlushCommandBuffer(VkCommandBuffer commandBuffer) {
	const uint64_t DEFAULT_FENCE_TIMEOUT = 100000000000;

	VkSubmitInfo endInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	endInfo.commandBufferCount = 1;
	endInfo.pCommandBuffers = &commandBuffer;
	auto err = vkEndCommandBuffer(commandBuffer);
	check_vk_result(err);

	// Create fence to ensure that the command buffer has finished executing
	VkFence fence;
	VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceCreateInfo.flags = 0;
	
	err = vkCreateFence(g_Device, &fenceCreateInfo, nullptr, &fence);
	check_vk_result(err);

	err = vkQueueSubmit(g_Queue, 1, &endInfo, fence);
	check_vk_result(err);

	err = vkWaitForFences(g_Device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
	check_vk_result(err);

	vkDestroyFence(g_Device, fence, nullptr);
}


void Application::SubmitResourceFree(std::function<void()>&& func) {
	s_ResourceFreeQueue[s_CurrentFrameIndex].emplace_back(func);
}

}