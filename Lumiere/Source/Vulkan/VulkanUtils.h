#pragma once
#include "Vulkan/vulkan.h"
#include <stdexcept>

namespace Lumiere {

//	Inlining because double definitions otherwise ;(
inline void checkTrueOrExcept(VkResult result, const char* message) {
	if (result != VK_SUCCESS)
		throw std::runtime_error(message);
}

}