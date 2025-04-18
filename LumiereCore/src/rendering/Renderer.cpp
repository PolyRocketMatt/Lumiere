#include "Renderer.h"
#include "Utils.h"

#include <iostream>
#include <limits>

namespace Lumiere {

void Renderer::OnResize(uint32_t width, uint32_t height) {
	if (m_FinalImage) {
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	} 
	else
		m_FinalImage = std::make_shared<Image>(width, height, ImageFormat::RGBA);
	
	delete[] m_ImageData;
	delete[] m_FinalImageData;

	m_ImageData = new glm::vec4[width * height];
	m_FinalImageData = new uint32_t[width * height];
}

void Renderer::Render(RenderMetadata& metaData, BaseCamera& camera, Scene& scene, Integrator& integrator) {
	uint32_t width = m_FinalImage->GetWidth();
	uint32_t height = m_FinalImage->GetHeight();
	Ray ray;
	ray.origin = camera.GetPosition();
	
	//	(Crude) Monte-Carlo
	for (uint32_t i = 0; i < width * height; i++)
		m_ImageData[i] = glm::vec4(0.0f);

	for (size_t sample = 0; sample < metaData.samplesPerPixel; sample++) {
		for (uint32_t y = 0; y < height; y++) {
			for (uint32_t x = 0; x < width; x++) {
				uint32_t index = x + y * width;
				ray.direction = camera.GetRayDirections()[index];

				glm::vec4 ld = integrator.Ld(ray);
				ld = glm::clamp(ld, glm::vec4(0.0f), glm::vec4(1.0f));
				m_ImageData[x + y * width] += ld;
			}
		}
	}

	//	(Crude) Monte-Carlo
	glm::vec3 a{ (float)metaData.samplesPerPixel };
	glm::vec4 factor(a, 1.0f);
	for (uint32_t i = 0; i < width * height; i++)
		m_ImageData[i] /= factor;
}

void Renderer::PostProcess() {
	uint32_t width = m_FinalImage->GetWidth();
	uint32_t height = m_FinalImage->GetHeight();
	for (uint32_t i = 0; i < width * height; i++) {
		m_FinalImageData[i] = ConvertToRGBA(m_ImageData[i]);
	}

	m_FinalImage->SetData(m_FinalImageData);
}

}