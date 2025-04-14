#pragma once

#include <string>
#include "vulkan/vulkan.h"

namespace Lumiere {

enum class ImageFormat {
	None = 0,
	RGBA,
	RGBA32F
};

class Image {
public:
	Image(std::string_view path);
	Image(uint32_t width, uint32_t height, ImageFormat format, const void* data = nullptr);
	~Image();

	uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }
	
	void Resize(uint32_t width, uint32_t height);
	
	void SetData(const void* data);

	VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }

private:
	void AllocateMemory(uint64_t size);
	void Release();

private:
	uint32_t		m_Width;
	uint32_t		m_Height;
	ImageFormat		m_ImageFormat;

	VkImage			m_Image = nullptr;
	VkImageView		m_ImageView = nullptr;
	VkDeviceMemory	m_Memory = nullptr;
	VkSampler		m_Sampler = nullptr;

	VkBuffer		m_StagingBuffer = nullptr;
	VkDeviceMemory	m_StagingBufferMemory = nullptr;
	
	size_t			m_AlignedSize = 0;
	
	VkDescriptorSet m_DescriptorSet = nullptr;

	std::string		m_FilePath;
};

}