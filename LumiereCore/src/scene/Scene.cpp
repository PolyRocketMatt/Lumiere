#include "rendering/RenderPrimitives.h"
#include "Scene.h"

namespace Lumiere {

void Scene::Intersect(const Ray& ray, Intersection& isect) const {
	return;
}

PrimitiveScene::PrimitiveScene(std::vector<std::shared_ptr<Primitive>>& primitives)
	: m_primitives(primitives) { }

void PrimitiveScene::Intersect(const Ray& ray, Intersection& isect) const {
	Intersection scratch;
	for (auto& primitive : m_primitives) {
		if (primitive->Intersect(ray, scratch) && scratch.hitDistance < isect.hitDistance) {
			isect.hitDistance = scratch.hitDistance;
			isect.hitPoint = scratch.hitPoint;
			isect.hitNormal = scratch.hitNormal;
			isect.hit = true;
			isect.hitColor = scratch.hitColor;
		}
	}
}

}