#include "Image.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

#include "Application.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Lumiere {

namespace Utils {

static uint32_t GetVulkanMemoryType(VkMemoryPropertyFlags properties, uint32_t typeBits) {
	VkPhysicalDeviceMemoryProperties props;
	vkGetPhysicalDeviceMemoryProperties(Application::GetPhysicalDevice(), &props);
	
	for (size_t i = 0; i < props.memoryTypeCount; i++) {
		if ((props.memoryTypes[i].propertyFlags & properties) == properties && typeBits & (1 << i))
			return i;
	}

	return 0xffffffff;
}

static uint32_t BytesPerPixel(ImageFormat format) {
	switch (format) {
			case ImageFormat::RGBA:				return 4;
			case ImageFormat::RGBA32F:			return 32;
	}

	return 0;
}

static VkFormat LumiereToVulkanFormat(ImageFormat format) {
	switch (format) {
			case Lumiere::ImageFormat::RGBA:	return VK_FORMAT_R8G8B8A8_UNORM;
			case Lumiere::ImageFormat::RGBA32F:	return VK_FORMAT_R32G32B32A32_SFLOAT;
	}

	return (VkFormat)0;
}

}

Image::Image(std::string_view path) : m_FilePath(path) {
	int width, height, channels;
	uint8_t* data = nullptr;

	//	Load image data using STB
	if (stbi_is_hdr(m_FilePath.c_str())) {
		data = (uint8_t*)stbi_loadf(m_FilePath.c_str(), &width, &height, &channels, 4);
		m_ImageFormat = ImageFormat::RGBA32F;
	}
	else {
		data = stbi_load(m_FilePath.c_str(), &width, &height, &channels, 4);
		m_ImageFormat = ImageFormat::RGBA;
	}

	m_Width = width;
	m_Height = height;

	//	Allocate memory for the image
	AllocateMemory(m_Width * m_Height * Utils::BytesPerPixel(m_ImageFormat));
	SetData(data);
	stbi_image_free(data);
}

Image::Image(uint32_t width, uint32_t height, ImageFormat format, const void* data)
	: m_Width(width), m_Height(height), m_ImageFormat(format) {
	AllocateMemory(m_Width * m_Height * Utils::BytesPerPixel(m_ImageFormat));
	if (data)
		SetData(data);
}

Image::~Image() {
	Release();
}

void Image::AllocateMemory(uint64_t size) {
	VkResult err;
	VkDevice device = Application::GetDevice();
	VkFormat vk_Format = Utils::LumiereToVulkanFormat(m_ImageFormat);
	
	//	Create the Image
	{
		VkImageCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = vk_Format;
		info.extent.width = m_Width;
		info.extent.height = m_Height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		
		err = vkCreateImage(device, &info, nullptr, &m_Image);
		check_vk_result(err);

		VkMemoryRequirements req;
		vkGetImageMemoryRequirements(device, m_Image, &req);
		
		VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		allocInfo.allocationSize = req.size;
		allocInfo.memoryTypeIndex = Utils::GetVulkanMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);
		
		err = vkAllocateMemory(device, &allocInfo, nullptr, &m_Memory);
		check_vk_result(err);

		err = vkBindImageMemory(device, m_Image, m_Memory, 0);
		check_vk_result(err);
	}

	//	Create the Image View
	{
		VkImageViewCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		info.image = m_Image;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = vk_Format;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.layerCount = 1;
		
		err = vkCreateImageView(device, &info, nullptr, &m_ImageView);
		check_vk_result(err);
	}

	//	Create the Sampler
	{
		VkSamplerCreateInfo info = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		info.magFilter = VK_FILTER_LINEAR;
		info.minFilter = VK_FILTER_LINEAR;
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.minLod = -1000;
		info.maxLod = 1000;
		info.maxAnisotropy = 1.0f;
		
		err = vkCreateSampler(device, &info, nullptr, &m_Sampler);
		check_vk_result(err);
	}

	//	Create the Descriptor Set
	m_DescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Image::Release() {
	Application::SubmitResourceFree([sampler = m_Sampler, imageView = m_ImageView, image = m_Image,
		memory = m_Memory, stagingBuffer = m_StagingBuffer, stagingBufferMemory = m_StagingBufferMemory]() {
			VkDevice device = Application::GetDevice();
			
			vkDestroySampler(device, sampler, nullptr);
			vkDestroyImageView(device, imageView, nullptr);
			vkDestroyImage(device, image, nullptr);
			vkFreeMemory(device, memory, nullptr);
			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);
	});

	m_Sampler = nullptr;
	m_ImageView = nullptr;
	m_Image = nullptr;
	m_Memory = nullptr;
	m_StagingBuffer = nullptr;
	m_StagingBufferMemory = nullptr;
}

void Image::SetData(const void* data) {
	VkDevice device = Application::GetDevice();
	size_t uploadSize = m_Width * m_Height * Utils::BytesPerPixel(m_ImageFormat);
	
	VkResult err;
	if (!m_StagingBuffer) {
		//	Create the upload buffer
		{
			VkBufferCreateInfo info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			info.size = uploadSize;
			info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			
			err = vkCreateBuffer(device, &info, nullptr, &m_StagingBuffer);
			check_vk_result(err);

			VkMemoryRequirements req;
			vkGetBufferMemoryRequirements(device, m_StagingBuffer, &req);
			m_AlignedSize = req.size;
			
			VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
			allocInfo.allocationSize = req.size;
			allocInfo.memoryTypeIndex = Utils::GetVulkanMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
			
			err = vkAllocateMemory(device, &allocInfo, nullptr, &m_StagingBufferMemory);
			check_vk_result(err);

			err = vkBindBufferMemory(device, m_StagingBuffer, m_StagingBufferMemory, 0);
			check_vk_result(err);
		}
	}

	//	Upload to Buffer
	{
		char* map = NULL;
		err = vkMapMemory(device, m_StagingBufferMemory, 0, m_AlignedSize, 0, (void**)(&map));
		check_vk_result(err);
		memcpy(map, data, uploadSize);
		
		VkMappedMemoryRange range[1] = { };
		range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range[0].memory = m_StagingBufferMemory;
		range[0].size = m_AlignedSize;
		
		err = vkFlushMappedMemoryRanges(device, 1, range);
		check_vk_result(err);
		vkUnmapMemory(device, m_StagingBufferMemory);
	}

	//	Copy to Image
	{
		VkCommandBuffer commandBuffer = Application::GetCommandBuffer(true);
		VkImageMemoryBarrier copyBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		copyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		copyBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		copyBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		copyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		copyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		copyBarrier.image = m_Image;
		copyBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyBarrier.subresourceRange.levelCount = 1;
		copyBarrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
			0, 0, NULL, 0, NULL, 1, &copyBarrier);

		VkBufferImageCopy region = { };
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.layerCount = 1;
		region.imageExtent.width = m_Width;
		region.imageExtent.height = m_Height;
		region.imageExtent.depth = 1;
		vkCmdCopyBufferToImage(commandBuffer, m_StagingBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		
		VkImageMemoryBarrier useBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		useBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		useBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		useBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		useBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		useBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		useBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		useBarrier.image = m_Image;
		useBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		useBarrier.subresourceRange.levelCount = 1;
		useBarrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
			0, 0, NULL, 0, NULL, 1, &useBarrier);

		Application::FlushCommandBuffer(commandBuffer);
	}
}

void Image::Resize(uint32_t width, uint32_t height) {
	if (m_Image && m_Width == width && m_Height == height)
		return;

	m_Width = width;
	m_Height = height;
	
	Release();
	AllocateMemory(m_Width * m_Height * Utils::BytesPerPixel(m_ImageFormat));
}

}