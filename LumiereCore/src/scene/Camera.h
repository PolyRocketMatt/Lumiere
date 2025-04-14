#pragma once

#include "glm/glm.hpp"

#include <vector>

namespace Lumiere {

struct CameraMetadata {
	//	--- Geometry & Optics ---
	glm::vec3	s_Position;
	glm::vec3	s_Direction;
	glm::vec3	s_Up;

	float		s_VerticalFov;		
	float		s_FocalLength;		
	float		s_Aperture;
	float		s_FocusDistance;
	float		s_NearClip;
	float		s_FarClip;

	//	--- Sensor ---
	float		s_SensorWidth;
	float		s_SensorHeight;
	float		s_SensorOpenTime;
	float		s_SensorCloseTime;

	//	--- Misc ---
	uint32_t	s_ImageWidth;
	uint32_t	s_ImageHeight;
	float		s_AspectRatio;
};

class BaseCamera {
public:
	BaseCamera(CameraMetadata& metaData);

	bool OnUpdate(float timeStep);
	void OnResize(uint32_t width, uint32_t height);

	const glm::mat4& GetProjection() const { return m_Projection; }

private:
	void RecalculateProjection();
	void RecalculateView();
	void RecalculateRayDirections();
	
private:
	glm::mat4				m_Projection{ 1.0f };
	glm::mat4				m_View{ 1.0f };
	glm::mat4				m_InverseProjection{ 1.0f };
	glm::mat4				m_InverseView{ 1.0f };

	glm::vec3				m_Origin;
	glm::vec3				m_Direction;
	glm::vec3				m_Up;

	float					m_VerticalFov;
	float					m_NearClip;
	float					m_FarClip;

	uint32_t				m_ImageWidth;
	uint32_t				m_ImageHeight;

	//	Cached ray directions
	std::vector<glm::vec3>	m_RayDirections;
};

}