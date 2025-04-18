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

struct Intersection {
	float		hitDistance = FLT_MAX;
	glm::vec3	hitPoint;
	glm::vec3	hitNormal;
	bool		hit = false;
	
	glm::vec3	hitColor;
};

}