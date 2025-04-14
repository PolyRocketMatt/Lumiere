#pragma once

#include "glm/glm.hpp"

namespace Lumiere {

struct Primitive {
	glm::vec3		origin;
};

struct Sphere : Primitive {
	float			radius;
};

struct Plane : Primitive {
	glm::vec2		extent;
};

struct Box : Primitive {
	glm::vec3		extent;
};

}