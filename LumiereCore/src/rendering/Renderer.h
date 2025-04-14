#pragma once

namespace Lumiere {

class Renderer {
	virtual ~Renderer() = default;

	virtual void Render();
};

}