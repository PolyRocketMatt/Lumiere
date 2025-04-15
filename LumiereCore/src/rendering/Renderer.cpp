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
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render(BaseCamera& camera, Scene& scene) {
	Ray ray;
	ray.origin = camera.GetPosition();

	uint32_t width = m_FinalImage->GetWidth();
	uint32_t height = m_FinalImage->GetHeight();
	
	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			uint32_t index = x + y * width;
			ray.direction = camera.GetRayDirections()[index];
			
			//glm::vec2 coord = { x / (float)m_FinalImage->GetWidth(), y / (float)m_FinalImage->GetHeight() };
			//coord = coord * 2.0f - 1.0f;
			//coord.x *= (float)m_FinalImage->GetWidth() / m_FinalImage->GetHeight();

			glm::vec4 color = TraceRay(ray);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

inline float IntersectSphere(const Ray& ray, glm::vec3 center, float radius) {
	float a = glm::dot(ray.direction, ray.direction);
	float b = 2.0f * glm::dot(ray.origin, ray.direction);
	float c = glm::dot(ray.origin, ray.origin) - radius * radius;

	//	Quadratic eq
	float discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0.0f)
		return std::numeric_limits<float>::max();

	float t1 = (-b - glm::sqrt(discriminant)) / (2.0f * a);
}

inline float IntersectPlane(const Ray& ray, float y) {
	return (y - ray.origin.y) / ray.direction.y;
}

glm::vec4 Renderer::TraceRay(const Ray& ray) {
	float sphereT = IntersectSphere(ray, glm::vec3(0.0f), 0.5f);

	glm::vec3 color(0.0f, 0.75f, 1.0f);

	if (sphereT < 0.0f)
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	//	t1 is always closest
	glm::vec3 hitPoint = ray.origin + ray.direction * sphereT;
	glm::vec3 normal = glm::normalize(hitPoint);

	glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));

	float d = glm::max(glm::dot(normal, -lightDir), 0.0f); //	== cos(angle)
	color *= d;

	return glm::vec4(color, 1.0f);
	
}

}