#pragma once

#include "Image.h"
#include "integrator/Integrator.h"
#include "RenderPrimitives.h"
#include "scene/Camera.h"
#include "scene/Scene.h"

#include <memory>

namespace Lumiere {

class Renderer {
public:
	virtual ~Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	
	virtual void Render(RenderMetadata& metaData, BaseCamera& camera, Scene& scene, Integrator& integrator);
	virtual void PostProcess();

	std::shared_ptr<Image> GetFinalImage() const { return m_FinalImage; }

private:
	std::shared_ptr<Image>	m_FinalImage;
	glm::vec4*				m_ImageData;
	uint32_t*				m_FinalImageData;
};

}