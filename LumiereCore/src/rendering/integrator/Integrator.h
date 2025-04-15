#pragma once

#include "rendering/RenderPrimitives.h"

#include "glm/glm.hpp"

namespace Lumiere {

class Integrator {
public: 
	virtual glm::vec4 Ld(const Ray& ray);
};

class DebugIntegrator : public Integrator {
	glm::vec4 Ld(const Ray& ray) override;
};

}