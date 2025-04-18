#include "Integrator.h"

namespace Lumiere {

glm::vec4 Integrator::Ld(const Ray& ray, const Scene& scene) {
	return glm::vec4(0.0f);
}

glm::vec4 DebugIntegrator::Ld(const Ray& ray, const Scene& scene) {
	Intersection isect;
	scene.Intersect(ray, isect);
	
	if (!isect.hit)
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	glm::vec3 color = isect.hitColor;
	glm::vec3 hitPoint = isect.hitPoint;
	glm::vec3 normal = isect.hitNormal;

	glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));

	float d = glm::max(glm::dot(normal, -lightDir), 0.0f); //	== cos(angle)
	color *= d;

	return glm::vec4(color, 1.0f);
}

}