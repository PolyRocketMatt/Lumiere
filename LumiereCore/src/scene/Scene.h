#pragma once

#include <memory>
#include <vector>

#include "geometry/Primitives.h"
#include "rendering/RenderPrimitives.h"

namespace Lumiere {

class Scene {
public:
	virtual void Intersect(const Ray& ray, Intersection& isect)const ;
};

class PrimitiveScene : public Scene {
public:
	PrimitiveScene() = default;
	PrimitiveScene(std::vector<std::shared_ptr<Primitive>>& primitives);
	~PrimitiveScene() = default;

	void Intersect(const Ray& ray, Intersection& isect) const override;

private:
	std::vector<std::shared_ptr<Primitive>>	m_primitives;
};

}