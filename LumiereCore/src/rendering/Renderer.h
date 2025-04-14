#pragma once

#include "scene/Scene.h"

namespace Lumiere {

class Renderer {
public:
	virtual ~Renderer() = default;

	virtual void Render(Scene& scene);
};

}