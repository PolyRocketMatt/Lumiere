#pragma once

#include "glm/glm.hpp"

namespace Lumiere {

struct RenderMetadata {
	int			samplesPerPixel;
	int			maxBounces;
};

struct Ray {
	glm::vec3	origin;
	glm::vec3	direction;
};

}