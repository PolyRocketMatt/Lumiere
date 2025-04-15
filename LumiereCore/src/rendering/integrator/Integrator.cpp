#include "Integrator.h"

namespace Lumiere {

glm::vec4 Integrator::Ld(const Ray& ray) {
	return glm::vec4(0.0f);
}

glm::vec4 DebugIntegrator::Ld(const Ray& ray) {
	float radius = 0.5f;

	float a = glm::dot(ray.direction, ray.direction);
	float b = 2.0f * glm::dot(ray.origin, ray.direction);
	float c = glm::dot(ray.origin, ray.origin) - radius * radius;

	//	Quadratic eq
	float discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0.0f)
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	float t1 = (-b - glm::sqrt(discriminant)) / (2.0f * a);
	glm::vec3 color(0.0f, 0.75f, 1.0f);

	//	t1 is always closest
	glm::vec3 hitPoint = ray.origin + ray.direction * t1;
	glm::vec3 normal = glm::normalize(hitPoint);

	glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));

	float d = glm::max(glm::dot(normal, -lightDir), 0.0f); //	== cos(angle)
	color *= d;

	return glm::vec4(color, 1.0f);
}

}