#pragma once

#include "scene/Camera.h"
#include "scene/Scene.h"

namespace Lumiere {

class Renderer {
public:
	virtual ~Renderer() = default;

	virtual void Render(BaseCamera& camera, Scene& scene);
};

}