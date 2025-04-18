#pragma once

#include "glm/glm.hpp"

#include "rendering/RenderPrimitives.h"

#include <vector>

namespace Lumiere {

struct Primitive {
	glm::vec3		origin = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3		color = glm::vec3(0.0f, 0.0f, 0.0f);

	virtual bool Intersect(const Ray& ray, Intersection& isect) const {
		return false;
	}
};

struct Sphere : Primitive {
	float			radius;

	bool Intersect(const Ray& ray, Intersection& isect) const override {
		glm::vec3 displacedOrigin = ray.origin - origin;
		
		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.0f * glm::dot(displacedOrigin, ray.direction);
		float c = glm::dot(displacedOrigin, displacedOrigin) - radius * radius;
		float discriminant = b * b - 4.0f * a * c;

		if (discriminant < 0.0f)
			return false;

		float hitDistance = (-b - glm::sqrt(discriminant)) / (2.0f * a);
		glm::vec3 hitPoint = ray.origin + ray.direction * hitDistance;

		isect.hitDistance = hitDistance;
		isect.hitPoint = hitPoint;
		isect.hitNormal = glm::normalize(hitPoint - origin);
		isect.hit = true;

		isect.hitColor = color;

		return true;
	}
};

struct Plane : Primitive {
	glm::vec2		extent;
};

struct Box : Primitive {
	glm::vec3		extent;
};

}