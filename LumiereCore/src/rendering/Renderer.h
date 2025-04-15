#pragma once

#include "Image.h"
#include "RenderPrimitives.h"
#include "scene/Camera.h"
#include "scene/Scene.h"

#include <memory>

namespace Lumiere {

class Renderer {
public:
	virtual ~Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	
	virtual void Render(BaseCamera& camera, Scene& scene);

	std::shared_ptr<Image> GetFinalImage() const { return m_FinalImage; }

private:
	std::shared_ptr<Image>	m_FinalImage;
	uint32_t*				m_ImageData;
	
	glm::vec4 TraceRay(const Ray& ray);
};

}