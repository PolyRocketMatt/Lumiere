#pragma once

#include "rendering/RenderPrimitives.h"
#include "scene/Scene.h"

#include "glm/glm.hpp"

namespace Lumiere {

class Integrator {
public: 
	virtual glm::vec4 Ld(const Ray& ray, const Scene& scene);
};

class DebugIntegrator : public Integrator {
	glm::vec4 Ld(const Ray& ray, const Scene& scene) override;
};

}